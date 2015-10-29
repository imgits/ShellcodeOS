#pragma once
#include "typedef.h"

#define PROCESS_NAME_SIZE		256
#define KERNEL_STACK_SIZE		(1024*32)
class PROCESS
{
private:
	uint32 m_pid;
	uint32 m_cr3;
	char   m_name[PROCESS_NAME_SIZE];
	byte   m_kernel_stack[KERNEL_STACK_SIZE];
public:
	PROCESS* prev;
	PROCESS* next;
public:
	PROCESS();
	~PROCESS();
};

