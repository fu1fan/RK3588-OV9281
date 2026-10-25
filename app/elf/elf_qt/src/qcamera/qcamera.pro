
TEMPLATE = app

QT += gui widgets serialport multimedia  multimediawidgets
include($$PWD/../appout.pri)

HEADERS += \
    $$PWD/qtcamera.h

SOURCES += \
    $$PWD/qtcamera.cpp \
    main.cpp

TARGET = qtdemo_qcamera
