#pragma once
#include <stdarg.h>
#include "typedef.h"

#define		VGA_START_ADDR		(0x000b8000) 
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

void gotoxy(uint32 x, uint32 y);
uint32 getx();
uint32 gety();
void clr(char fore = VGA_LIGHT_GRAY, char back = VGA_BLACK);
void screen_scroll(int lines);
void put_c(char ch, char fore = VGA_LIGHT_GRAY, char back = VGA_BLACK);
void put_char(int x, int y, char ch, char fore = VGA_LIGHT_GRAY, char back = VGA_BLACK);
void put_s(const char* str, char fore = VGA_LIGHT_RED, char back = VGA_BLACK);
void put_hex(uint32 hval);
void put_hex64(uint64 hval);

int     printf(const char *fmt, ...);
int     sprintf(char  *buf, const char *fmt, ...);
int		vsprintf(char *buf, const char *fmt, va_list args);

byte inportb(uint16 port);
void outportb(uint16 port, byte val);


