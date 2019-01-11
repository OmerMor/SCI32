	.386
	.MODEL	FLAT,SYSCALL

	.DATA
delta8	db	0,1,2,3,6,10,15,21
delta16	label	word
	dw	0
	dw	8
	dw	16
	dw	32,48
	dw	64,80,96,112
	dw	128,144,160,176,192,208,224,240
	dw	256,272,288,304,320,336,352,368,384,400,416,432,448,464,480,496
	dw	512,520,528,536,544,552,560,568,576,584,592,600,608,616,624,632
	dw	    640,648,656,664,672,680,688,696,704,712,720,728,736,744,752,760
	dw	    768,776,784,792,800,808,816,824,832,840,848,856,864,872,880,888
	dw	    896,904,912,920,928,936,944,952,960,968,976,984,992,1000,1008,1016
	dw	1024,1088,1152,1216,1280,1344,1408,1472,1536,1600,1664,1728,1792,1856,1920,1984
	dw	2048,2304,2560,2816,3072,3328,3584,3840
	dw	4096,5120,6144,7168
	dw	8192,12288
	dw	16384


	.CODE
; eax, ecx, edx do not have to be saved

Merge proc public uses es edi esi,\
	outb:far ptr, inb:near ptr, count:dword, bits:byte, check:byte

;	push	ds
;	pop	es
	les	edi, outb
	mov	esi, inb
	mov	ecx, count
	cmp	ecx, 0
	jle	mergeOut
	cmp	bits, 8
	jle	merge8
	shr	ecx, 1
	cmp	check, 0
	je	merge16nc
	xor	edx, edx
	xor	eax, eax
merge16c:
	mov	dx, es:[edi]
	lodsw
	sar	ax, 1
	sar	dx, 1
	add	ax, dx
	stosw
	loop	merge16c
	ret
merge16nc:
	lodsw
	add	ax, es:[edi]
	jno	merge16ok
	js	merge16p
	mov	ax, 8000h
	jmp	short merge16ok
merge16p:
	mov	ax, 7fffh
merge16ok:
	stosw
	loop	merge16nc
	ret
merge8:
	cmp	check, 0
	je	merge8nc
merge8c:
	mov	dx, 8080h
	lodsb
	sub	dl, al
	sub	dh, es:[edi]
	add	dl, dh
	sar	dl, 1
	mov	al, 80h
	sub	al, dl
	stosb
	loop	merge8c
	ret
merge8nc:
	mov	dx, 8080h
	lodsb
	sub	dl, al
	sub	dh, es:[edi]
	add	dl, dh
	jno	merge8ok
	js	merge8p
	mov	dl, 81h
	jmp	short merge8ok
merge8p:
	mov	dl, 80h
merge8ok:
	mov	al, 80h
	sub	al, dl
	stosb
	loop	merge8nc
mergeOut:
	ret

Merge endp

Distort proc public uses es edi,\
	outb:far ptr, count:dword, bits:byte, distort:byte

;	push	ds
;	pop	es
	les	edi, outb
	mov	ecx, count
	cmp	ecx, 0
	jle	distortOut
	mov	dl, distort
	cmp	bits, 8
	jle	distort8
	shr	ecx, 1
	shl	dx, 8
distort16:
	mov	ax, es:[edi]
	and	ax, dx
	stosw
	loop	distort16
	ret
distort8:
	mov	al, es:[edi]
	and	al, dl
	stosb
	loop	distort8
distortOut:
	ret

Distort endp

Volume proc public uses es edi,\
	outb:far ptr, count:dword, bits:byte, vol:byte, filter:byte 

;	push	ds
;	pop	es
	les	edi, outb
	mov	ecx, count
	cmp	bits, 8
	jle	vol8
	shr	ecx, 1
	cmp	vol, 0
	je	vol16_1
	cmp	vol, 1
	je	vol16_2
	cmp	vol, 2
	je	vol16_3
	cmp	vol, 3
	je	vol16_4
	cmp	vol, 4
	je	vol16_5
	cmp	vol, 5
	je	vol16_6
	cmp	vol, 6
	je	vol16_7
	cmp	vol, 7
	je	vol16_8
	cmp	vol, 8
	je	vol16_9
	cmp	vol, 9
	je	vol16_10
	cmp	vol, 10
	je	vol16_11
	cmp	vol, 11
	je	vol16_12
	cmp	vol, 12
	je	vol16_13
	cmp	vol, 13
	je	vol16_14
	cmp	vol, 14
	je	vol16_15
	ret
