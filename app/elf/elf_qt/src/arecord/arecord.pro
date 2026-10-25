TEMPLATE = app

TARGET = qtdemo_audiorecorder
QT += gui widgets serialport
include($$PWD/../appout.pri)

HEADERS += mainwindow.h
FORMS += mainwindow.ui
SOURCES +=  \
    mainwindow.cpp \
    main.cpp
