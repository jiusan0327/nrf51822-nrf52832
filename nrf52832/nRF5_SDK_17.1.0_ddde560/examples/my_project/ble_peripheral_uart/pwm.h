#ifndef PWM_H__
#define PWM_H__

#include "nrf_drv_pwm.h"
#include "nrfx_pwm.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"


void DC_motor_drvInit(void);
void DC_motor_test(uint16_t Tms);
void pwm_test(uint16_t duty1);
void motor_forward(uint8_t duty_cycle);
void motor_backward(uint8_t duty_cycle);
void motor_stop(void);
uint8_t PULL_DCMOTOR_INIT(uint32_t Tsys_now);
void DCM_Ctl_Fro_BLE(uint8_t *En,uint8_t *dir);
#endif
