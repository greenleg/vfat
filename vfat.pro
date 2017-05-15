TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

#QMAKE_CFLAGS += -std=c99

SOURCES += main.c \
    vbr.c \
    sector.c \
    test/testvbr.c \
    fdisk.c

HEADERS += \
    sector.h \
    vbr.h \
    minunit.h \
    fdisk.h \
    test/testsuite.h
