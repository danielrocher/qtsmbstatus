
TEMPLATE = app

CONFIG += release \
          warn_on

TARGET =    qtsmbstatusl

MOC_DIR =     ./build/moc
OBJECTS_DIR = ./build
DESTDIR =     ./bin
UI_DIR =      ./build/ui
RCC_DIR =     ./build/rcc

SOURCES += main.cpp \
           ../client/mainwindows.cpp \
           lmain_windows.cpp \
           ../client/server.cpp \
           ../client/machine.cpp \
           ../client/service.cpp \
           ../client/user.cpp \
           ../client/smbstatus.cpp \
           configure_windowsl.cpp \
           ../client/linecore.cpp \
           ../common/common.cpp \
           ../client/log.cpp \
           ../client/mysortfilterproxymodel.cpp \
           ../client/instances_dlg.cpp \
           ../server/qtserver/smbmanager.cpp \
           ../server/qtserver/process_smbd_exist.cpp \
           ../server/qtserver/sendmessage_manager.cpp \
           ../server/qtserver/disconnect_manager.cpp

HEADERS += ../client/mainwindows.h \
           lmain_windows.h \
           ../client/server.h \
           ../client/machine.h \
           ../client/service.h \
           ../client/user.h \
           ../client/smbstatus.h \
           ../client/linecore.h \
           ../common/core_syntax.h \
           configure_windowsl.h \
           ../client/log.h \
           ../client/mysortfilterproxymodel.h \
           ../client/instances_dlg.h \
           ../server/qtserver/smbmanager.h \
           ../server/qtserver/process_smbd_exist.h \
           ../server/qtserver/sendmessage_manager.h \
           ../server/qtserver/disconnect_manager.h

RESOURCES = ../client/client.qrc

RC_FILE = ../client/qtsmbstatus.rc

FORMS +=  ../client/form_smbstatus.ui \
          ../client/configure.ui \
          ../client/log.ui

# install
unix {
    # Translation
    TRANSDIR = /usr/local/share/qtsmbstatus
    TRANSFILES = ../client/tr/*.qm
    trans.path = $$TRANSDIR 
    trans.files = $$TRANSFILES 

    # Documentation
    DOCDIR = /usr/local/share/doc/qtsmbstatusl
    DOCFILES = ../README \
           ../README-FR \
           ../INSTALL \
           ../COPYING
    doc.path = $$DOCDIR
    doc.files = $$DOCFILES

    #icons
    ICONSDIR = /usr/share/pixmaps
    ICONSFILES = qtsmbstatusl.xpm
    icons.path = $$ICONSDIR
    icons.files = $$ICONSFILES

    #menu
    MENUDIR = /usr/share/applications
    MENUFILES = qtsmbstatusl.desktop
    menu.path = $$MENUDIR
    menu.files = $$MENUFILES

    #manpage
    MANDIR = /usr/local/share/man/man7
    MANFILES = qtsmbstatusl.7.gz
    manpage.path = $$MANDIR
    manpage.files = $$MANFILES


    target.path = /usr/local/bin

    INSTALLS += trans \
            target \
            doc \
            icons \
            menu \
            manpage
}


QT += network

