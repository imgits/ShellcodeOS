;http://www.rohitab.com/discuss/topic/35103-switch-between-real-mode-and-protected-mode/
; 
; Protected Mode BIOS Call Functionailty v2.0 - by Napalm
; -------------------------------------------------------
; 
; This is code shows how its POSSIBLE to execute BIOS interrupts
; by switch out to real-mode and then back into protected mode.
; 
; If you wish to use all or part of this code you must agree
; to the license at the following URL.
; 
; License: http://creativecommons.org/licenses/by-sa/2.0/uk/
;         
; Notes: This file is in NASM syntax.
;        Turn off paging before calling these functions.
;        int32() resets all selectors.
;
; C Prototype:
;	void _cdelc int32(unsigned char intnum, regs16_t *regs);
; 
; Example of usage:
;   regs.ax = 0x0013;
;   int32(0x10, &regs);
;   memset((char *)0xA0000, 1, (320*200));
;   memset((char *)0xA0000 + (100*320+80), 14, 80);
;   regs.ax = 0x0000;
;   int32(0x16, &regs);
;   regs.ax = 0x0003;
;   int32(0x10, &regs);
; 
;   栈布局
;   pregs     <--esp + 28
;   intnum    <--esp + 24
;   eip       <--esp + 20
;   eax       <--esp + 1C 
;   ecx       <--esp + 18
;   edx       <--esp + 14
;   ebx       <--esp + 10
;   ebp       <--esp + 0C
;   esp       <--esp + 08
;   esi       <--esp + 04
;   edi       <--esp + 00

[bits 32]

global callbios, _callbios, callbios_end,_callbios_end

struc regs16_t
	.edi	resd 1
	.esi	resd 1
	.ebp	resd 1
	.esp	resd 1
	.ebx	resd 1
	.edx	resd 1
	.ecx	resd 1
	.eax	resd 1
	.gs		resw 1
	.fs		resw 1
	.es		resw 1
	.ds		resw 1
	.eflags resd 1
endstruc
;+------------------------------------------------------+
;|    始终将代码复制到0x0000:7C00处执行                 |
;|    堆栈位于0x0000:7C00下方                           |
;+------------------------------------------------------+

%define CODE_BASE						       0x7C00
%define REBASE(addr)                           (CODE_BASE + (addr - reloc_base))

%define CODE32                                 0x08
%define DATA32                                 0x10
%define CODE16                                 0x18
%define DATA16                                 0x20
%define STACK16                                (CODE_BASE - regs16_t_size)

        SECTION .text

	callbios:                               
	_callbios:
		cli                                    ; disable interrupts
		pushad                                  ; save register state to 32bit stack
		mov  esi, reloc_base                   ; set source to code below
		mov  edi, CODE_BASE                ; set destination to new base address
		mov  ecx, (callbios_end - reloc_base)  ; set copy size to our codes size
		cld                                    ; clear direction flag (so we copy forward)
		rep  movsb                             ; do the actual copy (relocate code to low 16bit space)
		push CODE_BASE                      ; jump to new code location
		ret	
reloc_base:	
protect32_mode_entry:
[BITS 32]
		mov  [REBASE(pmode32_esp)], esp        ;#6 save 32bit stack pointer

		mov  eax, cr0
		mov  [REBASE(pmode32_CR0)],eax     

		sidt [REBASE(pmode32_idtr)]               ;#7 save 32bit idt pointer
		sgdt [REBASE(pmode32_gdtr)]               ;#7 save 32bit gdt pointer
		
		;设置GDT/IDT
        lgdt    [REBASE(GDTR)]
        lidt    [REBASE(IDTR)]

		;用中断号修正INT xx指令
		cld
		lea  esi, [esp + 0x24]                   ;#4 set position of intnum on 32bit stack
		lodsd     
		mov  [REBASE(intr) + 1], al                  ;#5 set intrrupt immediate byte from our arguments 
		
		;复制寄存器参数regs16_t到实模式栈中
		mov  esi, [esi]                        ;#2 read regs pointer in esi as source
		mov  edi, STACK16                      ;#5 set destination to 16bit stack
		mov  ecx, regs16_t_size                ;#5 set copy size to our struct size
		mov  esp, edi                          ;#2 save destination to as 16bit stack offset
		rep  movsb                             ;#2 do the actual copy (32bit stack to 16bit stack)
		;进入16位保护模式
		jmp  CODE16:REBASE(protect16_mode)      ;#? switch to 16bit selector (16bit protected mode)

	protect16_mode: 
[BITS 16]
		;退出保护模式
		mov  eax, cr0                           ;#3 get cr0 so we can modify it
		and  eax, ~0x80000000					;去掉内存分页标志
		and  eax, ~1								;去掉保护模式标志
		mov  cr0, eax                           ;#3 set cr0 to result
		;进入实地址模式
		jmp  dword 0x0000:REBASE(real_mode)      ;#? finally set cs:ip to enter real-mode

	real_mode: 
