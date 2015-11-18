#pragma once
#include "typedef.h"

static uint32 inportb(uint16 port)
{
	__asm
	{
		push		edx
			mov		dx, port
			xor		eax, eax
			in		al, dx
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

static uint32 inportw(uint16 port)
{
	__asm
	{
		push		edx
		mov		dx, port
			xor		eax, eax
			in		ax, dx
			pop		edx
	}
}

static void outportw(uint16 port, uint16 val)
{
	__asm
	{
		push		edx
		mov		dx, port
			mov		ax, val
			out		dx, ax
			pop		edx
			//pop eax
			//ret
	}
}


static uint32 inportd(uint16 port)
{
	__asm
	{
		push		edx
		mov		dx, port
			in		eax, dx
			pop		edx
	}
}

static void outportd(uint16 port, uint32 val)
{
	__asm
	{
		push		edx
		mov		dx, port
			mov		eax, val
			out		dx, eax
			pop		edx
			//pop eax
			//ret
	}
}