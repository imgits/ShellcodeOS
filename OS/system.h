#pragma once
#include "typedef.h"
#include "cpu.h"
#include "8253.h"
#include "8259.h"
#include "Keyboard.h"
#include "rtc.h"

#define MAX_CPU_NUM	32
class System
{
//private:
public:
	uint32 m_cpu_count;
	CPU*   m_cpus[MAX_CPU_NUM];
	PIT	   m_pit;
	PIC	   m_pic;
	RTC    m_rtc;
	Keyboard m_kbd;
public:
	System();
	~System();
};

