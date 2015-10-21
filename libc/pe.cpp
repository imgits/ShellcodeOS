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
	m_dos_hdr = m_image_base;
	m_nt_hdr = (PIMAGE_NT_HEADERS)((char*)m_image_base + m_dos_hdr->e_lfanew);
	return true;
}

uint32	PE::ImageSize()
{
	return m_nt_hdr->OptionalHeader.SizeOfImage;
}

byte* PE::EntryPoint()
{
	uint32 _EntryPoint = m_nt_hdr->OptionalHeader.ImageBase + m_nt_hdr->OptionalHeader.AddressOfEntryPoint;
	return (byte*)_EntryPoint;
}