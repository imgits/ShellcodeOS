#include "pe.h"


PE::PE(void* pe_base)
{
	Init(pe_base);
}

PE::~PE()
{

}

bool PE::Init(void* pe_base)
{
	m_image_base = (PIMAGE_DOS_HEADER)pe_base;
	return true;
}

byte* PE::EntryPoint()
{
	PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER)m_image_base;
	PIMAGE_NT_HEADERS pe_hdr = (PIMAGE_NT_HEADERS)((char*)m_image_base + dos_hdr->e_lfanew);
	uint32 _EntryPoint = pe_hdr->OptionalHeader.ImageBase + pe_hdr->OptionalHeader.AddressOfEntryPoint;
	return (byte*)_EntryPoint;
}