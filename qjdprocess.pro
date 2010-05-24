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
QT += xml

SOURCES += main.cpp \
    qjdprocessmainwindow.cpp \
    qjdproc.cpp \
    qjdmisc.cpp \
    qjdoptions.cpp \
    qjdtask.cpp \
    qjdstarttask.cpp
HEADERS += qjdprocessmainwindow.h \
    qjdproc.h \
    config.h \
    qjdoptions.h \
    qjdtask.h \
    qjdstarttask.h
FORMS += qjdprocessmainwindow.ui \
    qjdoptions.ui \
    qjdtask.ui \
    qjdstarttask.ui
