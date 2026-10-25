#-------------------------------------------------
#
# Project created by QtCreator 2017-08-15T11:20:17
#
#-------------------------------------------------

include($$PWD/../../appout.pri)

QT       += multimedia multimediawidgets
include($$PWD/../allwinner.pri)

TARGET = qtdemo_cameraui

SOURCES += \
        src/main_desktop.cpp \
    src/setfirst.cpp \
    src/settings.cpp \
    src/about.cpp \
    src/front_adas_setting.cpp \
    src/front_camera_setting.cpp \
    src/rear_adas_setting.cpp \
    src/rear_camera_setting.cpp \
    src/reverseline.cpp \
    src/timesetting.cpp \
    src/preview.cpp \
    src/adas.cpp \
    src/mylabel.cpp \
    src/dashboard.cpp \
    src/myspeed.cpp \
    src/slave_picturewidget.cpp \
    src/slave_videowidget.cpp \
    src/cprojectionpicture.cpp \
    src/frmmessagebox.cpp \
    src/reverselinewidget.cpp \
    src/reverseline_setting.cpp \
    src/midwindow.cpp \
    src/AutoMount.cpp \
    unicodeandutf8.cpp \
    src/picture_view.cpp \
    src/picturewidget.cpp \
    src/cameraseldlg.cpp \
    src/recordseldlg.cpp \
    main.cpp

HEADERS += \
        src/main_desktop.h \
    src/setfirst.h \
    src/settings.h \
    src/about.h \
    src/front_adas_setting.h \
    src/front_camera_setting.h \
    src/rear_adas_setting.h \
    src/rear_camera_setting.h \
    src/reverseline.h \
    src/timesetting.h \
    src/preview.h \
    src/adas.h \
    src/mylabel.h \
    src/picture_view.h \
    src/picturewidget.h \
    src/dashboard.h \
    src/myspeed.h \
    src/slave_picturewidget.h \
    src/slave_videowidget.h \
    src/cprojectionpicture.h \
    src/frmmessagebox.h \
    src/reverselinewidget.h \
    src/reverseline_setting.h \
    src/midwindow.h \
    src/AutoMount.h \
    unicodeandutf8.h \
    src/cameraseldlg.h \
    src/recordseldlg.h


FORMS += \
        ui/main_desktop.ui \
    ui/setfirst.ui \
    ui/settings.ui \
    ui/about.ui \
    ui/front_adas_setting.ui \
    ui/front_camera_setting.ui \
    ui/rear_adas_setting.ui \
    ui/rear_camera_setting.ui \
    ui/reverseline.ui \
    ui/timesetting.ui \
    ui/preview.ui \
    ui/adas.ui \
    ui/picture_view.ui \
    ui/videowidget.ui \
    ui/picturewidget.ui \
    ui/dashboard.ui \
    ui/slave_picturewidget.ui \
    ui/slave_videowidget.ui \
    ui/frmmessagebox.ui \
    ui/reverseline_setting.ui \
    ui/cameraseldlg.ui \
    ui/recordseldlg.ui

RESOURCES += \
    image/image.qrc \
    image/font.qrc


TRANSLATIONS +=filemanager_zh.ts

OTHER_FILES += \
    filemanager_zh.qm \
    filemanager_zh.ts
