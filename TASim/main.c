/**
*@mainpage  TrustNode Ethernetdriver
*@author M.Ulbricht, G.Eschemann
*
*This is the documentation for the TrustNode Ethernet driver. The code is mainly structured in PCI and NetWork communication. You should start browsing the <a href="files.html">filelist</a> first.<br>Questions and bugs please report to <a href="mailto:ulbricht@innoroute.de">ulbricht@innoroute.de</a>
**/
/**
*@file
*@brief main driver Function
*M.Ulbricht 2015
**/
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include "INR.h"
//#include "INR-PCI.h"
#include "INR-NW.h"
//#include "INR-ctl.h"
volatile uint8_t probed=0;
static void remove (void);

/**
*@brief PCI-Device definition
*
*/

struct net_device *INR_NW;

/**
*@brief init nw-device
*
*/
int
INR_NWDEV_init (void)
{
    uint8_t i = 0;
    for (i = 0; i < INR_NW_devcount; i++) {
        //INR_NW = alloc_netdev (sizeof (struct INR_NW_priv), "TN%d", NET_NAME_UNKNOWN, INR_NW_init);
        INR_NW = alloc_netdev_mq (sizeof (struct INR_NW_priv), "TN%d", NET_NAME_UNKNOWN, INR_NW_init,TX_queue_count);
        if (INR_NW == NULL) {
            INR_LOG_debug (loglevel_err"Cant alloc NWDEV %i !\n", i);
            return 1;
        } else {

            if (0 == register_netdev (INR_NW))
                INR_LOG_debug (loglevel_info"NWDev %i registerd, flags:%llx ", i, INR_NW->hw_features);
            INR_LOG_debug (loglevel_info"flags2:%llx\n", INR_NW->hw_features);
            set_nwdev (i, INR_NW);
        }
    }
    return 0;
}

//*****************************************************************************************************************
/**
*destroy nw-device
*/
void
INR_NWDEV_destroi (void)
{
    uint8_t i = 0;
    for (i = 0; i < INR_NW_devcount; i++) {
        INR_LOG_debug (loglevel_info"destroy INRNWDEV %i\n",i);
        unregister_netdev (get_nwdev(i));
        free_netdev (get_nwdev(i));
    }
}

//*****************************************************************************************************************
/**
*init driver function
*/
static int
probe(void)
{
    printk (loglevel_info "moep!\n"); //this is one of the verry important things needed to run the driver :D
    if (probed) {
        INR_LOG_debug (loglevel_err"driver already loaded... exit\n");
        return -1;
    }
    probed++;   //prevent driver from loaded twice
    INR_NW_zerovars();  //reset all global vars
    //INR_PCI_zerovars();
    //INR_zerovars();
    int result;
    //INR_LOG_timelog_init ();	//safe timestamp
    INR_LOG_debug (loglevel_info"Start load module\n");
    /*if ((result = pci_enable_device (dev)) < 0) {
        INR_LOG_debug (loglevel_err"device enable fail...\n");
        return result;
    } else {
        INR_LOG_debug (loglevel_info"device enabled\n");
        INR_STATUS_set (INR_STATUS_DEVICE);
    }*/
    if (0 == INR_NWDEV_init ()) {
        INR_STATUS_set (INR_STATUS_NW_enabled);
    }
    //INR_init_drv (dev);		//INIT pci and network
    //INR_CTL_init_proc (dev);	//init proc fs
    //INR_STATUS_set (INR_STATUS_DRV_INIT_done);
    return 0;
}

//*****************************************************************************************************************
/*static struct pci_driver pci_driver = {
    .name = "INRTrustnode",
    .id_table = ids,
    .probe = probe,
    .remove = remove,
};*/

//*****************************************************************************************************************
/**
*driver-end function, called by kernel
*@param *dev PCI-device
*/
static void
remove (void)
{
    INR_NWDEV_destroi();
    //INR_CTL_remove_proc (dev);
    INR_LOG_debug (loglevel_info"remove Module\n");
    INR_LOG_debug (loglevel_info"Reset Logic\n");
    //INR_remove_drv (dev);
    if(probed) {
        probed--;
    }
}

//*****************************************************************************************************************
static int __init
pci_skel_init (void)
{
    //return pci_register_driver (&pci_driver);
    probe();
    return 0;
}

//*****************************************************************************************************************
static void __exit
pci_skel_exit (void)
{
    //pci_unregister_driver (&pci_driver);
    remove();
}

//*****************************************************************************************************************
module_init (pci_skel_init);
module_exit (pci_skel_exit);
MODULE_LICENSE ("Dual BSD/GPL");
MODULE_AUTHOR ("M.Ulbricht");
MODULE_VERSION ("1.0");
MODULE_DESCRIPTION ("InnoRoute GTP dummy driver");
//MODULE_DEVICE_TABLE (pci, ids);
