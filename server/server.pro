# Fichier gnr par le module QMake de KDevelop. 
# -------------------------------------------------- 
# Sous dossier relatif au dossier principal du projet: ./server
# Cible: une application??:  ./bin/qtsmbstatusd

DATADIR= /etc/qtsmbstatusd
DATAFILES = ./etc/privkey.pem \
            ./etc/qtsmbstatusd.conf \
            ./etc/server.pem \
            ./etc/qtsmbstatusd.users
PAMDIR = /etc/pam.d
PAMFILE = ./pam.d/qtsmbstatusd
INITDIR= /etc/init.d
INITFILE = ./etc/qtsmbstatusd
DOCDIR = /usr/share/doc/qtsmbstatusd
DOCFILES = ../README \
           ../README-FR \
           ../INSTALL \
           ../COPYING
SOURCES += server.cpp \
           clientsocket.cpp \
           main.cpp \
           disconnect_manager.cpp \
           process_smbd_exist.cpp \
           sendmessage_manager.cpp \
           smbmanager.cpp \
           pam.cpp \
           ../common/common.cpp \
           ../common/core_syntax.cpp \
           pamthread.cpp 
HEADERS += server.h \
           clientsocket.h \
           disconnect_manager.h \
           process_smbd_exist.h \
           sendmessage_manager.h \
           smbmanager.h \
           pam.h \
           ../common/core_syntax.h \
           pamthread.h 
TEMPLATE = app 
CONFIG += release \
          warn_on \
          thread 
TARGET = ./bin/qtsmbstatusd 
LIBS += -lpam \
        -lssl 
data.path = $$DATADIR 
data.files += $$DATAFILES 
pam.path = $$PAMDIR 
pam.files += $$PAMFILE 
init.path = $$INITDIR 
init.files += $$INITFILE 
doc.path = $$DOCDIR 
doc.files += $$DOCFILES 
target.path = /usr/bin/ 
INSTALLS += data \
            pam \
            init \
            doc \
            target 
#The following line was inserted by qt3to4
QT +=  qt3support 
