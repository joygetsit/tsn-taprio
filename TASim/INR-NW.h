/**
*@file
*@brief settings and definitions Netork communication
*@author M.Ulbricht 2015
**/
#include <linux/netdev_features.h>
#include <linux/netdevice.h>

#include <linux/ethtool.h>

#define INR_NWDEV_features NETIF_F_SG | NETIF_F_FRAGLIST
#define INR_NWDEV_features_HW NETIF_F_SG | NETIF_F_FRAGLIST
#define TX_queue_count 8
//no, we dont support TCP-checsum.. NETIF_F_IP_CSUM
/**
*Network packet structure
*/
struct INR_NW_packet
{
    struct INR_NW_packet *next;
    struct net_device *dev;
    int datalen;
    u8 data[ETH_DATA_LEN];
};

#define INR_NW_devcount 4
#define INR_NW_repeatonbusy 0
#define INR_PCI_HW_timestamp 0
#define zerocopy_tx 0
//prototypes
void INR_HW_tx (char *buf, int len);
void INR_NW_rx (struct net_device *nwdev, struct INR_NW_packet *pkt);
int INR_NW_config (struct net_device *nwdev, struct ifmap *map);
int INR_NW_open (struct net_device *nwdev);
int INR_NW_stop (struct net_device *nwdev);
netdev_tx_t INR_NW_tx (struct sk_buff *skb, struct net_device *nwdev);
int INR_NW_header (struct sk_buff *skb, struct net_device *nwdev, unsigned short type, const void *daddr, const void *saddr, unsigned len);
int INR_NW_rebuild_header (struct sk_buff *skb);
int INR_NW_ioctl (struct net_device *nwdev, struct ifreq *rq, int cmd);
struct net_device_stats *INR_NW_stats (struct net_device *nwdev);
int INR_NW_change_mtu (struct net_device *nwdev, int new_mtu);
void INR_NW_tx_timeout (struct net_device *nwdev);
void INR_NW_init (struct net_device *nwdev);
struct net_device *get_nwdev (uint8_t index);
void set_nwdev (uint8_t index, struct net_device *dev);
int INR_NW_set_features (struct net_device *net, netdev_features_t features);
void set_send2cpu (uint8_t flag);
uint8_t get_send2cpu(void);
void INR_NW_zerovars(void);
static int INR_NW_get_ts_info(struct net_device *dev, struct ethtool_ts_info *info);

//*****************************************************************************************************************
/**
 *  @brief net_device definition
 */
static const struct net_device_ops INR_NW_netdev_ops = {
    .ndo_open = INR_NW_open,    /* define start callback */
    .ndo_stop = INR_NW_stop,    /* define stop callback */
    .ndo_set_config = INR_NW_config,    /* define change-config callback */
    .ndo_start_xmit = INR_NW_tx,    /* define tx callback */
    .ndo_get_stats = INR_NW_stats,  /* define statistics callback */
    .ndo_change_mtu = INR_NW_change_mtu,    /* define change_mtu callback */
    .ndo_tx_timeout = INR_NW_tx_timeout,    /* define timeout callback */
    .ndo_set_features = INR_NW_set_features,
    .ndo_do_ioctl = INR_NW_ioctl,
};

static const struct ethtool_ops INR_NW_ethtool_ops = {
    .get_ts_info		= INR_NW_get_ts_info,

};

//*****************************************************************************************************************
/**
 *  @brief network device header
 */
static const struct header_ops INR_NW_header_ops = {
    .create = INR_NW_header,
    .cache = NULL,
};

//*****************************************************************************************************************
/**
*private NW-Interface structure
*/
struct INR_NW_priv
{
    struct net_device *dev;
    struct napi_struct napi;
    struct net_device_stats stats;
    int status;
    struct INR_packet *ppool;
    struct INR_packet *rx_queue;
    int rx_int_enabled;
    int tx_packetlen;
    u8 *tx_packetdata;
    struct sk_buff *skb;
    spinlock_t lock;
    uint8_t port;
};


