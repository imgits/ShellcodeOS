#pragma once
#include "IDT.h"
#include "ioport.h"

#define	_8259A_MASTER_INTERRUPT_VECTOR0		0x20
#define	_8259A_SLAVE_INTERRUPT_VECTOR0		0x28

#define	IRQ_8253_COUNTER		_8259A_MASTER_INTERRUPT_VECTOR0 + 0
#define	IRQ_KEYBOARD			_8259A_MASTER_INTERRUPT_VECTOR0 + 1

#define HARDWARE_INTERRUPT void  __declspec(naked) 

class PIC
{
public:
	PIC();
	~PIC();

	void Init(IDT* idt);

	void enable_irq(int irq);
	void disable_irq(int irq);

private:
	void init_cmos_timer();
	void init_8253_counter(int freq);
};


