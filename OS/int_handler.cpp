#include "idt.h"
#include "gdt.h"
#include "stdio.h"
#include <string.h>
#include "intrin.h"
#include "int_handler.h"

static void  __declspec(naked) INT_DISPATCH()
{
	__asm pushad
	__asm push ds
	__asm push es
	__asm push fs
	__asm push gs
	__asm mov  ax, 0x10
	__asm mov  ds, ax
	__asm mov  es, ax
	__asm mov  fs, ax
	__asm mov  gs, ax
	__asm push esp
	__asm call INT_HANDLER::dispatch
	__asm add  esp, 4
	__asm pop  gs
	__asm pop  fs
	__asm pop  es
	__asm pop  ds
	__asm popad
	__asm add  esp, 12
	__asm iret
}

static void  __declspec(naked) INT_ENTRY()
{
	__asm cli
	__asm push 0x12345678  //error_code
	__asm push 0x12345678  //int_no
	__asm push 0x12345678  //int_handler
	__asm push INT_DISPATCH//int_dispatch
	__asm ret
}

/*
INT_ENTRY:
012013C0 FA                   cli
012013C1 68 78 56 34 12       push        12345678h
012013C6 68 78 56 34 12       push        12345678h
012013CB 68 78 56 34 12       push        12345678h
012013D0 C3                   ret
*/

INT_HANDLER::INT_HANDLER()
{
	memcpy(&m_int_stub, INT_ENTRY, sizeof(m_int_stub));
	m_int_stub.push_int_handler.push32_oprand = (uint32)this;
}

void INT_HANDLER::dispatch(INT_CONTEXT* context)
{
	INT_HANDLER* int_handler = context->int_handler;
	int_handler->handler(context);
}

void INT_HANDLER::handler(INT_CONTEXT* context)
{

}
