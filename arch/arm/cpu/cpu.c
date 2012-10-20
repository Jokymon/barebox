/*
 * cpu.c - A few helper functions for ARM
 *
 * Copyright (c) 2007 Sascha Hauer <s.hauer@pengutronix.de>, Pengutronix
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/**
 * @file
 * @brief A few helper functions for ARM
 */

#include <common.h>
#include <init.h>
#include <command.h>
#include <cache.h>
#include <asm/mmu.h>
#include <asm/system.h>
#include <asm/memory.h>

/**
 * Enable processor's instruction cache
 */
void icache_enable(void)
{
	u32 r;

	r = get_cr();
	r |= CR_I;
	set_cr(r);
}

/**
 * Disable processor's instruction cache
 */
void icache_disable(void)
{
	u32 r;

	r = get_cr();
	r &= ~CR_I;
	set_cr(r);
}

/**
 * Detect processor's current instruction cache status
 * @return 0=disabled, 1=enabled
 */
int icache_status(void)
{
	return (get_cr () & CR_I) != 0;
}

/**
 * Disable MMU and D-cache, flush caches
 * @return 0 (always)
 *
 * This function is called by shutdown_barebox to get a clean
 * memory/cache state.
 */
void arch_shutdown(void)
{
#ifdef CONFIG_MMU
	/* nearly the same as below, but this could also disable
	 * second level cache.
	 */
	mmu_disable();
#else
	asm volatile (
		"bl __mmu_cache_flush;"
		"bl __mmu_cache_off;"
		:
		:
		: "r0", "r1", "r2", "r3", "r6", "r10", "r12", "lr", "cc", "memory"
	);
#endif
}

#ifdef CONFIG_THUMB2_BAREBOX
static void thumb2_execute(void *func, int argc, char *argv[])
{
	/*
	 * Switch back to arm mode before executing external
	 * programs.
	 */
	__asm__ __volatile__ (
		"mov r0, #0\n"
		"mov r1, %0\n"
		"mov r2, %1\n"
		"bx %2\n"
		:
		: "r" (argc - 1), "r" (&argv[1]), "r" (func)
		: "r0", "r1", "r2"
	);
}

static int execute_init(void)
{
	do_execute = thumb2_execute;
	return 0;
}
postcore_initcall(execute_init);
#endif
