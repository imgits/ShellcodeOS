         ;代码清单17-1
         ;文件名：c17_mbr.asm
         ;文件说明：硬盘主引导扇区代码 
         ;创建日期：2012-07-13 11:20        ;设置堆栈段和栈指针 
		 ;%include "E:/OS/ShellcodeOS/kernel/include/kernel.h"

%define  KERNEL_LOAD_SEGMENT				  0x1000   ;常数，内核加载的起始内存段地址 
%define  KERNEL_LOAD_ADDRESS				  0X10000;常数，内核加载的起始内存地址 

;%define  KERNEL_MOVE_ADDRESS			      0x100000	
;%define  KERNEL_START_ADDRESS			  0x11c50	

%define  		 ARDS_COUNT				  0x00007e00 ;将内存分布数据存放于此处,以便kernel_main中进行处理
%define  		 ARDS_BUF			  	  0x00007e04

; 0x00100000 +--------------------+
;            |      ROM           |
; 0x000C0000 +--------------------+
;            |                    |
; 0x000B8000 |     视频缓冲区     |
;            |                    |
; 0x000A0000 +--------------------+
;            |     ......         |
; 0x00090000 +--------------------+
;            |     ......         |
; 0x00033000 +--------------------+
;            |  PageTable[0x300]  |
; 0x00032000 +--------------------+
;            |  PageTable[0]      |
; 0x00031000 +--------------------+
;            |     PageDir        |
; 0x00030000 +--------------------+
;            |     OsLoader       |
; 0x00020000 +--------------------+
;            |     BootLoader     |
; 0x00010000 +--------------------+
;            |        ...         |
; 0x00007F00 +--------------------+
;            |     内存映射表     |
; 0x00007E00 +--------------------+
;            |     boot           |
; 0x00007C00 +--------------------+
;            |     堆栈           |
; 0x00000000 +--------------------+
;===============================================================================
        ORG     0x7c00
        BITS    16
        SECTION .text
; Entry point for initial bootstap code
boot:
          jmp     0:start ;5byte
bootdrv	  db      0
boot_loader_main:
          dd      0
