TEMPLATE = app
QT += gui widgets serialport

include($$PWD/../../appout.pri)
include($$PWD/../allwinner.pri)

TARGET = qtdemo_camera

SOURCES += \
    Camera.cpp \
    videocapture.cpp \
    main.cpp

HEADERS += \
    Camera.h \
    videocapture.h \
	sunxi_camera_v2.h

FORMS += \
    Camera.ui


