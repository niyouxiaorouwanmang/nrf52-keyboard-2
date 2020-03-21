/*
Copyright (C) 2019 Jim Jiang <jim@lotlab.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "3led_status.h"
#include "app_timer.h"
#include "config.h"

#include "keyboard_evt.h"
#include "keyboard_led.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "power_save.h"
#include "usb_comm.h"

APP_TIMER_DEF(ble_led_blink_timer);
uint8_t blink_led_id;
bool blink_status = true; //当前蓝牙广播LED闪烁状态
static uint8_t saved_status_led_val; // 暂存的LED的值

enum status_led {
    BIT_LED_BLE,
    BIT_LED_USB,
    BIT_LED_CHARGING
};

static void status_led_set_internal(uint8_t val);

/** 
 * 蓝牙广播状态闪烁LED
 **/
void ble_led_blink_timer_handler(void* context)
{
    if (!usb_working()) {
        switch (blink_led_id) {
#ifdef LED_BLE_CHANNEL1
        case 0:
            status_led_set_internal(blink_status ? (1 << LED_BLE_CHANNEL1) : 0);
            break;
#endif
#ifdef LED_BLE_CHANNEL2
        case 1:
            status_led_set_internal(blink_status ? (1 << LED_BLE_CHANNEL2) : 0);
            break;
#endif
#ifdef LED_BLE_CHANNEL3
        case 2:
            status_led_set_internal(blink_status ? (1 << LED_BLE_CHANNEL3) : 0);
            break
#elif defined(LED_BLE_CHANNEL1) && defined(LED_BLE_CHANNEL2)
        case 2:
            status_led_set_internal(blink_status ? (1 << LED_BLE_CHANNEL1) | (1 << LED_BLE_CHANNEL2) : 0);
            break;
#endif
                default : break;
        }
        blink_status = !blink_status;
    }
}

/** 
 * 初始化状态LED
 **/
void status_led_init()
{
#ifdef LED_STATUS_BLE
    nrf_gpio_cfg_output(LED_STATUS_BLE);
#endif
#ifdef LED_STATUS_USB
    nrf_gpio_cfg_output(LED_STATUS_USB);
#endif
#ifdef LED_STATUS_CHARGING
    nrf_gpio_cfg_output(LED_STATUS_CHARGING);
#endif
    app_timer_create(&ble_led_blink_timer, APP_TIMER_MODE_REPEATED, ble_led_blink_timer_handler);
}

/**
 * 释放LED
 * 
 */
void status_led_deinit(void)
{
#ifdef LED_STATUS_BLE
    nrf_gpio_cfg_default(LED_STATUS_BLE);
#endif
#ifdef LED_STATUS_USB
    nrf_gpio_cfg_default(LED_STATUS_USB);
#endif
#ifdef LED_STATUS_CHARGING
    nrf_gpio_cfg_default(LED_STATUS_CHARGING);
#endif
}

/** 
 * 内部设置状态LED的值
 **/
static void status_led_set_internal(uint8_t val)
{
#ifdef LED_STATUS_BLE
    LED_WRITE(LED_STATUS_BLE, val & (1 << BIT_LED_BLE));
#endif
#ifdef LED_STATUS_USB
    LED_WRITE(LED_STATUS_USB, val & (1 << BIT_LED_USB));
#endif
#ifdef LED_STATUS_CHARGING
    LED_WRITE(LED_STATUS_CHARGING, val & (1 << BIT_LED_CHARGING));
#endif
}

/** 
 * 关闭状态LED的灯光
 **/
void status_led_off()
{
    status_led_set_internal(0);
}

/** 
 * 启用状态LED的灯光
 **/
void status_led_on()
{
    status_led_set_internal(saved_status_led_val);
}

/** 
 * 通知所有灯光启用
 **/
static void set_led_on()
{
    status_led_on();
    power_save_reset();
}

/**
 *  系统状态LED灯全开
 * 
 *  用于开关机时闪烁一次
 */
static void status_led_all_on(void)
{
    status_led_set_internal(0x0F);
}

/**
 * @brief 蓝牙广播状态开启闪烁灯
 * 
 */
static void ble_blink_led_on()
{
    blink_status = true;
    ble_led_blink_timer_handler(NULL); //即刻执行闪烁
    app_timer_start(ble_led_blink_timer, APP_TIMER_TICKS(500), NULL);
}
/**
 * @brief 蓝牙关闭广播后关闭闪烁灯
 * 
 */
static void ble_blink_led_off()
{
    status_led_off();
    app_timer_stop(ble_led_blink_timer);
}

/** 
 * 设置蓝牙状态灯的状态
 **/
void status_led_ble(bool state)
{
    if (state) {
        saved_status_led_val |= (1 << BIT_LED_BLE);
    } else {
        saved_status_led_val &= ~(1 << BIT_LED_BLE);
    }
    set_led_on();
}

/** 
 * 设置充电状态灯的状态
 **/
void status_led_charging(bool state)
{
    if (state) {
        saved_status_led_val |= (1 << BIT_LED_CHARGING);
    } else {
        saved_status_led_val &= ~(1 << BIT_LED_CHARGING);
    }
    set_led_on();
}

/** 
 * 设置USB状态灯的状态
 **/
void status_led_usb(bool state)
{
    if (state) {
        saved_status_led_val |= (1 << BIT_LED_USB);
    } else {
        saved_status_led_val &= ~(1 << BIT_LED_USB);
    }
    set_led_on();
}

static void status_led_evt_handler(enum user_event event, void* arg)
{
    uint8_t arg2 = (uint32_t)arg;
    switch (event) {
    case USER_EVT_STAGE:
        switch (arg2) {
        case KBD_STATE_POST_INIT: // 初始化LED
            status_led_init();
            status_led_all_on();
            nrf_delay_ms(10);
            break;
        case KBD_STATE_SLEEP: // 准备休眠
        case KBD_STATE_SYSTEMOFF: // 准备关机
            status_led_off();
            status_led_deinit();
            break;
        default:
            break;
        }
        break;
    case USER_EVT_POWERSAVE:
        switch (arg2) {
        case PWR_SAVE_ENTER: // 进入省电模式
            status_led_off();
            break;
        case PWR_SAVE_EXIT: // 退出省电模式
            status_led_on();
            break;
        default:
            break;
        }
        break;
    case USER_EVT_CHARGE: // 充电事件
        status_led_charging(arg2 == BATT_CHARGING);
        break;
    case USER_EVT_USB: // USB事件
        status_led_usb(arg2 == USB_WORKING);
        break;
    case USER_EVT_BLE_DEVICE_SWITCH: // 蓝牙设备通道切换事件
        blink_led_id = arg2;
        break;
    case USER_EVT_BLE_STATE_CHANGE: // 蓝牙状态事件
        if (arg2 == BLE_STATE_FAST_ADV) {
            ble_blink_led_on();
        } else {
            ble_blink_led_off();
        }
        status_led_ble(arg2 == BLE_STATE_CONNECTED);
        break;
    default:
        break;
    }
}

EVENT_HANDLER(status_led_evt_handler);