
	.486
	.MODEL	FLAT,SYSCALL

	.DATA
newstack	dq	10 dup(?)
oldstack	dq	10 dup(?)
gotoaddr	dd	10 dup(?)
savereg		dd	10 dup(?)

	.CODE

        public 	SetInterruptFlag

SetInterruptFlag proc val:WORD
	mov	ax, val
	push	ax
	popf
	ret
SetInterruptFlag endp




	public	ReadInterruptAndClear

ReadInterruptAndClear	proc
	pushf
	pop	ax
	cli
	cld
	ret
ReadInterruptAndClear	endp





	public	SetESToDS

SetESToDS	proc
	cld
	push	es
	mov	ax, ds
	mov	es, ax
	pop	eax
	ret
SetESToDS	endp




	public	SetStack
SetStack	proc

	movzx	eax, WORD PTR 8[esp]
	mov	savereg[eax*4], ebx
	
	; Save off the current "old" stack less return address
	; ----------------------------------------------------
	mov	bx, ss
	mov	WORD PTR oldstack + 4[eax*8], bx
	mov	ebx, esp
	add	ebx, 8		; Param and return address
	mov	DWORD PTR oldstack[eax*8], ebx

	; Initialize variable to new stack
	; --------------------------------
	mov	bx, ds
	mov	WORD PTR newstack + 4[eax*8], bx
	mov	ebx, 4[esp]
	mov	DWORD PTR newstack[eax*8], ebx

	; Set new stack, return
	; ---------------------
	mov	ebx, [esp]
	mov	gotoaddr[eax*4], ebx
	lss	esp, FWORD PTR newstack[eax*8]
	mov	ebx, savereg[eax*4]
 	jmp	DWORD PTR gotoaddr[eax*4]
SetStack	endp




	public	RestoreStack
RestoreStack	proc

	; Recover old stack, return
	; -------------------------
	movzx	eax, WORD PTR 4[esp]
	mov	savereg[eax*4], ebx
	mov	ebx, [esp]
	mov	gotoaddr[eax*4], ebx
	mov	ebx, savereg[eax*4]
	lss	esp, FWORD PTR oldstack[eax*8]
	jmp	DWORD PTR gotoaddr[eax*4]

RestoreStack	endp




	public	SetES

SetES	proc	val:WORD
	mov	ax, val
	mov	es, ax
	ret
SetES	endp
	END
