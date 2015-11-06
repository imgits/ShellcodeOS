#pragma once
#include "typedef.h"
#include "cpu.h"
#include "8253.h"
#include "8259.h"
#include "Keyboard.h"
#include "page_frame.h"
#include "rtc.h"
#include "gdt.h"
#include "idt.h"
#include "tss.h"
#include "mmu.h"
#include "kernel.h"
#include "bios.h"
#include "process.h"

#define MAX_CPU_NUM					32

class SYSTEM //: public Object
{
private:
	uint32 m_kernel_image_size;
	uint32 m_ram_size;
	memory_info m_meminfo;
public:
	friend KBD;
	friend PAGE_FRAME_DB;
private:
	GDT    m_gdt;
	IDT    m_idt;
	TSS    m_tss;
	RTC    m_rtc;

	PROCESS m_kproc;
public:
	uint32				m_cpu_count;
	CPU*				m_cpus[MAX_CPU_NUM];
	MMU<KERNEL_BASE, GB(2), PAGE_SIZE>   m_kmem;
private:
	bool   init_kernel_mmu();
public:
	SYSTEM();
	~SYSTEM();
	bool Init(uint32 kernel_image_size, memory_info* meminfo);

};

extern  SYSTEM System;

