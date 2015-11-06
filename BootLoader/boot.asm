         ;代码清单17-1
         ;文件名：boot.asm
         ;文件说明：硬盘主引导扇区代码 
         ;创建日期：2015-09-13 11:20        ;设置堆栈段和栈指针 

%define  BOOT_LOADER_ADDRESS				0x10000;常数，内核加载的起始内存地址 
%define  BOOT_LOADER_SEGMENT				(BOOT_LOADER_ADDRESS>>4)    ;常数，内核加载的起始内存段地址 
%define  BOOT_LOADER_OFFSET					(BOOT_LOADER_ADDRESS&0xffff);常数，内核加载的起始内存段偏移 

%define  BOOT_LOADER_STACK					0x00090000

; 0x00100000 +--------------------+
;            |      ROM           |
; 0x000C0000 +--------------------+
;            |                    |
; 0x000B8000 +     视频缓冲区     +
;            |                    |
; 0x000A0000 +--------------------+
;            |        ...         |
; 0x00090000 +--------------------+
;            | BootLoader stack   |
;            |        ...         |
;            |  OsLoader code     |
; 0x00020000 +--------------------+
;            | BootLoader code    |
; 0x00010000 +--------------------+
;            |        ...         |
; 0x00007F00 +--------------------+
;            |     内存映射表     |
; 0x00007E00 +--------------------+
;            |     boot           |
; 0x00007C00 +--------------------+
;            |     boot堆栈       |
; 0x00000000 +--------------------+
;===============================================================================

        ORG     0x7C00
        BITS    16
        SECTION .text
boot:
;BIOS启动代码入口
;入口可能是0000:7C00 或 07C0:0000 
;此处使用长跳转，统一调整为0000:7C00
;=========================================================================
		jmp     0000:start ;5byte
;------------------------------------------------------------------------
boot_drive		  db      0  ;启动磁盘
boot_loader_main dd      0  ;boot_loader模块入口地址，由CreateImage写入
Disk_Address_Packet:
		db		10h		;size of packet (16 bytes)
		db		00h		;always 0
Kernel_Sectors:
		dw		7Fh		;number of sectors to transfer(max 127)，由CreateImage写入
		dw		BOOT_LOADER_OFFSET	;transfer buffer offset
		dw		BOOT_LOADER_SEGMENT	;transfer buffer segment 1000:0000=10000
		dd		00000001h;start sector
		dd		00000000h

;------------------------------------------------------------------------
start:
        ; Setup initial environment
        mov     ax, cs
        mov     ds, ax
        mov		ss, ax
		mov		es, ax
        mov		esp, 0x7c00

        mov     [boot_drive], dl ; Save boot drive
		mov     ah,0x08

		;clear_screen
		mov		ax,	0x0003
		int		0x10

        mov     si, boot_msg ;"BootStrap booting ..."
        call    print
				
;-----------------------------------------------------------------
;加载内核代码加载器OSloader
load_osloader:
		mov     si, load_osloader_msg
        call    print
		
		mov		si, Disk_Address_Packet	; address of "disk address packet"
		mov		ah, 0x42			; AL is unused
		mov		dl, [boot_drive]	; drive number 0 (OR the drive # with 0x80)
		int		0x13
		jnc		load_osloader_ok
load_osloader_failed:
		mov     si, failed_msg
        call    print
		hlt
load_osloader_ok:
		mov     si, ok_msg
        call    print

;-----------------------------------------------------------------
;打开A20 (快速模式：http://wiki.osdev.org/A20_Line)
         in		al,0x92                         ;南桥芯片内的端口 
         or		al,0000_0010B
         out	0x92,al                        ;打开A20
;------------------------------------------------------------------
;初始化全局段描述符表，为进入保护模式作准备
		 cli      ;此处关闭中断非常重要,否则，VMware中执行lidt	[IDTR]时会产生异常
		 lgdt	[GDTR]
		 lidt	[IDTR]

;-------------------------------------------------------------------
;启动保护模式
         mov		eax,cr0                  
         or			eax,1
         mov		cr0,eax                        ;设置PE位
      
         ;以下进入保护模式... ...
         jmp dword GDT32.code32:boot_code32 ;16位的描述符选择子：32位偏移
                                            ;清流水线并串行化处理器

;===============================================================================
        [bits 32]               
   boot_code32:                                  
         mov		ax,		GDT32.data32                    ;加载数据段(4GB)选择子
         mov		ds,		ax
         mov		es,		ax
         mov		fs,		ax
         mov		gs,		ax
         mov		ss,		ax										;加载堆栈段(4GB)选择子

		 mov     esp,  BOOT_LOADER_STACK
		 movzx   eax,  byte [boot_drive]
		 push    eax
		 call	 [ds:boot_loader_main]

		 hlt

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

align   8
GDT32:
			dq	0x0000000000000000
.code32		equ  $ - GDT32
		    dq	0x00cf9A000000ffff
.data32		equ  $ - GDT32
			dq	0x00cf92000000ffff

GDTR		dw	$ - GDT32 - 1
			dd	GDT32 ;GDT的物理/线性地址

IDTR		dw	0
			dd	0 ;IDT的物理/线性地址

boot_msg			db      'BootStrap booting ',13,10,0
load_osloader_msg	db		'Load BootLoader ',0
ok_msg				db		'OK',13,10,0
failed_msg			db		'FAILED',13,10,0

;-------------------------------------------------------------------------------                             
     times	510-($-$$) db 0
	 dw 0xAA55