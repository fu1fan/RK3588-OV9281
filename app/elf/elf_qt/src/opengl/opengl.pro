TEMPLATE = app

QT += gui widgets serialport
include($$PWD/../appout.pri)

HEADERS += \
    $$PWD/glwidget.h

SOURCES += \
    $$PWD/main.cpp    \
    $$PWD/glwidget.cpp

TARGET = qtdemo_opengl
