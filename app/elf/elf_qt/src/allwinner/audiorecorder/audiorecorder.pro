TEMPLATE = app

TARGET = qtdemo_audiorecorder
include($$PWD/../../appout.pri)
QT += gui widgets serialport

include($$PWD/../allwinner.pri)

CONFIG += c++11

SOURCES += \
    widget.cpp \
    main.cpp

HEADERS += \
    widget.h

FORMS += \
    widget.ui

