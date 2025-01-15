#include "uart.h"

//23-2-1=19
static uint16_t m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH;

enum{
	frame_head1 = 0,
	frame_head2,
	frame_end1,
	frame_end2,
};
MsgHd_str gMsgObj = {0};
dMsg_str gDMsgObj = {0};

static inline uint8_t IsVaildCfgPacket(uint8_t data1,uint8_t data2)
{
	if((data1 == config_cmd[frame_head1])&&(data2 == config_cmd[frame_head2]))
	{
		return 1;
	}else{
		return 0;
	}
}

static inline uint8_t IsVaildDataPacket(uint8_t data1,uint8_t data2)
{
	if((data1 == data_cmd[frame_head1])&&(data2 == data_cmd[frame_head2]))
	{
		return 1;
	}else{
		return 0;
	}
}

static inline uint8_t IsVaildPacket(uint8_t data1,uint8_t data2)
{
	if((data1 == config_cmd[frame_end1])&&(data2 == config_cmd[frame_end2]))
	{
		return 1;
	}else{
		return 0;
	}
}


void PORT_transmit(MsgHd_str *udata)
{
	uint8_t data_tmps[16] = {0};

	switch (udata->dst)
  {
  	case uart2pc:
			if(udata->data_len == 0)
			{
				break;
			}
			for(int i=0;i<udata->data_len;i++)
			{
				app_uart_put(udata->data[i]);
			}
  		break;
  	case pc2nus:
			queue_enqueue(&gQueue_nus2pc,udata->data);
  		break;
		case nus2pc:
			break;
		case uart2pc_multi:
			for(int i=0;i<udata->multi_k;i++)
			{
				for(int j=0;j<gMsgObjArray[i].data_len;j++)
				{
					app_uart_put(gMsgObjArray[i].data[j]);
				}
				__NOP();
			}
		
			break;
  	default:
  		break;
  }
}
//FH1,FH2,FT1,FT2
enum{
	FH1 = 0,
	FH2 = 1,
	FT1 = 3,
	FT2 = 2,
	
};


//<@! not suitable for normal uart data
//<@! usseful for rengxing protocol only 
void pc_uart_evt_handle(app_uart_evt_t * p_event)
{
		static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
		static uint16_t idx_rx = 0;
		static uint8_t packet_rev =0;
		app_uart_get(&data_array[idx_rx]);//data_array[0],data_array[1],data_array[2],data_array[3]
		idx_rx++;
		if ((data_array[idx_rx - 1] == '\n') ||(data_array[idx_rx - 1] == '\r') ||(idx_rx >= (m_ble_nus_max_data_len)))
		{
			
			if((idx_rx>=5)&&(IsVaildPacket(data_array[idx_rx - FT1],data_array[idx_rx-FT2])))
			{
				
				if(IsVaildCfgPacket(data_array[FH1],data_array[FH2]))
				{
					cfg_proccess(data_array,idx_rx,&gMsgObj);
					PORT_transmit(&gMsgObj);
				}
				
				
				if(IsVaildDataPacket(data_array[FH1],data_array[FH2]))
				{
					data_proccess(data_array,idx_rx,&gMsgObj);
					PORT_transmit(&gMsgObj);
				}
			}			
			idx_rx = 0;
		}
		


		
	
}




void nrf52x_uart_init(void)
{
    ret_code_t err_code;
		/**@brief Function for initializing the UART. */
		app_uart_comm_params_t const comm_params =
		{
				.rx_pin_no    = RX_PIN_NUMBER,
				.tx_pin_no    = TX_PIN_NUMBER,
				.rts_pin_no   = RTS_PIN_NUMBER,
				.cts_pin_no   = CTS_PIN_NUMBER,
				.flow_control = APP_UART_FLOW_CONTROL_DISABLED,
				.use_parity   = false,
				.baud_rate    = UART_BAUDRATE_BAUDRATE_Baud115200
		};


    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       pc_uart_evt_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);

    APP_ERROR_CHECK(err_code);
}