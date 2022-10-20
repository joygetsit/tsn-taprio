#!/bin/bash

# The follwing set of codes (use newer iputils2 set of commands) to
# create a switch from multiple Ethernet ports of a NIC.
 #sudo brctl addbr br0
 #sudo brctl addif br0 TN1
 #sudo brctl addif br0 TN2
 #sudo ifconfig br0 up 
 #sudo modprobe br_netfilter
# sudo ip link add name br0 type bridge  
# # sudo ip link set dev br0 up
# sudo ip link set enp1s0np0 master br0
# sudo ip link set enp1s0np1 master br0

ip a # Show all network devices/interfaces
# Creates network namespaces (acts like computer nodes, with only deactivated loopback devices)
sudo ip netns add nsTX
sudo ip netns add nsRX
ip netns


# To communicate between the created namespaces and root namespaces,
# a communication channel has to be created. 
# You can use veth but this will create two interfaces that are not TSN-capable.
# We will use TSN interfaces created with our network driver module, 
# and assign them to respective network namespaces 
# and then create 2 namespaces to a bridge in the root network namespace
sudo ip link set TN0 netns nsTX
sudo ip link set TN3 netns nsRX

# Assign IP addressess to the interafces to make them functional and activate them
sudo ip netns exec nsTX ip addr add 10.0.100.10/24 dev TN0
sudo ip netns exec nsRX ip addr add 10.0.100.13/24 dev TN3



ip a # Show all network devices/interfaces
sudo ip link add br0 type bridge # we have to do this trick because iptables don't work with replayed packets directly
sudo ip addr add 10.0.100.1/24 dev br0 # Only needed to enable networking between root namespace and created network namespaces
# Set the interfaces to a bridge to connect them 
sudo ip link set TN1 master br0
sudo ip link set TN2 master br0

# Enable the loopback and other interfaces
sudo ip netns exec nsTX ip link set dev lo up
sudo ip netns exec nsRX ip link set dev lo up
sudo ip netns exec nsTX ip link set dev TN0 up
sudo ip netns exec nsRX ip link set dev TN3 up
sudo ip link set br0 up
sudo modprobe br_netfilter

# Call these before modifying qdisc becuase it resets the qdiscs
echo 1 > /proc/sys/net/bridge/bridge-nf-filter-vlan-tagged 
echo 1 > /proc/sys/net/bridge/bridge-nf-call-iptables 
echo 1 > /proc/sys/net/bridge/bridge-nf-call-ip6tables 

sudo tc qdisc replace dev TN2 parent root handle 100 taprio \
num_tc 2 \
map 1 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 \
queues 1@0 1@1 \
base-time 0 \
sched-entry S 01 100000000 sched-entry S 02 100000000 \
clockid CLOCK_TAI

sudo iptables -t mangle -A POSTROUTING -p udp --dport 3001 -j CLASSIFY --set-class 0:1 
sudo iptables -t mangle -A POSTROUTING -p udp --dport 3002 -j CLASSIFY --set-class 0:0 

#sudo iptables -t mangle -A POSTROUTING -d 1.1.1.1 -j CLASSIFY --set-class 0:1
#sudo iptables -t mangle -A POSTROUTING -d 2.2.2.2 -j CLASSIFY --set-class 0:0


