#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>

MODULE_DESCRIPTION("My kernel module");
MODULE_AUTHOR("Me");
MODULE_LICENSE("GPL");

struct net_device *TSNNodes;
int INR_NW_devcount = 5; //Number of interfaces

static int device_configuration(void){
        int i = 0;	
        printk(KERN_INFO "Starting network module make and configuration\n");
        for (i  = 0; i < INR_NW_devcount; i++){
                TSNNodes = alloc_netdev_mqs(sizeof(struct INR_NW_priv),"TSN_Interface%d", NET_NAME_UNKNOWN, INR_NW_init, 9, 9);
                if (TSNNodes == NULL){
                        print("Can't allocate TSN interfaces\n");
                }
                else{
                        if (0 == register_netdev(TSNNodes)){
                                print("Allocated TSN interfaces %d\n");
                        }

                }
        return 0;
}

void device_cleanup(void){
        int i = 0;
        for (i = 0; i < INR_NW_devcount; i++) {
                unregister_netdev (get_nwdev(i));
                free_netdev (get_nwdev(i));
        }        
}

/* Module Init and Exit functions */
static int dummy_init(void){
        // This writes to dmesg | tail -5 only when makefile flag has -DDEBUG
        pr_debug("Hi\n"); 
        // This writes to dmesg and kernel log, you can use this or above
        printk(KERN_INFO "My network module is being made and configured\n");
        device_configuration();
        return 0;
}

static void dummy_exit(void){
        pr_debug("Bye\n"); //This doesn't work
        printk(KERN_INFO "Hey, this module has been UNloaded\n"); 
        //This works, open 'tail -f /var/log/kern.log'
        device_cleanup();
        return;
}

module_init(dummy_init);
module_exit(dummy_exit);
