#pragma once
#include <Windows.h>
#include "typedef.h"

class PE
{
	PIMAGE_DOS_HEADER m_image_base;
public:
	PE(void* pe_base=NULL);
	~PE();
	bool Init(void* pe_base);

	byte* EntryPoint();

};

