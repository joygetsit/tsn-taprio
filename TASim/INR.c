/**
*@file INR.c
*@brief Global helper functions
*@author M.Ulbricht 2015
**/
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/time64.h>
#include <linux/ktime.h>
#include "INR.h"
const uint8_t debug_enable = 1;
const uint8_t fpga_read_check = 1;
const uint8_t timelog_enable = 1;
struct timespec64 INR_LOG_lasttime;
volatile uint64_t INR_status = 0;
volatile uint64_t INR_NW_status = 0;

/**
*zero all global vars
*/
void INR_zerovars() {
    INR_status = 0;
    INR_NW_status = 0;
}

/**
*error if 0xffffffffffffffff is readed from fpga
*@param val value to check
*/
void
INR_CHECK_fpga_read_val (uint64_t val, const char *msg, uint8_t bit64)
{
    if (fpga_read_check) {
        if (bit64) {
            if (val == 0xffffffffffffffff) {
                INR_LOG_debug (loglevel_err"FPGA read check faild: %s", msg);
            } else if (val == 0xffffffff) {
                INR_LOG_debug (loglevel_err"FPGA read check faild: %s", msg);
            }
        }
    }
}

//*****************************************************************************************************************
/**
*NW status management
*@param stat Status to set
*/
void
INR_NW_STATUS_set (uint64_t stat)
{
    INR_NW_status |= stat;
}

//*****************************************************************************************************************
/**
*NW status management
*@param stat Status to set
*/
uint8_t
INR_NW_STATUS_get (uint64_t stat)
{
    if (INR_NW_status & stat) {
        return 1;
    } else {
        return 0;
    }
}


//*****************************************************************************************************************
/**
*status management
*@param stat Status to set
*/
void
INR_STATUS_set (uint64_t stat)
{
    INR_status |= stat;
}

//*****************************************************************************************************************
/**
*status management
*@param stat Status to set
*/
uint8_t
INR_STATUS_get (uint64_t stat)
{
    if (INR_status & stat) {
        return 1;
    } else {
        return 0;
    }
}

void
INR_LOG_timelog_init ()
{
    ktime_get_ts64(&INR_LOG_lasttime);
}

//*****************************************************************************************************************
/**
*central debug print
*@param strg message
*/
/*void
INR_LOG_debug (const char *strg, ...)
{
    va_list format;
    va_start (format, strg);
    if (debug_enable) {
        printk (KERN_WARNING "INR:");
        vprintk (KERN_WARNING strg, format);
    }
}
*
/*****************************************************************************************************************/
/*
*central timelog print
*@param strg message
*/
void
INR_LOG_timelog (const char *strg, ...)
{
    va_list format;
    va_start (format, strg);
    if (timelog_enable) {
        struct timespec64 tmp;
        ktime_get_ts64(&tmp);
        printk ("INR_Time[%lds%ldns]Diff:[%lds,%ldns]:", tmp.tv_sec, tmp.tv_nsec, tmp.tv_sec - INR_LOG_lasttime.tv_sec, tmp.tv_nsec - INR_LOG_lasttime.tv_nsec);
        INR_LOG_lasttime = tmp;
        vprintk (strg, format);
    }
}
