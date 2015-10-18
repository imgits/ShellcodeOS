#pragma once
struct TSS 
{
	unsigned long link;
	unsigned long esp0, ss0;
	unsigned long esp1, ss1;
	unsigned long esp2, ss2;
	unsigned long cr3;
	unsigned long eip;
	unsigned long eflags;
	unsigned long eax, ecx, edx, ebx, esp, ebp;
	unsigned long esi, edi;
	unsigned long es, cs, ss, ds, fs, gs;
	unsigned long ldt;
	unsigned long iomap;
};
