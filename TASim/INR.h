/**
*@file
*@brief basic settings and definitions
*@author M.Ulbricht 2015
**/
//void INR_LOG_debug (const char *strg, ...);
void INR_LOG_timelog (const char *strg, ...);
void INR_LOG_timelog_init (void);
void INR_STATUS_set (uint64_t stat);
void INR_CHECK_fpga_read_val (uint64_t val, const char *msg, uint8_t bit64);
uint8_t INR_STATUS_get (uint64_t stat);
void INR_NW_STATUS_set (uint64_t stat);
uint8_t INR_NW_STATUS_get (uint64_t stat);
void INR_zerovars(void);
#define REGISTER_SIZE 30000	//FPGA has 4 32 Bit registers
#define INR_STATUS_BUSMASTER 1
#define INR_STATUS_BAR0 2
#define INR_STATUS_TX_RING 4
#define INR_STATUS_MSI 8
#define INR_STATUS_INT1 16
#define INR_STATUS_DEVICE 32
#define INR_STATUS_NW_enabled 64
#define INR_STATUS_RX_RING 128
#define INR_STATUS_HW_RUNNING 256
#define INR_STATUS_DRV_INIT_done 512
#define INR_STATUS_BAR1 1024

#define MSI0_IRQ_STATUS 0x104
#define IRQ_EOI 0x50
#define loglevel_err KERN_ERR
#define loglevel_warn KERN_WARNING
#define loglevel_info KERN_INFO
#define INR_LOG_debug printk
