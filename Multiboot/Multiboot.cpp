#include <stdint.h>
#include <stdarg.h>
#include "multiboot.h"

#define KERNEL_STACK	0x00110000

int		print(char* str);
int		printf(const char *fmt, ...);
void	main(unsigned long magic, multiboot_info* boot_info);
extern "C" int sprintf(char *buf, const char *fmt, ...);
extern "C" int vsprintf(char *buf, const char *fmt, va_list args);

__declspec(naked) void multiboot_entry(void)
{
	__asm
	{
		mov    esp, KERNEL_STACK
		xor    ecx, ecx
		push   ecx
		popfd

		push   ebx
		push   eax
		call   main
		jmp    $
	}
}

void main(unsigned long magic, multiboot_info* boot_info)
{
	int size = sizeof(multiboot_info);
	char *string = "Hello World!", *ch;
	printf("magic=%08X boot_info =%08X ", magic, boot_info);
	if (boot_info->flags & 0x01)
	{
		printf("mem_lower=%08X mem_upper=%08X ", boot_info->mem_lower, boot_info->mem_upper);
	}
}

int printf(const char *fmt, ...)
{
	char buf[1024];
	va_list args;
	int n;
	va_start(args, fmt);
	n = vsprintf(buf, fmt, args);
	va_end(args);
	print(buf);
	return n;
}

int print(char* str)
{
	uint16_t *video_frame = (uint16_t*)0xB8000;
	while (*str) *video_frame++ = 0x0700 + *str++;
	return 0;
}
