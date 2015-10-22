//http://wiki.osdev.org/RTC
//Real Time Clock and Memory: http://stanislavs.org/helppc/cmos_ram.html
#pragma once
#include "idt.h"
#include "8259.h"

class RTC
{
private:
	static uint64 m_tick_count;
public:
	static bool Init();
	static void	irq_handler(PIC_IRQ_CONTEXT* context);
};

