#! /bin/sh 
ID=/usr/bin/id
[ -z "$UID" ] && UID=`$ID -u`
ROOT_UID=0

service avahi-daemon start

#Check if run as root
if [ ${UID} -ne ${ROOT_UID} ] ; then
	echo $0 " must be run as root or sudo"
	exit 1
fi

ip netns del SW
ip link del LE1_0
ip link del C_0
ip link del vonuA_0
ip link del vonuB_0
ip link del vonuC_0
ifconfig LV1_0 down
brctl delbr LV1_0
