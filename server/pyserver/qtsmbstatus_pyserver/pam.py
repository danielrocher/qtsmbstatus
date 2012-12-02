# (c) 2007 Chris AtLee <chris@atlee.ca>
# Licensed under the MIT license:
# http://www.opensource.org/licenses/mit-license.php
"""
PAM module for python

Provides an authenticate function that will allow the caller to authenticate
a user against the Pluggable Authentication Modules (PAM) on the system.

Implemented using ctypes, so no compilation is necessary.

---
modified to work [better] with python3.2, 2011-12-6, david ford, <david@blue-labs.org>
i haven't paid any attention to making sure things work in python2. there may be
problems in my_conv()

"""

# Currently, this package doesn't exist for debian. In the future, delete this file in qtsmbstatus-server

import sys
if sys.version_info >= (3,):
    py3k = True
else:
    py3k = False

from ctypes import CDLL, POINTER, Structure, CFUNCTYPE, cast, pointer, sizeof
from ctypes import c_void_p, c_uint, c_char_p, c_char, c_int
from ctypes import memmove
from ctypes.util import find_library

class PamHandle(Structure):
    """wrapper class for pam_handle_t"""
    _fields_ = [
            ("handle", c_void_p)
            ]

    def __init__(self):
        Structure.__init__(self)
        self.handle = 0

class PamMessage(Structure):
    """wrapper class for pam_message structure"""
    _fields_ = [
            ("msg_style", c_int),
            ("msg", c_char_p),
            ]

    def __repr__(self):
        return "<PamMessage %i '%s'>" % (self.msg_style, self.msg)

class PamResponse(Structure):
    """wrapper class for pam_response structure"""
    _fields_ = [
            ("resp", c_char_p),
            ("resp_retcode", c_int),
            ]

    def __repr__(self):
        return "<PamResponse %i '%s'>" % (self.resp_retcode, self.resp)

CONV_FUNC = CFUNCTYPE(
    c_int,
    c_int,
    POINTER(POINTER(PamMessage)),
    POINTER(POINTER(PamResponse)),
    c_void_p)

class PamConv(Structure):
    """wrapper class for pam_conv structure"""
    _fields_ = [
            ("conv", CONV_FUNC),
            ("appdata_ptr", c_void_p)
            ]

# Various constants
PAM_PROMPT_ECHO_OFF       = 1
PAM_PROMPT_ECHO_ON        = 2
PAM_ERROR_MSG             = 3
PAM_TEXT_INFO             = 4

LIBC                      = CDLL(find_library("c"))
LIBPAM                    = CDLL(find_library("pam"))

CALLOC                    = LIBC.calloc
CALLOC.restype            = c_void_p
CALLOC.argtypes           = [c_uint, c_uint]

PAM_START                 = LIBPAM.pam_start
PAM_START.restype         = c_int
PAM_START.argtypes        = [c_char_p, c_char_p, POINTER(PamConv), POINTER(PamHandle)]

PAM_STRERROR              = LIBPAM.pam_strerror
PAM_STRERROR.restype      = c_char_p
PAM_STRERROR.argtypes     = [POINTER(PamHandle), c_int]

PAM_AUTHENTICATE          = LIBPAM.pam_authenticate
PAM_AUTHENTICATE.restype  = c_int
PAM_AUTHENTICATE.argtypes = [PamHandle, c_int]

class pam():
    code   = 0
    reason = None

    def __init__(self):
        pass

    def authenticate(self, username, password, service='login'):
        """username and password authenticate for the given service.
        
           Returns True for success, or False.  self.code is the integer
           value representing the numerice failure reason, or 0 on success. 
           self.reason is the textual reason.
    
           Python3 expects bytes() for ctypes inputs.  This function will make
           necessary conversions using the latin-1 coding.
        
        username: the username to authenticate
        password: the password in plain text
         service: the PAM service to authenticate against.
                  Defaults to 'login' """
    
        @CONV_FUNC
        def my_conv(n_messages, messages, p_response, app_data):
            """Simple conversation function that responds to any
               prompt where the echo is off with the supplied password"""
            # Create an array of n_messages response objects
            addr = CALLOC(n_messages, sizeof(PamResponse))
            p_response[0] = cast(addr, POINTER(PamResponse))
            for i in range(n_messages):
                if messages[i].contents.msg_style == PAM_PROMPT_ECHO_OFF:
                    cs  = c_char_p(password)
                    dst = CALLOC(sizeof(c_char_p), len(password)+1)
                    memmove(dst , cs, len(password))
                    p_response.contents[i].resp = dst
                    p_response.contents[i].resp_retcode = 0
            return 0
    
        # python3 ctypes prefers bytes, pretend everyone will be happy using latin-1
        if py3k:
            if isinstance(username, str):
                username = bytes(username, 'utf-8')
            if isinstance(password, str):
                password = bytes(password, 'utf-8')
            if isinstance(service, str):
                service  = bytes(service, 'utf-8')
    
        handle = PamHandle()
        conv   = PamConv(my_conv, 0)
        retval = PAM_START(service, username, pointer(conv), pointer(handle))
    
        if retval != 0:
            # This is not an authentication error, something has gone wrong starting up PAM
            self.code   = retval
            self.reason = PAM_STRERROR(pointer(handle), retval)
            return False
    
        retval = PAM_AUTHENTICATE(handle, 0)
        
        if retval == 0:
            # success
            logic = True
        else:
            logic = False
        
        # store information to inform the caller why we failed
        self.code   = retval
        self.reason = PAM_STRERROR(pointer(handle), retval)
        
        return logic
    

if __name__ == "__main__":
    pam = pam()

    if not pam.authenticate('david.ford','this would fail', service='wsgi'):
        print (pam.code, pam.reason)
    else:
        print ('success')

    if not pam.authenticate('david.ford','arealpassword', service='wsgi'):
        print(pam.code, pam.reason)
    else:
        print ('success')

