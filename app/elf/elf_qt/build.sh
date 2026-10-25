#如果平台支持，打开参数前边的#注符号进行编译，不开只编译默认,并安装到交叉系统.

#+++++++++++++++++++++++++++++++++++++配置参数开始 +++++++++++++++++++++++++++++++++++++++++++++++++++++
demo_qopengl="demo_qopengl"  ####支持GPU .

#demo_adc="demo_adc"  ####支持adc.


demo_spi="demo_spi"  ####支持spi.


#demo_ubootmenu="demo_ubootmenu"  ####qt应用配置屏幕开关.（3568demo）


#demo_wifiwapcli="demo_wifiwapcli"  ####系统支持 wapcli命令(目前T507和8mp测试)


#demo_arecord="demo_arecord"  ####平台支持arecord 和mPlayer命令.(6ull下demo)


#demo_usbcamera="demo_usbcamera"  ####平台支持v4l2./dev/videox（6ulldemo）


demo_qplayer="demo_qplayer" ####编译 qt多媒体视频播.


demo_qmusicplayer="demo_qmusicplayer"  ####编译 qt多媒音频.


demo_qcamera="demo_qcamera"  ####编译 qt多媒体摄像头.


#demo_qaudiorecorder="demo_qaudiorecorder"  ####编译 qt多媒体 录音


#demo_qmlvideofx="demo_qmlvideofx"  ####编译 qt qml视频播.


demo_qsimplebrowser="demo_qsimplebrowser"  ####平台支持qtwebengine模块.


#demo_btscanner="demo_btscanner"  ####qt 蓝牙. 系统支持BlueZ.


demo_matrix_browser="demo_matrix-browser" ###矩阵浏览器.

#demo_allwinner="demo_allwinner"   ####t507全志demo

#demo_webkit_browser="demo_webkit-browser" ####webkit浏览器，507 支持的.

dwkeyboard="DWKEYBOARD"   ####3568平台下支持的虚拟键盘.

#++++++++++++++++++++++++++++++++++配置参数结束++++++++++++++++++++++++++++++++++++++++++++++++++++++++

##使用qmake 编译.（此处qmake 路径添加到系统PATH ARM-g++ 必须在PATH 中）例子export PATH=xx/host/bin:xx/host/bin/aargch-64:$PATH
build_qmake=$RK_SDK_DIR/buildroot/output/elf2_fs/host/bin/qmake

##编译函数，无须修改.
function build_config()
{
    demo_arguments=""
    demos=("$demo_qopengl" "$demo_adc" "$demo_spi" "$demo_ubootmenu" "$demo_wifiwapcli" "$demo_arecord" "$demo_usbcamera" "$demo_qplayer" "$demo_qmusicplayer" "$demo_qcamera" "$demo_qaudiorecorder"   "$demo_qmlvideofx" "$demo_qsimplebrowser" "$demo_btscanner" "$demo_matrix_browser" "$demo_allwinner" "$demo_webkit_browser")
    for i in "${demos[@]}"; do
      if [ -n "$i" ]; then
        demo_arguments+="CONFIG+=$i "
     fi
    done
    $build_qmake  ../elf_qt-1.0/flapp.pro $demo_arguments
}

#编译执行函数函数.
function build()
{
        DIR=$PWD/../elf_qt_out
	if [ ! -d "$DIR" ]; then
	    mkdir "$DIR"
            echo "文件夹创建成功：$OUT"
	fi

	cd ../elf_qt_out
        make clean
        build_config

       if [ $? -eq 0 ]; then
    	 make -j4 && make install -j4
       else
        echo "build qt demo fail"
       fi
}

build $@
