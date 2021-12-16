#!/bin/bash
#killall dmesg
#dmesg -w &
#echo "7" | sudo tee /proc/sys/kernel/printk
sudo insmod TASim.ko
sudo ifconfig TN0 up
sudo ifconfig TN1 up
sudo ifconfig TN2 up
sudo ifconfig TN3 up