vol16_15: ;15x/16 = (1x+2x+4x+8x)/16
	movsx	eax, word ptr es:[edi]
	mov	edx, eax
	shl	eax, 1
	add	edx, eax 
	shl	eax, 1
	add	edx, eax 
	shl	eax, 1
	add	eax, edx 
	shr	eax, 4
	stosw
	loop	vol16_15
	ret
vol16_14: ;14x/16 = (2x+4x+8x)/16
	movsx	eax, word ptr es:[edi]
	shl	eax, 1
	mov	edx, eax 
	shl	eax, 1
	add	edx, eax 
	shl	eax, 1
	add	eax, edx 
	shr	eax, 4
	stosw
	loop	vol16_14
	ret
vol16_13: ;13x/16 = (1x+4x+8x)/16
	movsx	eax, word ptr es:[edi]
	mov	edx, eax
	shl	eax, 2
	add	edx, eax 
	shl	eax, 1
	add	eax, edx 
	shr	eax, 4
	stosw
	loop	vol16_13
	ret
vol16_12: ;12x/16 = (4x+8x)/16
	movsx	eax, word ptr es:[edi]
	shl	eax, 2
	mov	edx, eax 
	shl	eax, 1
	add	eax, edx 
	shr	eax, 4
	stosw
	loop	vol16_12
	ret
vol16_11: ;11x/16 = (1x+2x+8x)/16
	movsx	eax, word ptr es:[edi]
	mov	edx, eax
	shl	eax, 1
	add	edx, eax 
	shl	eax, 2
	add	eax, edx 
	shr	eax, 4
	stosw
	loop	vol16_11
	ret
vol16_10: ;10x/16 = (2x+8x)/16
	movsx	eax, word ptr es:[edi]
	shl	eax, 2
	mov	edx, eax 
	shl	eax, 1
	add	eax, edx 
	shr	eax, 4
	stosw
	loop	vol16_10
	ret
vol16_9: ;9x/16 = (1x+8x)/16
	movsx	eax, word ptr es:[edi]
	mov	edx, eax
	shl	eax, 3
	add	eax, edx 
	shr	eax, 4
	stosw
	loop	vol16_9
	ret
vol16_8: ;8x/16 = (8x)/16
	sar	word ptr es:[edi], 1
	add	edi, 2
	loop	vol16_8
	ret
vol16_7: ;7x/16 = (1x+2x+4x)/16
	movsx	eax, word ptr es:[edi]
	mov	edx, eax 
	shl	eax, 1
	add	edx, eax 
	shl	eax, 1
	add	eax, edx 
	shr	eax, 4
	stosw
	loop	vol16_7
	ret
vol16_6: ;6x/16 = (2x+4x)/16
	movsx	eax, word ptr es:[edi]
	shl	eax, 1
	mov	edx, eax 
	shl	eax, 1
	add	eax, edx 
	shr	eax, 4
	stosw
	loop	vol16_6
	ret
vol16_5: ;5x/16 = (1x+4x)/16
	movsx	eax, word ptr es:[edi]
	mov	edx, eax 
	shl	eax, 2
	add	eax, edx 
	shr	eax, 4
	stosw
	loop	vol16_5
	ret
vol16_4: ;4x/16 = (4x)/16
	sar	word ptr es:[edi], 2
	add	edi, 2
	loop	vol16_4
	ret
vol16_3: ;3x/16 = (1x+2x)/16
	movsx	eax, word ptr es:[edi]
	mov	edx, eax 
	shl	eax, 1
	add	eax, edx 
	shr	eax, 4
	stosw
	loop	vol16_3
	ret
vol16_2: ;2x/16 = (2x)/16
	sar	word ptr es:[edi], 3
	add	edi, 2
	loop	vol16_2
	ret
vol16_1: ;1x/16 = (1x)/16
	sar	word ptr es:[edi], 4
	add	edi, 2
	loop	vol16_1
	ret
vol8:
	cmp	vol, 0
	je	vol8_1
	cmp	vol, 1
	je	vol8_2
	cmp	vol, 2
	je	vol8_3
	cmp	vol, 3
	je	vol8_4
	cmp	vol, 4
	je	vol8_5
	cmp	vol, 5
	je	vol8_6
	cmp	vol, 6
	je	vol8_7
	cmp	vol, 7
	je	vol8_8
	cmp	vol, 8
	je	vol8_9
	cmp	vol, 9
	je	vol8_10
	cmp	vol, 10
	je	vol8_11
	cmp	vol, 11
	je	vol8_12
	cmp	vol, 12
	je	vol8_13
	cmp	vol, 13
	je	vol8_14
	cmp	vol, 14
	je	vol8_15
	ret
