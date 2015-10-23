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
; 
;   pregs     <--esp + 8
;   intnum    <--esp + 4
;   eip       <--esp + 0
[bits 32]

global callbios, _callbios

struc regs16_t
	.di	resw 1
	.si	resw 1
	.bp	resw 1
	.sp resw 1
	.bx	resw 1
	.dx	resw 1
	.cx	resw 1
	.ax	resw 1
	.gs	resw 1
	.fs	resw 1
	.es	resw 1
	.ds	resw 1
	.ef resw 1
endstruc

%define MODULE_SEG                             0x1000
%define MODULE_BASE                            0x10000
%define REBASE(addr)                           (addr - MODULE_BASE)
%define CODE32                                 0x08
%define DATA32                                 0x10
%define CODE16                                 0x18
%define DATA16                                 0x20
%define STACK16                                (0xfff0 - regs16_t_size)



        SECTION .text
	callbios:                               ; by Napalm
	_callbios:
	protect32_mode_entry:
	    BITS 32 
		cli
		mov  [pmode32_esp], esp        ; save 32bit stack pointer
		sidt [pmode32_idtr]               ; save 32bit idt pointer
		sgdt [pmode32_gdtr]               ; save 32bit gdt pointer
		
		mov  ax,0x0c30
		call putc32

		lea  esi, [esp + 4]                   ; set position of intnum on 32bit stack
		lodsd                                  ; read intnum into eax
		mov  [intr + 1], al                  ; set intrrupt immediate byte from our arguments 
		mov  esi, [esi]                        ; read regs pointer in esi as source
		mov  edi, STACK16                      ; set destination to 16bit stack
		mov  ecx, regs16_t_size                ; set copy size to our struct size
		mov  esp, edi                          ; save destination to as 16bit stack offset
		rep  movsb                             ; do the actual copy (32bit stack to 16bit stack)
		jmp  CODE16:REBASE(protect16_mode)      ; switch to 16bit selector (16bit protected mode)

	protect16_mode: 
		BITS 16
		mov  ax, DATA16                        ; get our 16bit data selector
		mov  ds, ax                            ; set ds to 16bit selector
		mov  es, ax                            ; set es to 16bit selector
		mov  fs, ax                            ; set fs to 16bit selector
		mov  gs, ax                            ; set gs to 16bit selector
		mov  ss, ax                            ; set ss to 16bit selector


		mov  eax, cr0                          ; get cr0 so we can modify it
		and  al,  ~0x01                        ; mask off PE bit to turn off protected mode
		mov  cr0, eax                          ; set cr0 to result

		jmp  dword MODULE_SEG:REBASE(real_mode)      ; finally set cs:ip to enter real-mode

	real_mode: use16
		mov  ax, MODULE_SEG                            ; set ax to zero
		mov  ds, ax                            ; set ds so we can access idt16
		mov  ss, ax                            ; set ss so they the stack is valid

		mov  ax,0x0c31
		call putc16
		

		a32 lidt [REBASE(rmode16_idtr)]               ; load 16bit idt
		;mov  bx, 0x0870                         ; master 8 and slave 112
		;call resetpic                           ; set pic's the to real-mode settings
		popa                                   ; load general purpose registers from 16bit stack
		pop  gs                                ; load gs from 16bit stack
		pop  fs                                ; load fs from 16bit stack
		pop  es                                ; load es from 16bit stack
		pop  ds                                ; load ds from 16bit stack
		sti                                    ; enable interrupts
		jmp $ 
	intr:	
	    int 0x00                               ; opcode of INT instruction with immediate byte
		cli                                    ; disable interrupts
		xor  sp, sp                            ; zero sp so we can reuse it
		mov  ss, sp                            ; set ss so the stack is valid
		mov  sp, STACK16                       ; set correct stack position so we can copy back
		pushf                                  ; save eflags to 16bit stack
		push ds                                ; save ds to 16bit stack
		push es                                ; save es to 16bit stack
		push fs                                ; save fs to 16bit stack
		push gs                                ; save gs to 16bit stack
		pusha                                  ; save general purpose registers to 16bit stack
		;mov  bx, 0x2028                        ; master 32 and slave 40
		;call resetpic                          ; restore the pic's to protected mode settings
		mov  eax, cr0                          ; get cr0 so we can modify it
		inc  eax                               ; set PE bit to turn on protected mode
		mov  cr0, eax                          ; set cr0 to result
		jmp  dword CODE32:REBASE(protect32_mode_leave)     ; switch to 32bit selector (32bit protected mode)
	protect32_mode_leave: use32
		mov  ax, DATA32                        ; get our 32bit data selector
		mov  ds, ax                            ; reset ds selector
		mov  es, ax                            ; reset es selector
		mov  fs, ax                            ; reset fs selector
		mov  gs, ax                            ; reset gs selector
		mov  ss, ax                            ; reset ss selector
		lgdt [pmode32_gdtr]               ; restore 32bit gdt pointer
		lidt [pmode32_idtr]               ; restore 32bit idt pointer
		mov  esi, esp                      ; set copy source to 16bit stack
		and  esi, 0xffff 
		mov  esp, [pmode32_esp]        ; restore 32bit stack pointer
		lea  edi, [esp+8]                   ; set position of regs pointer on 32bit stack
		mov  edi, [edi]                        ; use regs pointer in edi as copy destination
		mov  ecx, regs16_t_size                ; set copy size to our struct size
		cld                                    ; clear direction flag (so we copy forward)
		rep  movsb                             ; do the actual copy (16bit stack to 32bit stack)
		popa                                   ; restore registers
		sti                                    ; enable interrupts
		ret                                    ; return to caller
		
	resetpic:                                  ; reset's 8259 master and slave pic vectors
		push ax                                ; expects bh = master vector, bl = slave vector
		mov  al, 0x11                          ; 0x11 = ICW1_INIT | ICW1_ICW4
		out  0x20, al                          ; send ICW1 to master pic
		out  0xA0, al                          ; send ICW1 to slave pic
		mov  al, bh                            ; get master pic vector param
		out  0x21, al                          ; send ICW2 aka vector to master pic
		mov  al, bl                            ; get slave pic vector param
		out  0xA1, al                          ; send ICW2 aka vector to slave pic
		mov  al, 0x04                          ; 0x04 = set slave to IRQ2
		out  0x21, al                          ; send ICW3 to master pic
		shr  al, 1                             ; 0x02 = tell slave its on IRQ2 of master
		out  0xA1, al                          ; send ICW3 to slave pic
		shr  al, 1                             ; 0x01 = ICW4_8086
		out  0x21, al                          ; send ICW4 to master pic
		out  0xA1, al                          ; send ICW4 to slave pic
		pop  ax                                ; restore ax from stack
		ret                                    ; return to caller
	
