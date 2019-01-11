
	.386
	.MODEL	FLAT,SYSCALL

	.data

	DivideByZero equ 1
	GeneralProtection equ 2

	oldPFHandlerOff		dd	?
	oldPFHandlerCS		dd	?

	.code

	AsmPanicReturn proto faultNo:dword



PanicDiv0 proc far

	mov	ax, ds
	mov ss, ax
	invoke AsmPanicReturn, DivideByZero
	
PanicDiv0 endp

PanicGP proc far

	mov	ax, ds
	mov ss, ax
	invoke AsmPanicReturn, GeneralProtection

PanicGP endp

	public	InitPageFaultHandler
InitPageFaultHandler proc
	push	eax
	push	ebx
	push	ecx
	push	edx
	push	esi
	push	edi
	push	es
	push	ds

	;; get the current address of the Page Fault exception handler
	xor	ecx, ecx

	mov	ax, 0202h
	mov	bl, 14
	int	31h

	mov	oldPFHandlerCS, ecx
	mov	oldPFHandlerOff, edx

	;; set the current address of the Page Fault exception handler
	mov	ax, 0203h
	mov	bl, 14
	mov	cx, cs
	mov	edx, OFFSET pfHandler
	int	31h

	pop	ds
	pop	es
	pop	edi
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx
	pop	eax

	ret
InitPageFaultHandler endp

pfHandler  proc
	mov	ah, 0
	mov	al, 3
	int	10h

	jmp	fword ptr cs:oldPFHandlerOff

	iretd
pfHandler  endp


end

