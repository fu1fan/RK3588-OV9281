TEMPLATE = app

QT += gui widgets serialport sql opengl
include($$PWD/../appout.pri)

#UI 头文件和源文件
include($$PWD/common/common.pri)
include($$PWD/4g/4g.pri)
include($$PWD/adc/adc.pri)
include($$PWD/backlight/backlight.pri)
include($$PWD/key/key.pri)
include($$PWD/network/network.pri)
include($$PWD/opengl/opengl.pri)
include($$PWD/rtc/rtc.pri)
include($$PWD/serialport/serialport.pri)
include($$PWD/spi/spi.pri)
include($$PWD/sqlite/sqlite.pri)
include($$PWD/watchdog/watchdog.pri)
include($$PWD/wifi/wifi.pri)
include($$PWD/about/about.pri)
include($$PWD/terminal/terminal.pri)
include($$PWD/main/main.pri)

DEFINES +=  EXTRA
contains(DEFINES, EXTRA): include($$PWD/extra/extra.pri)
