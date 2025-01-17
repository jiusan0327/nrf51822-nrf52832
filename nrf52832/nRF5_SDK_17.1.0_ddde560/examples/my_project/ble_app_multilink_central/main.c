/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * @brief muti central 
 */
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "bsp_btn_ble.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_db_discovery.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_ble_scan.h"
#include "ble_nus_c.h"
/* user header */
#include "uart.h"
#include "myfds.h"
/* user var*/
const _firmware_info_t _firware_boot_info __attribute__((section("FMINFO"))) = {
	_FIRMWARE_INFO_FLAG,
	"MC100",
	"nrf52832aa",
	"v0.0",
	__DATE__,
	__TIME__,
	"0123456789abcde",
	0,
	_FIRMWARE_INFO_FLAG,
};


#define APP_BLE_CONN_CFG_TAG      1                                     /**< Tag that refers to the BLE stack configuration that is set with @ref sd_ble_cfg_set. The default tag is @ref APP_BLE_CONN_CFG_TAG. */
#define APP_BLE_OBSERVER_PRIO     3                                     /**< BLE observer priority of the application. There is no need to modify this value. */

#define CENTRAL_SCANNING_LED      BSP_BOARD_LED_0
#define CENTRAL_CONNECTED_LED     BSP_BOARD_LED_1
#define LEDBUTTON_LED             BSP_BOARD_LED_2                       /**< LED to indicate a change of state of the Button characteristic on the peer. */

#define LEDBUTTON_BUTTON          BSP_BUTTON_0                          /**< Button that writes to the LED characteristic of the peer. */
#define BUTTON_DETECTION_DELAY    APP_TIMER_TICKS(50)                   /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

NRF_BLE_GATT_DEF(m_gatt);                                               /**< GATT module instance. */
BLE_NUS_C_ARRAY_DEF(m_nus_c, NRF_SDH_BLE_CENTRAL_LINK_COUNT);           /**< LED button client instances. */
BLE_DB_DISCOVERY_ARRAY_DEF(m_db_disc, NRF_SDH_BLE_CENTRAL_LINK_COUNT);  /**< Database discovery module instances. */
NRF_BLE_SCAN_DEF(m_scan);                                               /**< Scanning Module instance. */
NRF_BLE_GQ_DEF(m_ble_gatt_queue,                                        /**< BLE GATT Queue instance. */
               NRF_SDH_BLE_CENTRAL_LINK_COUNT,
               NRF_BLE_GQ_QUEUE_SIZE);

static char eoat1_name[] = "EOAT_d_d";             /**< Name of the device to try to connect to. This name is searched for in the scanning report data. */
static char eoat2_name[] = "EOAT_d_d";  
circular_queue_t gQueue_nus2pc = {0};
void set_target_periph_name(char *name,int num1, int num2)
{
    sprintf(name, "EOAT_%d_%d", num1, num2);
}

/**@brief Function for handling asserts in the SoftDevice.
 *
 * @details This function is called in case of an assert in the SoftDevice.
 *
 * @warning This handler is only an example and is not meant for the final product. You need to analyze
 *          how your product is supposed to react in case of an assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num     Line number of the failing assert call.
 * @param[in] p_file_name  File name of the failing assert call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(0xDEADBEEF, line_num, p_file_name);
}

bool ble_nus_c_is_connected(ble_nus_c_t * p_nus)
{
    return (p_nus->conn_handle != BLE_CONN_HANDLE_INVALID);
}
/**@brief Function for handling the LED Button Service Client errors.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nus_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

void send_nus_data(uint8_t * data, uint16_t length,uint16_t conhandle)
{
	
    if (ble_nus_c_is_connected(&m_nus_c[conhandle]))
    {
        ret_code_t err_code = ble_nus_c_string_send(&m_nus_c[conhandle], data, length);

        if (err_code == NRF_SUCCESS)
        {
          // NRF_LOG_INFO("sent success\n");
        }
        else if (err_code == NRF_ERROR_BUSY)
        {
           // NRF_LOG_INFO("NUS service is busy.\n");
        }
        else if (err_code == NRF_ERROR_INVALID_STATE)
        {
            //NRF_LOG_INFO("NUS service not ready.\n");
        }
        else
        {
            //NRF_LOG_INFO("Error sending NUS data: 0x%X\n", err_code);
        }
    }
    else
    {
       // NRF_LOG_INFO("NUS service is not connected.\n");
    }
}

/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void)
{
    bsp_board_init(BSP_INIT_LEDS);
}


static void scan_evt_handler(scan_evt_t const * p_scan_evt)
{
    ret_code_t err_code;

    switch(p_scan_evt->scan_evt_id)
    {
        case NRF_BLE_SCAN_EVT_CONNECTING_ERROR:
        {
            err_code = p_scan_evt->params.connecting_err.err_code;
            APP_ERROR_CHECK(err_code);
        } break;

        default:
            break;
    }
}


/**@brief Function for initializing the scanning and setting the filters.
 */
