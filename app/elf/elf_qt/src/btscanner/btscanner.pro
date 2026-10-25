TEMPLATE = app

TARGET = qtdemo_btscanner
QT += gui widgets serialport
include($$PWD/../appout.pri)

QT += bluetooth

SOURCES = \
    device.cpp \
    service.cpp \
    main.cpp

HEADERS = \
    device.h \
    service.h

FORMS = \
    device.ui \
    service.ui