vol8_15: ;15x/16 = (1x+2x+4x+8x)/16
	mov	al, filter
	sub	al, es:[edi]
	movsx	ax, al
	mov	dx, ax
	shl	ax, 1
	add	dx, ax 
	shl	ax, 1
	add	dx, ax 
	shl	ax, 1
	add	ax, dx 
	shl	ax, 4
	mov	al, filter
	sub	al, ah
	stosb
	loop	vol8_15
	ret
vol8_14: ;14x/16 = (2x+4x+8x)/16
	mov	al, filter
	sub	al, es:[edi]
	movsx	eax, al
	shl	eax, 1
	mov	edx, eax 
	shl	eax, 1
	add	edx, eax 
	shl	eax, 1
	add	eax, edx 
	shl	ax, 4
	mov	al, filter
	sub	al, ah
	stosb
	loop	vol8_14
	ret
vol8_13: ;13x/16 = (1x+4x+8x)/16
	mov	al, filter
	sub	al, es:[edi]
	movsx	ax, al
	mov	dx, ax
	shl	ax, 2
	add	dx, ax 
	shl	ax, 1
	add	ax, dx 
	shl	ax, 4
	mov	al, filter
	sub	al, ah
	stosb
	loop	vol8_13
	ret
vol8_12: ;12x/16 = (4x+8x)/16
	mov	al, filter
	sub	al, es:[edi]
	movsx	ax, al
	shl	ax, 2
	mov	dx, ax 
	shl	ax, 1
	add	ax, dx 
	shl	ax, 4
	mov	al, filter
	sub	al, ah
	stosb
	loop	vol8_12
	ret
vol8_11: ;11x/16 = (1x+2x+8x)/16
	mov	al, filter
	sub	al, es:[edi]
	movsx	ax, al
	mov	dx, ax
	shl	ax, 1
	add	dx, ax 
	shl	ax, 2
	add	ax, dx 
	shl	ax, 4
	mov	al, filter
	sub	al, ah
	stosb
	loop	vol8_11
	ret
vol8_10: ;10x/16 = (2x+8x)/16
	mov	al, filter
	sub	al, es:[edi]
	movsx	ax, al
	shl	ax, 2
	mov	dx, ax 
	shl	ax, 1
	add	ax, dx 
	shl	ax, 4
	mov	al, filter
	sub	al, ah
	stosb
	loop	vol8_10
	ret
vol8_9: ;9x/16 = (1x+8x)/16
	mov	al, filter
	sub	al, es:[edi]
	movsx	ax, al
	mov	dx, ax
	shl	ax, 3
	add	ax, dx 
	shl	ax, 4
	mov	al, filter
	sub	al, ah
	stosb
	loop	vol8_9
	ret
vol8_8: ;8x/16 = (8x)/16
	mov	al, filter
	mov	ah, al
	sub	ah, es:[edi]
	sar	ah, 1
	sub	al, ah
	stosb
	loop	vol8_8
	ret
vol8_7: ;7x/16 = (1x+2x+4x)/16
	mov	al, filter
	sub	al, es:[edi]
	movsx	ax, al
	mov	dx, ax 
	shl	ax, 1
	add	dx, ax 
	shl	ax, 1
	add	ax, dx 
	shl	ax, 4
	mov	al, filter
	sub	al, ah
	stosb
	loop	vol8_7
	ret
vol8_6: ;6x/16 = (2x+4x)/16
	mov	al, filter
	sub	al, es:[edi]
	movsx	ax, al
	shl	ax, 1
	mov	dx, ax 
	shl	ax, 1
	add	ax, dx 
	shl	ax, 4
	mov	al, filter
	sub	al, ah
	stosb
	loop	vol8_6
	ret
vol8_5: ;5x/16 = (1x+4x)/16
	mov	al, filter
	sub	al, es:[edi]
	movsx	ax, al
	mov	dx, ax 
	shl	ax, 2
	add	ax, dx 
	shl	ax, 4
	mov	al, filter
	sub	al, ah
	stosb
	loop	vol8_5
	ret
vol8_4: ;4x/16 = (4x)/16
	mov	al, filter
	mov	ah, al
	sub	ah, es:[edi]
	sar	ah, 2
	sub	al, ah
	stosb
	loop	vol8_4
	ret
vol8_3: ;3x/16 = (1x+2x)/16
	mov	al, filter
	sub	al, es:[edi]
	movsx	ax, al
	mov	dx, ax 
	shl	ax, 1
	add	ax, dx 
	shl	ax, 4
	mov	al, filter
	sub	al, ah
	stosb
	loop	vol8_3
	ret
