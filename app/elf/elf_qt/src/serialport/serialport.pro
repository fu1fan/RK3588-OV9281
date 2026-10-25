TEMPLATE = app

QT += gui widgets serialport
include($$PWD/../appout.pri)

FORMS += \
    $$PWD/serialassistantwidget.ui

HEADERS += \
    $$PWD/serialassistantwidget.h

SOURCES += \
    $$PWD/serialassistantwidget.cpp \
    main.cpp

TARGET =qtdemo_serialport
