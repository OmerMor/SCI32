	.386
	.MODEL	FLAT,SYSCALL

	.data

	public	colorptr
	public	controlptr
	public	destptr
	public	pixelcount
	public	skipval
	public	beginindex
	public	scaleptr

	colorptr	dd	0
	destptr		dd	0
	scaleptr	dd	0
	beginindex	dd	0
	controlptr	dd	0
	pixelcount	dd	0
	postCount	dd	0
	skipval		db	0

	
	.code

; eax, ecx, edx do not have to be saved
;
;
;  WriteUncompLine
;  ---------------
;  In:  colorptr  =  first byte to read of buffer
;       destptr   =  first byte of buffer to write
;       pixelcount
;       skipval
;


        public	WriteUncompLine
WriteUncompLine	proc

	; get the pointer to the RGB colors
	push	esi
	push	edi
	push	ebx

	mov	edi, destptr
	mov	esi, colorptr
	mov	ecx, pixelcount

;	if (pixelCount < 4)  {
; 		preCount = pixelCount;
; 		mainCount = 0;
; 		postCount = 0;
;	}
; 	else  {
; 		precount = 0;
; 	 	int remain = dest & 3;
; 		mainCount = pixelCount;
;  		if (remain)  {
;  			preCount = 4 - remain;
; 			mainCount -= preCount;
; 		}
;			
; 		postCount = mainCount & 3;
;		mainCount >>= 2;
;	}



; Calculate pre, main, post counts


	.if ecx < 4
		sub	edx, edx
 		mov	postCount, edx		; Zero
	.else
		mov	edx, ecx		; edx = mainCount
		sub	ecx, ecx		; ecx = preCount (ZERO)
		mov	eax, edi
		and	eax, 3

		.if	eax
			mov	ecx, 4
			sub	ecx, eax
			sub	edx, ecx
		.endif

		mov	eax, edx
		and	eax, 3
		mov	postCount, eax
		shr	edx, 2
	.endif



	; Pre loop
	; ========

	.if	ecx
		mov	ah, skipval
		.repeat
			mov	al, [esi]
			inc	esi
			.if	al != ah
				mov	[edi], al
			.endif
			inc	edi
		.untilcxz
	.endif



	; Main loop (x4)
	; ==============

	mov	ecx, edx
	.if	ecx
		mov	ah, skipval
		mov	dl, ah
		mov	dh, ah
		shl	edx, 16
		mov	dl, ah
		mov	dh, ah

		.repeat
			mov	eax, [esi]
			add	esi, 4
			.if	eax != edx
				.if	al != dl
					mov	[edi], al
				.endif
				inc	edi
	
				.if	ah != dl
					mov	[edi], ah
				.endif
				inc	edi
				shr	eax, 16
				.if	al != dl
					mov	[edi], al
				.endif
				inc	edi
	
				.if	ah != dl
					mov	[edi], ah
				.endif
				inc	edi
			.else
				add	edi, 4
			.endif
		.untilcxz


		; Post Loop
		; =========

		mov	ecx, postCount
		.if	ecx
			.repeat
				mov	al, [esi]
				inc	esi
				.if	al != dl
					mov	[edi], al
				.endif
				inc	edi
			.untilcxz
		.endif
	.endif


	pop	ebx
	pop	edi
	pop	esi
	ret
WriteUncompLine	endp

;
;  WriteUncompLineBack
;  -------------------
;  In:  colorptr  =  first byte to read of buffer
;       destptr   =  first byte of buffer to write
;       pixelcount
;       skipval
;

        public	WriteUncompLineBack
WriteUncompLineBack	proc

	; get the pointer to the RGB colors
	push	esi
	push	edi

	mov	edi, destptr
	mov	esi, colorptr
	mov	ecx, pixelcount
	mov	ah, skipval

	.repeat
		mov	al, [esi]
		inc	esi
		.if	al != ah
			mov	[edi], al
		.endif
		dec	edi
	.untilcxz

	pop	edi
	pop	esi
	ret
WriteUncompLineBack	endp


;
;  WriteUncompLineBackNoSkip
;  -------------------------
;  In:  colorptr  =  first byte to read of buffer
;       destptr   =  first byte of buffer to write
;       pixelcount
;

        public	WriteUncompLineBackNoSkip
WriteUncompLineBackNoSkip	proc

	; get the pointer to the RGB colors
	push	esi
	push	edi

	mov	edi, destptr
	mov	esi, colorptr
	mov	ecx, pixelcount

	.repeat
		mov	al, [esi]
		inc	esi
		mov	[edi], al
		dec	edi
	.untilcxz

	pop	edi
	pop	esi
	ret
WriteUncompLineBackNoSkip	endp

