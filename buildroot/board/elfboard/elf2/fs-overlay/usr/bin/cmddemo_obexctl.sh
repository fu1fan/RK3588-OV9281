#! /bin/sh
export DBUS_SESSION_BUS_ADDRESS=unix:path=/var/run/dbus/system_bus_socket
res=`hciconfig  | grep hci`
if [ "$res" != "" ];then
	/usr/bin/obexctl
fi

