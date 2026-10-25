一. 编译和配置
	1.编译环境命令和配置
                #qtcreator 编译在projects ->Build->Build step->Details->addtional arguments: 输入：
                CONFIG+=demo_qopengl CONFIG+=demo_adc xxx

		#在命令行编译: 输入：  qmake   xxx/flapp  \
					CONFIG+=demo_qopengl \
					CONFIG+=demo_adc \
					xxx
						  
		编译输出目录为 flapp/../flapp_out/release

                安装命令 （安装所有可执行程序到 usr/bin   安装所有的库交叉 usr/lib  安装配置文件和资源 到交叉 usr/bin/data）
		make xxx/Makefiel  install -j4

                build.sh 平台方便的编译脚本，根据配置自动编译应用，并安装到交叉文件系统          

	2.默认编译qt 应用
			  4g 
			  wifi 
			  watchdog 
			  backlight 
			  key 
			  network 
			  rtc 
			  sqlite 
			  about  
			  terminal 
			  serialport
			  
			  
	3. 需要添加参数后编译的模块，平台按需要添加，如果对应qt没有对应模块，添加参数后不会编译，如果编译通过，确认模块系统依赖是否支持，并进行功能验证系统环境是否正常.
			#支持GPU .
			demo_qopengl

			#支持adc.
			demo_adc

			#支持spi.
			demo_spi

			#qt应用配置屏幕开关.（3568demo）
			demo_ubootmenu

			#系统支持 wapcli命令(目前T507和8mp测试)
			demo_wifiwapcli

			#平台支持arecord 和mPlayer命令.
			demo_arecord(6ull下demo)

			#平台支持v4l2./dev/videox
			demo_usbcamera （6ulldemo）

			# 平台支持qt多媒体模块.系统支持gstreamer.
				#编译 qt多媒体视频播.
				demo_qplayer
				
				#编译 qt多媒音频.
				demo_qmusicplayer 
				
				#编译 qt多媒体摄像头.
				demo_qcamera
				
				#编译 qt多媒体 录音
				demo_qaudiorecorder
				
				#编译 qt qml视频播.
				demo_qmlvideofx 

			#平台支持qtwebengine模块.
				demo_qsimplebrowser

			#qt 蓝牙. 系统支持BlueZ.
				demo_btscanner
				
			#平台支持qtwebkit模块.
				demo_matrix-browser
				
			#t507全志demo
			demo_allwinner

                        webkit浏览器，507 支持的.
                        #demo_webkit_browser="demo_webkit-browser"

        4.  资源文件/home/elf/T507/flapp/resource （安装时自动拷贝到交叉文件系统xxx/usr/bin/data）
                
		flapp/resource/data/rc/books  sqlite图片资源文件夹
		flapp/resource/data/rc/opengl  opengl 资源文件夹
		flapp/resource/data/rc/terminal 终端串口资源文件夹
		resource/data/rc/web         webengine 图标资源文件夹爱
                flapp/resource/data/conf.txt 应用配置文件
               


                系统配置文件应用目录下flapp/resource/data/conf.txt
		[backlight]
		devName=/sys/class/backlight/edp-backlight/brightness
		min=5
		max=100
		name=EVP

		[backlight]
		devName=/sys/class/backlight/lvds-backlight/brightness
		min=5
		max=200
		name=LVDS

		[backlight]
		devName=/sys/class/backlight/dsi1-backlight/brightness
		min=5
		max=100
		name=DSI

		[key]
		devName=/dev/input/event4
		names=KEY_V_UP;KEY_V_DOWN;KEY_V_HOME;KEY_V_ESC
		values=115;114;139;158

		[watchdog]
		devName=/dev/watchdog

		[4g]
		name=ec20
		bin=/usr/bin/quectelCM
		devName=mlan0
		tty=/dev/tty20
		baudrate=115200
		databits=8
		stopbit=1
		parity=0
		control=0
		type=0

		[adc]
		devName=/dev/input/event2
		open=echo gpadc0,1> /sys/class/gpadc/status
		close=echo gpadc0,0> /sys/class/gpadc/status
		name=adc0

		[adc]
		devName=/dev/input/event3
		open=echo gpadc1,1> /sys/class/gpadc/status
		close=echo gpadc1,0> /sys/class/gpadc/status
		name=adc1

		[adc]
		devName=/dev/input/event4
		open=echo gpadc2,1> /sys/class/gpadc/status
		close=echo gpadc2,0> /sys/class/gpadc/status
		name=adc2

		[adc]
		devName=/dev/input/event5
		open=echo gpadc3,1> /sys/class/gpadc/status
		close=echo gpadc3,0> /sys/class/gpadc/status
		name=adc3

		[wifi]
		names=mlan0;mlan1
		ssid=H3C_708_5G
		pawd=123456785.
		ip=192.168.1.111

		[wpacli]
		moduleName=nl80211
		cardName=wlan0
		driverName=8821cs
		ko=/lib/modules/4.9.170/8821cs.ko
   