;
;  WriteUncompScaleLine
;  --------------------
;  In:  colorptr  =  entire line read buffer
;       destptr   =  first byte of buffer to write
;       scaleptr  =  entire scaling table
;       beginindex = starting index to use
;       pixelcount
;       skipval
;
        public	WriteUncompScaleLine
WriteUncompScaleLine	proc

	push	esi
	push	edi
	push	ebx

	mov	esi, colorptr
	mov	edi, destptr
	mov	edx, beginindex
	shl	edx, 2
	add	edx, scaleptr
	mov	ecx, pixelcount
	mov	ah, skipval

	.repeat
		mov	ebx, [edx]
		add	edx, 4
		mov	al, [esi + ebx]
		.if	al != ah
			mov	[edi], al
		.endif
		inc	edi
	.untilcxz		

	pop	ebx
	pop	edi
	pop	esi
	ret


WriteUncompScaleLine	endp



        public	WriteUncompScaleLine2
WriteUncompScaleLine2	proc

	push	esi
	push	edi
	push	ebx

	mov	esi, colorptr
	mov	edi, destptr
	mov	edx, beginindex
	shl	edx, 2
	add	edx, scaleptr
	mov	ecx, pixelcount
	mov	ah, skipval

	mov	ebx, [edx]
	add	esi, ebx

	; See if we need to do an initial one first
	.if	ebx != [edx + 4]
		mov	al, [esi]
		inc	esi
		.if	al != ah
			mov	[edi], al
		.endif
		dec	ecx
		inc	edi
	.endif
	

	.while ecx > 1
		mov	al, [esi]
		inc	esi
		.if	al != ah
			mov	[edi], al
			mov	[edi+1], al
		.endif
		sub	ecx, 2
		add	edi, 2
	.endw

	.if ecx != 0
		mov	al, [esi]
		.if	al != ah
			mov	[edi], al
		.endif
	.endif

	pop	ebx
	pop	edi
	pop	esi
	ret


WriteUncompScaleLine2	endp


        public	WriteUncompScaleLine2Mirr
WriteUncompScaleLine2Mirr	proc

	push	esi
	push	edi
	push	ebx

	mov	esi, colorptr
	mov	edi, destptr
	mov	edx, beginindex
	shl	edx, 2
	add	edx, scaleptr
	mov	ecx, pixelcount
	mov	ah, skipval

	mov	ebx, [edx]
	add	esi, ebx

	; See if we need to do an initial one first
	.if	ebx != [edx + 4]
		mov	al, [esi]
		dec	esi
		.if	al != ah
			mov	[edi], al
		.endif
		dec	ecx
		inc	edi
	.endif
	

	.while ecx > 1
		mov	al, [esi]
		dec	esi
		.if	al != ah
			mov	[edi], al
			mov	[edi+1], al
		.endif
		sub	ecx, 2
		add	edi, 2
	.endw

	.if ecx != 0
		mov	al, [esi]
		.if	al != ah
			mov	[edi], al
		.endif
	.endif

	pop	ebx
	pop	edi
	pop	esi
	ret


WriteUncompScaleLine2Mirr	endp



;
;
;  Position
;
;  In:  EAX = desired starting X
;       EBX  -  Control data
;       ESI  -  Color data
;
;  Out: ECX  -  Count
;        AH  -  Type (0x00, 0x80, 0xC0, 0x40)
;
;       ESI  -  updated color buffer pointer
;       EBX  -  updated control pointer
;

;
;  

Position	PROC
	
	mov	ecx, eax		; CX = remainder count

	.while	1
		movzx	eax, BYTE PTR [ebx]
		inc	ebx
		mov	edx, eax
		and	dx, 3FH
		and	ax, 0C0H


		.if	!(ax & 80H)
			.if	dx > cx			; pcount > wanted
				sub	edx, ecx	; remain = pcount - wanted
				add	esi, ecx	; dataptr += wanted
				mov	ecx, edx	; 
				mov	ah, al
				.break
			.endif

			sub	ecx, edx		; wanted -= pcount
			add	esi, edx		; dataptr += count
			.continue
		.endif

		.if	ax == 80H
			.if	dx > cx			; pcount > wanted
				sub	edx, ecx	; remain = pcount - wanted
				mov	ecx, edx
				mov	ah, al
				.break
			.endif

			sub	ecx, edx		; wanted -= pcount
			inc	esi			;
			.continue
		.endif

		.if	dx > cx				; pcount > wanted
			sub	edx, ecx		; remain = pcount - wanted
			mov	ecx, edx
			mov	ah, al
			mov	al, skipval
			.break
		.endif

		sub	ecx, edx			; wanted -= pcount
	.endw
	ret
Position	ENDP
	
		
;
; eax, ecx, edx do not have to be saved
;						
	public	WriteCompSkipLine

