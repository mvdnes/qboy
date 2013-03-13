#-------------------------------------------------
#
# Project created by QtCreator 2013-03-12T15:51:17
#
#-------------------------------------------------

QT       -= core gui

TARGET = libqboy
TEMPLATE = lib

DEFINES += LIBQBOY_LIBRARY

SOURCES += libqboy.cpp \
    z80.cpp \
    z80alu.cpp \
    z80mmu.cpp

HEADERS += libqboy.h\
        libqboy_global.h \
    z80.h \
    z80alu.h \
    z80mmu.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
