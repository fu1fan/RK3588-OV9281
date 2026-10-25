TEMPLATE = app

QT += gui widgets serialport multimedia  multimediawidgets
include($$PWD/../appout.pri)


FORMS += \
    $$PWD/audiorecorder.ui

HEADERS += \
    $$PWD/audiorecorder.h \
    $$PWD/qaudiolevel.h

SOURCES += \
    $$PWD/audiorecorder.cpp \
    $$PWD/qaudiolevel.cpp \
    main.cpp

TARGET = qtdemo_audiorecorder
