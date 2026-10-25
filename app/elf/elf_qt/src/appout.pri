#编译输出配置脚本.
include(buildout.pri)

#调用lib库设置.
LIBS *=-lcore -lui
INCLUDEPATH *=$$APP_SRC_PATH/libs/core
INCLUDEPATH *=$$APP_SRC_PATH/libs/core/interface
INCLUDEPATH *=$$APP_SRC_PATH/libs/ui

#DEFINES +=DWKEYBOARD
