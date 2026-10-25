TEMPLATE = subdirs

#顺序编译
CONFIG += ordered

SUBDIRS += \
           $$PWD/core \
           $$PWD/ui \
           $$PWD/imvirtualkeyboard

#core 通用的接口功能实例类
#ui 圆角的主窗口 通用标题栏，定制的消息弹窗、 键盘 或者Ui 的定制等.
