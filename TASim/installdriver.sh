#!/bin/bash
#killall dmesg
#dmesg -w &
#echo "7" | sudo tee /proc/sys/kernel/printk
sudo insmod TASim.ko
sudo ip link set dev TN0 up
sudo ip link set dev TN1 up
sudo ip link set dev TN2 up
sudo ip link set dev TN3 up
#sudo ifconfig TN0 up
#sudo ifconfig TN1 up
#sudo ifconfig TN2 up
#sudo ifconfig TN3 up
