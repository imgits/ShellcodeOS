#pragma once
#include "typedef.h"
#include "cpu.h"
#include "8253.h"
#include "8259.h"
#include "Keyboard.h"
#include "rtc.h"
#include "page_frame.h"

#define MAX_CPU_NUM				32
#define SYSTEM_OBJECT_ADDRESS		0xBFFFE000

class SYSTEM
{
	friend KBD;
	friend PAGE_FRAME_DB;
private:
	KBD			 m_kbd;
public:
	uint32 m_cpu_count;
	CPU*   m_cpus[MAX_CPU_NUM];
	PIT	   m_pit;
	PIC	   m_pic;
	RTC    m_rtc;
	PAGE_FRAME_DB m_page_frame_db;
public:
	SYSTEM();
	~SYSTEM();
};

extern SYSTEM* SystemObject;
#define System (*SystemObject)

