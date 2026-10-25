#-------------------------------------------------
#
# Project created by QtCreator 2018-08-23T14:12:15
#
#-------------------------------------------------
TEMPLATE = app

QT       += core gui widgets

TARGET = qtdemo_player

include($$PWD/../../appout.pri)

include($$PWD/../allwinner.pri)

SOURCES += \
    musicplayer/Sources/musicplayer.cpp \
    musicplayer/Sources/playlist.cpp \
    videoplayer/Sources/videoplayer.cpp \
    videoplayer/Sources/videoplaylist.cpp \
    common/Sources/mediahome.cpp \
    common/Sources/main.cpp \
    common/Sources/waittingwidget.cpp \
    common/Sources/switchbutton.cpp \
    common/Sources/searchfiledialog.cpp \
    common/Sources/mount.cpp \
    common/Sources/frmmessagebox.cpp \
    common/Sources/commonfunction.cpp

HEADERS  += \
    musicplayer/Header/musicplayer.h \
    musicplayer/Header/playlist.h \
    videoplayer/Header/videoplayer.h \
    videoplayer/Header/videoplaylist.h \
    common/Header/mediahome.h \
    common/Header/mount.h \
    common/Header/commonfunction.h \
    common/Header/config.h \
    common/Header/frmmessagebox.h \
    common/Header/searchfiledialog.h \
    common/Header/switchbutton.h \
    common/Header/waittingwidget.h

FORMS    += \
    common/Forms/mediahome.ui \
    musicplayer/Forms/musicplayer.ui \
    musicplayer/Forms/playlist.ui \
    videoplayer/Forms/videoplayer.ui \
    videoplayer/Forms/videoplaylist.ui \
    common/Forms/frmmessagebox.ui \
    common/Forms/searchfiledialog.ui \
    common/Forms/waittingwidget.ui

RESOURCES += \
    image.qrc \
    font.qrc
	
