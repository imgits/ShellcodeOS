#pragma once
#include "typedef.h"
#include "ioport.h"

#define		VGA_BUFFER_ADDRESS (0x000b8000) 
#define		VGA_WIDTH		   80
#define		VGA_HEIGHT		   25

#define	VGA_BLACK							0
#define	VGA_BLUE							1
#define	VGA_GREEN							2
#define	VGA_CYAN							3
#define	VGA_RED								4
#define	VGA_MAGENTA					5
#define	VGA_BROWN						6
#define	VGA_LIGHT_GRAY				7
#define	VGA_DARK_GRAY					8
#define	VGA_LIGHT_BLUE					9
#define	VGA_LIGHT_GREEN				10
#define	VGA_LIGHT_CYAN				11
#define	VGA_LIGHT_RED					12
#define	VGA_LIGHT_MAGENTA		13
#define	VGA_LIGHT_BROWN			14
#define	VGA_WHITE							15

class VIDEO
{
private:
	static uint32 m_cursor_x;
	static uint32 m_cursor_y;
	static uint32 m_screen_width;
	static uint32 m_screen_height;
	static uint16 m_fore_color;
	static uint16 m_back_color;
	static uint16* m_video_buf;
public:
	static void set_video_buf(uint32 video_buf)
	{
		m_video_buf = (uint16*)video_buf;
	}

	static void set_color(byte fore_color, byte back_color) { m_fore_color = fore_color; m_back_color = back_color; }
	static void set_fore_color(byte color) { m_fore_color = color; }
	static void set_back_color(byte color) { m_back_color = color; }

	static void gotoxy(int x, int y)
	{
		//以下取当前光标位置
		uint32 offset = y*m_screen_width + x;
		byte    byte0 = offset & 0xff;
		byte    byte1 = (offset >> 8) & 0xff;

		outportb(0x3d4, 0x0f); //索引寄存器，选择光标位置低位寄存器
		outportb(0x3d5, byte0);
		outportb(0x3d4, 0x0e);
		outportb(0x3d5, byte1);
	}

	static int getx()
	{
		outportb(0x3d4, 0x0f); //索引寄存器，选择光标位置低位寄存器
		uint32 byte0 = inportb(0x3d5); //取出光标位置低位字节
		outportb(0x3d4, 0x0e); //索引寄存器，选择光标位置高位寄存器
		uint32 byte1 = inportb(0x3d5); //取出光标位置高位字节
		uint32 offset = (byte1 << 8) + byte0;
		uint32  x = offset%m_screen_width;
		return	x;
	}

	static int gety()
	{
		outportb(0x3d4, 0x0f); //索引寄存器，选择光标位置低位寄存器
		uint32 byte0 = inportb(0x3d5); //取出光标位置低位字节
		outportb(0x3d4, 0x0e); //索引寄存器，选择光标位置高位寄存器
		uint32 byte1 = inportb(0x3d5); //取出光标位置高位字节
		uint32 offset = (byte1 << 8) + byte0;
		uint32  y = offset / m_screen_width;
		return y;
	}

	static void clear()
	{
		for (int i = 0; i < m_screen_width * m_screen_height; i++)
		{
			m_video_buf[i] = (((uint16)((m_back_color << 4) | m_fore_color)) << 8) | 0x20;
		}
		gotoxy(0, 0);
	}

	static void screen_scroll(int lines)
	{
		char* vag_start = (char*)m_video_buf;
		char* start_line = vag_start + lines* m_screen_width * 2;
		int     scroll_lines = m_screen_height - lines;
		int     scroll_size = scroll_lines*m_screen_width * 2;
		for (int i = 0; i < scroll_size; i++) vag_start[i] = start_line[i];
		uint16* start_fill = (uint16*)(vag_start + (m_screen_height - lines)*m_screen_width * 2);
		for (int i = 0; i < lines* m_screen_width; i++)
		{
			start_fill[i] = 0x0720;
		}
	}

	static void putc(int x, int y, char ch, char fore, char back)
	{
		if (ch == 0x08)
		{
			if (x > 0) x--;
			byte* addr = (byte*)m_video_buf + (y*m_screen_width + x) * 2;
			*addr++ = ' ';
			*addr = (back << 4) | fore;
			gotoxy(x, y);
			return;
		}
		else if (ch == 0x0d)
		{
			x = 0;
			gotoxy(x, y);
			return;
		}
		else if (ch == 0x0a)
		{
			if (++y >= m_screen_height)
			{
				screen_scroll(1);
				--y;
			}
			x = 0;
			gotoxy(x, y);
			return;
		}
		if (x >= m_screen_width - 1)
		{
			if (y >= m_screen_height - 1)
			{
				screen_scroll(1);
			}
			else y++;
			x = 0;
		}

		byte* addr = (byte*)m_video_buf + (y*m_screen_width + x) * 2;
		*addr++ = ch;
		*addr = (back << 4) | fore;
		x++;
		if (x >= m_screen_width - 1)
		{
			if (y >= m_screen_height - 1)
			{
				screen_scroll(1);
			}
			else y++;
			x = 0;
		}
		gotoxy(x, y);
	}

};

uint16* VIDEO::m_video_buf = (uint16*)VGA_BUFFER_ADDRESS;
uint32  VIDEO::m_cursor_x=0;
uint32  VIDEO::m_cursor_y=0;
uint16	VIDEO::m_fore_color= VGA_LIGHT_GRAY;
uint16	VIDEO::m_back_color= VGA_BLACK;
uint32	VIDEO::m_screen_width = VGA_WIDTH;
uint32	VIDEO::m_screen_height=VGA_HEIGHT;

void init_vga(void* vga_buffer);
int  getx();
int  gety();
void gotoxy(int x, int y);
void clr(char fore = VGA_LIGHT_GRAY, char back = VGA_BLACK);
void screen_scroll(int lines);
void putc(char ch, char fore = VGA_LIGHT_GRAY, char back = VGA_BLACK);
void putchar(int x, int y, char ch, char fore = VGA_LIGHT_GRAY, char back = VGA_BLACK);
void puts(const char* str, char fore = VGA_LIGHT_RED, char back = VGA_BLACK);
void setchar(int x, int y, char ch, char fore, char back);


