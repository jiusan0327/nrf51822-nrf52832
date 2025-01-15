#include "app_user.h"
//<!
uint8_t ack_frame[4] = {0x61,0x63,0x65,0x64};
uint8_t config_cmd[4] = {0x5A,0x55,0xFE,0xFF};
uint8_t data_cmd[4] = {0x55,0x5A,0xFE,0xFF};
uint8_t cfg_info_ack_data = 0;
uint8_t master_ready = 0;

uint16_t simple_checksum(const uint8_t *data, size_t length) {
    uint16_t checksum = 0;
    for (size_t i = 0; i < length; i++) {
        checksum += data[i];
    }
    return checksum;
}


void cfg_packet_pack(MsgHd_str *FiMsg,uint8_t *data ,uint8_t len)
{
	memCpyUser(FiMsg->data,ack_frame,2);//head
	memCpyUser(FiMsg->data+2,data,len);
	memCpyUser(FiMsg->data+len+2,ack_frame+2,2);//tail
	FiMsg->data_len = 2+len+2;
}
MsgHd_str gMsgObjArray[10] = {0};
//5a 55 data checksum fe ff \r
uint8_t cfg_proccess(uint8_t *rawdata,uint16_t dlen,MsgHd_str *FiMsg)
{
	FiMsg->dst = initstate;
	FiMsg->data_len = 0;		
	
	if(dlen==5) 
	{
		if(cfg_info_ack_data == GOOD_CFG_STATE)//check params cfg
		{
			FiMsg->dst = uart2pc_multi;		
			FiMsg->multi_k = gParaCfgObj.slave_num+2;		
			cfg_packet_pack(&gMsgObjArray[0],&gParaCfgObj.slave_num,1);//0:1
			for(int i=0;i<gParaCfgObj.slave_num;i++)//1-8:8  2
			{	
				uint8_t dtemp[2] = {0};
				dtemp[0] = gParaCfgObj.slave_id[i];
				dtemp[1] = gParaCfgObj.slave_type[i];
				cfg_packet_pack(&gMsgObjArray[i+1],dtemp,2);
			}
			cfg_packet_pack(&gMsgObjArray[FiMsg->multi_k-1],gParaCfgObj.crcret,2);//1
			master_ready = 1;
		}else{
			FiMsg->dst = uart2pc;
			FiMsg->data_len = 5;
			FiMsg->data[0] = cfg_info_ack_data;
			memCpyUser(FiMsg->data+1,ack_frame,4);
		}
		
	}else if((dlen==7)&&(rawdata[2] == 0)&&(rawdata[3] == 0)){//save cfg paras
		gParaCfgObj.update_flag = 1;
		uint16_t crcnew = crc16(&gParaCfgObj.slave_num,CRC16_CFG_PARAS_LEN);
		gParaCfgObj.crcret[0] = crcnew >> 8;
		gParaCfgObj.crcret[1] = crcnew;
		ready_to_write = 1;
	}else{//check crc
		if(simple_checksum(&rawdata[2],dlen-6)!=rawdata[dlen-4])
		{
			FiMsg->dst = uart2pc;
			FiMsg->data_len = 5;
			FiMsg->data[0] = ERR_CFG_CRC;//crc error
			memCpyUser(FiMsg->data+1,ack_frame,FiMsg->data_len);
			return 0;
		}
	}
	
	uint16_t mainidx = (rawdata[2]<<8)|(rawdata[3]);
	uint8_t subidx = rawdata[4];
	uint8_t pkgdatalen = rawdata[5];
	switch (mainidx)
  {
  	case 0x1001:
			if(subidx == 1)
			{
				if(pkgdatalen == 1)
				{
					gParaCfgObj.slave_num = rawdata[cfg_DH];
				}					
			}else if(subidx == 2)
			{
				if(pkgdatalen == 3)
				{
					uint8_t devcnum = rawdata[cfg_DH];
					gParaCfgObj.slave_id[devcnum-1] = rawdata[cfg_DH+1];
					gParaCfgObj.slave_type[devcnum-1] = rawdata[cfg_DH+2];
				}
			}
  		break;
  	default:
  		break;
  }
}

	

static uint8_t crc_err_frame[4] = {0xfe,0xfe,0x00,0x02};

//dev_id   data_12byte    checksum_2byte
void data_proccess(uint8_t *rawdata,uint16_t dlen,MsgHd_str *FiMsg)
{
	if(simple_checksum(&rawdata[2],dlen-6)!=rawdata[dlen-4])
	{
		FiMsg->dst = uart2pc;
		FiMsg->data_len = 4;
		memCpyUser(FiMsg->data,crc_err_frame,FiMsg->data_len);
		return;
	}
	FiMsg->dst = pc2nus;
	FiMsg->data_len = dlen;
	FiMsg->conn_hook = rawdata[2];//device id
	memCpyUser(FiMsg->data,rawdata,dlen);
}


void cold_reboot(void)
{
	 NVIC_SystemReset();
}


//queue function

void queue_init(circular_queue_t *queue) {
    memset(queue->data, 0, sizeof(queue->data));
    queue->head = 0;
    queue->tail = 0;
}
bool queue_is_empty(circular_queue_t *queue) {
    return (queue->head == queue->tail);
}

bool queue_is_full(circular_queue_t *queue) {
    return ((queue->head + 1) % QUEUE_SIZE == queue->tail);
}

bool queue_enqueue(circular_queue_t *queue, const uint8_t *item) {
    if (queue_is_full(queue)) {
        return false; 
    }
    memcpy(queue->data[queue->head], item, 20);
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    return true;
}

bool queue_dequeue(circular_queue_t *queue, uint8_t *item) {
    if (queue_is_empty(queue)) {
        return false; 
    }
    memcpy(item, queue->data[queue->tail], 20);
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;
    return true;
}