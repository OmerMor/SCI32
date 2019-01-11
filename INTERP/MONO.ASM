;---------------------------------------------------------------------
; MONO.S
; Copies text strings to the graphics text page
;---------------------------------------------------------------------

	.386
	.MODEL	FLAT,SYSCALL
                                                                                                                                         	.CODE
	.DATA

MONO_CRTC_ADDR_REG	equ	3b4h
MONO_CRTC_DATA_REG	equ	3b5h
CURSOR_LOC_REG			equ	0fh

TEXT1			equ	0b0000h
TEXT2			equ	0b8000h

monoInited	byte	0
xlate			word	0
dOff			dword	0
dBase			dword	TEXT1
	.CODE


DoXlate	proc

	.if !monoInited
		call	MonoInit
	.endif
	.if monoInited != 1
		ret
	.endif
	call	MonoClear		; clear mono screen
	mov	xlate, 1			; allow foreign language translations to mono mon.
	ret

DoXlate	endp

MonoStr	proc uses ebx esi edi es, tOffset:dword

	.if !monoInited
		call	MonoInit
	.endif
	.if monoInited != 1
		ret
	.endif
	mov	esi, tOffset
	.if	(esi == 0)
		call	MonoClear
		ret
	.endif
	.if	(esi == 1)
		mov	dBase, TEXT1
		ret
	.endif
	.if	(esi == 2)
		mov	dBase, TEXT2
		ret
	.endif
	
	mov	ax, ds
	mov	es, ax
	mov	edi, dBase	
	add	edi, dOff
	mov	ebx, dBase
	add	ebx, 4000
ms1:
	lodsb
	cmp	al, 0		;end-of-string?
	je	ms9
	cmp	al, 10		;new-line?
	jne	ms2
	sub	edi, dBase
	mov	dOff, edi
	call	MonoNewline
	mov	edi, dOff
	add	edi, dBase
	jmp	ms1
ms2:
	cmp	al, 13		;carriage-return?
	jne	ms3
	sub	edi, dBase
	mov	dOff, edi
	call	MonoCReturn
	mov	edi, dOff
	add	edi, dBase
	jmp	ms1
ms3:
	cmp	al, 8		;back-space?
	jne	ms4
	dec	edi
	dec	edi
	jmp	ms1
ms4:
	stosb
	inc	edi
	cmp	edi, ebx
	jl	ms1
	call	MonoScroll
	mov	edi, dBase	
	add	edi, dOff
	jmp	ms1
ms9:
	sub	edi, dBase
	mov	dOff, edi
	ret

MonoStr	endp

MonoChar proc public uses ebx edi es, check:byte
; This routine can be safely called from a hardware interrupt
	mov	edi, dBase
	add	edi, dOff
	mov	ebx, dBase
	add	ebx, 4000
	mov	ax, ds
	mov	es, ax
	mov	al, check
	stosb
	inc	edi
	cmp	edi, ebx
	jl	sh1
	call	MonoScroll
	mov	edi, dBase	
	add	edi, dOff
sh1:
	sub	edi, dBase
	mov	dOff, edi
	ret
MonoChar endp

MonoSet	proc uses ebx esi edi es, row:dword, col:dword
; specify row 1-25 (or 0 for no change)
; and column 1-80 (or 0 for no change)

	.if !monoInited
		call	MonoInit
	.endif
	.if monoInited != 1
		ret
	.endif

	mov	eax, dOff
	xor	edx, edx
	mov	cx, 160
	div	cx
	mov	bx, dx		; bx contains col*2
	mul	cx		; ax contains row*160

	cmp	row, 0
	jle	pos1
	cmp	row, 25
	jg	pos1
; new row
	mov	eax, row
	dec	eax
	mul	cx
pos1:
	cmp	col, 0
	jle	pos2
	cmp	col, 80
	jg	pos2
; new col
	mov	ebx, col
	dec	ebx
	shl	ebx, 1
pos2:
	add	eax, ebx
	mov	dOff, eax
	ret

MonoSet	endp

MonoInit	proc	uses dx

	; detect presence of mono monitor
	mov	dx, MONO_CRTC_ADDR_REG
	mov	al, CURSOR_LOC_REG
	out	dx, al			; access cursor loc reg
	mov	dx, MONO_CRTC_DATA_REG
	in		al, dx
	inc	al					
	out	dx, al			; attempt to modify it
	mov	ah, al
	in		al, dx
	.if al == ah
		dec	al				; it's modifiable; ergo mono monitor exists
		out	dx, al		; (clean-up mess)
		call	MonoClear
		mov	monoInited, 1
	.else
		mov	monoInited, -1
	.endif
	ret

MonoInit	endp

MonoClear proc uses edi ecx es

	mov	ax, ds
	mov	es, ax
	xor	edi, edi
	mov	dOff, edi
	mov	edi, dBase
	mov	ax, 720h
	mov	ecx, 4000
	rep	stosw
	ret

MonoClear endp

MonoNewline proc uses ecx edx

	mov	eax, dOff
	mov	cx, 160
	xor	dx, dx
	div	cx
	mul	cx
	add	ax, cx
	mov	dOff, eax
	.if ax >= 4000
		call	MonoScroll
	.endif
	ret

MonoNewline endp

MonoCReturn proc uses ecx edx

	mov	eax, dOff
	mov	cx, 160
	xor	dx, dx
	div	cx
	mul	cx
	mov	dOff, eax
	ret

MonoCReturn endp

MonoScroll proc uses ebx esi edi es 

	mov	ax, ds
	mov	es, ax
	mov	esi, dBase
	add	esi, 160
	mov	edi, dBase
	mov	ebx, dBase
	add	ebx, 4000
ss1:
	lodsb
	inc	esi
	stosb
	inc	edi
	cmp	esi, ebx
	jl		ss1
	mov	esi, edi
	mov	al, ' '
ss2:
	stosb
	inc	edi
	cmp	edi, ebx
	jl		ss2
	sub	esi, dBase
	mov	dOff, esi
	ret

MonoScroll endp

	end

