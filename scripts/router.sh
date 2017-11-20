#!/bin/bash

# $1 -> Stand number

#Configure ethernet
./ethernet.sh 1 $1 1 253

#Enable IP forwarding
echo 1 > /proc/sys/net/ipv4/ip_forward

#Disable ICMP echo-ignore-broadcast
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts

