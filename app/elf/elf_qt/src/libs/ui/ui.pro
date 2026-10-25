TEMPLATE = lib

QT +=core serialport widgets

TARGET = ui

DEFINES +=UI_LIBRARY_EXPORT

include($$PWD/../../buildout.pri)

HEADERS += \
    $$PWD/appwindow.h \
    $$PWD/messagebox.h \
    $$PWD/titlebar.h \
    $$PWD/eventhandler.h

SOURCES += \
    $$PWD/appwindow.cpp \
    $$PWD/messagebox.cpp \
    $$PWD/titlebar.cpp \
    $$PWD/eventhandler.cpp

INCLUDEPATH +=$$PWD

# DWKEYBOARD键盘的使用编译和配置
DWKEYBOARD {
    message("use DWKEYBOARD..." + $$DEFINES)
    QT     +=  core-private virtualkeyboard
    LIBS +=-lvirtualkeyboard
}

LIBS *=-lcore
INCLUDEPATH *=$$APP_SRC_PATH/libs/core
INCLUDEPATH *=$$APP_SRC_PATH/libs/core/interface
