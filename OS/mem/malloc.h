#pragma once
#include "typedef.h"

class MALLOC
{
public:
	MALLOC();
	~MALLOC();

	void* malloc(int size);
};

