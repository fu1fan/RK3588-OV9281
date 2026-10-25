TEMPLATE = app

QT += gui widgets serialport
include($$PWD/../appout.pri)

FORMS += \
    $$PWD/about.ui

HEADERS += \
    $$PWD/abountwidget.h

SOURCES += \
    $$PWD/abountwidget.cpp \
    $$PWD/main.cpp

TARGET = qtdemo_about
