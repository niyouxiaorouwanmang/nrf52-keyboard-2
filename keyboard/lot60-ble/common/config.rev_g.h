#pragma once

#define PRODUCT "Fly 60" /* 硬件名称，用于蓝牙显示 */
#define DEVICE_VER 0x0003 /* 硬件版本 */

#define LED_CAPS 19
#define LED_RGB_R 24
#define LED_RGB_G 23
#define LED_RGB_B 22
#define LED_STATUS_BLE 22
#define LED_STATUS_CHARGING 23
#define LED_STATUS_USB 24

#define UART_RXD 17
#define UART_TXD 18

#define BATTERY_ADC_PIN NRF_SAADC_INPUT_AIN0

static const uint8_t row_pin_array[MATRIX_ROWS] = { 27, 26, 30, 29, 25 };
static const uint8_t column_pin_array[MATRIX_COLS] = { 28, 11, 12, 13, 14, 16, 15, 9, 8, 7, 6, 5, 4, 3 };

#define ROW_IN // 二极管方向是从COL->ROW

#define SWD_DAT_IO T2
#define SWD_DAT_MASK bT2
#define SWD_DAT_PORT P1
#define SWD_CLK_IO T2EX
#define SWD_CLK_MASK bT2EX
#define SWD_CLK_PORT P1

// Bootloader指示灯
#define LED_DFU_INIT 22
#define LED_DFU_START 23
#define LED_DFU_FINISH 24
// #define LED_DFU_POSITIVE
