TEMPLATE = app

QT += gui widgets serialport
include($$PWD/../appout.pri)

TARGET =qtdemo_wifi

HEADERS += \
    wifiwidget.h \
    wifiview.h \
    hostapdcli.h \
    switchwidget.h \
    wpaclicmd.h

SOURCES += \
    wifiwidget.cpp \
    wifiview.cpp \
    hostapdcli.cpp \
    switchwidget.cpp \
    wpaclicmd.cpp \
    main.cpp

FORMS += \
    wifiwidget.ui
