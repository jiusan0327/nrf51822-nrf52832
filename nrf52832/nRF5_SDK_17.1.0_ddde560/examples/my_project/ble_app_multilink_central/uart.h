#ifndef  UART_H_
#define UART_H_
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "nordic_common.h"
#include "app_error.h"
#include "app_uart.h"
#include "ble_gatt.h"
#include "ble_nus_c.h"
#include "pca10040.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

//<! user head file 
#include "app_user.h"

#define UART_TX_BUF_SIZE        256                                     /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE        256  

void nrf52x_uart_init(void);
#endif
