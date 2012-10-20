/*
 * Copyright (C) 2007 Sascha Hauer, Pengutronix
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 */

#include <common.h>
#include <net.h>
#include <init.h>
#include <environment.h>
#include <asm/armlinux.h>
#include <generated/mach-types.h>
#include <partition.h>
#include <fs.h>
#include <fcntl.h>
#include <io.h>
#include <asm/hardware.h>
#include <nand.h>
#include <sizes.h>
#include <linux/mtd/nand.h>
#include <mach/at91_pmc.h>
#include <mach/board.h>
#include <mach/gpio.h>
#include <mach/io.h>
#include <mach/at91sam9_smc.h>
#include <mach/sam9_smc.h>
#include <dm9000.h>
#include <gpio_keys.h>
#include <readkey.h>
#include <led.h>

static struct atmel_nand_data nand_pdata = {
	.ale		= 22,
	.cle		= 21,
/*	.det_pin	= ... not connected */
	.rdy_pin	= AT91_PIN_PC15,
	.enable_pin	= AT91_PIN_PC14,
#if defined(CONFIG_MTD_NAND_ATMEL_BUSWIDTH_16)
	.bus_width_16	= 1,
#else
	.bus_width_16	= 0,
#endif
	.on_flash_bbt	= 1,
};

static struct sam9_smc_config ek_nand_smc_config = {
	.ncs_read_setup		= 0,
	.nrd_setup		= 1,
	.ncs_write_setup	= 0,
	.nwe_setup		= 1,

	.ncs_read_pulse		= 3,
	.nrd_pulse		= 3,
	.ncs_write_pulse	= 3,
	.nwe_pulse		= 3,

	.read_cycle		= 5,
	.write_cycle		= 5,

	.mode			= AT91_SMC_READMODE | AT91_SMC_WRITEMODE | AT91_SMC_EXNWMODE_DISABLE,
	.tdf_cycles		= 2,
};

static void ek_add_device_nand(void)
{
	/* setup bus-width (8 or 16) */
	if (nand_pdata.bus_width_16)
		ek_nand_smc_config.mode |= AT91_SMC_DBW_16;
	else
		ek_nand_smc_config.mode |= AT91_SMC_DBW_8;

	/* configure chip-select 3 (NAND) */
	sam9_smc_configure(3, &ek_nand_smc_config);

	at91_add_device_nand(&nand_pdata);
}

/*
 * DM9000 ethernet device
 */
#if defined(CONFIG_DRIVER_NET_DM9K)
static struct dm9000_platform_data dm9000_data = {
	.srom		= 0,
};

/*
 * SMC timings for the DM9000.
 * Note: These timings were calculated for MASTER_CLOCK = 100000000 according to the DM9000 timings.
 */
static struct sam9_smc_config __initdata dm9000_smc_config = {
	.ncs_read_setup		= 0,
	.nrd_setup		= 2,
	.ncs_write_setup	= 0,
	.nwe_setup		= 2,

	.ncs_read_pulse		= 8,
	.nrd_pulse		= 4,
	.ncs_write_pulse	= 8,
	.nwe_pulse		= 4,

	.read_cycle		= 16,
	.write_cycle		= 16,

	.mode			= AT91_SMC_READMODE | AT91_SMC_WRITEMODE | AT91_SMC_EXNWMODE_DISABLE | AT91_SMC_BAT_WRITE | AT91_SMC_DBW_16,
	.tdf_cycles		= 1,
};

static void __init ek_add_device_dm9000(void)
{
	/* Configure chip-select 2 (DM9000) */
	sam9_smc_configure(2, &dm9000_smc_config);

	/* Configure Reset signal as output */
	at91_set_gpio_output(AT91_PIN_PC10, 0);

	/* Configure Interrupt pin as input, no pull-up */
	at91_set_gpio_input(AT91_PIN_PC11, 0);

	add_dm9000_device(0, AT91_CHIPSELECT_2, AT91_CHIPSELECT_2 + 4,
			  IORESOURCE_MEM_16BIT, &dm9000_data);
}
#else
static void __init ek_add_device_dm9000(void) {}
#endif /* CONFIG_DRIVER_NET_DM9K */

#if defined(CONFIG_USB_GADGET_DRIVER_AT91)
/*
 * USB Device port
 */
