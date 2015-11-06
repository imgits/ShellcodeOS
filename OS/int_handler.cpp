#include "idt.h"
#include "gdt.h"
#include "stdio.h"
#include <string.h>
#include "intrin.h"
#include "int_handler.h"
#include "c++.h"
#include "8259.h"

static void  __declspec(naked) INT_DISPATCH()
{
	__asm cld
	__asm pushad
	__asm push ds
	__asm push es
	__asm push fs
	__asm push gs
	__asm mov  ax, SEL_KERNEL_DATA
	__asm mov  ds, ax
	__asm mov  es, ax
	__asm mov  fs, ax
	__asm mov  gs, ax
	__asm push esp
	__asm call INT_HANDLER::int_dispatch //dispatch
	__asm add  esp, 4
	__asm pop  gs
	__asm pop  fs
	__asm pop  es
	__asm pop  ds
	__asm popad
	__asm add  esp, 12
	__asm sti
	__asm iret
}

static void  __declspec(naked) INT_ENTRY()
{
	/*FA             */__asm      cli
	/*68 78 56 34 12 */__asm      push        12345678h//error_code 
	/*68 78 56 34 12 */__asm      push        12345678h//int_no
	/*68 78 56 34 12 */__asm      push        12345678h//int_handler_object
	/*68 78 56 34 12 */__asm	  push		  INT_DISPATCH//int_dispatch 
	/*C3             */__asm	  ret
}

INT_HANDLER::INT_HANDLER()
{
	memcpy(&m_int_entry_stub, INT_ENTRY, sizeof(m_int_entry_stub));
	m_int_entry_stub.push_int_handler.push32_oprand = (uint32)this;
}

void	INT_HANDLER::Register(IDT* idt, int int_no, bool has_error_code)
{
	if (has_error_code)
	{
		m_int_entry_stub.push_error_code.push32_opcode = 0x90;
		m_int_entry_stub.push_error_code.push32_oprand = 0x90909090;
	}
	m_int_entry_stub.push_int_no.push32_oprand = int_no;
	
	idt->set_intr_gate(int_no, &m_int_entry_stub); //因为有虚函数，所以 &m_int_entry_stub !=this

	if (int_no >= PIC_INT_FIRST && int_no <= PIC_INT_LAST)
	{
		PIC::enable_irq(int_no - PIC_INT_FIRST);
	}

}

void INT_HANDLER::int_dispatch(INT_CONTEXT* context)
{
	context->handler_obj->int_handler(context);
	if (context->int_no >= PIC_INT_FIRST && context->int_no <= PIC_INT_LAST)
	{
		//发送中断结束命令EOI（0x20)
		outportb(PIC1_CMD_PORT, 0x20);//向主片发送中断结束命令
		outportb(PIC2_CMD_PORT, 0x20);//向从片发送中断结束命令
	}
}

