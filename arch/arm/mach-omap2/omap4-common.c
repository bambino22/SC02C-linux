/*
 * OMAP4 specific common source file.
 *
 * Copyright (C) 2010 Texas Instruments, Inc.
 * Author:
 *	Santosh Shilimkar <santosh.shilimkar@ti.com>
 *
 *
 * This program is free software,you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>

#include <asm/hardware/gic.h>
#include <asm/hardware/cache-l2x0.h>
<<<<<<< HEAD
=======
#include <asm/mach/map.h>
#include <asm/memblock.h>

#include <plat/irqs.h>
#include <plat/sram.h>
>>>>>>> 716a3dc... ARM: Add arm_memblock_steal() to allocate memory away from the kernel

#include <mach/hardware.h>
#include <mach/omap4-common.h>

#ifdef CONFIG_CACHE_L2X0
void __iomem *l2cache_base;
#endif

<<<<<<< HEAD
void __iomem *gic_dist_base_addr;
=======
static void __iomem *sar_ram_base;

#ifdef CONFIG_OMAP4_ERRATA_I688
/* Used to implement memory barrier on DRAM path */
#define OMAP4_DRAM_BARRIER_VA			0xfe600000

void __iomem *dram_sync, *sram_sync;

void omap_bus_sync(void)
{
	if (dram_sync && sram_sync) {
		writel_relaxed(readl_relaxed(dram_sync), dram_sync);
		writel_relaxed(readl_relaxed(sram_sync), sram_sync);
		isb();
	}
}

static int __init omap_barriers_init(void)
{
	struct map_desc dram_io_desc[1];
	phys_addr_t paddr;
	u32 size;

	if (!cpu_is_omap44xx())
		return -ENODEV;

	size = ALIGN(PAGE_SIZE, SZ_1M);
	paddr = arm_memblock_steal(size, SZ_1M);

	dram_io_desc[0].virtual = OMAP4_DRAM_BARRIER_VA;
	dram_io_desc[0].pfn = __phys_to_pfn(paddr);
	dram_io_desc[0].length = size;
	dram_io_desc[0].type = MT_MEMORY_SO;
	iotable_init(dram_io_desc, ARRAY_SIZE(dram_io_desc));
	dram_sync = (void __iomem *) dram_io_desc[0].virtual;
	sram_sync = (void __iomem *) OMAP4_SRAM_VA;

	pr_info("OMAP4: Map 0x%08llx to 0x%08lx for dram barrier\n",
		(long long) paddr, dram_io_desc[0].virtual);
>>>>>>> 716a3dc... ARM: Add arm_memblock_steal() to allocate memory away from the kernel


void __init gic_init_irq(void)
{
	void __iomem *gic_cpu_base;

	/* Static mapping, never released */
	gic_dist_base_addr = ioremap(OMAP44XX_GIC_DIST_BASE, SZ_4K);
	BUG_ON(!gic_dist_base_addr);

	/* Static mapping, never released */
	gic_cpu_base = ioremap(OMAP44XX_GIC_CPU_BASE, SZ_512);
	BUG_ON(!gic_cpu_base);

	gic_init(0, 29, gic_dist_base_addr, gic_cpu_base);
}

#ifdef CONFIG_CACHE_L2X0

static void omap4_l2x0_disable(void)
{
	/* Disable PL310 L2 Cache controller */
	omap_smc1(0x102, 0x0);
}

static void omap4_l2x0_set_debug(unsigned long val)
{
	/* Program PL310 L2 Cache controller debug register */
	omap_smc1(0x100, val);
}

static int __init omap_l2_cache_init(void)
{
	u32 aux_ctrl = 0;

	/*
	 * To avoid code running on other OMAPs in
	 * multi-omap builds
	 */
	if (!cpu_is_omap44xx())
		return -ENODEV;

	/* Static mapping, never released */
	l2cache_base = ioremap(OMAP44XX_L2CACHE_BASE, SZ_4K);
	BUG_ON(!l2cache_base);

	/*
	 * 16-way associativity, parity disabled
	 * Way size - 32KB (es1.0)
	 * Way size - 64KB (es2.0 +)
	 */
	aux_ctrl = ((1 << L2X0_AUX_CTRL_ASSOCIATIVITY_SHIFT) |
			(0x1 << 25) |
			(0x1 << L2X0_AUX_CTRL_NS_LOCKDOWN_SHIFT) |
			(0x1 << L2X0_AUX_CTRL_NS_INT_CTRL_SHIFT));

	if (omap_rev() == OMAP4430_REV_ES1_0) {
		aux_ctrl |= 0x2 << L2X0_AUX_CTRL_WAY_SIZE_SHIFT;
	} else {
		aux_ctrl |= ((0x3 << L2X0_AUX_CTRL_WAY_SIZE_SHIFT) |
			(1 << L2X0_AUX_CTRL_SHARE_OVERRIDE_SHIFT) |
			(1 << L2X0_AUX_CTRL_DATA_PREFETCH_SHIFT) |
			(1 << L2X0_AUX_CTRL_INSTR_PREFETCH_SHIFT) |
			(1 << L2X0_AUX_CTRL_EARLY_BRESP_SHIFT));
	}
	if (omap_rev() != OMAP4430_REV_ES1_0)
		omap_smc1(0x109, aux_ctrl);

	/* Enable PL310 L2 Cache controller */
	omap_smc1(0x102, 0x1);

	l2x0_init(l2cache_base, aux_ctrl, L2X0_AUX_CTRL_MASK);

	/*
	 * Override default outer_cache.disable with a OMAP4
	 * specific one
	*/
	outer_cache.disable = omap4_l2x0_disable;
	outer_cache.set_debug = omap4_l2x0_set_debug;

	return 0;
}
early_initcall(omap_l2_cache_init);
#endif
