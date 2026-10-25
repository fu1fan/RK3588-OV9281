TEMPLATE = app

QT += gui widgets serialport
include($$PWD/../appout.pri)

HEADERS += \
    $$PWD/keywidget.h

SOURCES += \
    $$PWD/keywidget.cpp \
    main.cpp

TARGET = qtdemo_keypad
