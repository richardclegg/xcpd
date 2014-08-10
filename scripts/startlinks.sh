#!/bin/bash

ip link add type veth
ifconfig veth0 up
ifconfig veth1 up

ip link add type veth
ifconfig veth2 up
ifconfig veth3 up

ip link add type veth
ifconfig veth4 up
ifconfig veth5 up

ip link add type veth
ifconfig veth6 up
ifconfig veth7 up
