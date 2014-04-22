#-------------------------------------------------
#
# Project created by QtCreator 2012-09-05T10:50:00
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = LinuxCallerId
TEMPLATE = app

include(qextserialport-1.2beta2/src/qextserialport.pri)

SOURCES += main.cpp\
        configWindow.cpp \
    editNumber.cpp

HEADERS  += configWindow.h \
    editNumber.h

FORMS    += configWindow.ui \
    editNumber.ui

RESOURCES += \
    resources.qrc
