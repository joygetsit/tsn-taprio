# TSN
Programs and scripts to test TSN on Linux

To load multi-queue TSN interface driver and configure 4 TSN interfaces, execute './installdriver.sh'.

To create namespaces and assign interfaces to appropriate namespaces and bridges, and configure iptables and TAPRIO on egress port of output interface,
execute './install_bridge.sh'.

To start UDP generation and capture, execute 'sudo ./Script_TSN.sh'.
