TEMPLATE = app

QT += gui widgets serialport
include($$PWD/../appout.pri)

FORMS += \
    $$PWD/m4gwidget.ui

HEADERS += \
    $$PWD/m4gwidget.h

SOURCES += \
    $$PWD/m4gwidget.cpp \
    main.cpp

TARGET = qtdemo_4g
