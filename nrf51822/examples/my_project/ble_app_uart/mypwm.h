#ifndef  MYPWM_H_
#define MYPWM_H_
#include <nrf.h>
#include <nrf_gpio.h>
#include <app_pwm.h>
#include <nrf_drv_ppi.h>
#include <nrf_drv_clock.h>
#include "nrf_delay.h"
#include "myadc.h"
void smipwm_init(uint32_t fullduty);
void pwm_test(void);
void motor_backward(uint32_t duty_cycle);
void motor_forward(uint32_t duty_cycle);
void motor_stop(void);
#endif