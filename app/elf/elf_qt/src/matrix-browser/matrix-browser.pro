TEMPLATE = app

QT += gui widgets serialport

include($$PWD/../buildout.pri)
QT +=  webenginewidgets

TARGET = matrix-browser


SOURCES += main.cpp
