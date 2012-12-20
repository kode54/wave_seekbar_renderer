#-------------------------------------------------
#
# Project created by QtCreator 2012-12-18T17:03:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = wave_seekbar_renderer
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x

DEFINES += _7ZIP_ST

SOURCES += main.cpp\
        mainwindow.cpp \
    lzma/XzIn.c \
    lzma/XzEnc.c \
    lzma/XzDec.c \
    lzma/XzCrc64.c \
    lzma/Xz.c \
    lzma/Sha256.c \
    lzma/Ppmd7Enc.c \
    lzma/Ppmd7Dec.c \
    lzma/Ppmd7.c \
    lzma/LzmaLib.c \
    lzma/Lzma2Dec.c \
    lzma/Alloc.c \
    lzma/7zStream.c \
    lzma/7zIn.c \
    lzma/LzFind.c \
    lzma/Delta.c \
    lzma/7zFile.c \
    lzma/7zDec.c \
    lzma/7zCrcOpt.c \
    lzma/LzmaEnc.c \
    lzma/LzmaDec.c \
    lzma/Lzma86Enc.c \
    lzma/Lzma86Dec.c \
    lzma/Lzma2Enc.c \
    lzma/CpuArch.c \
    lzma/BraIA64.c \
    lzma/Bra86.c \
    lzma/7zCrc.c \
    lzma/7zBuf2.c \
    lzma/Bra.c \
    lzma/Bcj2.c \
    lzma/7zBuf.c \
    lzma/7zAlloc.c \
    Pack.cc

HEADERS  += mainwindow.h \
    lzma/XzEnc.h \
    lzma/XzCrc64.h \
    lzma/Xz.h \
    lzma/Types.h \
    lzma/Threads.h \
    lzma/Sha256.h \
    lzma/RotateDefs.h \
    lzma/Ppmd7.h \
    lzma/Ppmd.h \
    lzma/LzmaLib.h \
    lzma/LzmaEnc.h \
    lzma/LzHash.h \
    lzma/7zVersion.h \
    lzma/LzFind.h \
    lzma/Delta.h \
    lzma/7zFile.h \
    lzma/LzmaDec.h \
    lzma/Lzma86.h \
    lzma/Lzma2Enc.h \
    lzma/Lzma2Dec.h \
    lzma/CpuArch.h \
    lzma/7zCrc.h \
    lzma/7zBuf.h \
    lzma/Bra.h \
    lzma/Bcj2.h \
    lzma/Alloc.h \
    lzma/7zAlloc.h \
    lzma/7z.h \
    Pack.h

FORMS    += mainwindow.ui

unix|win32: LIBS += -lsqlite3 -lz
