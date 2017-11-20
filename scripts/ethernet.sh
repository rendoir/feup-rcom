#!/bin/bash

# $1 -> Ethernet number
# $2 -> Stand number
# $3 -> Network ID
# $4 -> Host

/etc/init.d/networking restart

ifconfig eth$1 up
ifconfig eth$1 172.16.$2$3.$4/24

