.686p
.model flat
option casemap:none

.CODE

__inbyte	PROC		StdCall port
	mov		dx,	word ptr port
	in		al,	dx
	ret
__inbyte	ENDP

__inword	PROC		StdCall port
	mov		dx,	word ptr port
	in		ax,	dx
	ret
__inword	ENDP

__indword	PROC		StdCall port
	mov		dx,	word ptr port
	in		eax,	dx
	ret
__indword	ENDP

__outbyte	PROC		StdCall port, val
	mov		dx,	word ptr port
	mov		al,	byte ptr val
	out		dx,	al
	ret
__outbyte	ENDP

__outword	PROC		StdCall port, val
	mov		dx,	word ptr port
	mov		ax,	word ptr val
	out		dx,	ax
	ret
__outword	ENDP

__outdword	PROC		StdCall port, val
	mov		dx,	word ptr port
	mov		eax,dword ptr val
	out		dx,	eax
	ret
__outdword	ENDP

__readcr0 PROC 
	mov	eax,	cr0
	ret
__readcr0 ENDP

__readcr2 PROC 
	mov	eax,	cr2
	ret
__readcr2 ENDP

__readcr3 PROC 
	mov	eax,	cr3
	ret
__readcr3 ENDP

__readcr4 PROC 
	mov	eax,	cr4
	ret
__readcr4 ENDP

__writecr0 PROC  StdCall val
	mov	eax,	dword ptr val
	mov cr0,		eax
	ret
__writecr0 ENDP

__writecr2 PROC StdCall val
	mov	eax,	dword ptr val
	mov cr2,		eax
	ret
__writecr2 ENDP

__writecr3 PROC StdCall val
	mov	eax,	dword ptr val
	mov cr3,		eax
	ret
__writecr3 ENDP

__writecr4 PROC StdCall val
	mov	eax,	dword ptr val
	mov cr4,		eax
	ret
__writecr4 ENDP

_enable PROC
	sti
	ret
_enable ENDP

_disable PROC
	sti
	ret
_disable ENDP

END