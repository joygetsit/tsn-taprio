#!/bin/bash

#cd ~/Documents/TASim/

#./installdriver.sh
#./install_bridge.sh

xterm -title "UDPRx#1" -hold -e "sudo ip netns exec nsRX iperf -s -u -p 3001" &
xterm -title "UDPRx#2" -hold -e "sudo ip netns exec nsRX iperf -s -u -p 3002" &
xterm -title "UDPRx#3" -hold -e "sudo ip netns exec nsRX iperf -s -u -p 3003" &

xterm -title "CaptureTxHost" -hold -e "sudo ip netns exec nsTX wireshark" &
xterm -title "CaptureRxHost" -hold -e "sudo ip netns exec nsRX wireshark" &

sleep 10

xterm -title "UDP#1" -hold -e "sudo ip netns exec nsTX iperf -c 10.0.100.13 -u -t 5 -p 3001 -i 1 -b 10pps" &
xterm -title "UDP#2" -hold -e "sudo ip netns exec nsTX iperf -c 10.0.100.13 -u -t 5 -p 3002 -i 1 -b 10pps" &
xterm -title "UDP#3" -hold -e "sudo ip netns exec nsTX iperf -c 10.0.100.13 -u -t 5 -p 3003 -i 1 -b 10pps" 
