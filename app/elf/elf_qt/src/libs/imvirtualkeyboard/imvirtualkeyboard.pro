include($$PWD/../../plugin.pri)

TARGET = IM

QT += gui gui-private core-private widgets

CONFIG += c++11

HEADERS += \
    abstractinputmethod.h \
    implatforminputcontext.h \
    iminputcontext.h \
    inputmethodmanager.h \
    keyboardwidget.h

SOURCES += \
    abstractinputmethod.cpp \
    implatforminputcontext.cpp \
    iminputcontext.cpp \
    inputmethodmanager.cpp \
    main.cpp \
    keyboardwidget.cpp

OTHER_FILES += $$PWD/im.json

include($$PWD/pinyininputmethod/pinyin/pinyin.pri)
include($$PWD/pinyininputmethod/pinyin2/pinyin2.pri)


target.path = $$[QT_INSTALL_PLUGINS]/platforminputcontexts
INSTALLS += target

imdata.path = $$[QT_SYSROOT]/usr/share/pinyin/3rdparty/pinyin
system("mkdir -p $${pinyin.path}")
imdata.files = $$PWD/pinyininputmethod/pinyin/data
INSTALLS +=imdata

#作为qt插件使用.
#qt5中英文软键盘.
#只支持QWidget框架使用，不支持qml框架, 不支持模态窗口输入, weston(wayland)只支持全屏QWidget窗口输入.
#支持显示架构 X11协议(Xserver) /wayland协议(weston)/ qt eglfs

#配置
 #1.拷贝dict_pinyin.dat 到 /share/pinyin/3rdparty/pinyin/data/dict_pinyin.dat
 #2.拷贝 libIM.so  到 xxx/plugins/platforminputcontexts/  .xxx=(/usr/lib/plugins/platforminputcontext  /usr/lib/qt/plugins/platforminputcontext)
 #3.设置环境变量 export QT_IM_MODULE=im

#两种显示模式
 #1根据输入框自动确定显示位置
 #2. qApp->setProperty("bottom", !bFollow);  显示到应用窗口的底部固定显示

#键盘大小按屏幕尺寸缩放大小比例.
