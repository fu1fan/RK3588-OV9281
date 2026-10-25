TEMPLATE = app

QT += gui widgets serialport
include($$PWD/../appout.pri)

FORMS += \
    $$PWD/terminalwidget.ui \
    $$PWD/settingsdialog.ui

HEADERS += \
    $$PWD/console.h \
    $$PWD/terminalwidget.h \
    $$PWD/settingsdialog.h

SOURCES += \
    $$PWD/main.cpp    \
    $$PWD/console.cpp \
    $$PWD/terminalwidget.cpp \
    $$PWD/settingsdialog.cpp

TARGET = qtdemo_terminal
