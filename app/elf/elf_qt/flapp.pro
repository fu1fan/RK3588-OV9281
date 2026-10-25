# 编译参数和配置 阅读 readme.txt
#make install时候 conf.txt文件自动安装到交叉编译系统
OTHER_FILES += $$PWD/resource/data/conf.txt \
               $$PWD/build.sh  \
               $$PWD/readme.txt

TEMPLATE = subdirs
# 按照 SUBDIRS 添加顺序编译，先编译动态库在编译其他依赖模块.
CONFIG += ordered

#功能库依赖.
SUBDIRS += src/libs

#平台无关模块编译.
SUBDIRS += \
          src/4g \
          src/wifi \
          src/watchdog \
          src/backlight \
          src/key \
          src/network \
          src/rtc \
          src/sqlite \
          src/about  \
          src/terminal \
          src/serialport

#openg测试如果有GPU 添加 .
demo_qopengl {
 SUBDIRS *= src/opengl
}

#支持adc.
demo_adc{
 SUBDIRS *= src/adc
}
#支持spi.
demo_spi {
 SUBDIRS *= src/spi
}
#qt应用配置屏幕开关.
demo_ubootmenu {
 SUBDIRS += src/ubootmenu
}
#系统支持 wapcli命令.
demo_wifiwapcli{
 SUBDIRS += src/wifiwapcli
}
#平台支持arecord 和mPlayer命令.
demo_arecord{
 SUBDIRS += src/arecord
}
#平台支持v4l2./dev/videox
demo_usbcamera{
 SUBDIRS += src/usbcamera
}
# 平台支持qt多媒体模块.系统支持gstreamer.
qtHaveModule(multimedia){
    #编译 qt多媒体视频播.
    demo_qplayer {
        SUBDIRS *= src/qplayer
    }
    #编译 qt多媒音频.
    demo_qmusicplayer {
        SUBDIRS *= src/musicplayer
    }
    #编译 qt多媒体摄像头.
    demo_qcamera {
        SUBDIRS *= src/qcamera
    }
    #编译 qt多媒体 录音
    demo_qaudiorecorder {
        SUBDIRS *= src/audiorecorder
    }
    #编译 qt qml视频播.
    demo_qmlvideofx {
        SUBDIRS *= src/qmlvideofx
    }
}
#平台支持qtwebengine模块.
qtHaveModule(webengine){
    #编译矩阵浏览器.
    demo_matrix-browser {
        SUBDIRS *= src/matrix-browser
    }

    #编译 qt多媒体视频播.
    demo_qsimplebrowser {
        SUBDIRS *= src/simplebrowser
    }
}
#qt 蓝牙. 系统支持BlueZ.
qtHaveModule(bluetooth){
    demo_btscanner {
     SUBDIRS *= src/btscanner
    }
}
#平台支持qtwebkit模块.
qtHaveModule(webkit){
    #编译矩阵浏览器.
    demo_matrix-browser {
        SUBDIRS *= src/matrix-browser
    }

    #编译webkit浏览器.
    demo_webkit-browser {
        SUBDIRS *= src/webkit-browser
    }
}
#t507
demo_allwinner {
 SUBDIRS *= src/allwinner
}