vol8_2: ;2x/16 = (2x)/16
	mov	al, filter
	mov	ah, al
	sub	ah, es:[edi]
	sar	ah, 3
	sub	al, ah
	stosb
	loop	vol8_2
	ret
vol8_1: ;1x/16 = (1x)/16
	mov	al, filter
	mov	ah, al
	sub	ah, es:[edi]
	sar	ah, 4
	sub	al, ah
	stosb
	loop	vol8_1
	ret

Volume endp

Silence proc public uses es edi,\
	outb:far ptr, count:dword, bits:byte

;	push	ds
;	pop	es
	les	edi, outb
	mov	ecx, count
	cmp	bits, 8
	jle	silence8
	shr	ecx, 1
	mov	ax, 0
	rep	stosw
	ret
silence8:
	mov	ax, 80h
	rep	stosb
	ret

Silence endp

Decomp4To8 proc public uses es edi esi ebx\
	outb:near ptr, inb:near ptr, count:dword, compVal:byte

	push	ds
	pop	es
	mov	edi, outb
	mov	esi, inb
	mov	dh, compVal
	mov	ecx, count
	cmp	ecx, 0
	jle	decompOut
	xor	ebx, ebx

decomp:
	lodsb
	mov	dl, al
	shr	al, 4
	and	al, 0fh
	cmp	al, 8
	jl	d1
	and	al, 7
	mov	bl, al
	mov	al, delta8[ebx]
	sub	dh, al
	jmp	d2
d1:
	mov	bl, al
	mov	al, delta8[ebx]
	add	dh, al
d2:
	mov	al, dh
	stosb

	mov	al, dl
	and	al, 0fh
	cmp	al, 8
	jl	d3
	and	al, 7
	mov	bl, al
	mov	al, delta8[ebx]
	sub	dh, al
	jmp	d4
d3:
	mov	bl, al
	mov	al, delta8[ebx]
	add	dh, al
d4:
	mov	al, dh
	stosb
	loop	decomp
decompOut:
	mov	al, dh
	ret

Decomp4To8 endp

Decomp8To16 proc public uses es edi esi ebx\
	outb:near ptr, inb:near ptr, count:dword, compVal:word

	push	ds
	pop	es
	mov	edi, outb
	mov	esi, inb
	mov	dx, compVal
	mov	ecx, count
	cmp	ecx, 0
	jle	ddecompOut
	xor	ebx, ebx
ddecomp:
	lodsb
	mov	bl, al
	shl	bl, 1
	jc	dd1
	add	dx, delta16[ebx]
	jmp	dd2
dd1:
	sub	dx, delta16[ebx]
dd2:
	mov	ax, dx
	stosw
	loop	ddecomp
ddecompOut:
	mov	ax, dx
	ret

Decomp8To16 endp

Conv8To16 proc public uses es edi esi,\
	buff:near ptr, count:dword

	push	ds
	pop	es
	mov	esi, buff
	mov	ecx, count
	cmp	ecx, 0
	jle	conv8Out
	add	esi, ecx
	dec	esi
	mov	edi, esi
	add	edi, ecx
	dec	edi
	pushf
	std
conv8to16:
	lodsb
	mov	ah, 80h
	sub	ah, al
	jc	c1
	mov	al, 0
	jmp 	c2
c1:
	mov	al, 0ffh
c2:
	stosw
	loop	conv8to16
	popf
conv8Out:
	ret

Conv8To16 endp

Conv16To8 proc public uses es edi esi,\
	outb:near ptr, inb:near ptr, count:dword

	push	ds
	pop	es
	mov	edi, outb
	mov	esi, inb
	mov	ecx, count
	cmp	ecx, 0
	jle	conv16Out
conv16to8:
	lodsw
	mov	al, 80h
	sub	al, ah
	stosb
	loop	conv16to8
conv16Out:
	ret

Conv16To8 endp

Conv11KTo22K proc public uses es edi esi,\
	buff:near ptr, count:dword, bits:byte

	push	ds
	pop	es
	mov	esi, buff
	mov	ecx, count
	cmp	ecx, 0
	jle	conv11Out
	add	esi, ecx
	dec	esi
	mov	edi, esi
	add	edi, ecx
	pushf
	std
	cmp	bits, 8
	je	conv11to22_8
	dec	esi
	dec	edi
	shr	ecx, 1
conv11to22_16:
	lodsw
	stosw
	stosw
	loop	conv11to22_16
	popf
	ret
conv11to22_8:
	lodsb
	stosb
	stosb
	loop	conv11to22_8
	popf
conv11Out:
	ret

