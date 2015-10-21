#pragma once
#include "typedef.h"
#include "intrin.h"
#include "8259.h"

#define KBD_DATA_PORT   0x60

class Keyboard
{
private:
	static void kbd_irq_handler(IRQ_CONTEXT* context);
	void irq_handler(IRQ_CONTEXT* context);
public:
	Keyboard();
	~Keyboard();
	void Init(PIC* pic);
};

extern class Keyboard g_keyboard;


