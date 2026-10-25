TEMPLATE = app

QT += gui widgets serialport
include($$PWD/../appout.pri)

HEADERS += \
    $$PWD/adcwidget.h

SOURCES += \
    $$PWD/main.cpp    \
    $$PWD/adcwidget.cpp \

TARGET = qtdemo_adc
