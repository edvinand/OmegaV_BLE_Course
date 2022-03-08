/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <logging/log.h>
#include <dk_buttons_and_leds.h>
#include "motor_control.h"
#include "remote.h"

#define LOG_MODULE_NAME app
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

static struct bt_conn *current_conn;

/* LEDs */
#define RUN_STATUS_LED DK_LED1
#define CONN_STATUS_LED DK_LED2
#define RUN_LED_BLINK_INTERVAL 1000

void on_connected(struct bt_conn *conn, uint8_t err);
void on_disconnected(struct bt_conn *conn, uint8_t reason);

struct bt_conn_cb bluetooth_callbacks = {
    .connected      = on_connected,
    .disconnected   = on_disconnected,
};

/* Callbacks */

void on_connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("connection failed, err %d", err);
    }
    LOG_INF("Connected to central");
    current_conn = bt_conn_ref(conn);
    dk_set_led_on(CONN_STATUS_LED);
}

void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("Disconnected (reason: %d)", reason);
	dk_set_led_off(CONN_STATUS_LED);
	if(current_conn) {
		bt_conn_unref(current_conn);
		current_conn = NULL;
	}
}

void button_handler(uint32_t button_state, uint32_t has_changed)
{
    //int err;
	int button_pressed = 0;
	if (has_changed & button_state)
	{
		switch (has_changed)
		{
			case DK_BTN1_MSK:
				button_pressed = 1;
                set_motor_angle(1);
				break;
			case DK_BTN2_MSK:
				button_pressed = 2;
                set_motor_angle(2);
				break;
			case DK_BTN3_MSK:
				button_pressed = 3;
				break;
			case DK_BTN4_MSK:
				button_pressed = 4;
				break;
			default:
				break;
		}
        LOG_INF("Button %d pressed.", button_pressed);
        set_button_press(button_pressed);        
    }
}


/* Configurations */
static void configure_dk_buttons_and_leds(void)
{
    int err;
    err = dk_leds_init();
    if (err) {
        LOG_ERR("Couldn't init LEDs (err %d)", err);
    }
    err = dk_buttons_init(button_handler);
    if (err) {
        LOG_ERR("Couldn't init buttons (err %d)", err);
    }
}

void main(void)
{
    int err;
    int blink_status = 0;
	LOG_INF("Hello World! %s", CONFIG_BOARD);

    configure_dk_buttons_and_leds();
    err = motor_init();
    if (err) {
        LOG_ERR("motor_init() failed. (err %d)", err);
    }

    err = bluetooth_init(&bluetooth_callbacks);
    if (err) {
        LOG_ERR("Bluetooth_init() failed. (err %d)", err);
    }

    LOG_INF("Running");

    for (;;) {
        dk_set_led(RUN_STATUS_LED, (blink_status++)%2);
        k_sleep(K_MSEC(1000));
    }
}
