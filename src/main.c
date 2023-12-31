/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/sys/printk.h>
#include <zephyr/drivers/clock_control.h>
#include <zephyr/drivers/clock_control/nrf_clock_control.h>
#include "radio_test.h"
#include "radio_cmd.c"
#include <zephyr/shell/shell.h>
#include <zephyr/drivers/gpio.h>

static void clock_init(void)
{
	int err;
	int res;
	struct onoff_manager *clk_mgr;
	struct onoff_client clk_cli;

	clk_mgr = z_nrf_clock_control_get_onoff(CLOCK_CONTROL_NRF_SUBSYS_HF);
	if (!clk_mgr) {
		printk("Unable to get the Clock manager\n");
		return;
	}

	sys_notify_init_spinwait(&clk_cli.notify);

	err = onoff_request(clk_mgr, &clk_cli);
	if (err < 0) {
		printk("Clock request failed: %d\n", err);
		return;
	}

	do {
		err = sys_notify_fetch_result(&clk_cli.notify, &res);
		if (!err && res) {
			printk("Clock could not be started: %d\n", res);
			return;
		}
	} while (err);

	printk("Clock has started\n");
}

int main(void)
{
	printk("Starting Radio Test example\n");

	clock_init();

	// toggle leds
	const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
	gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
	gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
	gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);

	k_sleep(K_SECONDS(2));

	struct shell *shell = shell_backend_uart_get_ptr();
	char *argv[] = {"start_channel", "20"};
	cmd_start_channel_set(shell, 2, argv);
	char *argv2[] = {"start_tx_carrier"};
	cmd_tx_carrier_start(shell, 1, argv2);

	// print alive every seconds
	// while (1) {
	// 	shell_print(shell, "alive");
	// 	k_sleep(K_SECONDS(1));
	// }

	return 0;
}
