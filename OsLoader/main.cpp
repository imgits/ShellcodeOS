#include <Windows.h>
//#include <stdio.h>
#include "typedef.h"
#include "ioport.h"
#include "vga.h"
#include "page.h"
#include "C++.h"

CPager pager;

int main()
{
	CppInit();
	puts("\nHello world\n", 10);
	puts("OsLoader.exe is starting...\n", 10);
	__asm jmp $
}

