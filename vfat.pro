TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

#QMAKE_CFLAGS += -std=c99

SOURCES += main.c \
    vbr.c \
    test/testvbr.c \
    fdisk.c \
    binaryreader.c \
    fat.c \
    test/testfat.c \
    clusterchain.c \
    test/testclusterchain.c

HEADERS += \
    vbr.h \
    minunit.h \
    fdisk.h \
    test/testsuite.h \
    binaryreader.h \
    fat.h \
    clusterchain.h
