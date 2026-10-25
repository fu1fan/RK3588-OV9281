TEMPLATE = app

QT += gui widgets serialport multimedia  multimediawidgets
include($$PWD/../appout.pri)

HEADERS += \
    $$PWD/musicplayer.h \
    $$PWD/volumebutton.h

SOURCES += \
    $$PWD/musicplayer.cpp \
    $$PWD/volumebutton.cpp \
    main.cpp

TARGET = qtdemo_musicplayer
