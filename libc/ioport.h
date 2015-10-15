#pragma once
#include "typedef.h"

static byte inportb(uint16 port)
{
	__asm
	{
		push		edx
			mov		dx, port
			xor		eax, eax
			in			al, dx
			pop		edx
	}
}

static void outportb(uint16 port, byte val)
{
	__asm
	{
		push		edx
			mov		dx, port
			mov		al, val
			out		dx, al
			pop		edx
			//pop eax
			//ret
	}
}