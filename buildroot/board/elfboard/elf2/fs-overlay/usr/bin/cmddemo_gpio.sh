#! /bin/sh
if [ $# != 2 ]; then
	echo "$0 <GPIO_NAME> <1/0>"
	echo "User:$0 GPIO3_A7 1"
	exit 0
fi

VAL=$1

## GPIO[N]_[A-D][0-9]
N=${VAL:4:1}
M=${VAL:6:1}
I=${VAL:7:1}

j=0
for i in A B C D
do
	if [ "$M" == "$i" ]; then
	break;
	fi
	j=`expr $j + 1`
done

GPIOID=$((${N} * 32 + ${j} * 8 + ${I}))
if [ ! -d /sys/class/gpio/gpio$GPIOID ]; then
	echo $GPIOID > /sys/class/gpio/export
	echo out > /sys/class/gpio/gpio${GPIOID}/direction
fi

echo "===$1===$2"
echo $2 > /sys/class/gpio/gpio${GPIOID}/value
