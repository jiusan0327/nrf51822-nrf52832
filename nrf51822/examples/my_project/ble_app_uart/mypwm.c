#include "mypwm.h"

APP_PWM_INSTANCE(PWM1,1);                   // Create the instance "PWM1" using TIMER1.
static uint32_t FULL_DUTY_CYCLE_DEFAULT = 20000;//67L
static volatile bool ready_flag;            // A flag indicating PWM status.
void pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
{
    ready_flag = true;
}

void smipwm_init(uint32_t fullduty)
{
    ret_code_t err_code;
		if(fullduty == 0 || fullduty<66 )
		{
			fullduty = FULL_DUTY_CYCLE_DEFAULT;
		}else{
			FULL_DUTY_CYCLE_DEFAULT = fullduty;
		}
    /* 2-channel PWM, 15kHz, output on DK LED pins.  67us*/
    app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_2CH(fullduty, 3,4);
    /* Switch the polarity of the second channel. */
		pwm1_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;
		pwm1_cfg.pin_polarity[1] = APP_PWM_POLARITY_ACTIVE_HIGH;
    /* Initialize and enable PWM. */
    err_code = app_pwm_init(&PWM1,&pwm1_cfg,pwm_ready_callback);
    APP_ERROR_CHECK(err_code);
    app_pwm_enable(&PWM1);

}

void pwm_test(void)
{
    static uint32_t value = 0;
    static bool increasing = true; 

    // Update the duty cycle
    APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM1, 0, 0));
		APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM1, 1, value));
    // Change the value for breathing effect
    if (increasing)
    {
        value += 1; 
        if (value >= 67)
        {
            value = 67; 
            increasing = false; 
        }
    }
    else
    {
        value -= 1; 
        if (value <= 0)
        {
            value = 0; 
            increasing = true; 
        }
    }

    nrf_delay_ms(5); 
}

void motor_backward(uint32_t duty_cycle)
{
	if(duty_cycle>FULL_DUTY_CYCLE_DEFAULT)
	{
		duty_cycle = FULL_DUTY_CYCLE_DEFAULT;
	}
	
	APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM1, 0, 0));
	nrf_delay_us(100);
	APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM1, 1, duty_cycle));
}

void motor_forward(uint32_t duty_cycle)
{
	if(duty_cycle>FULL_DUTY_CYCLE_DEFAULT)
	{
		duty_cycle = FULL_DUTY_CYCLE_DEFAULT;
	}
	APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM1, 1, 0));
	nrf_delay_us(100);
	APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM1, 0, duty_cycle));
	
}


void servo_m1_degree(uint32_t duty_cycle)
{
	if(duty_cycle>FULL_DUTY_CYCLE_DEFAULT)
	{
		duty_cycle = FULL_DUTY_CYCLE_DEFAULT;
	}
	APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM1, 0, duty_cycle));
}


void servo_m2_degree(uint32_t duty_cycle)
{
	if(duty_cycle>FULL_DUTY_CYCLE_DEFAULT)
	{
		duty_cycle = FULL_DUTY_CYCLE_DEFAULT;
	}
	APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM1, 1, duty_cycle));
}

void motor_stop(void)
{
	APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM1, 0, 0));
	APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM1, 1, 0));
}