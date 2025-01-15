#include "pwm.h"


// PWM 0 instance
static nrf_drv_pwm_t m_pwm0 = NRFX_PWM_INSTANCE(0);

nrf_pwm_values_individual_t seq_values;
nrf_pwm_sequence_t const seq =
{
    .values.p_individual = &seq_values,
    .length              = NRF_PWM_VALUES_LENGTH(seq_values),
    .repeats             = 0,
    .end_delay           = 0
};

//L298N
#define IN1_PIN     						11  // GPIO  1
#define IN2_PIN   						  12  // GPIO  2
#define ENA_PIN     						19  // L298N output ENA 
#define PULL_DCM_INIT_TIMEms     1800  //can not be modfiy
static uint8_t pull_init_f = 0;
uint8_t PULL_DCMOTOR_INIT(uint32_t Tsys_now)
{

	if(!pull_init_f)
	{
		if((PULL_DCM_INIT_TIMEms>=Tsys_now) && (Tsys_now>0))
		{
			motor_backward(0);
		}else if ((Tsys_now>PULL_DCM_INIT_TIMEms) && (Tsys_now<(PULL_DCM_INIT_TIMEms*2))){
			motor_forward(0);
		}else if(Tsys_now>=(PULL_DCM_INIT_TIMEms*2)){
			motor_stop();
			pull_init_f = 1;
		}
		return pull_init_f;
	}else{
		return pull_init_f;
	}
}
void pwm_init(void)
{
    nrf_drv_pwm_config_t const config0 =
    {
        .output_pins =
        {
            ENA_PIN, 
            NRF_DRV_PWM_PIN_NOT_USED,
            NRF_DRV_PWM_PIN_NOT_USED,
            NRF_DRV_PWM_PIN_NOT_USED
        },
        .irq_priority = APP_IRQ_PRIORITY_LOWEST,
        .base_clock   = NRF_PWM_CLK_1MHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = 1000,  // frqe = 1MHz / 1000 = 1kHz
        .load_mode    = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode    = NRF_PWM_STEP_AUTO
    };

    nrf_drv_pwm_init(&m_pwm0, &config0, NULL);
}

void motor_backward(uint8_t duty_cycle)
{
    //  IN1 1,IN2 0
    nrf_gpio_pin_set(IN1_PIN);
    nrf_gpio_pin_clear(IN2_PIN);

    // PWM output 
    seq_values.channel_0 = duty_cycle * 10;  //  0-100
    nrf_drv_pwm_simple_playback(&m_pwm0, &seq, 1, NRF_DRV_PWM_FLAG_LOOP);
}

void motor_forward(uint8_t duty_cycle)
{
     //  IN1 0,IN2 1
    nrf_gpio_pin_clear(IN1_PIN);
    nrf_gpio_pin_set(IN2_PIN);
    
    seq_values.channel_0 = duty_cycle * 10;  //  0-100
    nrf_drv_pwm_simple_playback(&m_pwm0, &seq, 1, NRF_DRV_PWM_FLAG_LOOP);
}

void motor_stop(void)
{
    // stop PWM
    nrf_drv_pwm_stop(&m_pwm0, true);
    
    // IN1 0 IN2 0
    nrf_gpio_pin_clear(IN1_PIN);
    nrf_gpio_pin_clear(IN2_PIN);
}


void DC_motor_drvInit(void)
{
    // GPIO init
    nrf_gpio_cfg_output(IN1_PIN);
    nrf_gpio_cfg_output(IN2_PIN);

    pwm_init();

    motor_stop();
}



void DC_motor_test(uint16_t Tms)
{

    motor_forward(50); // 50% 
    nrf_delay_ms(Tms);


    motor_backward(75); // 75% 
    nrf_delay_ms(Tms);
	
		motor_stop();
}


void pwm_test(uint16_t duty1)
{
		nrf_gpio_pin_clear(IN1_PIN);
    nrf_gpio_pin_set(IN2_PIN);
		seq_values.channel_0 = duty1 * 10;  //  0-100
    nrf_drv_pwm_simple_playback(&m_pwm0, &seq, 1, NRF_DRV_PWM_FLAG_LOOP);
}
static uint32_t Trun = 0;
static uint8_t last_dir = 0;
//run in 1ms evt
void DCM_Ctl_Fro_BLE(uint8_t *En,uint8_t *dir)
{
	if((*En)== 0)
	{
		motor_stop();
		return;
	}
			if((*dir) == 1)
			{
				motor_forward(0);
			}else if((*dir) == 2){
				motor_backward(0);
			}else{
				motor_stop();
			}
}
