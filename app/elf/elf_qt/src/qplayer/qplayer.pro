TEMPLATE = app

QT += gui widgets serialport multimedia  multimediawidgets
include($$PWD/../appout.pri)


HEADERS += \
    $$PWD/qplayer.h

SOURCES += \
    $$PWD/qplayer.cpp \
    main.cpp

TARGET = qtdemo_qplayer
