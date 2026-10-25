#需要编译生成动态库，添加此文件在pro 中 include($$PWD/../../buildout.pri)
APP_SRC_PATH=$$PWD

APP_OUT_DIR=$${APP_SRC_PATH}/../../elf_qt_out

BUILD_TYPE=debug

CONFIG *=c++11

CONFIG(release, debug|release): BUILD_TYPE=release

LIBS *= -L$$APP_OUT_DIR/$${BUILD_TYPE}/lib
DEPENDPATH *=$$APP_OUT_DIR/$${BUILD_TYPE}/lib

#动态库安装到交叉编译 /usr/lib
equals(TEMPLATE, "lib") {
     target_path=$$[QT_SYSROOT]/usr/lib
     MODULE_TYPE=lib
}

#应用安装 交叉编译/usr/bin
equals(TEMPLATE, "app") {
     MODULE_TYPE=bin
     target_path=$$[QT_SYSROOT]/usr/bin
}

#编译输出目录.
DESTDIR = $$APP_OUT_DIR/$${BUILD_TYPE}/$${MODULE_TYPE}


#运行时查找路径
QMAKE_LFLAGS *= '-Wl,-rpath,\'\$$ORIGIN\' -Wl,-rpath,\'\$$ORIGIN\'/lib -Wl,-rpath,\'\$$ORIGIN\'/../lib'

#安装目录.
target.path = $${target_path}
INSTALLS += target

#应用配置文件和数据安装到应用程序目录下,默认为 QString BasePath::resourcePath()
appdata.path = $$[QT_SYSROOT]/usr/bin
appdata.files = $$PWD/../resource/*
INSTALLS +=appdata
