//ported from linux
//ported from SGOS1

#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "typedef.h"
#include "vga.h"
#include "ioport.h"

void * video_frame_buf = (void*)VGA_BUFFER_ADDRESS;

void init_vga(void* vga_buffer)
{
	video_frame_buf = vga_buffer;
}

void gotoxy(int x, int y)
{
	//以下取当前光标位置
	uint32 offset = y*VGA_WIDTH + x;
	byte    byte0 = offset & 0xff;
	byte    byte1 = (offset >> 8) & 0xff;

	outportb(0x3d4, 0x0f); //索引寄存器，选择光标位置低位寄存器
	outportb(0x3d5, byte0);
	outportb(0x3d4, 0x0e);
	outportb(0x3d5, byte1);
}

int getx()
{
	outportb(0x3d4, 0x0f); //索引寄存器，选择光标位置低位寄存器
	uint32 byte0 = inportb(0x3d5); //取出光标位置低位字节
	outportb(0x3d4, 0x0e); //索引寄存器，选择光标位置高位寄存器
	uint32 byte1 = inportb(0x3d5); //取出光标位置高位字节
	uint32 offset = (byte1 << 8) + byte0;
	uint32  x = offset%VGA_WIDTH;
	return	x;
}

int gety()
{
	outportb(0x3d4, 0x0f); //索引寄存器，选择光标位置低位寄存器
	uint32 byte0 = inportb(0x3d5); //取出光标位置低位字节
	outportb(0x3d4, 0x0e); //索引寄存器，选择光标位置高位寄存器
	uint32 byte1 = inportb(0x3d5); //取出光标位置高位字节
	uint32 offset = (byte1 << 8) + byte0;
	uint32  y = offset / VGA_WIDTH;
	return y;
}

void clr(char fore, char back)
{
	uint16* vga_mem = (uint16*)video_frame_buf;
	for (int i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++)
	{
		vga_mem[i] = (((uint16)((back << 4) | fore)) << 8) | 0x20;
	}
	gotoxy(0, 0);
}

void screen_scroll(int lines)
{
	char* vag_start = (char*)video_frame_buf;
	char* start_line = vag_start + lines* VGA_WIDTH * 2;
	int     scroll_lines = VGA_HEIGHT - lines;
	int     scroll_size = scroll_lines*VGA_WIDTH * 2;
	for (int i = 0; i < scroll_size; i++) vag_start[i] = start_line[i];
	uint16* start_fill = (uint16*)(vag_start + (VGA_HEIGHT - lines)*VGA_WIDTH * 2);
	for (int i = 0; i < lines* VGA_WIDTH; i++)
	{
		start_fill[i] = 0x0720;
	}
}

void putchar(int x, int y, char ch, char fore, char back)
{
	if (ch == 0x08)
	{
		if (x > 0) x--;
		ch = ' ';
	}
	else if (ch == 0x0d)
	{
		x = 0;
		gotoxy(x, y);
		return;
	}
	else if (ch == 0x0a)
	{
		if (++y >= VGA_HEIGHT)
		{
			screen_scroll(1);
			--y;
		}
		x = 0;
		gotoxy(x, y);
		return;
	}
	if (x >= VGA_WIDTH - 1)
	{
		if (y >= VGA_HEIGHT - 1)
		{
			screen_scroll(1);
		}
		else y++;
		x = 0;
	}

	byte* addr = (byte*)video_frame_buf + (y*VGA_WIDTH + x) * 2;
	*addr++ = ch;
	*addr = (back << 4) | fore;
	x++;
	if (x >= VGA_WIDTH - 1)
	{
		if (y >= VGA_HEIGHT - 1)
		{
			screen_scroll(1);
		}
		else y++;
		x = 0;
	}
	gotoxy(x, y);
}

void putc(char ch, char fore, char back)
{
	int x = getx();
	int y = gety();
	putchar(x, y, ch, fore, back);
}

//字符串显示例程（适用于平坦内存模型） 
void puts(const char* str, char fore, char back)
{
	int len = strlen(str);
	if (len == 0) len = 13;
	for (int i = 0; i < len; i++) putc(str[i], fore, back);
}

void setchar(int x, int y, char ch, char fore, char back)
{
	if (ch > 0x20 &&  x >=0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT)
	{
		byte* addr = (byte*)video_frame_buf + (y*VGA_WIDTH + x) * 2;
		*addr++ = ch;
		*addr = (back << 4) | fore;
	}
}

#include "stdio.h"
//extern "C" int __cdecl printf(const char *fmt, ...);
//extern "C" int __cdecl sprintf(char *buf, const char *fmt, ...);
//extern "C" int __cdecl vsprintf(char *buf, const char *fmt, va_list args);
extern "C" int __cdecl printf(const char *fmt, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	int len = 0;
	len = vsprintf(buf, fmt, args);
	va_end(args);
	if (len > sizeof(buf)) len = sizeof(buf) - 1;
	buf[len] = 0;
	puts(buf, 15, 0);
	return len;
}