[BITS 16]
		;恢复实模式IVT
		a32 lidt [cs:REBASE(real_idtr)]    ;#? load 16bit idt

		xor  ax, ax                            ;#3 set ax to zero
		mov  ss, ax                            ;#2 set ss so they the stack is valid
		mov  sp, STACK16   

		;将调用参数赋给相应寄存器
		popad                                   ;#1 load general purpose registers from 16bit stack
		pop  gs                                ;#2 load gs from 16bit stack
		pop  fs                                ;#2 load fs from 16bit stack
		pop  es                                ;#1 load es from 16bit stack
		pop  ds                                ;#1 load ds from 16bit stack

		sti                                    ;#1 enable interrupts
	intr:	
	    int 0x00                               ;#2 opcode of INT instruction with immediate byte

		cli                                    ;#1 disable interrupts
		xor  sp, sp                            ;#3 zero sp so we can reuse it
		mov  ss, sp                            ;#2 set ss so the stack is valid
		mov  sp, STACK16                       ;#4 set correct stack position so we can copy back
		
		;保存寄存器状态
		pushfd                                  ;#1 save eflags to 16bit stack
		push ds                                ;#1 save ds to 16bit stack
		push es                                ;#1 save es to 16bit stack
		push fs                                ;#2 save fs to 16bit stack
		push gs                                ;#2 save gs to 16bit stack
		pushad                                  ;#1 save general purpose registers to 16bit stack

		;恢复保护模式GDT/IDT
        a32 lgdt    [cs:REBASE(pmode32_gdtr)]
        a32 lidt    [cs:REBASE(pmode32_idtr)]
		;返回保护模式
		mov  eax, cr0                          ;#3 get cr0 so we can modify it
		or   eax, 1                            ;#3 set PE bit to turn on protected mode
		a32  mov  eax, [cs:REBASE(pmode32_CR0)]
		mov  cr0, eax                          ;#3 set cr0 to result
		jmp  dword CODE32:protect32_mode_leave     ;#6 switch to 32bit selector (32bit protected mode)

	protect32_mode_leave: 
[BITS 32]

		mov  ax, DATA32                        ;#4 get our 32bit data selector
		mov  ds, ax                            ;#2 reset ds selector
		mov  es, ax                            ;#2 reset es selector
		mov  fs, ax                            ;#2 reset fs selector
		mov  gs, ax                            ;#2 reset gs selector
		mov  ss, ax                            ;#2 reset ss selector

		mov  esi, esp						   ;#2 set copy source to 16bit stack
		and  esi, 0x0000ffff 
		mov  esp, [REBASE(pmode32_esp)]				   ;#6 restore 32bit stack pointer

		lea  edi, [esp+0x28]                   ;#4 set position of regs pointer on 32bit stack
		mov  edi, [edi]                        ;#2 use regs pointer in edi as copy destination
		mov  ecx, regs16_t_size                ;#5 set copy size to our struct size
		cld                                    ;#1 clear direction flag (so we copy forward)
		rep  movsb                             ;#2 do the actual copy (16bit stack to 32bit stack)
		;sti                                    ; enable interrupts
		popad
		ret                                    ;#1 return to caller
	
	pmode32_esp	dd	0x00000000
	pmode32_CR0 dd  0x00000000

	pmode32_idtr:                              ; IDT table pointer for 32bit access
		dw 0x0000                              ; table limit (size)
		dd 0x00000000                          ; table base address
		
	pmode32_gdtr:                                 ; GDT table pointer for 32bit access
		dw 0x0000                              ; table limit (size)
		dd 0x00000000                          ; table base address
		
	real_idtr:                                 ; IDT table pointer for 16bit access
		dw 0x03FF                              ; table limit (size)
		dd 0x00000000                          ; table base address
		
	GDT:                                ; GDT descriptor table
		.null		dq 0x0000000000000000	   ; 0x00 - null segment descriptor
		.code32		dq 0x00CF9A000000FFFF     ; 0x01 - 32bit code base 0x00000000 limit 4G
		.data32		dq 0x00CF92000000FFFF     ; 0x02 - 32bit data base 0x00000000 limit 4G
		.code16		dq 0x00009A000000FFFF     ; 0x03 - 16bit code base 0x00010000 limit 64K
		.data16		dq 0x000092000000FFFF     ; 0x04 - 16bit data base 0x00010000 limit 64K

	GDTR:                                 ; GDT table pointer for 16bit access
		dw $ - GDT - 1				  ; table limit (size)
		dd GDT                            ; table base address

    IDTR:
	    dw 0
		dd 0 

	callbios_end
	_callbios_end
	
