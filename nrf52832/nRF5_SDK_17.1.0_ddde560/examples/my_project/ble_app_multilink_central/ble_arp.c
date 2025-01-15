#include "ble_arp.h"
//connect handle combine with subdevice 's id and type
ble_dev_info_t arp_table[MAX_DEVICES] = {0};
void arp_tab_init(uint8_t flag)
{
	if(flag == 1)
	{
		for(int i=0;i<gParaCfgObj.slave_num;i++)
		{
			arp_table[i].id = gParaCfgObj.slave_id[i];
			arp_table[i].type = gParaCfgObj.slave_type[i];
		}
		return;
	}else{
		return;
	}
}




//void on_connected_arp(ble_evt_t const * p_ble_evt) {
//    if (device_count < MAX_DEVICES) {
//        arp_table[device_count].conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
//        memCpyUser(arp_table[device_count].peer_addr, p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr, BLE_GAP_ADDR_LEN);
//        device_count++;
//    }
//}

//int my_memcmp(const void *ptr1, const void *ptr2, size_t n) {
//    const unsigned char *p1 = ptr1;
//    const unsigned char *p2 = ptr2;

//    for (size_t i = 0; i < n; i++) {
//        if (p1[i] != p2[i]) {
//            return p1[i] - p2[i];
//        }
//    }
//    return 0;
//}


//uint32_t send_data_to_device(ble_nus_c_t * p_ble_nus_c,uint8_t *target_addr, uint8_t *data, size_t data_len) {
//    for (uint8_t i = 0; i < device_count; i++) {
//        if (my_memcmp(arp_table[i].peer_addr, target_addr, BLE_GAP_ADDR_LEN) == 0) {
//						p_ble_nus_c->conn_handle = arp_table[i].conn_handle;
//            return ble_nus_c_string_send(p_ble_nus_c, data, data_len);
//        }
//    }
//    return NRF_ERROR_NOT_FOUND;  
//}


//static void on_disconnected(ble_evt_t const * p_ble_evt) {
//    for (uint8_t i = 0; i < device_count; i++) {
//        if (arp_table[i].conn_handle == p_ble_evt->evt.gap_evt.conn_handle) {
//            my_memmove(&arp_table[i], &arp_table[i + 1], (device_count - i - 1) * sizeof(ble_device_info_t));
//            device_count--;
//            break;
//        }
//    }
//}
