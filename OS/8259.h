//Programmable Interrupt Controller 
//http://wiki.osdev.org/PIC

#pragma once
#include "IDT.h"
#include "ioport.h"

#define	PIC1_INT0			0x20
#define	PIC2_INT0			0x28

#define PIC1_CMD_PORT			0x20
#define PIC1_DATA_PORT			0x21

#define PIC2_CMD_PORT			0xA0
#define PIC2_DATA_PORT			0xA1

#define	IRQ_PIT					0
#define	IRQ_KEYBOARD			1
#define	IRQ_SLAVE				2
#define	IRQ_COM1				3
#define	IRQ_COM2				4
#define	IRQ_PRINT0				5
#define	IRQ_FLOPY				6
#define	IRQ_PRINT1				7

#define	IRQ_RTC					8
#define	IRQ_GENERIC1			9
#define	IRQ_GENERIC2			10
#define	IRQ_GENERIC3			11
#define	IRQ_MOUSE				12
#define	IRQ_INTERNAL			13
#define	IRQ_STAT0				14
#define	IRQ_STAT1				15

struct INTERRUPT_CONTEXT
{
	uint32  eflags;
	uint32	cs;
	uint32	eip;
	uint32	int_no;
	uint32	eax;
	uint32	ecx;
	uint32	edx;
	uint32	ebx;
	uint32	esp;
	uint32	ebp;
	uint32	esi;
	uint32	edi;
};

typedef void(*IRQ_HANDLER)(INTERRUPT_CONTEXT* context);

class PIC
{
public:
	IRQ_HANDLER m_irq_handlers[16];
public:
	PIC();
	~PIC();

	void	Init(IDT* idt);
	void	enable_irq(int irq);
	void	disable_irq(int irq);

	bool	register_irq(int irq_no, IRQ_HANDLER irq_handler);

};