putc16:
	push ds
	push bx
	mov  bx, 0xb800
	mov  ds, bx
	mov  word [ds:0],	ax
	pop  bx
	pop  ds
	ret

putc32:
	mov  [0xb8000],	ax
	ret

	pmode32_esp	dd	0x00000000

	stack32_ptr:                               ; address in 32bit stack after we
		dd 0x00000000                          ;   save all general purpose registers
		
	pmode32_idtr:                                 ; IDT table pointer for 32bit access
		dw 0x0000                              ; table limit (size)
		dd 0x00000000                          ; table base address
		
	pmode32_gdtr:                                 ; GDT table pointer for 32bit access
		dw 0x0000                              ; table limit (size)
		dd 0x00000000                          ; table base address
		
	rmode16_idtr:                                 ; IDT table pointer for 16bit access
		dw 0x03FF                              ; table limit (size)
		dd 0x00000000                          ; table base address
		
	gdt16_base:                                ; GDT descriptor table
		.null:                                 ; 0x00 - null segment descriptor
			dd 0x00000000                      ; must be left zero'd
			dd 0x00000000                      ; must be left zero'd
			
		.code32:                               ; 0x01 - 32bit code segment descriptor 0xFFFFFFFF
			dw 0xFFFF                          ; limit  0:15
			dw 0x0000                          ; base   0:15
			db 0x00                            ; base  16:23
			db 0x9A                            ; present, iopl/0, code, execute/read
			db 0xCF                            ; 4Kbyte granularity, 32bit selector; limit 16:19
			db 0x00                            ; base  24:31
			
		.data32:                               ; 0x02 - 32bit data segment descriptor 0xFFFFFFFF
			dw 0xFFFF                          ; limit  0:15
			dw 0x0000                          ; base   0:15
			db 0x00                            ; base  16:23
			db 0x92                            ; present, iopl/0, data, read/write
			db 0xCF                            ; 4Kbyte granularity, 32bit selector; limit 16:19
			db 0x00                            ; base  24:31
			
		.code16:                               ; 0x03 - 16bit code segment descriptor 0x000FFFFF
			dw 0xFFFF                          ; limit  0:15
			dw 0x0000                          ; base   0:15
			db 0x00                            ; base  16:23
			db 0x9A                            ; present, iopl/0, code, execute/read
			db 0x0F                            ; 1Byte granularity, 16bit selector; limit 16:19
			db 0x00                            ; base  24:31
			
		.data16:                               ; 0x04 - 16bit data segment descriptor 0x000FFFFF
			dw 0xFFFF                          ; limit  0:15
			dw 0x0000                          ; base   0:15
			db 0x00                            ; base  16:23
			db 0x92                            ; present, iopl/0, data, read/write
			db 0x0F                            ; 1Byte granularity, 16bit selector; limit 16:19
			db 0x00                            ; base  24:31
			
	gdt16_ptr:                                 ; GDT table pointer for 16bit access
		dw gdt16_ptr - gdt16_base - 1          ; table limit (size)
		dd gdt16_base                          ; table base address
		
	callbios_end:                                 ; end marker (so we can copy the code)
	
	