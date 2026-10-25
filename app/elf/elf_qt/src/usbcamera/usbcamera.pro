#-------------------------------------------------
#
# Project created by QtCreator 2017-04-10T09:08:26
#
#-------------------------------------------------

TEMPLATE = app

QT += gui widgets
include($$PWD/../appout.pri)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtdemo_usbcamera

LIBS += -L. -ljpeg

SOURCES += main.cpp\
        camera.cpp

HEADERS  += camera.h

FORMS    += camera.ui
