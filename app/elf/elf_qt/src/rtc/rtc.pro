TEMPLATE = app

QT += gui widgets serialport multimedia  multimediawidgets
include($$PWD/../appout.pri)

FORMS += \
    $$PWD/datetimewidget.ui

HEADERS += \
    $$PWD/datetimewidget.h

SOURCES += \
    $$PWD/datetimewidget.cpp \
    main.cpp

TARGET = qtdemo_rtc
