#! /bin/sh 
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
ifconfig C_0 hw ether 00:00:00:00:02:01
ifconfig C_1 hw ether 00:00:00:00:02:02
ip link set C_1 netns SW
ifconfig C_0 10.100.2.1/24 up
ip netns exec SW ifconfig C_1 10.100.2.2/24 up
arp -s 10.100.2.2 00:00:00:00:02:02
ip netns exec SW arp -s 10.100.2.1 00:00:00:00:02:01

ip link add LE1_0 type veth peer name LE1_1
ifconfig LE1_0 hw ether 00:00:00:00:03:01
ifconfig LE1_1 hw ether 00:00:00:00:03:02
ifconfig LE1_0 10.100.3.1/24 up
ip link set LE1_1 netns SW
ip netns exec SW ifconfig LE1_1 10.100.3.2/24 up
ip netns exec SW route add default LE1_1
arp -s 10.100.3.2 00:00:00:00:03:02
ip netns exec SW arp -s 10.100.3.1 00:00:00:00:03:01

ip link add vonuA_0 type veth peer name vonuA_1
ifconfig vonuA_0 hw ether 00:00:00:00:04:01
ifconfig vonuA_1 hw ether 00:00:00:00:04:01
vconfig add vonuA_1 10
ip link set vonuA_1 netns SW
ip link set vonuA_1.10 netns SW
ifconfig vonuA_0 10.100.4.1/24 up
ip netns exec SW ifconfig vonuA_1 up
ip netns exec SW ifconfig vonuA_1.10 10.100.4.2/24 up
arp -s 10.100.4.2 00:00:00:00:04:02
ip netns exec SW arp -s 10.100.4.1 00:00:00:00:04:01

ip link add vonuB_0 type veth peer name vonuB_1
ifconfig vonuB_0 hw ether 00:00:00:00:05:01
ifconfig vonuB_1 hw ether 00:00:00:00:05:02
vconfig add vonuB_1 11
ip link set vonuB_1 netns SW
ip link set vonuB_1.11 netns SW
ifconfig vonuB_0 10.100.5.1/24 up
ip netns exec SW ifconfig vonuB_1 up
ip netns exec SW ifconfig vonuB_1.11 10.100.5.2/24 up
arp -s 10.100.5.2 00:00:00:00:05:02
ip netns exec SW arp -s 10.100.5.1 00:00:00:00:05:01

ip link add vonuC_0 type veth peer name vonuC_1
ifconfig vonuC_0 hw ether 00:00:00:00:06:01
ifconfig vonuC_1 hw ether 00:00:00:00:06:02
vconfig add vonuC_1 12
ip link set vonuC_1 netns SW
ip link set vonuC_1.12 netns SW
ifconfig vonuC_0 10.100.6.1/24 up
ip netns exec SW ifconfig vonuC_1 up
ip netns exec SW ifconfig vonuC_1.12 10.100.6.2/24 up
arp -s 10.100.6.2 00:00:00:00:06:02
ip netns exec SW arp -s 10.100.6.1 00:00:00:00:06:01

ip link add LV1_0 type veth peer name LV1_1

brctl addbr br
brctl addif br LV1_1 vonuA_0 vonuB_0 vonuC_0 

ifconfig LV1_0 10.9.0.99/30 up
