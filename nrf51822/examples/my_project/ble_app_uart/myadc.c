#include "myadc.h"
//<!  channal 1 2 7 
#include "app_error.h"
#include "nrf_drv_adc.h"
#include "nrf_log.h"
static uint8_t flag_id_type = 0;
uint8_t adc_id=0;
uint8_t adc_type=0;
static nrf_adc_value_t adc_buffer[ADC_BUFFER_SIZE];
static nrf_drv_adc_channel_t m_channels[ADC_BUFFER_SIZE] = {
    NRF_DRV_ADC_DEFAULT_CHANNEL(NRF_ADC_CONFIG_INPUT_3), // pin1
    NRF_DRV_ADC_DEFAULT_CHANNEL(NRF_ADC_CONFIG_INPUT_2), //  2
    NRF_DRV_ADC_DEFAULT_CHANNEL(NRF_ADC_CONFIG_INPUT_7)  //  6
};


uint8_t get_dev_id(void)
{
	return adc_id;
}
uint8_t get_dev_type(void)
{
	return adc_type;
}
static void adc_event_handler(nrf_drv_adc_evt_t const * p_event)
{
}
static inline uint8_t  determine_adc_category(float temp) {
    if (temp < 0.1) {
        return 1;
    } else if (temp < 0.4) {
        return 2;
    } else if (temp < 0.5) {
        return 3;
    }
    return 0;
}
uint32_t adc_config(void)
{
    ret_code_t ret_code;
    nrf_drv_adc_config_t config = NRF_DRV_ADC_DEFAULT_CONFIG;
    ret_code = nrf_drv_adc_init(&config, adc_event_handler);

    APP_ERROR_CHECK(ret_code);

    // Enable the ADC channels
    for (uint32_t i = 0; i < ADC_BUFFER_SIZE; i++)
    {
        nrf_drv_adc_channel_enable(&m_channels[i]);
    }
		
		adc_start_conversion();
		nrf_delay_ms(50);
		float temp = 0;
		temp = (adc_buffer[0]/(float)ADC_MAX_VALUE) * VREF;
		adc_id = determine_adc_category(temp);
	  temp = (adc_buffer[1]/(float)ADC_MAX_VALUE) * VREF;
		adc_type = determine_adc_category(temp);
		nrf_drv_adc_channel_disable(&m_channels[0]);
		nrf_drv_adc_channel_disable(&m_channels[1]);

		switch (adc_type) {
			case 1:
        return PWM_DUTY0;
			case 3:
        return PWM_DUTY3;
			default:
        return PWM_DUTY5;
		}

}

void adc_start_conversion(void)
{
   APP_ERROR_CHECK(nrf_drv_adc_buffer_convert(adc_buffer, ADC_BUFFER_SIZE));
	 nrf_drv_adc_sample(); // Start ADC conversion
}


void adc_read_values(float *adc_values)
{
     adc_values[0] = adc_buffer[0]; //ma
}



// ADC FSM
 enum {
			ADC_IDLE,
			ADC_CONVERTING,
			ADC_WAIT,
	} adc_state = ADC_IDLE;
uint32_t convt_tgap = 600;
uint8_t flag_over_curr = 0;
float dbg_adc = 0;
void manage_adc_sampling(float *adc_values) {
	static uint32_t convt_count = 0;
	switch (adc_state) {
			case ADC_IDLE:
				  APP_ERROR_CHECK(nrf_drv_adc_buffer_convert(adc_buffer, 1));
					nrf_drv_adc_sample(); // Start ADC conversion
					adc_state = ADC_CONVERTING;
					convt_count = NRF_RTC0->COUNTER;
					break;

			case ADC_CONVERTING:
					if(NRF_RTC0->COUNTER - convt_count >convt_tgap)
					{
						adc_read_values(adc_values);
						dbg_adc = adc_values[0];
						adc_state = ADC_IDLE;
					}else{
						adc_state = ADC_WAIT;
					}
					 // Adjust as needed for timing        
					break;

			case ADC_WAIT:
          adc_state = ADC_CONVERTING;
					break;
	}
}

