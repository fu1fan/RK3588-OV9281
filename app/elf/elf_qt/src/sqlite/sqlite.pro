TEMPLATE = app

QT += gui widgets serialport sql
include($$PWD/../appout.pri)

FORMS += \
    $$PWD/bookwindow.ui

HEADERS += \
    $$PWD/bookwindow.h

SOURCES += \
    $$PWD/main.cpp    \
    $$PWD/bookwindow.cpp

TARGET = qtdemo_sqlite