static struct at91_udc_data __initdata ek_udc_data = {
	.vbus_pin	= AT91_PIN_PB29,
	.pullup_pin	= 0,
};

static void ek_add_device_udc(void)
{
	at91_add_device_udc(&ek_udc_data);
}
#else
static void ek_add_device_udc(void) {}
#endif

#ifdef CONFIG_KEYBOARD_GPIO
struct gpio_keys_button keys[] = {
	{
		.code = KEY_UP,
		.gpio = AT91_PIN_PA26,
	}, {
		.code = KEY_DOWN,
		.gpio = AT91_PIN_PA25,
	}, {
		.code = KEY_ENTER,
		.gpio = AT91_PIN_PA24,
	},
};

struct gpio_keys_platform_data gk_pdata = {
	.buttons = keys,
	.nbuttons = ARRAY_SIZE(keys),
};

static void ek_add_device_keyboard_buttons(void)
{
	int i;

	for (i = 0; i < gk_pdata.nbuttons; i++) {
		/* user push button, pull up enabled */
		keys[i].active_low = 1;
		at91_set_GPIO_periph(keys[i].gpio, keys[i].active_low);
		at91_set_deglitch(keys[i].gpio, 1);
	}

	add_gpio_keys_device(-1, &gk_pdata);
}
#else
static void ek_add_device_keyboard_buttons(void) {}
#endif

static void __init ek_add_device_buttons(void)
{
	at91_set_gpio_input(AT91_PIN_PA27, 1);
	at91_set_deglitch(AT91_PIN_PA27, 1);
	export_env_ull("dfu_button", AT91_PIN_PA27);
	ek_add_device_keyboard_buttons();
}

#ifdef CONFIG_LED_GPIO
struct gpio_led ek_leds[] = {
	{
		.gpio		= AT91_PIN_PA23,
		.led = {
			.name = "ds1",
		},
	}, {
		.gpio		= AT91_PIN_PA14,
		.active_low	= 1,
		.led = {
			.name = "ds7",
		},
	}, {
		.gpio		= AT91_PIN_PA13,
		.active_low	= 1,
		.led = {
			.name = "ds8",
		},
	},
};

static void ek_device_add_leds(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ek_leds); i++) {
		at91_set_gpio_output(ek_leds[i].gpio, ek_leds[i].active_low);
		led_gpio_register(&ek_leds[i]);
	}
	led_set_trigger(LED_TRIGGER_HEARTBEAT, &ek_leds[0].led);
}
#else
static void ek_device_add_leds(void) {}
#endif

static int at91sam9261ek_mem_init(void)
{
	at91_add_device_sdram(0);

	return 0;
}
mem_initcall(at91sam9261ek_mem_init);

static int at91sam9261ek_devices_init(void)
{

	ek_add_device_nand();
	ek_add_device_dm9000();
	ek_add_device_udc();
	ek_add_device_buttons();
	ek_device_add_leds();

	devfs_add_partition("nand0", 0x00000, SZ_128K, DEVFS_PARTITION_FIXED, "at91bootstrap_raw");
	devfs_add_partition("nand0", SZ_128K, SZ_256K, DEVFS_PARTITION_FIXED, "self_raw");
	dev_add_bb_dev("self_raw", "self0");
	devfs_add_partition("nand0", SZ_256K + SZ_128K, SZ_128K, DEVFS_PARTITION_FIXED, "env_raw");
	dev_add_bb_dev("env_raw", "env0");
	devfs_add_partition("nand0", SZ_512K, SZ_128K, DEVFS_PARTITION_FIXED, "env_raw1");
	dev_add_bb_dev("env_raw1", "env1");

	armlinux_set_bootparams((void *)(AT91_CHIPSELECT_1 + 0x100));
	if (machine_is_at91sam9g10ek())
		armlinux_set_architecture(MACH_TYPE_AT91SAM9G10EK);
	else
		armlinux_set_architecture(MACH_TYPE_AT91SAM9261EK);

	return 0;
}

device_initcall(at91sam9261ek_devices_init);

static int at91sam9261ek_console_init(void)
{
	at91_register_uart(0, 0);
	return 0;
}

console_initcall(at91sam9261ek_console_init);
