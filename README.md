# TSN
Programs and scripts to test TSN on Linux

To make and load the module for the first time,
execute 'sudo make'.    
To delete all interfaces and remove the module, execute 'sudo make unload'  

To run the scripts when they are first downloaded to the PC, execute following commands to give permissions:  
sudo chmod 777 TASim/installdriver.sh   
sudo chmod 777 install_bridge.sh uninstallbridge.sh Script_TSN.sh

To load multi-queue TSN interface driver and configure 4 TSN interfaces, execute './installdriver.sh'.

To create namespaces and assign interfaces to appropriate namespaces and bridges, and configure iptables and TAPRIO on egress port of output interface,
execute './install_bridge.sh'.

To start UDP generation and capture, execute 'sudo ./Script_TSN.sh
