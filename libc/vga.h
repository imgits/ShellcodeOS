#pragma once
#include "typedef.h"

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


