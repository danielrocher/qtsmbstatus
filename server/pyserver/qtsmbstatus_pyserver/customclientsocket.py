#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Written by Daniel Rocher <daniel.rocher@adella.org>
# Copyright (C) 2012

import re, time, threading
from qtsmbstatus_pyserver.serverssl import ClientSocketSSL 
from qtsmbstatus_pyserver.global_env import Global
from qtsmbstatus_pyserver.smbmanager import SmbManager
from qtsmbstatus_pyserver.disconnect_manager import DisconnectManager
from qtsmbstatus_pyserver.sendmessage_manager import SendMessageManager


class CustomClientSocket(ClientSocketSSL):
    # enum commands
    class Command:
        auth_rq=       0
        auth_ack=      1
        end=           2
        kill_user=     3
        send_msg=      4
        smb_rq=        5
        smb_data=      6
        end_smb_rq=    7
        whoiam=        8
        server_info=   9
        error_auth=   10
        error_command=11
        error_obj=    12
        echo_request= 13
        echo_reply=   14

    def __init__(self, conn, debug_mode=False):
        ClientSocketSSL.__init__(self, conn, debug_mode)
        # if user authenticated
        self.AuthUser=False
        # if client is authorized to disconnect user
        self.permitDisconnectUser=False
        #if client is authorized to send popup message
        self.permitSendMsg=False
        self.timerTestAuth = threading.Timer(60.0, self.testIfAuthenticated)
        self.timerTestAuth.start()

    def stop(self):
        self.timerTestAuth.cancel()
        ClientSocketSSL.stop(self)


    def addEscapeKeys(self, arg):
        """Return string with escape keys"""
        return arg.replace("\\","\\\\").replace(";","\\;")

    def removeEscapeKeys(self, arg):
        """Return string without escape keys"""
        return arg.replace('\x11',"\\").replace('\x12',";")


    def sendToClient(self, cmd, inputArg1="", inputArg2=""):
        """Send datas to client"""
        MyTxt=""
        if inputArg1:
            MyTxt=self.addEscapeKeys(inputArg1)
        if inputArg2:
            MyTxt+=";"+self.addEscapeKeys(inputArg2)
        self.sendDatas("[{0}]{1}\n".format(str(cmd),MyTxt))



    def readyReadHandler(self, data):
        """Ready to read datas."""
        string=data.strip()
        # self.debug(string)

        # replace escape keys
        string=string.replace("\\\\",'\x11').replace("\\;",'\x12')
        
        lines=string.split("\n")
        for line in lines:
            MyRegExp=re.compile("^\[(\d+)\]([^;\\n\\r]*);?([^;\\n\\r]*)")
            result=MyRegExp.findall(line)

            for res in result:
                
                enum_cmd=self.Command
                command=0
                try:
                    command=int(res[0])
                except:
                    print ("Command error !")
                    continue
                arg1=self.removeEscapeKeys(res[1])
                arg2=self.removeEscapeKeys(res[2])
                self.debug ("{0} | {1} | {2}".format(res[0],arg1, arg2))
                
                if command==enum_cmd.auth_rq:
                    self.debug ("Receive: auth_rq")
                    self.cmdAuthRq(arg1, arg2)
                elif command==enum_cmd.end:
                    self.debug ("Receive: end")
                    self.stop()
                elif command==enum_cmd.kill_user:
                    self.debug ("Receive: kill_user")
                    self.cmdKillUser(arg1, arg2)
                elif command==enum_cmd.send_msg:
                    self.debug ("Receive: send_msg")
                    self.cmdSendMsg(arg1, arg2)
                elif command==enum_cmd.smb_rq:
                    self.debug ("Receive: smb_rq")
                    self.cmdSmbRq()
                elif command==enum_cmd.whoiam:
                    self.debug ("Receive: whoiam")
                elif command==enum_cmd.error_command:
                    self.debug ("Receive: error_command")
                elif command==enum_cmd.echo_request:
                    self.debug ("Receive: echo_request")
                    self.sendToClient(enum_cmd.echo_reply)
                elif command==enum_cmd.echo_reply:
                    self.debug ("Receive: echo_reply")
                    # now, ignore it (compatibility to qtsmbstatus =< 2.0.6)
                else:
                    self.debug ("[{}] not implemented !".format(str(command)))
                    self.sendToClient(enum_cmd.error_command,"[{}] not implemented".format(str(command)))



    def connectedHandler(self):
        self.sendToClient(self.Command.whoiam,str(Global.int_qtsmbstatus_version),"QtSmbstatus python server {0} - {1}".format(Global.version_qtsmbstatus, Global.date_qtsmbstatus))

    def cmdKillUser(self, pid, user):
        """Request from client to disconnect an user"""
        if not self.AuthUser: # if not authenticated
            self.debug("Client not authenticated !")
            self.sendToClient(self.Command.error_auth,"Not authenticated !")
            return
        disconnectmanager=DisconnectManager( pid, user, self.debug_mode, self.callbackKillUser)
        disconnectmanager.start()

    def callbackKillUser(self, datas):
        """call back from DisconnectManager"""
        stdout, stderr, user, eventErrorIsSet = datas
        if eventErrorIsSet:
            self.sendToClient(self.Command.error_obj,"Failed to disconnect user {}".format(user))



    def cmdSendMsg(self, machine, message):
        """Request from client to send popup message"""
        if not self.AuthUser: # if not authenticated
            self.debug("Client not authenticated !")
            self.sendToClient(self.Command.error_auth,"Not authenticated !")
            return

        sendmsgmanager=SendMessageManager(machine, message, self.debug_mode, callback=self.callbackSendMsg)
        sendmsgmanager.start()

    def callbackSendMsg(self, datas):
        """call back from SendMessageManager"""
        stdout, stderr, machine, eventErrorIsSet = datas
        if eventErrorIsSet:
            self.sendToClient(self.Command.error_obj, "Could not send message to {}".format(machine))


    def cmdSmbRq(self):
        """Run Smbstatus request"""
        if not self.AuthUser: # if not authenticated
            self.debug("Client not authenticated !")
            self.sendToClient(self.Command.error_auth,"Not authenticated !")
            return

        smb_th=SmbManager(debug_mode=self.debug_mode, cache_timeout=30, callback=self.callbackSmbRq)
        smb_th.start()


    def callbackSmbRq(self, datas):
        """call back from SmbManager"""
        def serialize(cmd1, cmd2, lines):
            """serialize dataset to send to client"""
            if len(lines)==0: return
            toSend=""
            size=0
            lines=lines.split('\n')
            nbline=len (lines)
            for i, line in enumerate(lines):
                toSend+="[{0}]{1}\n".format(str(cmd1),line)
                size+=len(toSend)
                # if size > 2048 char or if it's the end of stringlist, send data
                if size>2048 or nbline==i+1:
                    if i+1<nbline:
                        self.sendDatas(toSend)
                    else:
                        self.sendDatas("{0}[{1}]\n".format(toSend, str(cmd2))) # end of reply. All datas has been sent
                    toSend=""
                    size=0
        
        self.debug ("smbstatus request finished")
        stdout, stderr = datas
        serialize(self.Command.smb_data, self.Command.end_smb_rq, stdout)
        serialize(self.Command.error_obj, self.Command.error_obj, stderr)


    def cmdAuthRq(self, login, password):
        """Authentication Request"""
        # if client is authorized to disconnect user
        if login in Global.AllowUserDisconnect or "all" in Global.AllowUserDisconnect:
            self.permitDisconnectUser=True
    
        # if client is authorized to send popup message
        if login in Global.AllowUserSendMsg or "all" in Global.AllowUserSendMsg:
            self.permitSendMsg=True

        self.debug("permitDisconnectUser: {0}\npermitSendMsg: {1}".format(self.permitDisconnectUser, self.permitSendMsg))
        try:
            from qtsmbstatus_pyserver.pam import pam
            mypam = pam()
            if not mypam.authenticate(login, password, service='qtsmbstatusd'):
                # client is not authenticated. disconnect it
                self.debug ("client is not authenticated\npam code: {0}\npam reason: {1} ".format(mypam.code, mypam.reason.decode("utf-8")))
                self.sendToClient(self.Command.error_auth)
                time.sleep(0.2)
                self.stop()
                return
            else:
                # client authenticated
                self.AuthUser=True
                self.timerTestAuth.cancel()
                self.sendToClient(self.Command.auth_ack)
                
                # infoserver : right for current client
                # 0000 0001 : permit client to disconnect an user
                # 0000 0010 : permit client to send popup messages (popupwindows)
    
                infoserver=0
                if self.permitDisconnectUser: infoserver=1
                if self.permitSendMsg: infoserver+=2
                # send client's rights
                self.sendToClient(self.Command.server_info,str(infoserver))
                return
        except:
            Global.error ("PAM ERROR !")

        self.stop()


    def testIfAuthenticated (self):
        """Test if client is authenticated (1 minute after the connection is started). Disconnect client if is False"""
        self.debug("testIfAuthenticated()")
        if not self.AuthUser: # if not authenticated
            self.debug("Client not authenticated !")
            self.stop()


if __name__ == "__main__":
    pass

