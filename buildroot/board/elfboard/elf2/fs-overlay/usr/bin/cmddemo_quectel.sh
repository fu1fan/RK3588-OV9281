#!/bin/sh
#(ifconfig -a | grep -v grep | grep eth0) && ifconfig eth0 down
#(ifconfig -a | grep -v grep | grep eth1) && ifconfig eth1 down
#(ifconfig -a | grep -v grep | grep mlan0) && ifconfig mlan0 down
for dev in `/bin/ls /sys/class/net`
do
	res=`/bin/echo $dev | /bin/grep "can"`
	if [ "$dev" != "usb0" -a "$dev" != "lo"  -a "$res" == "" ];then
		/sbin/ifconfig $dev down
	fi
done
/usr/bin/quectelCM 
