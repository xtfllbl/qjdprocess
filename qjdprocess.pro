# -------------------------------------------------
# Project created by QtCreator 2010-02-20T09:58:50
# -------------------------------------------------
TARGET = qjdprocess
TEMPLATE = app
QMAKE_LFAGS+= -static
CONFIG += warn_off \
    qt \
    precompile_header \
    debug # debug
CONFIG += qtestlib
QT += dbus
SOURCES += main.cpp \
    qjdprocessmainwindow.cpp \
    qjdproc.cpp \
    qjdmisc.cpp \
    qjdoptions.cpp \
    qjdtable.cpp
HEADERS += qjdprocessmainwindow.h \
    qjdproc.h \
    config.h \
    qjdoptions.h \
    qjdtable.h
FORMS += qjdprocessmainwindow.ui \
    qjdoptions.ui
