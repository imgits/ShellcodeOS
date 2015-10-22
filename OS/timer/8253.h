//Programmable Interval Timer
//http://wiki.osdev.org/PIT
//"8042" PS/2 Controller:http://wiki.osdev.org/%228042%22_PS/2_Controller

#pragma once
#include "8259.h"

#define	MAX_PIT_HANDLERS		100
#define	PIT_CLOCK			1193180
#define	TIMER_FREQ			20	

typedef bool(*TIMER_HANDLER)(uint32 timer_id);

struct PIT_HANDLER
{
	uint32			id;
	uint32			period;
	uint32          counter;
	TIMER_HANDLER	handler;
};

class PIT
{
private:
	static uint32       m_counter;
	static uint64       m_timer;
	static PIT_HANDLER  m_handlers[MAX_PIT_HANDLERS];
private:
	static void	irq_handler(PIC_IRQ_CONTEXT* context);
public:
	static bool Init();
	static bool set_timer(uint32 id, uint32 period, TIMER_HANDLER handler);
	static bool kill_timer(uint32 id);
};

