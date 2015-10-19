#pragma once
#include "typedef.h"

#define KBD_DATA_PORT   0x60

class Keyboard
{
public:
	Keyboard();
	~Keyboard();
	void Init();
};

