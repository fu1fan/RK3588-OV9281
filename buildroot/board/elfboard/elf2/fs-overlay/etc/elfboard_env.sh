#! /bin/sh

if [ ! -f /usr/sbin/fw_printenv ];then
	exit 0
fi 

/usr/sbin/alsactl restore

echo "export QT_QPA_PLATFORM=wayland" > /etc/profile.d/weston.sh
echo "export XDG_RUNTIME_DIR=/var/run" >> /etc/profile.d/weston.sh
echo "export WESTON_DISABLE_ATOMIC=1" >> /etc/profile.d/weston.sh
echo "export WESTON_DRM_PRIMARY=HDMI-A-1" >> /etc/profile.d/weston.sh
DISP=`fw_printenv disp_type`
VAL=${DISP#*=}
if [ "$VAL" != "async" ];then
	echo "export WESTON_DRM_MIRROR=1" >> /etc/profile.d/weston.sh
fi
