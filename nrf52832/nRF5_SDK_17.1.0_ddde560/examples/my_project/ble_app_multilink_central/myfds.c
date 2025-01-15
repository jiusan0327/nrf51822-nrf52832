#include "myfds.h"
#include "mem_manger.h"


para_info_str gParaCfgObj = {0};
uint8_t ready_to_write = 0;
cfg_tab_str cfg_Tab[10] = {
	{1,1,1},//a flag of configuration information update  [1]
	{1,2,1},//num of slave                                [1]
	{1,3,8},//slave id [8]
	{1,4,8},//slave type [8]
	{1,5,2},//crc,check infomation ok                      [4]
};
void Fsd_CfgPara_Get(void)
{
	load_data(cfg_Tab[up_h].id,cfg_Tab[up_h].k, &gParaCfgObj.update_flag, cfg_Tab[up_h].s);
	__NOP(); 
	load_data(cfg_Tab[fsd_sn].id,cfg_Tab[fsd_sn].k, &gParaCfgObj.slave_num, cfg_Tab[fsd_sn].s);
	__NOP(); 
	load_data(cfg_Tab[fsd_sid].id,cfg_Tab[fsd_sid].k, gParaCfgObj.slave_id, cfg_Tab[fsd_sid].s);
	__NOP(); 
	load_data(cfg_Tab[fsd_stype].id,cfg_Tab[fsd_stype].k, gParaCfgObj.slave_type, cfg_Tab[fsd_stype].s);
	__NOP(); 
	load_data(cfg_Tab[crc_chk].id,cfg_Tab[crc_chk].k, gParaCfgObj.crcret, cfg_Tab[crc_chk].s);
	__NOP(); 
}

void Fsd_CfgPara_Save(void)
{
	__NOP();
	save_data(cfg_Tab[up_h].id,cfg_Tab[up_h].k, &gParaCfgObj.update_flag,cfg_Tab[up_h].s);
	__NOP(); 
	save_data(cfg_Tab[fsd_sn].id,cfg_Tab[fsd_sn].k, &gParaCfgObj.slave_num,cfg_Tab[fsd_sn].s);
	__NOP();
	save_data(cfg_Tab[fsd_sid].id,cfg_Tab[fsd_sid].k, gParaCfgObj.slave_id, cfg_Tab[fsd_sid].s);
	__NOP();
	save_data(cfg_Tab[fsd_stype].id,cfg_Tab[fsd_stype].k, gParaCfgObj.slave_type,cfg_Tab[fsd_stype].s);
	__NOP();
	save_data(cfg_Tab[crc_chk].id,cfg_Tab[crc_chk].k, gParaCfgObj.crcret, cfg_Tab[crc_chk].s);
	__NOP();
}

uint16_t crc16(const uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;  

    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];  
        for (uint8_t j = 0; j < 8; j++) {  
            if (crc & 0x0001) {  
                crc = (crc >> 1) ^ 0xA001;  //CRC-16-ANSI: 0xA001
            } else {
                crc >>= 1;  
            }
        }
    }
    return crc;
}

/*<!
		return 0 :without any cfg paras
		1:has been cfged
		2:illegal paras 
		3:crc error
*/
uint8_t Fsd_StatusCheck(void)
{
	Fsd_CfgPara_Get();
	uint8_t retr;
	uint16_t crcsult;
	uint16_t crclen = CRC16_CFG_PARAS_LEN;
	uint16_t crcstorge = (gParaCfgObj.crcret[0]<<8)|(gParaCfgObj.crcret[1]);
	switch (gParaCfgObj.update_flag)
  {
  	case 0x01:
			//check crc
			crcsult = crc16(&gParaCfgObj.slave_num,crclen);
			if(crcstorge!= crcsult)
			{ 
				retr = ERR_CFG_CRC;
			}else{
				retr = GOOD_CFG_STATE;
			}
  		break;
  	case 0x00:
			retr = NONE_CFG_STATE;
  		break;
  	default:
			retr = ERR_CFG_STATE;
  		break;
  }
	return retr;
}	


void fds_event_handler(fds_evt_t const * const p_fds_evt)
{

    switch (p_fds_evt->id)
    {
        case FDS_EVT_WRITE:
            // 
						__nop();
            break;
        case FDS_EVT_UPDATE:
            // 
						__nop();
            break;
        case FDS_EVT_DEL_RECORD:
            // 
            break;
        default:
            break;
    }
}

void myfds_init(void)
{
    ret_code_t err_code;
    err_code = fds_register(fds_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = fds_init();
    APP_ERROR_CHECK(err_code);
}
uint16_t dbg_a= 0;
void save_data(uint16_t file_id, uint16_t record_key, uint8_t *data, size_t length)
{
    fds_record_t record;
    fds_record_desc_t record_desc;
    fds_find_token_t token = {0};
    ret_code_t err_code = fds_record_find(file_id, record_key, &record_desc, &token);
		record.file_id = file_id;       
		record.key = record_key;  
		record.data.p_data = data;
		record.data.length_words = (length + 3) / sizeof(uint32_t); 
    if (err_code == NRF_SUCCESS)
    {
        //NRF_LOG_INFO("Record found, updating...");
        err_code = fds_record_update(&record_desc, &record);
    }
    else
    {
        //NRF_LOG_INFO("Record not found, writing new record...");       
        err_code = fds_record_write(&record_desc, &record);
    }

    APP_ERROR_CHECK(err_code);
}



void load_data(uint16_t file_id, uint16_t record_key, uint8_t *data, size_t length) 
{
    fds_record_desc_t record_desc;
    fds_find_token_t token = {0};

   
    ret_code_t err_code = fds_record_find(file_id, record_key, &record_desc, &token);

    if (err_code == NRF_SUCCESS) 
    {
        fds_flash_record_t record;
        err_code = fds_record_open(&record_desc, &record);
        APP_ERROR_CHECK(err_code);

        memcpy(data, record.p_data, length);
        err_code = fds_record_close(&record_desc);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
       // NRF_LOG_INFO("Record not found.");
    }
}



void delete_data(uint16_t file_id, uint16_t record_key) {
    fds_record_desc_t record_desc;
    fds_find_token_t token = {0};

    ret_code_t err_code = fds_record_find(file_id, record_key, &record_desc, &token);
    if (err_code == NRF_SUCCESS) {

        err_code = fds_record_delete(&record_desc);
        APP_ERROR_CHECK(err_code);
    } else {
       // NRF_LOG_INFO("Record not found for deletion.");
    }
}




