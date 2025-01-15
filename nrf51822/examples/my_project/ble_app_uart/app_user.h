#ifndef  APP_USER_H_
#define APP_USER_H_
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "nordic_common.h"
#include "ble_nus.h"
#include "mypwm.h"
#include "myadc.h"
//<! id  type
#define USER_DEVICE_ID          1
#define USER_TYPE_ID            1
//err
enum{
  chks_err = 0x1010,
};



typedef enum _eoat_e_{
	none_type =0,
	gear_box_dc_motor = 1,
	digital_servo_motor = 2,
	linear_actuator = 3,
	eoat_type_num,
}eoat_e;
typedef struct _packet_str_{
	uint8_t head1;
	uint8_t head2;
	uint8_t dev_id;
	uint8_t index[2];
	uint8_t subidx;
	uint8_t len;
	uint8_t reserved;
	uint8_t data[8];
	uint8_t cks[2];
	uint8_t tail1;
	uint8_t tail2;
}packet_str;



typedef struct _MessageHeader_{
	uint8_t len;
	uint8_t data[23];
}MsgPk_str;



extern float adc_values[ADC_BUFFER_SIZE];




void over_current_event(void);
void app_handler(uint8_t *data,uint16_t len,MsgPk_str *MsgObj);
void linear_actuator_fun(packet_str *obj);
void gear_box_dc_motor_fun(packet_str *obj);
void digital_servo_motor_fun(packet_str *obj);
#endif