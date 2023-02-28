/**
*@file
*@brief Functions for the Network-stack
*@author M.Ulbricht 2015/Joydeep 2021
**/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/if_vlan.h>
#include <linux/netdev_features.h>
#include <linux/etherdevice.h>
#include <linux/dma-mapping.h>
#include <linux/skbuff.h>
#include <linux/net_tstamp.h>
#include <linux/ethtool.h>
#include <linux/ptp_clock_kernel.h>
#include "INR-NW.h"
#include "INR.h"
//#include "INR-PCI.h"
//#include "INR-TIME.h"
volatile uint8_t nwdev_counter = 0;
volatile uint8_t send2cpu = 0;
struct net_device *globnwdev[INR_NW_devcount];
struct hwtstamp_config INR_tstamp_config;

/**
*return pointer to net-dev
*@param index index of NW-device
*/
struct net_device *
get_nwdev (uint8_t index)
{
    return globnwdev[index];
}

/**
*reset all global variables
*/
void
INR_NW_zerovars()
{
    nwdev_counter = 0;
    send2cpu = 0;
}

/**
*store net-dev point for later use
*@param index index of NW-device
*@param dev NW-dev
*/
void
set_nwdev (uint8_t index, struct net_device *dev)
{
    globnwdev[index] = dev;
    INR_NW_STATUS_set (1 << index);
}

/**
*set send2cpu-flag
*@param flag
*/
void
set_send2cpu (uint8_t flag)
{
    send2cpu = flag;
}

/**
*get send2cpu-flag
*/
uint8_t
get_send2cpu ()
{
    return send2cpu;
}

/**
*Packet Reception (NW-rx)
*@param nwdev
*@param pkt Ethernetpacket
*/
void INR_NW_rx (struct net_device *nwdev, struct INR_NW_packet *pkt){
    
    struct sk_buff *skb;
    struct INR_NW_priv *priv = netdev_priv(nwdev);
        
    INR_LOG_debug (loglevel_info"rx called\n");

    skb = dev_alloc_skb(pkt->datalen + 2);
    if (!skb) {
        priv->stats.rx_dropped++;
        return;
    }

    skb_reserve (skb, 2);

    memcpy(skb_put(skb, pkt->datalen), pkt->data, pkt->datalen);
    skb->dev = nwdev;
    skb->protocol = eth_type_trans(skb, nwdev);
    skb->ip_summed = CHECKSUM_UNNECESSARY;
    priv->stats.rx_packets++;
    priv->stats.rx_bytes += pkt->datalen;
    netif_rx(skb);
    return;
}

//*****************************************************************************************************************
/**
*NW-config
*@param nwdev network device
*@param config change-map
*/
int
INR_NW_config (struct net_device *nwdev, struct ifmap *map)
{
    INR_LOG_debug (loglevel_info"NW-config called\n");
    return 0;
}

//*****************************************************************************************************************
/**
*NW-dev open
*@param nwdev network device
*/
int
INR_NW_open (struct net_device *nwdev)
{

    struct INR_NW_priv *priv = netdev_priv (nwdev);
    uint8_t i = 0;
    struct netdev_queue *queue;    
    
    INR_LOG_debug (loglevel_info "NWDev open\n");
    memcpy (nwdev->dev_addr, "\0SNUL1", ETH_ALEN);
    memcpy (nwdev->broadcast, "\0\0\0\0\0\0", ETH_ALEN);

    // nwdev->dev_addr[ETH_ALEN - 1] = priv->port;
    //nwdev->num_tx_queues=2;
    for (i = 0; i < ETH_ALEN; i++) {
        nwdev->broadcast[i] = (uint8_t) 0xff;
    }
    INR_LOG_debug (loglevel_info"HW-addr:%x Broadcast-addr:%x\n", nwdev->dev_addr, nwdev->broadcast);
    //netif_start_queue (nwdev);

    for (i=0; i<TX_queue_count; i++) {
        queue= netdev_get_tx_queue(nwdev,
                                   i);

        netif_tx_start_queue(queue);
    }

    return 0;
}

//*****************************************************************************************************************
/**
*NW-dev stop
*@param nwdev network device
*/
int
INR_NW_stop (struct net_device *nwdev)
{
    INR_LOG_debug (loglevel_info"NWDev stop\n");
    netif_stop_queue (nwdev);
    return 0;
}

