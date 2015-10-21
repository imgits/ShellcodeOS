#include "Keyboard.h"

Keyboard g_keyboard;

Keyboard::Keyboard()
{
}

Keyboard::~Keyboard()
{
}

void Keyboard::Init(PIC* pic)
{
	pic->register_irq(IRQ_KEYBOARD, Keyboard::kbd_irq_handler);
}

void Keyboard::kbd_irq_handler(IRQ_CONTEXT* context)
{
	g_keyboard.irq_handler(context);
}

void Keyboard::irq_handler(IRQ_CONTEXT* context)
{

}
