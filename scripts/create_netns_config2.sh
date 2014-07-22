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



ip netns add SW
ip netns exec SW ip link set dev lo up

ip link add C_0 type veth peer name C_1
ifconfig C_0 hw ether 00:00:10:99:00:01
ifconfig C_1 hw ether 00:00:10:99:00:02
ip link set C_1 netns SW
ifconfig C_0 10.100.0.1/30 up
ip netns exec SW ifconfig C_1 10.100.0.2/30 up
arp -s 10.100.0.2 00:00:10:99:00:02
ip netns exec SW arp -s 10.100.0.1 00:00:10:99:00:01

ip link add LE1_0 type veth peer name LE1_1
ifconfig LE1_0 up
ip link set LE1_1 netns SW
ip netns exec SW ifconfig LE1_1 up
ip netns exec SW route add default LE1_1

ip link add vonuA_0 type veth peer name vonuA_1
ip link set vonuA_1 netns SW
ifconfig vonuA_0 up
ip netns exec SW ifconfig vonuA_1 up

ip link add vonuB_0 type veth peer name vonuB_1
ip link set vonuB_1 netns SW
ifconfig vonuB_0 up
ip netns exec SW ifconfig vonuB_1 up

ip link add vonuC_0 type veth peer name vonuC_1
ip link set vonuC_1 netns SW
ifconfig vonuC_0 up
ip netns exec SW ifconfig vonuC_1 up

ip link add LV1_0 type veth peer name LV1_1
sudo ifconfig LV1_0 up
sudo ifconfig LV1_1 up

vconfig add LV1_1 10
vconfig add LV1_1 11
vconfig add LV1_1 12

brctl addbr br10
brctl addbr br11
brctl addbr br12

brctl addif br10 LV1_1.10
brctl addif br10 vonuA_0

brctl addif br11 LV1_1.11
brctl addif br11 vonuB_0

brctl addif br12 LV1_1.12
brctl addif br12 vonuC_0