static void scan_init(void)
{
    ret_code_t          err_code;
    nrf_ble_scan_init_t init_scan;

    memset(&init_scan, 0, sizeof(init_scan));

    init_scan.connect_if_match = true;
    init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;

    err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_scan_filter_set(&m_scan, SCAN_NAME_FILTER, eoat1_name);
    APP_ERROR_CHECK(err_code);
	
		err_code = nrf_ble_scan_filter_set(&m_scan, SCAN_NAME_FILTER, eoat2_name);
    APP_ERROR_CHECK(err_code);
	
	
    err_code = nrf_ble_scan_filters_enable(&m_scan, NRF_BLE_SCAN_NAME_FILTER, false);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting scanning. */
static void scan_start(void)
{
    ret_code_t ret;

    //NRF_LOG_INFO("Start scanning for multi devices ");
    ret = nrf_ble_scan_start(&m_scan);
    APP_ERROR_CHECK(ret);
    // Turn on the LED to signal scanning.
    bsp_board_led_on(CENTRAL_SCANNING_LED);
}


/**@brief Handles events coming from central module.
 */
//rx
static uint32_t nus_T_lst[8] = {0};
static uint16_t vaild_conn_handle[8]= {0};
static uint8_t count_con = 0;
uint8_t id_arp_state = 0;
static uint8_t requestdata[4] = {0x5a,0x55,0xfe,0xff};//send vaild frame
static uint8_t okstatus[4] = {0xff,0xff,0x00,0x01};
static uint8_t disconnect1_status[4] = {0xfe,0xfe,0x01,0x01};
static uint8_t disconnect2_status[4] = {0xfe,0xfe,0x01,0x02};
static uint8_t loss_slave1 = 0;
static uint8_t loss_slave2 = 0;
static uint8_t data_to_send[20];
struct {
	uint8_t id;
	uint16_t conhook;
}idarp_map[8] = {0};
static void handle_id_arp_event(ble_nus_c_evt_t *p_nus_c_evt) {
uint8_t slave_id = p_nus_c_evt->p_data[2];
for (uint8_t i = 0; i < gParaCfgObj.slave_num; i++) {
	if (slave_id == gParaCfgObj.slave_id[i]) {
		if (id_arp_state == 1) {
				nus_T_lst[0] = NRF_RTC0->COUNTER; 
				idarp_map[0].id = slave_id;
				idarp_map[0].conhook = vaild_conn_handle[0];
				id_arp_state = 2;
				if (ble_nus_c_is_connected(&m_nus_c[vaild_conn_handle[1]]) != 0) {
						send_nus_data(requestdata, 4, vaild_conn_handle[1]);
				}
		} else if (id_arp_state == 2) {
				nus_T_lst[1] = NRF_RTC0->COUNTER; 
				idarp_map[1].id = slave_id;
				idarp_map[1].conhook = vaild_conn_handle[1];
				id_arp_state = 3;
				gMsgObj.dst = uart2pc;
				gMsgObj.data_len = 4;
				memCpyUser(gMsgObj.data,okstatus,gMsgObj.data_len);
        PORT_transmit(&gMsgObj);
		}
		return; // Exit after processing the event
	 } 
	}
}

//arp table should be set  in  nus_c_evt_handler
static void nus_c_evt_handler(ble_nus_c_t * p_nus_c, ble_nus_c_evt_t * p_nus_c_evt)
{
    switch (p_nus_c_evt->evt_type)
    {
        case BLE_NUS_C_EVT_DISCOVERY_COMPLETE:
        {
            ret_code_t err_code;
            //NRF_LOG_INFO("NUS Service discovered on conn_handle 0x%x",p_nus_c_evt->conn_handle);
						err_code = ble_nus_c_handles_assign(p_nus_c, p_nus_c_evt->conn_handle, &p_nus_c_evt->handles);
					
            APP_ERROR_CHECK(err_code);
						err_code = ble_nus_c_tx_notif_enable(p_nus_c);
						//NRF_LOG_INFO("enable tx")
            APP_ERROR_CHECK(err_code);
						vaild_conn_handle[count_con] = p_nus_c_evt->conn_handle;
						if(count_con == 1)
						{
							if(loss_slave1 == 1)
							{
								nus_T_lst[0] = NRF_RTC0->COUNTER; 
								idarp_map[0].conhook = vaild_conn_handle[1];
								id_arp_state=3;
								gMsgObj.dst = uart2pc;
								gMsgObj.data_len = 4;
								memCpyUser(gMsgObj.data,okstatus,gMsgObj.data_len);
								PORT_transmit(&gMsgObj);
							}
							if(loss_slave2 == 1)
							{
								nus_T_lst[1] = NRF_RTC0->COUNTER; 
								idarp_map[1].conhook = vaild_conn_handle[1];
								id_arp_state=3;
								gMsgObj.dst = uart2pc;
								gMsgObj.data_len = 4;
								memCpyUser(gMsgObj.data,okstatus,gMsgObj.data_len);
								PORT_transmit(&gMsgObj);
							}
						}
						count_con++;
						
			
        } break; 

        case BLE_NUS_C_EVT_NUS_TX_EVT:
        {
					handle_id_arp_event(p_nus_c_evt);
					
        } break; 				
				case BLE_NUS_C_EVT_DISCONNECTED:
				{
					if((ble_nus_c_is_connected(&m_nus_c[idarp_map[0].conhook])==0)&&(id_arp_state==3))
					{
						gMsgObj.dst = uart2pc;
						gMsgObj.data_len = 4;
						memCpyUser(gMsgObj.data,disconnect1_status,gMsgObj.data_len);
						PORT_transmit(&gMsgObj);
						nus_T_lst[0] = 0;
						count_con--;
						loss_slave1 = 1;
					}else if((ble_nus_c_is_connected(&m_nus_c[idarp_map[1].conhook])==0)&&(id_arp_state==3)){						
						gMsgObj.dst = uart2pc;
						gMsgObj.data_len = 4;
						memCpyUser(gMsgObj.data,disconnect2_status,gMsgObj.data_len);
						PORT_transmit(&gMsgObj);
						nus_T_lst[1] = 0;
						count_con--;
						loss_slave2 = 1;
					}
					//NRF_LOG_INFO("disconnected");					
					if(count_con == 0)
					{
						queue_init(&gQueue_nus2pc);
						id_arp_state = 0;
						loss_slave1 = 0;
						loss_slave2 = 0;
					}else if(count_con == 1)
					{
						queue_init(&gQueue_nus2pc);
						id_arp_state = 4;
					}
					scan_start();
				}break;
				
        default:
            // No implementation needed.na'ni
            break;
    }
}


/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code;

    // For readability.
    ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id)
    {
        // Upon connection, check which peripheral is connected, initiate DB
        // discovery, update LEDs status, and resume scanning, if necessary.
        case BLE_GAP_EVT_CONNECTED:
        {
            NRF_LOG_INFO("Connection 0x%x established, starting DB discovery.",
                         p_gap_evt->conn_handle);

            APP_ERROR_CHECK_BOOL(p_gap_evt->conn_handle < NRF_SDH_BLE_CENTRAL_LINK_COUNT);

            err_code = ble_nus_c_handles_assign(&m_nus_c[p_gap_evt->conn_handle],
                                                p_gap_evt->conn_handle,
                                                NULL);
            APP_ERROR_CHECK(err_code);
            err_code = ble_db_discovery_start(&m_db_disc[p_gap_evt->conn_handle],
                                              p_gap_evt->conn_handle);
						APP_ERROR_CHECK(err_code);
				  	
            // Update LEDs status and check whether it is needed to look for more
            // peripherals to connect to.
            bsp_board_led_on(CENTRAL_CONNECTED_LED);
            if (ble_conn_state_central_conn_count() == NRF_SDH_BLE_CENTRAL_LINK_COUNT)
            {
                bsp_board_led_off(CENTRAL_SCANNING_LED);
            }
            else
            {
                // Resume scanning.
                bsp_board_led_on(CENTRAL_SCANNING_LED);
                scan_start();
            }
        } break; // BLE_GAP_EVT_CONNECTED

        // Upon disconnection, reset the connection handle of the peer that disconnected, update
        // the LEDs status and start scanning again.
        case BLE_GAP_EVT_DISCONNECTED:
        {
            NRF_LOG_INFO("NUS central link 0x%x disconnected (reason: 0x%x)",
                         p_gap_evt->conn_handle,
                         p_gap_evt->params.disconnected.reason);

            if (ble_conn_state_central_conn_count() == 0)
            {
								NRF_LOG_INFO("NUS central link nothing");
            }

            // Start scanning.
            scan_start();

            // Turn on the LED for indicating scanning.
            bsp_board_led_on(CENTRAL_SCANNING_LED);

        } break;

        case BLE_GAP_EVT_TIMEOUT:
        {
            // Timeout for scanning is not specified, so only the connection requests can time out.
            if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN)
            {
                NRF_LOG_DEBUG("Connection request timed out.");
            }
        } break;

        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST.");
            // Accept parameters requested by peer.
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                        &p_gap_evt->params.conn_param_update_request.conn_params);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT:
        {
            // Disconnect on GATT client timeout event.
            NRF_LOG_DEBUG("GATT client timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTS_EVT_TIMEOUT:
        {
            // Disconnect on GATT server timeout event.
            NRF_LOG_DEBUG("GATT server timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
        } break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief nus initialization. */
static void nus_c_init(void)
{
    ret_code_t       err_code;
    ble_nus_c_init_t nus_c_init_obj;

    nus_c_init_obj.evt_handler   = nus_c_evt_handler;
    nus_c_init_obj.p_gatt_queue  = &m_ble_gatt_queue;
    nus_c_init_obj.error_handler = nus_error_handler;

    for (uint32_t i = 0; i < NRF_SDH_BLE_CENTRAL_LINK_COUNT; i++)
    {
        err_code = ble_nus_c_init(&m_nus_c[i], &nus_c_init_obj);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupts.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}





/**@brief Function for handling database discovery events.
 *
 * @details This function is a callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function forwards the events
 *          to their respective services.
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    //NRF_LOG_DEBUG("call to ble_nus_c_on_db_disc_evt for instance %d and link 0x%x!",
     //             p_evt->conn_handle,
     //             p_evt->conn_handle);

    ble_nus_c_on_db_disc_evt(&m_nus_c[p_evt->conn_handle], p_evt);
}


/** @brief Database discovery initialization.
 */
static void db_discovery_init(void)
{
    ble_db_discovery_init_t db_init;

    memset(&db_init, 0, sizeof(ble_db_discovery_init_t));

    db_init.evt_handler  = db_disc_handler;
    db_init.p_gatt_queue = &m_ble_gatt_queue;

    ret_code_t err_code = ble_db_discovery_init(&db_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}



void slave_arp_get(void)
{
	//
	if((count_con==2)&&(id_arp_state == 0))
	{
		if(ble_nus_c_is_connected(&m_nus_c[vaild_conn_handle[0]])!=0)
		{

			send_nus_data(requestdata,4,vaild_conn_handle[0]);
		}	
		id_arp_state = 1;
	}
		
}

void send_data_if_ready(uint8_t slave_index,uint8_t abandon) {
	if (NRF_RTC0->COUNTER - nus_T_lst[slave_index] > 3000) {
		if (queue_dequeue(&gQueue_nus2pc, data_to_send)) {
			if (data_to_send[DATAID_j] == idarp_map[slave_index].id) {
					send_nus_data(data_to_send, data_to_send[DATALEN_j] + 11, idarp_map[slave_index].conhook);
			}else{
				if(!abandon)
				{
					queue_enqueue(&gQueue_nus2pc, data_to_send);
				}					
			}
		}
			nus_T_lst[slave_index] = NRF_RTC0->COUNTER;
	}
}

/**@brief Function for handling the idle state (main loop).
 *
 * @details This function handles any pending log operations, then sleeps until the next event occurs.
 */

static void idle_state_handle(void)
{
		//NRF_LOG_PROCESS();
//    if (NRF_LOG_PROCESS() == false)
//    {
//        nrf_pwr_mgmt_run();
//    }
		if(ready_to_write)
		{
			ready_to_write = 0;
			__disable_irq();
			Fsd_CfgPara_Save();
			cold_reboot();
		}
		slave_arp_get();
		if (id_arp_state == 3) {
				for (uint8_t i = 0; i < gParaCfgObj.slave_num; i++) {
						send_data_if_ready(i,0);
				}
		} else if (id_arp_state == 4) {
				if (loss_slave1 == 0){
						send_data_if_ready(0,1);
				}
				if (loss_slave2 == 0) {
						send_data_if_ready(1,1);
				}
		}

}


/** @brief Function for initializing the log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/** @brief Function for initializing the timer.
 */
static void timer_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}

int main(void)
{
    // Initialize.
		__enable_irq();
    log_init();
    timer_init();
		nrf52x_uart_init();
    leds_init();
    power_management_init();
		while (_firware_boot_info.flag_head != _FIRMWARE_INFO_FLAG)
		{
			__nop();
		}

		myfds_init();
		cfg_info_ack_data = Fsd_StatusCheck();
		if(cfg_info_ack_data == GOOD_CFG_STATE)
		{
			set_target_periph_name(eoat1_name,gParaCfgObj.slave_id[0], gParaCfgObj.slave_type[0]);
			set_target_periph_name(eoat2_name,gParaCfgObj.slave_id[1], gParaCfgObj.slave_type[1]);
		}
    ble_stack_init();
    gatt_init();
    db_discovery_init();
		queue_init(&gQueue_nus2pc);
    nus_c_init();
    ble_conn_state_init();
    scan_init();
    // Start execution.
    // NRF_LOG_INFO("waiting for scanning cmd");
    scan_start();
    for (;;)
    {
        idle_state_handle();
    }
}
