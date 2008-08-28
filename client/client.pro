
TEMPLATE = app

CONFIG += release \
          warn_on

TARGET =    qtsmbstatus
DESTDIR +=  bin

LIBS += -lssl

SOURCES += main.cpp \
           main_windows.cpp \
           server.cpp \
           machine.cpp \
           service.cpp \
           user.cpp \
           smbstatus.cpp \
           clientssl.cpp \
           clientsocket.cpp \
           login_windows.cpp \
           configure_windows.cpp \
           linecore.cpp \
           ../common/core_syntax.cpp \
           ../common/common.cpp \
           log.cpp \
           mysortfilterproxymodel.cpp

HEADERS += main_windows.h \
           server.h \
           machine.h \
           service.h \
           user.h \
           smbstatus.h \
           clientssl.h \
           clientsocket.h \
           linecore.h \
           login_windows.h \
           ../common/core_syntax.h \
           configure_windows.h \
           log.h \
           mysortfilterproxymodel.h

RESOURCES = client.qrc

RC_FILE = qtsmbstatus.rc

TRANSLATIONS += ./tr/qtsmbstatus_fr.ts \
                ./tr/qtsmbstatus_es.ts \
                ./tr/qtsmbstatus_pl.ts \
                ./tr/qtsmbstatus_de.ts \
                ./tr/qtsmbstatus_it.ts \
                ./tr/qtsmbstatus_ru.ts \
                ./tr/qtsmbstatus_uk.ts \
                ./tr/qtsmbstatus_sv.ts \
                ./tr/qtsmbstatus_hu.ts \
                ./tr/qtsmbstatus_nl.ts \
                ./tr/qtsmbstatus_pt_BR.ts

FORMS +=  form_smbstatus.ui \
          configure.ui \
          login_dialog.ui \
          log.ui

# install
!win32 {
# Translation
TRANSDIR   = /usr/local/share/qtsmbstatus
TRANSFILES = ./tr/*.qm
trans.path = $$TRANSDIR 
trans.files = $$TRANSFILES 

# Documentation
DOCDIR   = /usr/local/share/doc/qtsmbstatus
DOCFILES = ../README \
           ../README-FR \
           ../INSTALL \
           ../COPYING
doc.path = $$DOCDIR
doc.files = $$DOCFILES

#icons
ICONSDIR   = /usr/share/pixmaps
ICONSFILES = qtsmbstatus.xpm
icons.path = $$ICONSDIR
icons.files = $$ICONSFILES

#menu
MENUDIR   = /usr/share/applications
MENUFILES = qtsmbstatus.desktop
menu.path = $$MENUDIR
menu.files = $$MENUFILES

#manpage
MANDIR   = /usr/local/share/man/man7
MANFILES = qtsmbstatus.7.gz
manpage.path = $$MANDIR
manpage.files = $$MANFILES


target.path = /usr/bin

INSTALLS += trans \
            target \
            doc \
            icons \
            menu \
            manpage
}


QT += network  qt3support