//*****************************************************************************************************************
/**
*Software transmit function, called by kernel
*@param skb socket buffer
*@param nwdev network device
*/
netdev_tx_t
INR_NW_tx (struct sk_buff *skb, struct net_device *nwdev)
{
    if (1) {
        struct INR_NW_priv *priv = netdev_priv (nwdev);
        uint8_t toport = priv->port;
        uint8_t error = 0;
        unsigned int from;
        unsigned int to = skb->len;
        struct skb_seq_state st;
        const u8 *data;
        unsigned int len;
        unsigned int consumed = 0;
        uint8_t to_port=0;
        uint8_t *skb_data;
        
        //printk("port %i\n",priv->port);
        if (priv->port==0) to_port=1;
        if (priv->port==1) to_port=0;
        if (priv->port==2) to_port=3;
        if (priv->port==3) to_port=2;

        // if(zerocopy_tx)skb_shinfo(skb)->tx_flags |= SKBTX_DEV_ZEROCOPY; //maybe this fix the memory drain
        //######Timestamping
        if (skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP) { //check if timestamp is requested
            if(INR_PCI_HW_timestamp) { //hw timestamping
                skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS; //announce HW will do timestamping
            } else { //no hw timestamping
                skb_tx_timestamp(skb);
            }
        }
        if (skb_shinfo(skb)->tx_flags & SKBTX_SW_TSTAMP)skb_tx_timestamp(skb);

        //#################
        skb->dev = get_nwdev (to_port);

        skb_data = kmemdup (skb->data, skb->len, GFP_DMA);

        priv->stats.tx_packets++;
        priv->stats.tx_bytes += skb->len;
#define insert_data_length 90
#define insert_data_pos 14 //behind mac


        if (to_port==0) {

            struct sk_buff *new_skb=netdev_alloc_skb(skb->dev,skb->len+2);
            skb_reserve(new_skb,2);
            skb_put(new_skb,skb->len);
            skb_store_bits(new_skb,0,skb_data,skb->len);//copy head
            new_skb->ip_summed = CHECKSUM_UNNECESSARY;	//set checksumflag in skb
            new_skb->protocol = eth_type_trans (new_skb, new_skb->dev);	//set ethertype in skb

            netif_receive_skb(new_skb);
        }
        if (to_port==1) {




            struct sk_buff *new_skb2=netdev_alloc_skb(skb->dev,skb->len+2);
            skb_reserve(new_skb2,2);
            skb_put(new_skb2,skb->len);
            skb_store_bits(new_skb2,0,skb_data,skb->len);//copy head
            new_skb2->ip_summed = CHECKSUM_UNNECESSARY;	//set checksumflag in skb
            new_skb2->protocol = eth_type_trans (new_skb2, new_skb2->dev);	//set ethertype in skb

            netif_receive_skb(new_skb2);
        }
        if (to_port==2) {




            struct sk_buff *new_skb3=netdev_alloc_skb(skb->dev,skb->len+2);
            skb_reserve(new_skb3,2);
            skb_put(new_skb3,skb->len);
            skb_store_bits(new_skb3,0,skb_data,skb->len);//copy head
            new_skb3->ip_summed = CHECKSUM_UNNECESSARY;	//set checksumflag in skb
            new_skb3->protocol = eth_type_trans (new_skb3, new_skb3->dev);	//set ethertype in skb

            netif_receive_skb(new_skb3);
        }
        if (to_port==3) {




            struct sk_buff *new_skb4=netdev_alloc_skb(skb->dev,skb->len+2);
            skb_reserve(new_skb4,2);
            skb_put(new_skb4,skb->len);
            skb_store_bits(new_skb4,0,skb_data,skb->len);//copy head
            new_skb4->ip_summed = CHECKSUM_UNNECESSARY;	//set checksumflag in skb
            new_skb4->protocol = eth_type_trans (new_skb4, new_skb4->dev);	//set ethertype in skb

            netif_receive_skb(new_skb4);
        }
        kfree_skb (skb);
        kfree(skb_data);

errorhandling:
        if (error) {
            priv->stats.tx_dropped++;   //fpga descriptorrin full pkt dropped
            if (INR_NW_repeatonbusy) {
                return NETDEV_TX_BUSY;  //dont drop, just tell nw stack to repeat
            } else {
                if (skb)
                    kfree_skb (skb);    //free skb if dropped (handled by nw_stack if returned busy)
                return NETDEV_TX_OK;
            }
        }
        return NETDEV_TX_OK;
    }
    return NETDEV_TX_BUSY;
}

//*****************************************************************************************************************
/**
*change device options
*@param nwdev network device
*@param rq
*@param cmd
*/
int
INR_NW_ioctl (struct net_device *nwdev, struct ifreq *rq, int cmd)
{

    struct INR_NW_priv *priv = netdev_priv(nwdev);
    struct hwtstamp_config config;
    INR_LOG_debug (loglevel_info"INR_NW_ioctl called\n");

    if (copy_from_user(&config, rq->ifr_data, sizeof(config)))
        return -EFAULT;
    if (config.flags)
        return -EINVAL;

    switch (cmd) {
    case SOF_TIMESTAMPING_TX_SOFTWARE:
        break;
    case SIOCGHWTSTAMP:
        copy_to_user(rq->ifr_data, &INR_tstamp_config,sizeof(INR_tstamp_config));
        break;
    case SIOCSHWTSTAMP:
        switch (config.tx_type) {
        case HWTSTAMP_TX_OFF:
            INR_tstamp_config.tx_type=config.tx_type;
            break;

        case HWTSTAMP_TX_ON:
            if(INR_PCI_HW_timestamp==0) {
                return -EOPNOTSUPP;
            }
            else {
                INR_tstamp_config.tx_type=config.tx_type;
            }
            break;

        default:
            return -ERANGE;
        }
        switch (config.rx_filter) {
        case HWTSTAMP_FILTER_NONE:
            INR_tstamp_config.rx_filter=config.rx_filter;
            break;

        case HWTSTAMP_FILTER_ALL:
            if(INR_PCI_HW_timestamp==0) {
                return -EOPNOTSUPP;
            }
            else {
                INR_tstamp_config.rx_filter=config.rx_filter;
            }
            break;
        default:
            return -ERANGE;
        }

        memcpy(&INR_tstamp_config, &config, sizeof(config));
        break;
    default:
        return -EOPNOTSUPP;
    }
    return 0;
}

