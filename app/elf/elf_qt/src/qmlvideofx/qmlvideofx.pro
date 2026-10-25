TEMPLATE = app

include($$PWD/../buildout.pri)
TARGET = qtdemo_qmlvideofx

QT += quick multimedia

SOURCES += filereader.cpp main.cpp
HEADERS += filereader.h trace.h

RESOURCES += qmlvideofx.qrc

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

OTHER_FILES += \
    android/AndroidManifest.xml

QMAKE_INFO_PLIST = Info.plist
