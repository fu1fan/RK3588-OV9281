TEMPLATE = lib

QT +=core serialport

include($$PWD/../../buildout.pri)

DEFINES +=CORE_LIBRARY_EXPORT

TARGET = core

include($$PWD/interface/interface.pri)

HEADERS += \
    basefactory.h \
    processhandler.h \
    serialport.h \
    settings.h \
    terminal.h \
    conf.h \
    ustring.h \
    resourceloader.h \
    basepath.h \
    threadexec.h \
    threadexec.h

SOURCES += \
    processhandler.cpp \
    serialport.cpp \
    settings.cpp \
    terminal.cpp \
    conf.cpp \
    basefactory.cpp \
    resourceloader.cpp \
    basepath.cpp

