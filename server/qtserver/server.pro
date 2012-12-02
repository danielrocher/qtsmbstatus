
SOURCES += server.cpp \
           clientsocket.cpp \
           main.cpp \
           disconnect_manager.cpp \
           process_smbd_exist.cpp \
           sendmessage_manager.cpp \
           smbmanager.cpp \
           pam.cpp \
           ../../common/common.cpp \
           ../../common/core_syntax.cpp \
           pamthread.cpp 

HEADERS += server.h \
           clientsocket.h \
           disconnect_manager.h \
           process_smbd_exist.h \
           sendmessage_manager.h \
           smbmanager.h \
           pam.h \
           ../../common/core_syntax.h \
           pamthread.h

TEMPLATE = app 

QT +=  network
QT -=  gui

CONFIG += release \
          warn_on \
          thread

TARGET = qtsmbstatusd

MOC_DIR =     ./build/moc
OBJECTS_DIR = ./build
DESTDIR =     ./bin
UI_DIR =      ./build/ui
RCC_DIR =     ./build/rcc


LIBS += -lpam

DATADIR= /etc/qtsmbstatusd
DATAFILES = ../etc/privkey.pem \
            ../etc/qtsmbstatusd.conf \
            ../etc/server.pem \
            ../etc/qtsmbstatusd.users
data.path = $$DATADIR 
data.files += $$DATAFILES

PAMDIR = /etc/pam.d
PAMFILE = ../pam.d/qtsmbstatusd
pam.path = $$PAMDIR 
pam.files += $$PAMFILE

INITDIR= /etc/init.d
INITFILE = ../etc/qtsmbstatusd
init.path = $$INITDIR 
init.files += $$INITFILE

#doc
DOCDIR = /usr/local/share/doc/qtsmbstatusd
DOCFILES = ../../README \
           ../../README-FR \
           ../../INSTALL \
           ../../COPYING
doc.path = $$DOCDIR 
doc.files += $$DOCFILES 

#manpage
MANDIR   = /usr/local/share/man/man7
MANFILES = ../qtsmbstatusd.7.gz
manpage.path = $$MANDIR
manpage.files = $$MANFILES

target.path = /usr/local/bin/

postinstall.path=/
postinstall.extra+= sh $$(PWD)/../postinst.sh \$(INSTALL_ROOT)

unix {
INSTALLS += data \
            pam \
            init \
            doc \
            manpage \
            target \
            postinstall
}

