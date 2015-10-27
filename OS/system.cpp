#include "system.h"
#include <string.h>
#include "page_frame.h"
#include "trap.h"

SYSTEM  System;

SYSTEM::SYSTEM()
{
}

SYSTEM::~SYSTEM()
{
}

bool SYSTEM::Init(uint32 page_frame_min, uint32 page_frame_max)
{
	PAGE_FRAME_DB::Init(page_frame_min, page_frame_max);
	uint32 CR3 = __readcr3();
	m_gdt.Init();
	m_idt.Init();
	m_tss.Init(CR3, &m_gdt);
	TRAP::Init(&m_idt);
	PIC::Init(&m_idt);
	PIT::Init();
	RTC::Init();
	KBD::Init();
	return false;
}