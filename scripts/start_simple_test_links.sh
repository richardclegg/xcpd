#!/bin/bash


service avahi-daemon stop

ip link add V1 type veth peer name V1a
ifconfig V1 up
ifconfig V1a up


ip link add V2 type veth peer name V2a
ifconfig V2 up
ifconfig V2a up


ip link add V3 type veth peer name V3a
ifconfig V3 up
ifconfig V3a up


ip link add V4 type veth peer name V4a
ifconfig V4 up
ifconfig V4a up

