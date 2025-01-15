#ifndef MYADC_H
#define MYADC_H
#include <stdint.h>
#include <stdbool.h>
#include "nrf_delay.h"
#include "nrf_drv_common.h"
#define ADC_BUFFER_SIZE 3 // Buffer size for 3 ADC channels
#define ADC_MAX_VALUE 1023
#define VREF 3.3
enum{
	PWM_DUTY0 = 67,   //15khz
	PWM_DUTY1 = 100,  //10khz
	PWM_DUTY2 = 200,  //5khz
	PWM_DUTY3 = 1000, //1KHZ
  PWM_DUTY4 = 2000, //500HZ
	PWM_DUTY5 = 5000, //200HZ
	PWM_DUTY6	= 10000,//100HZ
	PWM_DUTY7	= 20000,//50HZ  
};
extern uint8_t adc_id ;
extern uint8_t adc_type;
extern uint8_t flag_over_curr;
uint32_t  adc_config(void);
void adc_start_conversion(void);
void adc_read_values(float *adc_values);
void manage_adc_sampling(float *adc_values) ;
void servo_m1_degree(uint32_t duty_cycle);
void servo_m2_degree(uint32_t duty_cycle);
uint8_t get_dev_id(void);
uint8_t get_dev_type(void);
#endif // ADC_EXAMPLE_H,
