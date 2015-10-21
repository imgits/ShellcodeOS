#pragma once
#include <Windows.h>
#include "typedef.h"

class PE
{
	PIMAGE_DOS_HEADER m_image_base;
	PIMAGE_DOS_HEADER m_dos_hdr;
	PIMAGE_NT_HEADERS m_nt_hdr;
public:
	PE(void* pe_base=NULL);
	~PE();
	bool Init(void* pe_base);
	uint32	ImageSize();
	byte*	EntryPoint();

};

