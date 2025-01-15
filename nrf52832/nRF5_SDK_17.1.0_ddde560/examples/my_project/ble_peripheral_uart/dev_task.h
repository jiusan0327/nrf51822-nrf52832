#ifndef DEV_TASK_H_
#define DEV_TASK_H_

#include <stdint.h>
#include <string.h>
#include "ble_nus.h"
#include "pwm.h"
#define DEV_ID   0x01U
#define DEV_TYPE 0x01U



#define EFF_DATA_HEAD1 0x5A
#define EFF_DATA_HEAD2 0x55
#define EFF_CFG_HEAD1  0x55
#define EFF_CFG_HEAD2  0x5A

#define ACK_DATA_HEAD1  0x61//a
#define ACK_DATA_HEAD2  0x64 //d
#define ACK_CFG_HEAD1   0x61 //a
#define ACK_CFG_HEAD2   0x63 //c

#define HEAD_LENS     2
#define RN_LENS       3
#define END_LENS      2
#define EFF_DATA_LENS   20
#define PROCOTOL_LENS  12


#define EFF_FREAM_END1  0x65//e
#define EFF_FREAM_END2  0x64//d
enum{
	cfg_ret = 1,
	data_ret,
	none_ret,
};
enum {
	DC_MTR = 1,

};
enum{
	cfg_sta = 1,
	data_sta,
};

typedef enum{
	OK_ret = 0,
	cfg_rev_err,
}uart_ret;
/* rev struct */
typedef union _DATArev_Un{
	uint8_t buffer[EFF_DATA_LENS];
	struct _data_str_{
		 uint32_t timestamp;
		 uint8_t sync_flag;
		 uint8_t reserved;
		 uint8_t data[PROCOTOL_LENS]; // index subindex datalen(1-8) data(0-7)
		 uint16_t checksum;
	}data_Str;	
}DataRev_t;

typedef struct _rev_data_str_
{
	uint8_t index;
	uint8_t subindex;
	uint8_t datalen;
	uint8_t data[8];
	uint8_t revsered;
}revdata_str;

/* send struct */

typedef struct _ACK_Str_{
	uint8_t head1;
	uint8_t head2;
	uint8_t state;
	uint8_t self_id;
}Ack_str;

uart_ret dev_task(ble_nus_t* nus_obj,const uint8_t* rev_raw,uint8_t rev_len,uint16_t mhandle);
#endif