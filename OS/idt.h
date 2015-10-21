#pragma once
#include  "typedef.h"

#pragma pack(push, 1)
// 门描述符
typedef struct GATE_DESC
{
	uint16		offset_low;
	uint16		selector;
	uint8		dcount;
	uint8		attr;
	uint16		offset_high;
}GATE_DESC;

struct IDTR
{
	uint16 limit;
	void*  base;
};

#pragma pack(pop)

#define		MAX_IDT_NUM		256

#define	IDT_TSS_GATE		0x89	/* 可用 386 任务状态段类型值		*/
#define	IDT_CALL_GATE	0x8C	/* 386 调用门类型值			*/
#define	IDT_INTR_GATE	0x8E	/* 386 中断门类型值			*/
#define	IDT_TRAP_GATE	0x8F	/* 386 陷阱门类型值			*/

struct IRQ_CONTEXT
{
	uint32  eflags;
	uint32	cs;
	uint32	eip;
	uint32	error_code;
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

typedef void(*IRQ_HANDLER)(IRQ_CONTEXT* context);

class IDT
{
private:
	GATE_DESC	m_idt[MAX_IDT_NUM];	//	256 gdt items
	IRQ_HANDLER  m_irq_handlers[MAX_IDT_NUM];
private:	
	void set_idt_entry(int vector, uchar desc_type, void* handler);
public:
	IDT(void);
	~IDT(void);
	void Init();
	void set_tss_gate(int vector,  IRQ_HANDLER* handler);
	void set_call_gate(int vector, IRQ_HANDLER* handler);
	void set_intr_gate(int vector, IRQ_HANDLER* handler);
	void set_trap_gate(int vector, IRQ_HANDLER* handler);
	void irq_dispatch(IRQ_CONTEXT* context);
};

extern IDT g_idt;