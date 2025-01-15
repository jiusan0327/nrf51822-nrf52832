#include "app_user.h"
#define CHECK_SAME_TAIL(a,b)    (a!=b)     
#define PACKET_MIN_LEN             4

static float last_current = 0;
static float current_sum = 0;

static uint8_t err_frame[4] = {0xfe,0xfe,0,0};
static uint8_t config_cmd[4] = {0x5A,0x55,0xFE,0xFF};
static uint8_t data_cmd[4] = {0x55,0x5A,0xFE,0xFF};
static uint8_t ack_frame[4] = {0x61,0x63,0x65,0x64};
packet_str gframe = {0};
//can not be changed
void (*actuator_fun_array[eoat_type_num])(packet_str *obj) = {linear_actuator_fun,
																															gear_box_dc_motor_fun,
																															digital_servo_motor_fun};



uint16_t simple_checksum(const uint8_t *data, size_t length) {
    uint16_t checksum = 0;
    for (size_t i = 0; i < length; i++) {
        checksum += data[i];
    }
    return checksum;
}

void app_handler(uint8_t *data,uint16_t len,MsgPk_str *MsgObj)
{		
		MsgObj->len = 0;
		if(len<PACKET_MIN_LEN)
		{
			return;
		}

		if(CHECK_SAME_TAIL(data_cmd[2],data[len-2])||CHECK_SAME_TAIL(data_cmd[3],data[len-1]))
		{
			return;
		}
		if((data[0] == config_cmd[0])&&(data[1] == config_cmd[1]))
		{
			if(len == 4)
			{
				MsgObj->len = 6;
				MsgObj->data[0] = ack_frame[0];
				MsgObj->data[1] = ack_frame[1];
				MsgObj->data[2] = adc_id;
				MsgObj->data[3] = adc_type;
				MsgObj->data[4] = ack_frame[2];
				MsgObj->data[5] = ack_frame[3];
			}
		}else if((data[0] == data_cmd[0])&& (data[1] == data_cmd[1]))
		{
			gframe.head1 = data[0];
			gframe.head2 = data[1];
			gframe.dev_id = data[2];
			gframe.index[0] =data[3];
			gframe.index[1] = data[4];
			gframe.subidx = data[5];
			gframe.len = data[6];
			
			if(gframe.dev_id!=adc_id)
			{
				return;
			}
			
			for(int i=0;i<gframe.len;i++)
			{
				gframe.data[i] = data[7+i];
			}
			gframe.cks[0] = data[7+gframe.len];
			gframe.cks[1] = data[7+gframe.len+1];
			uint16_t cks_tamp = (gframe.cks[0]<<8)|(gframe.cks[1]);
			if(simple_checksum(&gframe.dev_id,7+gframe.len)!=cks_tamp)
			{
				MsgObj->len = 4;
				MsgObj->data[0] = err_frame[0];
				MsgObj->data[1] = err_frame[1];
				MsgObj->data[2] = (uint8_t )chks_err>>8;
				MsgObj->data[3] = (uint8_t )chks_err;
				return;
				//chk err
			}
			actuator_fun_array[adc_type](&gframe);
			MsgObj->len = 0;
		}else{
			__nop();
		}
	
}


void linear_actuator_fun(packet_str *obj)
{
    if (obj->len != 2) return; // ????

    uint16_t mainindex = (obj->index[0] << 8) | obj->index[1];

    if (mainindex == 0x0001U && obj->subidx == 0x01)
    {
        if (obj->data[0] == 1)
        {
            switch (obj->data[1])
            {
                case 0:
                    motor_stop();
                    break;
                case 1:
                    motor_backward(PWM_DUTY3);
                    break;
                case 2:
                    motor_forward(PWM_DUTY3);
                    break;
                default:
                    // ????
                    break;
            }
        }
        else
        {
            motor_stop();
        }
    }
}



void gear_box_dc_motor_fun(packet_str *obj)
{
    if (obj->len != 2 && obj->len != 3) return; // ????

    uint16_t mainindex = (obj->index[0] << 8) | obj->index[1];
		over_current_event();
    switch (mainindex)
    {
        case 0x0001U:
            if (obj->subidx != 0x01) break;			
            if (obj->data[0] == 0)
            {
							motor_stop();
            }else if (obj->data[0] == 1 || obj->data[0] == 2)
            {
							uint16_t duty = PWM_DUTY0 / 3;
//							duty *= (obj->data[1] + 1); 

							if (obj->data[0] == 1)
							{
									motor_forward(duty);
							}
							else
							{
									motor_backward(duty);
							}
            }
            else
            {
                motor_stop();
            }
            break;
        case 0x0002U:
            if (obj->data[0] == 0)
            {
                motor_forward(obj->data[1]);
            }
            else if (obj->data[0] == 1)
            {
                motor_backward(obj->data[1]);
            }
            break;
        default:
            break;
    }
}

#define MIN_PULSE_WIDTH 500         //zero degree          
//0-279 rds3280
void digital_servo_motor_fun(packet_str *obj)
{
	if (obj->len != 4)return;
	uint16_t mainindex = (obj->index[0] << 8) | obj->index[1];
	uint32_t pulse_width = 0;
	uint16_t angle[2] = {0}; 
	angle[0] = (obj->data[0]<<8)|(obj->data[1]);
	angle[1] = (obj->data[2]<<8)|(obj->data[3]);
	angle[0] = angle[0]>270?270:angle[0];
	angle[1] = angle[1]>270?270:angle[1];
	switch (mainindex)
    {
    	case 0x0001U:
//			pulse_width = MIN_PULSE_WIDTH + angle[0] * (MIN_PULSE_WIDTH) / 60;		
//			motor_forward(pulse_width);		
//			nrf_delay_ms(1000);
//			pulse_width = MIN_PULSE_WIDTH + angle[1] * (MIN_PULSE_WIDTH) / 60;		
//			motor_backward(pulse_width);		
//			nrf_delay_ms(1000);
			
				servo_m1_degree(500);		
			  nrf_delay_ms(500);
				servo_m1_degree(1000);		
				nrf_delay_ms(500);
				servo_m2_degree(500);		
			  nrf_delay_ms(500);
				servo_m2_degree(1000);		
				nrf_delay_ms(500);			
    		break;
    	default:
    		break;
    }
}



void over_current_event(void)
{
	static uint32_t lstdet_T = 0;
	if(flag_over_curr)
	{
		flag_over_curr = 0;
		motor_stop();
		return;
	}
}
