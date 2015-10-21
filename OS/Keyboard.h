#pragma once
#include "typedef.h"
#include "intrin.h"
#include "8259.h"

#define KBD_DATA        0x60    // I/O port for keyboard data
#define KBD_COMMAND     0x64    // I/O port for keyboard commands (write)
#define KBD_STATUS      0x64    // I/O port for keyboard status (read)

//
// Keyboard Status Register Bits
//

#define KBD_STAT_OBF            0x01    // Keyboard output buffer full
#define KBD_STAT_IBF            0x02    // Keyboard input buffer full
#define KBD_STAT_SELFTEST       0x04    // Self test successful
#define KBD_STAT_CMD            0x08    // Last write was a command write
#define KBD_STAT_UNLOCKED       0x10    // Zero if keyboard locked
#define KBD_STAT_MOUSE_OBF      0x20    // Mouse output buffer full
#define KBD_STAT_GTO            0x40    // General receive/xmit timeout
#define KBD_STAT_PERR           0x80    // Parity error

class Keyboard
{
private:
	static byte led_status;
	static byte control_keys;
	static int ext;
private:
	static void   irq_handler(PIC_IRQ_CONTEXT* context);
	static uint32 decode(byte scancode);
public:
	static void Init();
	
};



