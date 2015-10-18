#include "gdt.h"
#include "tss.h"
#include <string.h>
#include "typedef.h"

static  TSS sys_tss;

//GDT32:
//dq	0x0000000000000000
//.code32		equ  $ - GDT32
//dq	0x00cf98000000ffff
//.data32		equ  $ - GDT32
//dq	0x00cf92000000ffff
//.real_code  equ  $ - GDT32
//dq	0x00009C000000ffff
//.real_data  equ  $ - GDT32
//dq	0x000092000000ffff

GDT::GDT()
{
	memset(&m_gdt, 0, sizeof(m_gdt));
}

GDT::~GDT()
{
}

void GDT::Init()
{
	// Setup kernel text segment (4GB read and execute, ring 0)
	init_seg(&m_gdt[GDT_KERNEL_CODE], 0, 0x100000, D_CODE | D_DPL0 | D_READ | D_PRESENT, D_BIG | D_BIG_LIM);

	// Setup kernel data segment (4GB read and write, ring 0)
	init_seg(&m_gdt[GDT_KERNEL_DATA], 0, 0x100000, D_DATA | D_DPL0 | D_WRITE | D_PRESENT, D_BIG | D_BIG_LIM);

	// Setup user text segment (2GB read and execute, ring 3)
	init_seg(&m_gdt[GDT_USER_CODE], 0, 0x80000, D_CODE | D_DPL3 | D_READ | D_PRESENT, D_BIG | D_BIG_LIM);

	// Setup user data segment (2GB read and write, ring 3)
	init_seg(&m_gdt[GDT_USER_DATA], 0, 0x80000, D_DATA | D_DPL3 | D_WRITE | D_PRESENT, D_BIG | D_BIG_LIM);

	// Setup TSS segment
	init_seg(&m_gdt[GDT_SYS_TSS], (unsigned long)&sys_tss, sizeof(TSS), D_TSS | D_DPL0 | D_PRESENT, 0);

	// Setup TIB segment
	init_seg(&m_gdt[GDT_TIB], 0, 4096, D_DATA | D_DPL3 | D_WRITE | D_PRESENT, 0);

	struct GDTR
	{
		uint16		limit;
		segment*	base;
	}gdtr;
	
	gdtr.limit = (sizeof(struct segment) * GDT_MAX) - 1;
	gdtr.base = m_gdt;
	__asm { lgdt gdtr }
}

void GDT::init_seg(struct segment *seg, unsigned long addr, unsigned long size, int access, int granularity)
{
	seg->base_low = (unsigned short)(addr & 0xFFFF);
	seg->base_med = (unsigned char)((addr >> 16) & 0xFF);
	seg->base_high = (unsigned char)((addr >> 24) & 0xFF);
	seg->limit_low = (unsigned short)((size - 1) & 0xFFFF);
	seg->limit_high = (unsigned char)((((size - 1) >> 16) & 0xF) | (granularity << 4));
	seg->access = access;
}