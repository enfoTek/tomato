
#ifndef _MIPS_MIPS64_CACHE_H
#define _MIPS_MIPS64_CACHE_H

#include <asm/asm.h>
#include <asm/cacheops.h>

static inline void flush_icache_line_indexed(unsigned long addr)
{
	unsigned long waystep = icache_size/mips_cpu.icache.ways;
	unsigned int way;

	for (way = 0; way < mips_cpu.icache.ways; way++)
	{
		__asm__ __volatile__(
			".set noreorder\n\t"
			"cache %1, (%0)\n\t"
			".set reorder"
			:
			: "r" (addr),
			"i" (Index_Invalidate_I));

		addr += waystep;
	}
}

static inline void flush_dcache_line_indexed(unsigned long addr)
{
	unsigned long waystep = dcache_size/mips_cpu.dcache.ways;
	unsigned int way;

	for (way = 0; way < mips_cpu.dcache.ways; way++)
	{
		__asm__ __volatile__(
			".set noreorder\n\t"
			"cache %1, (%0)\n\t"
			".set reorder"
			:
			: "r" (addr),
			"i" (Index_Writeback_Inv_D));

		addr += waystep;
	}
}

static inline void flush_scache_line_indexed(unsigned long addr)
{
	unsigned long waystep = scache_size/mips_cpu.scache.ways;
	unsigned int way;

	for (way = 0; way < mips_cpu.scache.ways; way++)
	{
		__asm__ __volatile__(
			".set noreorder\n\t"
			"cache %1, (%0)\n\t"
			".set reorder"
			:
			: "r" (addr),
			"i" (Index_Writeback_Inv_SD));

		addr += waystep;
	}
}

static inline void flush_icache_line(unsigned long addr)
{
	__asm__ __volatile__(
		".set noreorder\n\t"
		"cache %1, (%0)\n\t"
		".set reorder"
		:
		: "r" (addr),
		  "i" (Hit_Invalidate_I));
}

static inline void flush_dcache_line(unsigned long addr)
{
	__asm__ __volatile__(
		".set noreorder\n\t"
		"cache %1, (%0)\n\t"
		".set reorder"
		:
		: "r" (addr),
		  "i" (Hit_Writeback_Inv_D));
}

static inline void invalidate_dcache_line(unsigned long addr)
{
	__asm__ __volatile__(
		".set noreorder\n\t"
		"cache %1, (%0)\n\t"
		".set reorder"
		:
		: "r" (addr),
		  "i" (Hit_Invalidate_D));
}

static inline void invalidate_scache_line(unsigned long addr)
{
	__asm__ __volatile__(
		".set noreorder\n\t"
		"cache %1, (%0)\n\t"
		".set reorder"
		:
		: "r" (addr),
		  "i" (Hit_Invalidate_SD));
}

static inline void flush_scache_line(unsigned long addr)
{
	__asm__ __volatile__(
		".set noreorder\n\t"
		"cache %1, (%0)\n\t"
		".set reorder"
		:
		: "r" (addr),
		  "i" (Hit_Writeback_Inv_SD));
}

/*
 * The next two are for badland addresses like signal trampolines.
 */
static inline void protected_flush_icache_line(unsigned long addr)
{
	__asm__ __volatile__(
		".set noreorder\n\t"
		"1:\tcache %1,(%0)\n"
		"2:\t.set reorder\n\t"
		".section\t__ex_table,\"a\"\n\t"
		".dword\t1b,2b\n\t"
		".previous"
		:
		: "r" (addr), "i" (Hit_Invalidate_I));
}

static inline void protected_writeback_dcache_line(unsigned long addr)
{
	__asm__ __volatile__(
		".set noreorder\n\t"
		"1:\tcache %1,(%0)\n"
		"2:\t.set reorder\n\t"
		".section\t__ex_table,\"a\"\n\t"
		".dword\t1b,2b\n\t"
		".previous"
		:
		: "r" (addr), "i" (Hit_Writeback_D));
}

#define cache_unroll(base,op)			\
	__asm__ __volatile__("			\
		.set noreorder;			\
		cache %1, (%0);			\
		.set reorder"			\
		:				\
		: "r" (base),			\
		  "i" (op));


static inline void blast_dcache(void)
{
	unsigned long start = KSEG0;
	unsigned long end = (start + dcache_size);

	while(start < end) {
		cache_unroll(start,Index_Writeback_Inv_D);
		start += dc_lsize;
	}
}

static inline void blast_dcache_page(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = (start + PAGE_SIZE);

	while(start < end) {
		cache_unroll(start,Hit_Writeback_Inv_D);
		start += dc_lsize;
	}
}

static inline void blast_dcache_page_indexed(unsigned long page)
{
	unsigned long start;
	unsigned long end = (page + PAGE_SIZE);
	unsigned long waystep = dcache_size/mips_cpu.dcache.ways;
	unsigned int way;

	for (way = 0; way < mips_cpu.dcache.ways; way++) {
		start = page + way*waystep;
		while(start < end) {
			cache_unroll(start,Index_Writeback_Inv_D);
			start += dc_lsize;
		}
	}
}

static inline void blast_icache(void)
{
	unsigned long start = KSEG0;
	unsigned long end = (start + icache_size);

	while(start < end) {
		cache_unroll(start,Index_Invalidate_I);
		start += ic_lsize;
	}
}

static inline void blast_icache_page(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = (start + PAGE_SIZE);

	while(start < end) {
		cache_unroll(start,Hit_Invalidate_I);
		start += ic_lsize;
	}
}

static inline void blast_icache_page_indexed(unsigned long page)
{
	unsigned long start;
	unsigned long end = (page + PAGE_SIZE);
	unsigned long waystep = icache_size/mips_cpu.icache.ways;
	unsigned int way;

	for (way = 0; way < mips_cpu.icache.ways; way++) {
		start = page + way*waystep;
		while(start < end) {
			cache_unroll(start,Index_Invalidate_I);
			start += ic_lsize;
		}
	}
}

static inline void blast_scache(void)
{
	unsigned long start = KSEG0;
	unsigned long end = KSEG0 + scache_size;

	while(start < end) {
		cache_unroll(start,Index_Writeback_Inv_SD);
		start += sc_lsize;
	}
}

static inline void blast_scache_page(unsigned long page)
{
	unsigned long start = page;
	unsigned long end = page + PAGE_SIZE;

	while(start < end) {
		cache_unroll(start,Hit_Writeback_Inv_SD);
		start += sc_lsize;
	}
}

static inline void blast_scache_page_indexed(unsigned long page)
{
	unsigned long start;
	unsigned long end = (page + PAGE_SIZE);
	unsigned long waystep = scache_size/mips_cpu.scache.ways;
	unsigned int way;

	for (way = 0; way < mips_cpu.scache.ways; way++) {
		start = page + way*waystep;
		while(start < end) {
			cache_unroll(start,Index_Writeback_Inv_SD);
			start += sc_lsize;
		}
	}
}

#endif /* !(_MIPS_MIPS64_CACHE_H) */


