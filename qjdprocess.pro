# -------------------------------------------------
# Project created by QtCreator 2010-02-20T09:58:50
# -------------------------------------------------
TARGET = qjdprocess
TEMPLATE = app
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
    qjdreport.cpp
HEADERS += qjdprocessmainwindow.h \
    qjdproc.h \
    config.h \
    qjdoptions.h \
    qjdreport.h
FORMS += qjdprocessmainwindow.ui \
    qjdoptions.ui \
    qjdreport.ui