//*****************************************************************************************************************
/**
*return device statistics
*@param nwdev network device
*/
struct net_device_stats *
INR_NW_stats (struct net_device *nwdev)
{
    struct INR_NW_priv *priv = netdev_priv (nwdev);
    return &priv->stats;
}

//*****************************************************************************************************************
/**
*change mtu
*@param cahnge mtu
*@param nwdev network device
*/
int
INR_NW_change_mtu (struct net_device *nwdev, int new_mtu)
{
    INR_LOG_debug (loglevel_info"change_mtu called\n");
    return 0;
}

//*****************************************************************************************************************
/**
*tx-timeut function
*@param nwdev network device
*/
void
INR_NW_tx_timeout (struct net_device *nwdev)
{
    INR_LOG_debug (loglevel_info"tx_timeour called\n");
    return;
}


//*****************************************************************************************************************
/**
*init nw-device structure
*@param nwdev network device
*/
//*****************************************************************************************************************
void
INR_NW_init (struct net_device *nwdev)
{
    struct INR_NW_priv *priv = netdev_priv (nwdev);
    memset (priv, 0, sizeof (struct INR_NW_priv));
    spin_lock_init (&priv->lock);
    priv->dev = nwdev;
    priv->port = nwdev_counter++;
    nwdev->netdev_ops = &INR_NW_netdev_ops;
    //nwdev->num_tx_queues=2;
    nwdev->features |= INR_NWDEV_features;
    nwdev->hw_features |= INR_NWDEV_features_HW;
    nwdev->ethtool_ops = &INR_NW_ethtool_ops;
    //SET_ETHTOOL_OPS(nwdev, &INR_NW_ethtool_ops);
    ether_setup (nwdev);
    INR_LOG_debug (loglevel_info"Init NWDev %i done\n", priv->port);
}

//*****************************************************************************************************************
/**
*ETHtool callback function for timstamping and ptp parameters
*@param *dev network device
*@param *info info request
*/
//*****************************************************************************************************************
static int INR_NW_get_ts_info(struct net_device *nwdev, struct ethtool_ts_info *info)
{   
    struct INR_NW_priv *priv = netdev_priv(nwdev);
    INR_LOG_debug (loglevel_info"INR_NW_get_ts_info called\n");

    info->so_timestamping =
        SOF_TIMESTAMPING_TX_SOFTWARE |
        SOF_TIMESTAMPING_RX_SOFTWARE |
        SOF_TIMESTAMPING_SOFTWARE;// |
    if(INR_PCI_HW_timestamp) {
        info->so_timestamping|=
            SOF_TIMESTAMPING_TX_HARDWARE |
            SOF_TIMESTAMPING_RX_HARDWARE |
            SOF_TIMESTAMPING_RAW_HARDWARE;
    }

    if(INR_PCI_HW_timestamp) {
        info->phc_index = -1;
    } else {
        info->phc_index = -1;
    }

    info->tx_types = (1 << HWTSTAMP_TX_OFF);
    if(INR_PCI_HW_timestamp)info->tx_types |=(1 << HWTSTAMP_TX_ON);

    info->rx_filters =(1 << HWTSTAMP_FILTER_NONE);
    if(INR_PCI_HW_timestamp)info->rx_filters |=(1 << HWTSTAMP_FILTER_ALL);

    return 0;
}

int
INR_NW_set_features (struct net_device *net, netdev_features_t features)
{
    u8 tmp;
    struct usbnet *dev = netdev_priv (net);
    netdev_features_t changed = net->features ^ features;

    if (changed & NETIF_F_TSO) {
        net->features ^= NETIF_F_TSO;
    }
    if (changed & NETIF_F_SG) {
        net->features ^= NETIF_F_SG;
    }
    return 0;
}

static int igb_setup_tc(struct net_device *dev, enum tc_setup_type type,void *type_data)
{

    struct igb_adapter *adapter = netdev_priv(dev);
   
    INR_LOG_debug ("info: %s:%d %s ", __FILE__, __LINE__, __FUNCTION__);
    return 0;
    /*	switch (type) {
    	case TC_SETUP_QDISC_CBS:
    		return igb_offload_cbs(adapter, type_data);
    	case TC_SETUP_BLOCK:
    		return flow_block_cb_setup_simple(type_data,
    						  &igb_block_cb_list,
    						  igb_setup_tc_block_cb,
    						  adapter, adapter, true);

    	case TC_SETUP_QDISC_ETF:
    		return igb_offload_txtime(adapter, type_data);

    	default:
    		return -EOPNOTSUPP;
    	}*/
}
