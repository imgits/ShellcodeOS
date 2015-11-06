//Programmable Interrupt Controller 
//http://wiki.osdev.org/PIC

#pragma once
#include "IDT.h"
#include "ioport.h"

#define	PIC1_INT0				0x20
#define	PIC2_INT0				0x28

#define PIC_INT_FIRST			0x20
#define PIC_INT_LAST			0x2F

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

struct PIC_IRQ_CONTEXT
{
	uint32	edi;
	uint32	esi;
	uint32	ebp;
	uint32	tmp;
	uint32	ebx;
	uint32	edx;
	uint32	ecx;
	uint32	eax;
	uint32	int_id;
	uint32	eip;
	uint32	cs;
	uint32  eflags;
	uint32  esp;
	uint32  ss;
};

typedef void(*PIC_IRQ_HANDLER)(PIC_IRQ_CONTEXT* context);

class PIC
{
private:
	static PIC_IRQ_HANDLER m_irq_handlers[16];
public:
	static void		Init(IDT* idt);
	static void		enable_irq(int irq);
	static void		disable_irq(int irq);
	static void		irq_dispatch(PIC_IRQ_CONTEXT* context);
	static bool		register_irq(int irq_no, PIC_IRQ_HANDLER irq_handler);
	static void		dump_pic_irq_context(PIC_IRQ_CONTEXT* context);
};
