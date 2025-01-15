#ifndef  BLE_ARP_H_
#define BLE_ARP_H_
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "mem_manger.h"
#include "nordic_common.h"
#include "ble_gap.h"
#include "ble.h"
#include "ble_nus_c.h"
#include "myfds.h"
typedef struct {
	uint16_t conn_handle;  // like a hook
	uint8_t id;
	uint8_t type;
} ble_dev_info_t;

#define MAX_DEVICES 8
extern ble_dev_info_t arp_table[MAX_DEVICES];


void arp_tab_init(uint8_t flag);
#endif