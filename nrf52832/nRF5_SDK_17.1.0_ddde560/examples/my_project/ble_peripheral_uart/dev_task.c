#include "dev_task.h"

static uint16_t device_type = 0;
static DataRev_t dev_revbox = {0};
static revdata_str databox = {0};



void dev_comm_init(void)
{
	
}


uint8_t dev_frame_judge(const uint8_t *rev)
{
	uint8_t ret_temp = none_ret;
	if(rev[0] == EFF_DATA_HEAD1 && rev[1] == EFF_DATA_HEAD2)
	{
		
		return data_ret;
	}else if(rev[0] == EFF_CFG_HEAD1 && rev[1] == EFF_CFG_HEAD2)
	{
		return cfg_ret;
	}else
	{
		return none_ret;
	}
}

inline uint8_t is_data_f(uint8_t lens,const uint8_t *rev)
{
	uint8_t ret1,ret2,ret3;
	ret1 = (lens == 25 )? 1:0;//EFF_DATA_LENS+HEAD_LENS+RN_LENS
	ret2 = (rev[lens-3]==0xFE)?1:0;
	ret3 = (rev[lens-2]==0xFF)?1:0;
	if(ret1&ret2&ret3 == 1)
	{
		return 1;
	}else{
		return 0;
	}
}

void data_stright(uint8_t* buffer)
{
	memcpy(&databox,buffer,12);
	if(DEV_TYPE == DC_MTR)
	{
		if(databox.index == 1 && databox.subindex ==1)
		{
			DCM_Ctl_Fro_BLE(&databox.data[0],&databox.data[1]);//byte0 en; byte1:dir
		}
	}
}




//rev and ack
uart_ret dev_task(ble_nus_t* nus_obj,const uint8_t* rev_raw,uint8_t rev_len,uint16_t mhandle)
{
	uint8_t f_ret;
	f_ret = dev_frame_judge(rev_raw);
	uint8_t *ack_ary;
  static uint16_t acklens = 6;
	static uint8_t send_flag = 0;
	switch(f_ret){
		case cfg_ret:
			if((rev_raw[rev_len-3]==0xFE)&&(rev_raw[rev_len-2]==0xFF))
			{
				acklens = 6;
				ack_ary[0] = ACK_CFG_HEAD1;
				ack_ary[1] = ACK_CFG_HEAD2;
				ack_ary[2] = cfg_sta+0x30; //ask data rev
				ack_ary[3] = DEV_ID+0x30;
				ack_ary[4] = EFF_FREAM_END1;
				ack_ary[5] = EFF_FREAM_END2;
				ble_nus_data_send(nus_obj,ack_ary,&acklens,mhandle);
					
			}else{
				return cfg_rev_err;
			}
			break;
		case data_ret:
			memcpy(&dev_revbox.buffer,rev_raw+2,sizeof(DataRev_t)+2);	
			if(is_data_f(rev_len,rev_raw))
			{
				data_stright(&dev_revbox.data_Str.data[0]);
				ack_ary[0] = ACK_DATA_HEAD1;
				ack_ary[1] = ACK_DATA_HEAD2;
				ack_ary[2] = data_sta + 0x30; //ask data rev
				ack_ary[3] = DEV_ID +0x30;
				ack_ary[4] = EFF_FREAM_END1;
				ack_ary[5] = EFF_FREAM_END2;
				ble_nus_data_send(nus_obj,ack_ary,&acklens,mhandle);
			}
		  break;
		default:
			break;
	}
	//copy the data right


	
	
}


//dev_con



