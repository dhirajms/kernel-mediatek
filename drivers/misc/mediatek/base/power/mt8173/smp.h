/*
 * Copyright (c) 2015 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __MT_SMP_H
#define __MT_SMP_H

#include <asm/cputype.h>
#include <mt-plat/mt_chip.h>
#include <linux/of.h>

#ifdef CONFIG_ARM64
static inline int get_HW_cpuid(void)
{
	u64 mpidr;
	u32 id;
	int num_little;
	struct device_node *node = NULL;

	for (num_little = 0;; num_little++) {
		node = of_find_compatible_node(node, "cpu", "arm,cortex-a53");
		if (!node)
			break;
	}

	mpidr = read_cpuid_mpidr();
	id = (mpidr & 0xf) + ((mpidr >> 8) & 0xf) * num_little;

	return id;
}
#else
static inline int get_HW_cpuid(void)
{
	int id;

	asm("mrc     p15, 0, %0, c0, c0, 5 @ Get CPUID\n":"=r"(id));
	return (id & 0x3) + ((id & 0xF00) >> 6);
}
#endif

#endif
