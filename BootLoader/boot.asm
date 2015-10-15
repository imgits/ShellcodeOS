         ;代码清单17-1
         ;文件名：boot.asm
         ;文件说明：硬盘主引导扇区代码 
         ;创建日期：2015-09-13 11:20        ;设置堆栈段和栈指针 

%define  BOOT_LOADER_ADDRESS				0X10000;常数，内核加载的起始内存地址 
%define  BOOT_LOADER_SEGMENT				(BOOT_LOADER_ADDRESS>>4)    ;常数，内核加载的起始内存段地址 
%define  BOOT_LOADER_OFFSET					(BOOT_LOADER_ADDRESS&0xffff);常数，内核加载的起始内存段偏移 

%define  BOOT_LOADER_STACK					0x00090000

%define  MEM_MAP_COUNT						0x00007e00 ;将内存分布数据存放于此处,以便kernel_main中进行处理
%define  MEM_MAP_BUF			  			0x00007e04

; 0x00100000 +--------------------+
;            +      ROM           +
; 0x000C0000 +--------------------+
;            +                    +
; 0x000B8000 +     视频缓冲区     +
;            +                    +
; 0x000A0000 +--------------------+
;            +        ...         +
; 0x00090000 +--------------------+
;            + BootLoader stack   +
;            +        ...         +
;            + BootLoader code    +
; 0x00010000 +--------------------+
;            +        ...         +
; 0x00007F00 +--------------------+
;            +     内存映射表     +
; 0x00007E00 +--------------------+
;            +     boot           +
; 0x00007C00 +--------------------+
;            +     boot堆栈       +
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
bootdrv			 db      0  ;启动磁盘
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
		cli
		cld
        mov     ax, cs
        mov     ds, ax
        mov		ss, ax
		mov		es, ax
        mov		esp, 0x7c00

        mov     [bootdrv], dl ; Save boot drive

		;clear_screen
		mov		ax,	0x0003
		int		0x10

        mov     si, boot_msg ;"BootStrap booting ..."
        call    print
		
;-----------------------------------------------------------------
;加载内核代码加载器OSloader
		mov     si, load_osloader_msg
        call    print
		
		mov		si, Disk_Address_Packet	; address of "disk address packet"
		mov		ah, 0x42			; AL is unused
		mov		dl, [bootdrv]	; drive number 0 (OR the drive # with 0x80)
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
; 通过BIOS INT 15H 获取内存分布信息
		;MEM_MAP_BUF的结构
		;uint32 MEM_MAP_COUNT
		;struct RAM_ARDS // Address Range Descriptor Structure
		;{
		;	uint32 Size; 此字段是自己添加的,不需要
		;	uint64 BaseAddr;
		;	uint64 Length;
		;	uint32 Type;
		;}[];
get_memory_map:
		mov     si, check_memory_msg
		call    print
		mov		ebx, 0                 ;ebx中存放后续值, 第一次调用时ebx 必须为 0;
		mov		edi,  MEM_MAP_BUF;es:di 指向一段缓冲区，用来存放ARDS（Address Range Descriptor Structure），即内存分布描述符
		mov		dword [MEM_MAP_COUNT], 0
next_mem_block:
		mov		eax, 0E820h
		mov		ecx, 64 ;ecx用来存放这个(ARDS)描述符的大小，以字节为单位。不过，通常情况下BIOS总是填充20字节的信息到ARDS中
		mov		edx, 0534D4150h ; 'SMAP' 中断要求
		int		15h
		jc		mem_check_failed ;	CF位置位表明调用出错，否则无错 
		add		edi, ecx
		inc		dword [MEM_MAP_COUNT]
		cmp		ebx, 0;	bx存放下一地址描述符所需的后续值，如果ebx中值为0则说明已经到了最后一个地址范围描述符了。
		jne		next_mem_block
		jmp		mem_check_ok
mem_check_failed:
		mov     si, failed_msg
		call    print
		hlt
mem_check_ok:
		mov     si, ok_msg
        call    print

;-----------------------------------------------------------------
;打开A20 (快速模式：http://wiki.osdev.org/A20_Line)
         in		al,0x92                         ;南桥芯片内的端口 
         or		al,0000_0010B
         out		0x92,al                        ;打开A20

;------------------------------------------------------------------
;初始化全局段描述符表，为进入保护模式作准备
		 ;lidt	[idtr]
		 lgdt	[gdtr]

;-------------------------------------------------------------------
;启动保护模式
         mov		eax,cr0                  
         or		eax,1
         mov		cr0,eax                        ;设置PE位
      
         ;以下进入保护模式... ...
         jmp dword GDT32.code32:boot_code32 ;16位的描述符选择子：32位偏移
                                            ;清流水线并串行化处理器

;===============================================================================
        [bits 32]               
;-------------------------------------------------------------------------------
   boot_code32:                                  
         mov		ax,		GDT32.data32                    ;加载数据段(4GB)选择子
         mov		ds,		ax
         mov		es,		ax
         mov		fs,		ax
         mov		gs,		ax
         mov		ss,		ax										;加载堆栈段(4GB)选择子
		 
		 ;mov ah,    0xac
		 ;mov edi,   0xb8000
		 ;mov esi,   MEM_MAP_COUNT;KERNEL_START_ADDRESS
		 ;mov ecx,   4+20+20
next_hex_char:		 
		 ;lodsb
		 ;call print_hex
		 ;loop next_hex_char 
		 mov     esp,  BOOT_LOADER_STACK
		 push    dword [MEM_MAP_COUNT]
		 push    MEM_MAP_BUF
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
			align   8
GDT32:
			dq	0x0000000000000000
.code32		equ  $ - GDT32
		    dq	0x00cf98000000ffff
.data32		equ  $ - GDT32
			dq	0x00cf92000000ffff
.real_code  equ  $ - GDT32
			dq	0x00009C000000ffff
.real_data  equ  $ - GDT32
			dq	0x000092000000ffff

gdtr		dw	$ - GDT32 - 1
			dd	GDT32 ;GDT的物理/线性地址

idtr		dw	0
			dd	0 ;IDT的物理/线性地址

; Message strings
boot_msg			db      'BootStrap booting ...',13,10,0
load_osloader_msg	db		'Load BootLoader ... ',0
check_memory_msg	db		'Get MemoryMap ... ',0
ok_msg				db		'OK',13,10,0
failed_msg			db		'FAILED',13,10,0
hex_chars			db      '0123456789ABCDEF dddddddddddddddddd                    '

;-------------------------------------------------------------------------
	times 510-64-($-$$) db 0
;disk_partition_table1:
;disk_partition_table2:
;disk_partition_table3:
;disk_partition_table4:

;-------------------------------------------------------------------------------                             
     times	510-($-$$) db 0
	 dw 0xAA55