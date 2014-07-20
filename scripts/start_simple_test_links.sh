#!/bin/bash

ip link add V1 type veth peer name V1_1
ifconfig V1 up
ifconfig V1_1 up


ip link add V2 type veth peer name V2_1
ifconfig V2 up
ifconfig V2_1 up


ip link add V3 type veth peer name V3_1
ifconfig V3 up
ifconfig V3_1 up


ip link add V4 type veth peer name V4_1
ifconfig V4 up
ifconfig V4_1 up

