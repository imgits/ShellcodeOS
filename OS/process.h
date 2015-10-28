#pragma once
#include "typedef.h"

#define PROCESS_NAME_SIZE		256

class PROCESS
{
private:
	uint32 m_pid;
	uint32 m_cr3;
	char   m_name[PROCESS_NAME_SIZE];
public:
	PROCESS();
	~PROCESS();
};

