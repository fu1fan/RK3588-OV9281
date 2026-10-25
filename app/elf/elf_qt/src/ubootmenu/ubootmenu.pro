TEMPLATE = app

QT += gui widgets serialport
include($$PWD/../appout.pri)

FORMS += \
    $$PWD/ubootmenuwidget.ui

HEADERS += \
    $$PWD/ubootmenuwidget.h

SOURCES += \
    $$PWD/ubootmenuwidget.cpp \
    main.cpp

TARGET = qtdemo_ubootmenu
