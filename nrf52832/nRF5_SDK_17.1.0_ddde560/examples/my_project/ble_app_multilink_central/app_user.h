#ifndef  APP_USER_H_
#define  APP_USER_H_
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "nordic_common.h"
#include "myfds.h"
#include "mem_manger.h"
extern uint8_t cfg_info_ack_data;
#define cfg_DH  6
#define  DATAFLOW_RX        1
#define  DATAFLOW_TX        0

#define  DATALEN_j   6
#define  DATAID_j   2
typedef struct{
	uint32_t flag_head;
	uint8_t moudluename[32];
	uint8_t mcu_id[32];
	uint8_t fw_ver[16];
	uint8_t date[16];
	uint8_t time[16];
	uint8_t rsv[16];
	uint32_t crc;
	uint32_t flag_tail;
}_firmware_info_t;
#define _FIRMWARE_INFO_FLAG			(0xa5a5a5a5)


enum{
	initstate = 0,
	uart2pc,
	uart2pc_multi,
	pc2nus,
	nus2pc,
};

#define  QUEUE_SIZE 50 
typedef struct {
    uint8_t data[QUEUE_SIZE][20]; 
    uint32_t head;
    uint32_t tail;
} circular_queue_t;



typedef struct _MessageHeader_{
	uint8_t data_len;
  uint8_t dst;
	uint8_t multi_k;//multi paras
	uint8_t reserved;
	uint16_t conn_hook;
	uint8_t data[16];
}MsgHd_str;
//Currently supports eight types of devices

typedef struct _DMSG_STR_
{
	uint16_t midx;
	uint8_t subidx;
	uint8_t len;
	uint8_t data[8];
}dMsg_str;


extern MsgHd_str gMsgObj;
extern dMsg_str gDMsgObj;
extern MsgHd_str gMsgObjArray[10];
extern uint8_t master_ready;
extern uint8_t ack_frame[4];
extern uint8_t config_cmd[4];
extern uint8_t data_cmd[4];
extern uint8_t nus_app_frame[8][12];
extern circular_queue_t gQueue_nus2pc;

uint8_t cfg_proccess(uint8_t *rawdata,uint16_t dlen,MsgHd_str *FiMsg);
void data_proccess(uint8_t *rawdata,uint16_t dlen,MsgHd_str *FiMsg);
void PORT_transmit(MsgHd_str *udata);
void queue_init(circular_queue_t *queue);
bool queue_is_empty(circular_queue_t *queue) ;
bool queue_is_full(circular_queue_t *queue) ;
bool queue_enqueue(circular_queue_t *queue, const uint8_t *item);
bool queue_dequeue(circular_queue_t *queue, uint8_t *item) ;
#endif  //end of APP_USER_H_