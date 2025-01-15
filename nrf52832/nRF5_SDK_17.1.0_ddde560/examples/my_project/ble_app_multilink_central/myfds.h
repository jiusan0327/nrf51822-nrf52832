#ifndef  _MYFDS_H_
#define _MYFDS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "nordic_common.h"
#include "fds.h"
#include "nrf.h"
#include "nrf_soc.h"
#include "nrf_error.h"
#include "app_util_platform.h"

#define CRC16_CFG_PARAS_LEN  (cfg_Tab[fsd_sn].s + cfg_Tab[fsd_stype].s + cfg_Tab[fsd_sid].s)
#define NONE_CFG_STATE      0
#define GOOD_CFG_STATE      1
#define ERR_CFG_STATE       2
#define ERR_CFG_CRC					3
#define FOUR_ALIGN   __attribute__((aligned(4)))
enum{
	up_h =0,
	fsd_sn,
	fsd_sid,
	fsd_stype,
	crc_chk,
};

typedef struct  _para_info_{
	uint8_t FOUR_ALIGN update_flag;
	uint8_t FOUR_ALIGN slave_num;
	uint8_t FOUR_ALIGN slave_id[8];
	uint8_t FOUR_ALIGN slave_type[8];
	uint8_t FOUR_ALIGN crcret[2];
}para_info_str ;

typedef struct _cfg_tab_str_{
	uint16_t id;
	uint16_t k;//key
	uint16_t s;//size
}cfg_tab_str;



extern cfg_tab_str cfg_Tab[10];
extern  para_info_str gParaCfgObj;
extern uint8_t ready_to_write;

uint8_t Fsd_StatusCheck(void);
extern void Fsd_CfgPara_Get(void);
extern void Fsd_CfgPara_Save(void);
void myfds_init(void);
void save_data(uint16_t file_id, uint16_t record_key, uint8_t *data, size_t length);
void load_data(uint16_t file_id, uint16_t record_key, uint8_t *data, size_t length) ;
uint16_t crc16(const uint8_t *data, uint16_t length);
void cold_reboot(void);
#endif // FDS_H__
