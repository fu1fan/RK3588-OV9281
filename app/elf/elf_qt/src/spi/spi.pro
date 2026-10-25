TEMPLATE = app
QT += gui widgets serialport sql opengl
include($$PWD/../appout.pri)

FORMS += \
    $$PWD/spiwidget.ui

HEADERS += \
    $$PWD/spiwidget.h

SOURCES += \
    $$PWD/main.cpp    \
    $$PWD/spiwidget.cpp

TARGET = qtdemo_spi
