
TEMPLATE = app

CONFIG += release \
          warn_on

TARGET =  utest

MOC_DIR =     ./build/moc
OBJECTS_DIR = ./build
DESTDIR =     ./bin
UI_DIR =      ./build/ui
RCC_DIR =     ./build/rcc

SOURCES += main.cpp \
           utestlinecore.cpp \
           utestsmbstatus.cpp \
           ../client/linecore.cpp \
           ../client/smbstatus.cpp

HEADERS += utestlinecore.h \
           utestsmbstatus.h \
           ../client/linecore.h \
           ../client/smbstatus.h


QT += testlib
QT-= gui

