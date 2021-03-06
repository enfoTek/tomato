/*
 * arch/ppc/platforms/pal4_setup.c
 *
 * Board setup routines for the SBS PalomarIV.
 *
 * Author: Dan Cox
 *
 * Copyright 2002 MontaVista Software Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/reboot.h>
#include <linux/time.h>
#include <linux/irq.h>
#include <linux/kdev_t.h>
#include <linux/blk.h>
#include <linux/console.h>
#include <linux/seq_file.h>

#include <asm/todc.h>
#include <asm/bootinfo.h>

#include "cpc700.h"
#include "pal4.h"

extern void pal4_find_bridges(void);

unsigned int cpc700_irq_assigns[][2] = {
        {1, 1},    /* IRQ 0: ECC correctable error */
        {1, 1},    /* IRQ 1: PCI write to memory range */
        {0, 1},    /* IRQ 2: PCI write to command register */
        {0, 1},    /* IRQ 3: UART 0 */
        {0, 1},    /* IRQ 4: UART 1 */
        {0, 1},    /* IRQ 5: ICC 0 */
        {0, 1},    /* IRQ 6: ICC 1 */
        {0, 1},    /* IRQ 7: GPT compare 0 */
        {0, 1},    /* IRQ 8: GPT compare 1 */
        {0, 1},    /* IRQ 9: GPT compare 2 */
        {0, 1},    /* IRQ 10: GPT compare 3 */
        {0, 1},    /* IRQ 11: GPT compare 4 */
        {0, 1},    /* IRQ 12: GPT capture 0 */
        {0, 1},    /* IRQ 13: GPT capture 1 */
        {0, 1},    /* IRQ 14: GPT capture 2 */
        {0, 1},    /* IRQ 15: GPT capture 3 */
        {0, 1},    /* IRQ 16: GPT capture 4 */
        {0, 0},    /* IRQ 17: reserved */
        {0, 0},    /* IRQ 18: reserved */
        {0, 0},    /* IRQ 19: reserved */
        {0, 0},    /* IRQ 20: reserved */
        {0, 1},    /* IRQ 21: Ethernet */
        {0, 0},    /* IRQ 22: reserved */
        {0, 0},    /* IRQ 23: reserved */
        {0, 0},    /* IRQ 24: resreved */
        {0, 0},    /* IRQ 25: reserved */
        {0, 0},    /* IRQ 26: reserved */
        {0, 0},    /* IRQ 27: reserved */
        {0, 0},    /* IRQ 28: reserved */
        {0, 0},    /* IRQ 29: reserved */
        {0, 0},    /* IRQ 30: reserved */
        {0, 0},    /* IRQ 31: reserved */
};

static int
pal4_show_cpuinfo(struct seq_file *m)
{
        seq_printf(m, "board\t\t: SBS Palomar IV\n");

        return 0;
}

static void
pal4_restart(char *cmd)
{
        __cli();
        __asm__ __volatile__("lis  3,0xfff0\n \
                              ori  3,3,0x100\n \
                              mtspr 26,3\n \
                              li   3,0\n \
                              mtspr 27,3\n \
                              rfi");

        for(;;);
}

static void
pal4_power_off(void)
{
	__cli();
	for(;;);
}

static void
pal4_halt(void)
{
	pal4_power_off();
}

static unsigned long __init
pal4_find_end_of_memory(void)
{
	int i;
	unsigned long len, amt, batu, batl;

        __asm__ __volatile__("lis  %0,0xf000\n \
                              ori  %1,%0,0x002a\n \
                              ori  %0,%0,0x1ffe\n \
                              mtspr 0x21e,%0\n \
                              mtspr 0x21f,%1\n \
                              isync\n \
                              sync" : "=r" (batu), "=r" (batl));

	for(len = 0, i = CPC700_MB1EA; i <= CPC700_MB4EA; i+=4) {
		amt = cpc700_read_memreg(i);
		if (amt == 0)
			break;
		len = amt;
	}

	return len;
}

TODC_ALLOC();

static void __init
pal4_setup_arch(void)
{
	unsigned long l2;

	TODC_INIT(TODC_TYPE_MK48T37, 0, 0, 
		  ioremap(PAL4_NVRAM, PAL4_NVRAM_SIZE), 8);

	pal4_find_bridges();

#ifdef CONFIG_BLK_DEV_INITRD
	if (initrd_start)
		ROOT_DEV = MKDEV(RAMDISK_MAJOR, 0);
	else
#endif
		ROOT_DEV = to_kdev_t(0x00ff);

	/* The L2 gets disabled in the bootloader, but all the proper
	   bits should be present from the fw, so just re-enable it */
	l2 = _get_L2CR();
	if (!(l2 & L2CR_L2E)) {
		/* presume that it was initially set if the size is
		   still present. */
		if (l2 ^ L2CR_L2SIZ_MASK)
			_set_L2CR(l2 | L2CR_L2E);
		else
			printk("L2 not set by firmware; left disabled.\n");
	}
}

static void __init
pal4_map_io(void)
{
	io_block_mapping(0xf0000000, 0xf0000000, 0x10000000, _PAGE_IO);
}

void __init
platform_init(unsigned long r3, unsigned long r4, unsigned long r5,
	      unsigned long r6, unsigned long r7)
{
	parse_bootinfo(find_bootinfo());

	isa_io_base = 0 /*PAL4_ISA_IO_BASE*/;
	pci_dram_offset = 0 /*PAL4_PCI_SYS_MEM_BASE*/;
	
	ppc_md.setup_arch = pal4_setup_arch;
	ppc_md.show_cpuinfo = pal4_show_cpuinfo;
	
	ppc_md.find_end_of_memory = pal4_find_end_of_memory;
	ppc_md.setup_io_mappings = pal4_map_io;

	ppc_md.init_IRQ = cpc700_init_IRQ;
	ppc_md.get_irq = cpc700_get_irq;

	ppc_md.restart = pal4_restart;
	ppc_md.halt = pal4_halt;
	ppc_md.power_off = pal4_power_off;

	ppc_md.time_init = todc_time_init;
	ppc_md.set_rtc_time = todc_set_rtc_time;
	ppc_md.get_rtc_time = todc_get_rtc_time;
	ppc_md.calibrate_decr = todc_calibrate_decr;

	ppc_md.nvram_read_val = todc_direct_read_val;
	ppc_md.nvram_write_val = todc_direct_write_val;
}

