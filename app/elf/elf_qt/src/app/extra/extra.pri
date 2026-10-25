QT     +=  multimedia  multimediawidgets core-private virtualkeyboard

include($$PWD/audiorecorder/audiorecorder.pri)
include($$PWD/musicplayer/musicplayer.pri)
include($$PWD/qplayer/qplayer.pri)
include($$PWD/qcamera/qcamera.pri)
include($$PWD/ubootmenu/ubootmenu.pri)
include($$PWD/generator/generator.pri)
include($$PWD/simplebrowser/simplebrowser.pri)

#键盘库和头文件指定
LIBSDK_PATH = /home/elf/3568/OK3568-linux-source/app/elf/DWKeyboard
LIBSDK_LIB_PATH = $$[QT_SYSROOT]/lib64/

INCLUDEPATH +=  $$PWD/$$LIBSDK_PATH  \
                $$PWD/$$LIBSDK_PATH/include
unix:!macx: LIBS += -L$$PWD/$$LIBSDK_PATH/  -L$$PWD/$$LIBSDK_LIB_PATH -lvirtualkeyboard
