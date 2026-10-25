TEMPLATE = app

QT += gui widgets serialport
include($$PWD/../appout.pri)

FORMS += \
    $$PWD/backlightwidget.ui

HEADERS += \
    $$PWD/backlightwidget.h \
    sliderwidget.h

SOURCES += \
    $$PWD/main.cpp    \
    $$PWD/backlightwidget.cpp \
    sliderwidget.cpp

TARGET = qtdemo_backlight

DISTFILES += \
    backlight.pri
