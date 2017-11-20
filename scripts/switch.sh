#!/bin/bash

# $1 -> Stand number
# $2 -> Port number
# $3 -> VLAN number

enable
#8nortel

#Creating an Ethernet VLAN
configure terminal
vlan "$1"0
end

#Add port to vlan
configure terminal
interface fastethernet 0/$2
switchport mode access
switchport access vlan $1$3
end