Conv11KTo22K endp

Conv22KTo11K proc public uses es edi esi,\
	outb:near ptr, inb:near ptr, count:dword, bits:byte

	push	ds
	pop	es
	mov	edi, outb
	mov	esi, inb
	mov	ecx, count
	cmp	ecx, 0
	jle	conv22Out
	cmp	bits, 8
	je	conv22to11_8
	shr	ecx, 1
conv22to11_16:
	lodsw
	stosw
	inc	esi
	inc	esi
	loop	conv22to11_16
	ret
conv22to11_8:
	lodsb
	stosb
	inc	esi
	loop	conv22to11_8
conv22Out:
	ret

Conv22KTo11K endp

CopyEOS proc public uses es edi esi\
	outb:near ptr, inb:near ptr, count:dword, bits:byte

	push	ds
	pop	es
	mov	edi, outb
	mov	esi, inb
	mov	ecx, count
	cmp	ecx, 0
	jle	copyeosOut
	cmp	bits, 8
	je	copyeos8
copyeos16:
	lodsw
	stosw
	add	edi, 2
	loop	copyeos16
	ret
copyeos8:
	lodsb
	stosb
	add	edi, 1
	loop	copyeos8
copyeosOut:
	ret

CopyEOS endp

;DupeChannel proc public uses es edi esi\
;	inb:near ptr, count:dword, channel:dword, bits:byte
;
;	push	ds
;	pop	es
;	mov	esi, inb
;	mov	edi, esi
;	mov	ecx, count
;	cmp	ecx, 0
;	jle	dupeOut
;	xor	eax, eax
;	mov	al, bits
;	shr	eax, 3
;	.if channel == 0
;		add	esi, eax
;	.else
;		add	edi, eax
;	.endif
;	cmp	bits, 8
;	je	dupe8
;dupe16:
;	lodsw
;	stosw
;	add	esi, 2
;	add	edi, 2
;	loop	dupe16
;	ret
;dupe8:
;	lodsb
;	stosb
;	add	esi, 1
;	add	edi, 1
;	loop	dupe8
;dupeOut:
;	ret
;
;DupeChannel endp

DupeChannel proc public uses es edi esi\
	inb:near ptr, count:dword, channel:dword, bits:byte

	push	ds
	pop	es
	mov	esi, inb
	mov	edi, esi
	mov	ecx, count
	cmp	ecx, 0
	jle	avgOut
	xor	eax, eax
	mov	al, bits
	shr	eax, 3
	.if channel == 0
		add	esi, eax
		cmp	bits, 8
		je	avg8_even
		lodsw
		sub	esi, 2
		movsx	edx, ax
		jmp	short avg16
avg8_even:
		lodsb
		sub	esi, 1
		movsx	edx, al
		and	edx, 0ffh
		jmp	short avg8
	.else
		add	edi, eax
		cmp	bits, 8
		je	avg8_odd
		lodsw
		add	esi, 2
		dec	ecx
		jz	avg16_1
		movsx	edx, ax
		jmp	short avg16
avg8_odd:
		lodsb
		add	esi, 1
		dec	ecx
		jz	avg8_1
		movsx	edx, al
		and	edx, 0ffh
		jmp	short avg8
	.endif
avg16:
	lodsw
	movsx	eax, ax
	add	edx, eax
	xchg	eax, edx
	shr	eax, 1
	stosw
	add	esi, 2
	add	edi, 2
	loop	avg16
avg16_1:
	.if channel != 0
		sub	esi, 4
		lodsw
		stosw
	.endif
	ret
avg8:
	lodsb
	add	edx, eax
	xchg	eax, edx
	shr	eax, 1
	stosb
	add	esi, 1
	add	edi, 1
	loop	avg8
avg8_1:
	.if channel != 0
		sub	esi, 2
		lodsb
		stosb
	.endif
avgOut:
	ret

DupeChannel endp

MonitorNoise proc public uses esi\
	inb:near ptr, count:dword, bits:byte

	mov	esi, inb
	mov	ecx, count
	cmp	ecx, 0
	jle	noiOut
	cmp	bits, 8
	je	noi8
	shr	ecx, 1
noi16:
	lodsw
	cmp	ah, 5
	jg	noiOut
	cmp	ah, -5
	jl	noiOut
	loop	noi16
	xor	eax, eax
	ret
noi8:
	lodsb
	cmp	al, 7ah
	jb	noiOut
	cmp	al, 85h
	ja	noiOut
	loop	noi8
	xor	eax, eax
	ret
noiOut:
	mov	eax, 1
	ret

MonitorNoise endp

	end

