#! /bin/sh -v

ID=/usr/bin/id
[ -z "$UID" ] && UID=`$ID -u`
ROOT_UID=0

#Check if run as root
if [ ${UID} -ne ${ROOT_UID} ] ; then
	echo $0 " must be run as root or sudo"
	exit 1
fi

echo "1" > /proc/sys/net/ipv6/conf/all/disable_ipv6
service avahi-daemon stop
modprobe 8021q



ip link add LE1_0 type veth peer name LE1_1
ifconfig LE1_0 up
ifconfig LE1_1 up

ip link add vonuA_0 type veth peer name vonuA_1
vconfig add vonuA_1 10
ifconfig vonuA_0 up
ifconfig vonuA_1 up
ifconfig vonuA_1.10 up

ip link add vonuB_0 type veth peer name vonuB_1
vconfig add vonuB_1 11
ifconfig vonuB_0 up
ifconfig vonuB_1 up
ifconfig vonuB_1.11 up

ip link add vonuC_0 type veth peer name vonuC_1
vconfig add vonuC_1 12
ifconfig vonuC_0 up
ifconfig vonuC_1 up
ifconfig vonuC_1.12 up

brctl addbr LV1_0
brctl addif LV1_0 vonuA_0
brctl addif LV1_0 vonuB_0
brctl addif LV1_0 vonuC_0
ifconfig LV1_0 up
