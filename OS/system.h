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

#define MAX_CPU_NUM					32

//#define SYSTEM_OBJECT_BASE			0xBFFFE000
struct memory_info;

class SYSTEM //: public Object
{
private:
	uint32 m_kernel_image_size;
	uint32 m_mem_size;
	memory_info m_meminfo;
public:
	friend KBD;
	friend PAGE_FRAME_DB;
private:
	GDT    m_gdt;
	IDT    m_idt;
	TSS    m_tss;
public:
	uint32			m_cpu_count;
	CPU*				m_cpus[MAX_CPU_NUM];
	MMU<KERNEL_BASE, GB(2), PAGE_SIZE>   m_kmem;
private:
	uint32 get_mem_size();
	bool   map_mem_space();
private:
	bool   init_kernel_mem();
public:
	SYSTEM();
	~SYSTEM();
	bool Init(uint32 kernel_image_size, memory_info* meminfo);

};

extern  SYSTEM System;

