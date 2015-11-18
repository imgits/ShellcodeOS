//http://wiki.osdev.org/RTC
//Real Time Clock and Memory: http://stanislavs.org/helppc/cmos_ram.html
#pragma once
#include "8259.h"
#include "int_handler.h"

class RTC : public INT_HANDLER
{
private:
	uint64 m_clock_ticks;
public:
	RTC();
	static	bool	Init(IDT *idt);
	virtual void	int_handler(INT_CONTEXT* context);

private:
	bool			_Init(IDT *idt);

};