;Offset	Size	Description
; 0	1	size of packet (16 bytes)
; 1	1	always 0
; 2	2	number of sectors to transfer (max 127 on some BIOSes)
; 4	4	-> transfer buffer (16 bit segment:16 bit offset) (see note #1)
; 8	4	starting LBA
;12	4	used for upper part of 48 bit LBAs
Disk_Address_Packet:
		db	10h; size of packet (16 bytes)
		db  00h; always 0
Kernel_Sectors:
		dw  7Fh; number of sectors to transfer
		dw  0000h ;transfer buffer offset
		dw  1000h ;transfer buffer segment
		dd  00000001h
		dd  00000000h
start:
        ; Setup initial environment
		cli
		cld
        mov     ax, cs
        mov     ds, ax
        mov		ss, ax
		mov		es, ax
        mov		esp, 0x7c00

        ; Save boot drive
        mov     [bootdrv], dl

		call		clear_screen

        mov     si, boot_msg
        call    print
		
;-----------------------------------------------------------------
;加载内核加载器osloader
		mov		si, Disk_Address_Packet		; address of "disk address packet"
		mov		ah, 0x42			; AL is unused
		mov		dl, [bootdrv]	; drive number 0 (OR the drive # with 0x80)
		int		0x13
		jnc		load_osloader_ok
load_osloader_failed:
		mov     si, load_osloader_failed_msg
        call    print
		jmp		$

load_osloader_ok:
		mov     si, load_osloader_ok_msg
        call    print
;-----------------------------------------------------------------
; 通过BIOS INT 15H 获取内存分布信息
		;ARDS_BUF的结构
		;uint32 ARDS_COUNT
		;struct RAM_ARDS // Address Range Descriptor Structure
		;{
		;	uint32 Size; 此字段是自己添加的,不需要
		;	uint64 BaseAddr;
		;	uint64 Length;
		;	uint32 Type;
		;}[];

get_memory_map:
		mov	ebx, 0                 ;ebx中存放后续值, 第一次调用时ebx 必须为 0;
		mov	edi,  ARDS_BUF;es:di 指向一段缓冲区，用来存放ARDS（Address Range Descriptor Structure），即内存分布描述符
		mov	dword [ARDS_COUNT], 0

next_mem_block:
		mov	eax, 0E820h
		mov	ecx, 64 ;ecx用来存放这个(ARDS)描述符的大小，以字节为单位。不过，通常情况下BIOS总是填充20字节的信息到ARDS中
		mov	edx, 0534D4150h ; 'SMAP' 中断要求
		int	15h
		;	CF位置位表明调用出错，否则无错 
		;	eax存放"SMAP" 的ASCII码
		;	es:di存放描述信息
		;	ecx存放ARDS描述符字节数
		;	bx存放下一地址描述符所需的后续值，如果ebx中值为0则说明已经到了最后一个地址范围描述符了。
		jc	LABEL_MEM_CHK_FAIL
		add	edi, ecx
		inc	dword [ARDS_COUNT]
		cmp	ebx, 0
		jne	next_mem_block
		jmp	LABEL_MEM_CHK_OK

LABEL_MEM_CHK_FAIL:
		mov     si, check_memory_failed_msg
        call    print
		jmp		$

LABEL_MEM_CHK_OK:
		mov     si, check_memory_ok_msg
        call    print
;-----------------------------------------------------------------
;打开A20
         in al,0x92                         ;南桥芯片内的端口 
         or al,0000_0010B
         out 0x92,al                        ;打开A20
;------------------------------------------------------------------
;初始化全局段描述符表，为进入保护模式作准备
		 cli
		 lgdt [gdtr]

;-------------------------------------------------------------------
;启动保护模式
         mov eax,cr0                  
         or  eax,1
         mov cr0,eax                        ;设置PE位
      
         ;以下进入保护模式... ...
         jmp dword 0x0008:flush             ;16位的描述符选择子：32位偏移
                                            ;清流水线并串行化处理器
         [bits 32]               
  flush:                                  
         mov eax,	0x00010                    ;加载数据段(4GB)选择子
         mov ds,		eax
         mov es,		eax
         mov fs,		eax
         mov gs,		eax
         mov ss,		eax										;加载堆栈段(4GB)选择子
		 
		 ;mov ah,    0xac
		 ;mov edi,   0xb8000
		 ;mov esi,   ARDS_COUNT;KERNEL_START_ADDRESS
		 ;mov ecx,   4+20+20
next_hex_char:		 
		 ;lodsb
		 ;call print_hex
		 ;loop next_hex_char 
		 mov     esp,  0x90000
		 push    dword [ARDS_COUNT]
		 push    ARDS_BUF
		 call	 [ds:boot_loader_main]
		 hlt
;-----------------------------------------------------------
; clear the screen via an interrupt
clear_screen:
		mov     al, 02h		; al = 02h, code for video mode (80x25)
		mov     ah, 00h		; code for the change video mode function
		int     10h		; trigger interrupt to call function
		ret

;-----------------------------------------------------------
; Print string to console
; si = ptr to first character of a null terminated string
print:
        cld
        mov     ah, 0x0e
next_char:
        lodsb
        cmp     al, 0
        je      print_done
        int     0x10
        jmp     next_char
print_done:
        ret

;----------------------------------------------------------------------
;32位保护模式下显示16进制字符
;AH = 显示颜色
;AL = 显示字符
;EDI= 显示缓冲区指针
print_hex:
		 ;mov bl,al
		 ;mov dl,al
		 
		 ;shr bl,4
		 ;and ebx,0x0000000f
		 ;mov al,[ebx + hex_chars]
		 ;stosw

		 ;and edx,0x0000000f
		 ;mov al,[edx + hex_chars]
		 ;stosw
		 ;ret
;-------------------------------------------------------------------------------
gdtr		dw	(0x18-1)
			dd	GDT_NULL ;GDT的物理/线性地址

GDT_NULL	dd	00
			dd	00
GDT_CODE	dd	0x0000ffff
			dd	0x00cf9800
GDT_DATA	dd	0x0000ffff
			dd	0x00cf9200

; Message strings
boot_msg:
        db      'Booting ...',13,10,0
load_osloader_ok_msg:
		db		'Load boot-loader OK',13,10,0
load_osloader_failed_msg:
		db		'Load boot-loader failed',13,10,0
check_memory_ok_msg:
		db		'Get memory map OK',13,10,0
hex_chars:
        ;db      '0123456789ABCDEF'
check_memory_failed_msg:
		db		'Get memory map failed',13,10,0
;-------------------------------------------------------------------------------                             
         times	510-($-$$) db 0
sign0x55aa:
		 db 0x55,0xaa