WriteCompSkipLine	PROC
	; Save registers
	push	ebx
	push	esi
	push	edi

	; Initialize to point within line
	mov	eax, beginindex
	mov	ebx, controlptr		; ebx = controlptr
	mov	esi, colorptr		; esi = colorptr
	mov	edi, destptr
	
	call	Position
					; ecx = packet count
	mov	edx, pixelcount		; edx = remaining count
	jmp	loopenter

	.while 1
		; Get next control value, split apart
		mov	ah, [ebx]
		inc	ebx
		movzx	ecx, ah
		and	cx, 3FH
		and	ah, 0C0H

		; Adjust count to not exceed remaining buffer space
loopenter:
		.if	ecx > edx			; pcount > wanted
			mov	ecx, edx		; pcount = wanted
		.endif
		sub	edx, ecx			; wanted -= pcount

		; Branch based on what needs to be done
		.if	ah & 80H
			.if	ah == 0C0H
				add	edi, ecx
				.break .if !edx
				.continue
			.endif
			lodsb
			shr	cx, 1
			.if	carry?
				stosb
			.endif
			mov	ah, al
			rep	stosw
			.break .if !edx
			.continue
		.endif

		mov	ah, skipval
		.repeat
			lodsb
			.if	al != ah
				mov	[edi], al
			.endif
			inc	edi
		.untilcxz

		.break .if !edx
	.endw

	pop	edi
	pop	esi
	pop	ebx
	ret

WriteCompSkipLine	ENDP
;
; eax, ecx, edx do not have to be saved
;						
	public	WriteCompSkipLine

WriteCompSkipLineBack	PROC
	; Save registers
	push	ebx
	push	esi
	push	edi

	; Initialize to point within line
	mov	eax, beginindex
	mov	ebx, controlptr		; ebx = controlptr
	mov	esi, colorptr		; esi = colorptr
	mov	edi, destptr
	
	call	Position
					; ecx = packet count
	mov	edx, pixelcount		; edx = remaining count
	jmp	loopenter

	.while 1
		; Get next control value, split apart
		mov	ah, [ebx]
		inc	ebx
		movzx	ecx, ah
		and	cx, 3FH
		and	ah, 0C0H

		; Adjust count to not exceed remaining buffer space
loopenter:
		.if	ecx > edx			; pcount > wanted
			mov	ecx, edx		; pcount = wanted
		.endif
		sub	edx, ecx			; wanted -= pcount

		; Branch based on what needs to be done
		.if	ah & 80H
			.if	ah == 0C0H
				sub	edi, ecx
				.break .if !edx
				.continue
			.endif

			mov	al, [esi]
			inc	esi
			.repeat
				mov	[edi], al
				dec	edi
			.untilcxz
			.break .if !edx
			.continue
		.endif

		mov	ah, skipval
		.repeat
			mov	al, [esi]
			inc	esi
			.if	al != ah
				mov	[edi], al
			.endif
			dec	edi
		.untilcxz
		.break .if !edx
	.endw

	pop	edi
	pop	esi
	pop	ebx
	ret

WriteCompSkipLineBack	ENDP

;
; eax, ecx, edx do not have to be saved
;						
	public	WriteCompSkipLine

WriteWholeCompSkipLine	PROC
	; Save registers
	push	ebx
	push	esi
	push	edi

	; Initialize to point within line
	mov	eax, beginindex
	mov	ebx, controlptr		; ebx = controlptr
	mov	esi, colorptr		; esi = colorptr
	mov	edi, destptr
	
	call	Position
	mov	edx, pixelcount		; edx = remaining count
	jmp	loopenter

	.repeat
		; Get next control value, split apart
		mov	ah, [ebx]
		inc	ebx
		movzx	ecx, ah
		and	cx, 3FH
		and	ah, 0C0H

		; Adjust count to not exceed remaining buffer space
loopenter:
		.if	ecx > edx			; pcount > wanted
			mov	ecx, edx		; pcount = wanted
		.endif
		sub	edx, ecx			; wanted -= pcount

		; Branch based on what needs to be done
		.if	ah & 80H
			.if	ah == 0C0H
				mov	al, skipval
				shr	cx, 1
				.if	carry?
					stosb
				.endif
				mov	ah, al
				rep	stosw
				.continue
			.endif
			lodsb
			shr	cx, 1
			.if	carry?
				stosb
			.endif
			mov	ah, al
			rep	stosw
			.continue
		.endif

		shr	cx, 1
		.if	carry?
			movsb
		.endif
		rep	movsw
	.until edx == 0

	pop	edi
	pop	esi
	pop	ebx
	ret

WriteWholeCompSkipLine	ENDP

			
	end

