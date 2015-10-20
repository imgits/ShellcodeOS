//Programmable Interval Timer
//http://wiki.osdev.org/PIT

#pragma once
#include "8259.h"

#define MAX_PIT_HANDLERS		100
#define PIT_CLOCK				1193180
#define	TIMER_FREQ				20	

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
	PIC*         m_pic;
	uint32       m_counter;
	uint64       m_timer;
	PIT_HANDLER  m_handlers[MAX_PIT_HANDLERS];
private:
	static void	pit_irq_handler(INTERRUPT_CONTEXT* context);
	void	irq_handler(INTERRUPT_CONTEXT* context);
public:
	PIT();
	~PIT();
	bool Init(PIC* pic);
	bool register_timer(uint32 id, uint32 period, TIMER_HANDLER handler);
};

