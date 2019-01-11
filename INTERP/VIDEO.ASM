

	.386
	.MODEL	FLAT,SYSCALL

.data

	passCount	dw 0
	VESAName		db	"VESA"
	grainsPerBank	dd 0
	public windowNo
	windowNo		dw  0
	public bankSize
	bankSize		dd	0
	public granularity
	granularity		dd 0
	vesaInfoSegment dw 0
	vesaInfoSelector dw 0
	modeInfoSegment dw 0
	public modeInfoSelector
	modeInfoSelector dw 0
	public readWindow
	readWindow	db	0
	public readSegment
	readSegment	dd 0
	public writeSegment
	writeSegment dd 0  
	stAddrSav	dw	0

	vmapWidth	dd 0
	
	VgaInfoBlock struc 1
		VESASignature db 4 dup (?)
		VESAVersion	  dw ?
		OEMStringPtr  dd ?
		Capabilities  db 4 dup (?)
		VideoModePtr  dd ?
		TotalMemory	  dw ?
		Reserved	  db 236 dup (?);
	VgaInfoBlock ends

	realStack struct
		r_edi	dd	?
		r_esi	dd	?
		r_ebp	dd	?
		r_dummy	dd	?
		r_ebx	dd	?
		r_edx	dd	?
		r_ecx	dd	?
		r_eax	dd	?
		r_flags dw	?
		r_es	dw	?
		r_ds	dw	?
		r_fs	dw	?
		r_gs	dw	?
		r_ip	dw	?
		r_cs	dw	?
		r_sp	dw	0
		r_ss	dw	0
	realStack ends

	public aRealStack
	aRealStack	realStack <>


	ModeInfo struct
		attributes	dw	?
		winA		db	?
		winB		db	?
		modeGranularity	dw	?
		winSize		dw	?
		winASegment	dw	?
		winBSegment	dw	?
		funcPtr		dd	?
		bytesPerScan dw	?
		xResolution	dw	?
		yResolution	dw	?
		xCharSize	db	?
		yCharSize	db	?
		numberPlanes db ?
		bitsPerPixel db	?
		noBanks		db	?
		memoryModel db	?
		modeBankSize db	?
		reserved	db 229 dup(?)
	ModeInfo ends

.CODE



SetVesaWindow proto theWindowNo:dword

;Rectangular enclosure of Points
IRect	struc
	x1		dd	?
	y1	  	dd	?
	x2		dd	?
	y2		dd	?
IRect	ends

HRESOLUTION  		equ	320
VIDEOSEG		EQU	01ch		; metaware

extrn currentVideoMode:byte
SelectReadBank proto 

;
        public SetVGAPal
SetVGAPal	proc ptra:DWORD

	; get the pointer to the RGB colors

	pushad
	mov	esi, ptra
;
; Setup for waiting vertical blank
; --------------------------------	
	mov	dx, 03DAH
	xor	ax, ax
	mov	ecx, 256
;
; Wait for vertical blank to occur
; --------------------------------
vBlank1:
	in	al, dx
	and	al, 8
	jne	vBlank1
vBlank2:
	in	al, dx
	and	al, 8
	je	vBlank2
; 
; Init to write to hardware palette
; ---------------------------------
	pushf
	cli
;
; Setup write palette starting at index 0
; ---------------------------------------
	mov	dx, 3C8H		
	mov	ax, 0
	out	dx, al
;
; Write out 1 entry from palette
; ------------------------------
pEntry:
	mov	dx, 03C9H
	lodsb
	out	dx, al
	lodsb
	out	dx, al
	lodsb
	out	dx, al
;
; See if we still have vertical retrace
; -------------------------------------
	mov	dx, 03DAH
	in	al, dx
	test	al, 9
	jne	next
	popf					; Turn on interrupts for a moment
vBlank3:
	in	al, dx
	and	al, 8
	jne	vBlank3

vBlank4:
	in	al, dx
	and	al, 8
	je	vBlank4
	pushf
	cli

next:
	loop	pEntry
;
; Allow interrupts 	
; ----------------
	popf
	popad
	ret

SetVGAPal	endp

;---------------------------------------------------------------------------

PrepareForCursor proc public bufferPtr:dword

	
	

	ret

PrepareForCursor endp
;---------------------------------------------------------------------------

ShowRectX proto rectptr:dword, vmap:dword
ShowRect320 proto rectptr:dword, vmap:dword
ShowRectVesa proto rectptr:dword, vmap:dword


ShowRect proc public rectptr:DWORD, vmap:DWORD

	.if currentVideoMode == 1
		invoke	ShowRectX, rectptr, vmap
		ret
	.elseif currentVideoMode == 0
		invoke ShowRect320, rectptr, vmap
		ret
	.elseif currentVideoMode == 80h		; VESA
		invoke ShowRectVesa, rectptr, vmap
		ret

	.endif

	ret

ShowRect endp

;----------------------

ShowRect320 proc rectptr:DWORD, vmap:DWORD

	push	ebx
	push	esi
	push	edi


	; Calculate the pointer	to the rectangle
	mov	esi, rectptr
	assume	esi:ptr IRect
	mov	ecx, [esi].x2
	or	ecx, 1
	mov	edi, [esi].x1
	and	edi, 0FFFEH
	sub	ecx, edi
	inc	ecx			; ecx = xlen

	mov	ebx, [esi].y2
	mov	eax, [esi].y1
	sub	ebx, eax
	inc	ebx			; ebx = ylen

	assume si:nothing

	mov	edx, HRESOLUTION
	mul	edx			; eax = 320 * y1
	add	edi, eax		; edi += 320 * y1
	mov	esi, vmap
	add	esi, edi		; 
	add	edi, 0A0000H

	mov	edx, HRESOLUTION
	sub	edx, ecx
	shr	ecx, 1

	; ecx = hRun
	; ebx = vRun
	; edx = skip to next line value
	; edi = target video
	; esi = source video
loopa:
	cmp	ebx, 0
	; get the pointer to the RGB colors
	je	exita
	push	ecx
	rep	movsw
	pop	ecx
	add	edi, edx
	add	esi, edx
	dec	ebx
	jmp	loopa
exita:

	pop	edi
	pop	esi
	pop	ebx
	ret

ShowRect320 endp

;----------------------

SelectBank proto

ShowRectX proc uses ebx esi edi, rectptr:DWORD, vmap:DWORD 

	; ModeX (vertical)

	local currBank:dword

	mov	currBank, 0	
	mov	esi, rectptr
	assume	esi:ptr IRect

	; adjust start location for bank 0 and ending locations for bank 3
	and	[esi].x1, 0fffch	; mod 4, start with bank 0
	or	[esi].x2, 3

	; this routine works for modeX vertical only!
	mov	eax, [esi].y1
	mov	edx, HRESOLUTION
	mul	edx
	add	eax, [esi].x1
	push	eax			; save the row/col offset for later use by esi
	shr	eax, 1
	shr	eax, 1			; offsets are /4 because of video paging
	mov	edi, eax
	add	edi, 0a0000h	; now pointing to page0 mode x

	; set horizontal run count (line) in cx
	mov	ecx, [esi].x2	; right end
	sub	ecx, [esi].x1	; left end
	inc ecx
	
	; set vertical run count in DX
	mov	edx, [esi].y2	; bottom
	sub	edx, [esi].y1	; top
	inc	edx

	shr	cx, 1		; count/4 = count per bank (always forced even)
	shr	cx, 1

	pop	eax				; the row/col offset
	assume esi:nothing
	mov	esi, eax
	add	esi, vmap

	.while currBank < 4

		push	edi
		push	esi
		push	dx

		mov	eax, currBank
		call	SelectBank	; make the bank addressable in video memory

		add	esi, currBank	; point to data for this bank (every 4th byte)

		showByteY:

			push	edi
			push	ecx
			push	esi
			; transfer an entire line of rectangle
			.repeat
				mov	al,[esi]
				stosb
				add	esi, 4
			.untilcxz
		
	
			pop	esi
			add	esi, HRESOLUTION
			pop	ecx
			pop	edi
			add	edi, 80		; 320/4 = # bytes per row
			; add width of vMap line to SI

			dec	dx			; line count

		jne	showByteY 	; more lines?
		
		pop	dx			; row count
		pop	esi			; original data location
		pop	edi			; original video location

		inc	currBank

	.endw
		

	ret

ShowRectX endp

;----------------------

ShowRectVesa proc rectptr:DWORD, vmap:DWORD
	
	local	lineOffset:dword
	local	sourceOffset:dword

	push	ebx
	push	esi
	push	edi	
	push	ecx

	; Calculate the pointer	to the rectangle
	mov	esi, rectptr
	assume	esi:ptr IRect
	mov	ecx, [esi].x2
	or	ecx, 1
	mov	edi, [esi].x1
	and	edi, 0FFFEH
	sub	ecx, edi
	inc	ecx			; ecx = xlen

	mov	ebx, [esi].y2
	mov	eax, [esi].y1
	sub	ebx, eax
	inc	ebx			; ebx = ylen

	assume si:nothing


	mov	edx, 640
	mul edx					; eax = 640 * y1
	add	edi, eax			; edi = x + y

	mov eax, [esi].y1
	mov edx, vmapWidth
	mul	edx
	add	eax, [esi].x1
	mov	esi, eax

	mov	eax, edi			; find the starting window #
	xor	edx, edx
	div	bankSize			; ax now has the starting window #
	mov	windowNo, ax
	call	SetWindow
	mov	edi, edx			; the offset is the remainder
	mov	eax, bankSize
	sub	eax, edx			; how much of the bank is consumed at the start
	mov	edx, eax			; keep it here for quick access

	add	esi, vmap
	add	edi, writeSegment

	mov	eax, 640
	sub	eax, ecx
	mov	lineOffset, eax
	mov eax, vmapWidth
	sub eax, ecx
	mov sourceOffset, eax

	shr	ecx, 1			; doin a word at a time since banks are always even

	; ecx = hRun
	; ebx = vRun
	; edx = running count of remaining in bank
	; edi = target video
	; esi = source video

loopa:
	cmp	ebx, 0
	je	exita

	push	ecx

	; if we can do a row without changing banks, do it quickly
	mov	eax, ecx
	shl	eax, 1
	.if edx > eax	 	
		sub	edx, eax
		rep	movsw
	.else				; need to change banks in this row
		xchg ecx, edx
		shr	ecx, 1		; bytes = words in the old bank
		sub	edx, ecx 	; leftover words for the new bank
		rep	movsw		; do the old bank
		mov ecx, edx	; words in new bank
		inc	windowNo	
		call SetWindow
		mov	edi, writeSegment
		mov	edx, bankSize
		sub	edx, ecx	; twice to subtract in bytes, not words
		sub	edx, ecx
		rep	movsw		; output the new bank

	.endif

	pop	ecx
	add	edi, lineOffset
	add	esi, sourceOffset
	sub	edx, lineOffset
	.if carry? || zero?
		mov	eax, edx
		inc	windowNo;
		call	SetWindow
		add	edx, bankSize
		mov	edi, writeSegment
		neg	eax			; how far we went over is now our offset
		add	edi, eax
	.endif

	dec	ebx
	jmp	loopa
exita:

	pop	ecx
	pop	edi
	pop	esi
	pop	ebx

	ret

ShowRectVesa endp


;-------------------------------------------------------------------------

ReadVideoRectX proto rectptr:dword, buffer:dword
ReadVideoRect320 proto rectptr:dword, buffer:dword
ReadVideoRectVesa proto rectptr:dword, buffer:dword

ReadVideoRect proc public, rectptr:dword, buffer:dword

	.if currentVideoMode == 1
		invoke	ReadVideoRectX, rectptr, buffer
		ret
	.elseif currentVideoMode == 0
		invoke ReadVideoRect320, rectptr, buffer
		ret
	.elseif currentVideoMode == 80h
		invoke ReadVideoRectVesa, rectptr, buffer
		ret
	.endif

	ret

ReadVideoRect endp

;----------------------

ReadVideoRect320 proc public uses ebx esi edi, rectptr:dword, buffer:dword

	local lineOffset:dword

	mov	esi, rectptr
	assume	esi:ptr IRect
	mov	eax, [esi].x1
	mov	ecx, [esi].x2
	sub	ecx, eax
	inc	ecx
	mov	eax, 320
	sub	eax, ecx
	mov	lineOffset, eax

	mov	ebx, [esi].y2
	mov	eax, [esi].y1
	sub	ebx, eax
	inc	ebx

	mov	edx, 320
	mul edx		; eax has row
	mov	esi, [esi].x1
	add	esi, eax
	add	esi, 0A0000H

	mov	edi, buffer

	cld

	.while ebx

		push ecx
		rep movsb
		pop ecx
		add	esi, lineOffset
		dec ebx

	.endw

	ret

ReadVideoRect320 endp

;----------------------

ReadVideoRectX proc public  uses ebx esi edi, rectptr:dword, buffer:dword

	local columnCount:dword
	local xOffset:dword
	local sourceWidth:dword

	mov	ecx, 0

	; setup read mode
	mov	al, 5
	mov	dx, 03ceh	; graphics reg/set mode
	out	dx, al
	mov	dx, 3cfh
	in		al, dx
	and	al, 0f7h		; mask out bit 3 for read mode 0
	out	dx, al

	mov	dx, 3ceh
	mov	al, 4
	out	dx, al
	mov	dx, 3cfh

	mov	esi, rectptr
	assume	esi:ptr IRect
	mov columnCount, 0

	mov	eax, [esi].x1
	mov	xOffset, eax
	mov	ecx, [esi].x2
	sub	ecx, eax
	inc	ecx
	mov	sourceWidth, ecx

	mov	ebx, [esi].y2
	mov	eax, [esi].y1
	sub	ebx, eax
	inc	ebx


	assume esi:nothing
	mov	edx, 80
	mul edx		; eax has row
	mov	esi, eax
	add	esi, 0a0000h
	mov	edi, buffer

	; ecx = x length
	; ebx = y length
	; esi = video memory row start address
	; edi = start of buffer
	; xOffset = absolute x offset
	; columnCount = 0 - counts number of columns processed
	; sourceWidth = setup

	.while ecx

		push esi
		push edi
		push ebx

		mov	eax, xOffset
		and	eax, 3			; find the bank No
		call SelectReadBank
		add edi, columnCount	; offset destination start by the column no
		mov eax, xOffset
		shr	eax, 2
		add esi, eax	

		inc	xOffset
		inc columnCount

		.while ebx

			mov	al, byte ptr [esi]
			mov	[edi], al
			add esi, 80
			add edi, sourceWidth

			dec	ebx

		.endw

		pop	ebx
		pop	edi				; buffer addr
		pop	esi				; video mem start address 
		dec ecx

	.endw

	mov	dx, 3cfh
	mov	al, 3
	out	dx, al
	
	ret

ReadVideoRectX endp

;----------------------

SetReadWindow proto theWindow:word


ReadVideoRectVesa proc public uses ebx esi edi, rectptr:dword, buffer:dword

	local	lineOffset:dword
	local	savedWindowNo:dword
	local	theWindow: word


	.if readWindow == 0	; normally we only change the read window (B), but ...
		call	GetVesaWindow	; if the read window is WindowA ...
		mov	savedWindowNo, eax
	.endif

	; Calculate the pointer	to the rectangle
	mov	esi, rectptr
	assume	esi:ptr IRect
	mov	ecx, [esi].x2
	mov	edi, [esi].x1
	sub	ecx, edi
	inc	ecx			; ecx = xlen

	mov	ebx, [esi].y2
	mov	eax, [esi].y1
	sub	ebx, eax
	inc	ebx			; ebx = ylen

	assume si:nothing

	mov	edx, 640
	mul	edx				; eax had starting row
	add		eax, edi	; edi has starting col
	xor	edx, edx
	div	bankSize
	mov	theWindow, ax
	invoke SetReadWindow, theWindow	; the window for reading probably is different from writing

	mov	esi, edx		; the offset is the remainder
	add	esi, readSegment
	mov	eax, bankSize
	sub	eax, edx
	mov	edx, eax

	mov	eax, 640
	sub	eax, ecx
	mov	lineOffset, eax

	mov	edi, buffer

	; ecx = hRun
	; ebx = vRun
	; edx = running count of remaining in bank
	; edi = target buffer
	; esi = source video

	inc	passCount
	.if passCount == 15
		mov	passCount, 0
	.endif

	cld

	.while ebx			; for each row

		push	ecx

		.if edx > ecx
			sub	edx, ecx
			rep	movsb
		.else
			xchg ecx, edx
			sub	edx, ecx
			rep	movsb
			mov	ecx, edx
			inc theWindow
			invoke SetReadWindow, theWindow

			mov	esi, readSegment
			mov	edx, bankSize
			sub	edx, ecx
			rep	movsb		; output the new bank

		.endif

		pop	ecx
		add	esi, lineOffset
		sub	edx, lineOffset
		.if carry? || zero?
			mov	eax, edx
			inc	theWindow
			invoke SetReadWindow, theWindow
	cld

			add	edx, bankSize
			mov	esi, readSegment
			neg	eax			; how far we went over is now our offset
			add	esi, eax
		.endif

		dec	ebx

	.endw

	.if readWindow == 0
		invoke	SetVesaWindow, savedWindowNo
	.endif


	ret

ReadVideoRectVesa endp


;-------------------------------------------------------------------------
WriteVideoRectX proto rectptr:dword, buffer:dword, sourceYOffset:dword
WriteVideoRect320 proto rectptr:dword, buffer:dword, sourceYOffset:dword
WriteVideoRectVesa proto rectptr:dword, buffer:dword, sourceYOffset:dword

WriteVideoRect proc public, rectptr:dword, buffer:dword, sourceXOffset:dword

	.if currentVideoMode == 1
		invoke	WriteVideoRectX, rectptr, buffer, sourceXOffset
	.elseif currentVideoMode == 0
		invoke WriteVideoRect320, rectptr, buffer, sourceXOffset
	.elseif currentVideoMode == 80h
		invoke WriteVideoRectVesa, rectptr, buffer, sourceXOffset
	.endif

	ret

WriteVideoRect endp
;----------------------

WriteVideoRect320 proc public  uses ebx esi edi, rectptr:dword, buffer:dword, sourceXOffset:dword

	local lineOffset:dword

	mov	esi, rectptr
	assume	esi:ptr IRect
	mov	eax, [esi].x1
	mov	ecx, [esi].x2
	sub	ecx, eax
	inc	ecx
	mov	eax, 320
	sub	eax, ecx
	mov	lineOffset, eax

	mov	ebx, [esi].y2
	mov	eax, [esi].y1
	sub	ebx, eax
	inc	ebx

	mov	edx, 320
	mul	edx 			; eax has row
	mov	edi, eax
	add	edi, [esi].x1
	add	edi, 0a0000h

	mov	esi, buffer

	cld

	.while ebx

		push ecx
		rep movsb
		pop ecx
		add	edi, lineOffset
		add	esi, sourceXOffset
		dec ebx

	.endw


	ret

WriteVideoRect320 endp


;----------------------

WriteVideoRectX proc public  uses ebx esi edi, rectptr:dword, buffer:dword, sourceXOffset:dword

	local columnCount:dword
	local xOffset:dword
	local sourceWidth:dword

	mov	esi, rectptr
	assume	esi:ptr IRect
	mov columnCount, 0

	mov	eax, [esi].x1
	mov	xOffset, eax
	mov	ecx, [esi].x2
	sub	ecx, eax
	inc	ecx
	mov	sourceWidth, ecx

	mov	ebx, [esi].y2
	mov	eax, [esi].y1
	sub	ebx, eax
	inc	ebx

	assume esi:nothing
	mov	edx, 80
	mul edx		; eax has row
	mov	edi, eax
	add	edi, 0a0000h

	mov	esi, buffer

	; ecx = x length
	; ebx = y length
	; edi = video memory row start address
	; esi = start of buffer
	; xOffset = absolute x offset
	; columnCount = 0 - counts number of columns processed
	; height = setup

	.while ecx

		push esi
		push edi
		push ebx

		mov	eax, xOffset
		and	eax, 3			; find the bank No
		call SelectBank
		mov eax, xOffset
		shr	eax, 2
		add edi, eax	
		add esi, columnCount	; offset destination start by the column no

		inc	xOffset
		inc columnCount

		.while ebx

			mov	al, byte ptr [esi]
			mov	[edi], al
			add edi, 80
			add	esi, sourceWidth
			add esi, sourceXOffset

			dec	ebx

		.endw

		pop	ebx
		pop	edi				; buffer addr
		pop	esi				; video mem start address 
		dec ecx

	.endw

	ret

WriteVideoRectX endp

;----------------------

WriteVideoRectVesa proc public  uses ebx esi edi, rectptr:dword, buffer:dword, sourceXOffset:dword

	local lineOffset:dword

	mov	esi, rectptr
	assume	esi:ptr IRect

	mov	eax, [esi].x1
	mov	ecx, [esi].x2
	sub	ecx, eax
	inc	ecx

	mov	ebx, [esi].y2
	mov	eax, [esi].y1
	sub	ebx, eax
	inc	ebx

	mov	edx, 640
	mul edx
	add	eax, [esi].x1
	xor	edx, edx
	div	bankSize
	mov	windowNo, ax
	Call SetWindow

	mov	edi, edx
	add	edi, writeSegment

	mov	eax, bankSize
	sub	eax, edx
	mov	edx, eax		; how much of this bank remains

	mov	eax, 640
	sub	eax, ecx
	mov	lineOffset, eax

	mov	esi, buffer
	cld

	.while ebx

		push ecx

		; if we can do a row without changing banks, do it quickly
		.if edx > ecx	 	
			sub	edx, ecx
			rep	movsb
		.else				; need to change banks in this row
			xchg ecx, edx
			sub	edx, ecx 	; leftover for the new bank
			rep	movsb		; do the old bank
			mov ecx, edx	; words in new bank
			inc	windowNo	
			call SetWindow
			mov	edi, writeSegment
			mov	edx, bankSize
			sub	edx, ecx
			rep	movsb		; output the new bank

		.endif

		pop	ecx
		add	esi, sourceXOffset
		add	edi, lineOffset
		sub	edx, lineOffset
		.if carry? || zero?
			mov	eax, edx
			inc	windowNo;
			call	SetWindow
			add	edx, bankSize
			mov	edi, writeSegment
			neg	eax			; how far we went over is now our offset
			add	edi, eax
		.endif

		dec	ebx

	.endw


	ret

WriteVideoRectVesa endp


SelectBank proto
;---------------------------------------------------------------------------

SetupVesa proc public  uses ebx esi edi

	mov currentVideoMode, 80h

	; allocate two blocks of 256 bytes (16 paragraphs)
	mov	ax, 100h
	mov bx, 16
	int 31h
	mov	vesaInfoSegment, ax
	mov	vesaInfoSelector, dx

	mov ax, 100h
	mov	bx, 16
	int	31h
	mov modeInfoSegment, ax
	mov	modeInfoSelector, dx

	; setup the real mode stack with appropriate register values
	mov	esi, offset aRealStack
	mov	[esi].r_eax, 4f00h

	mov	ax, vesaInfoSegment
	mov	[esi].r_es, ax

	xor	eax, eax
	mov	[esi].r_edi, eax	

	mov	ax, 300h
	mov	bl, 10h
	xor	bh, bh
	mov	cx, 0
	mov	edi, esi
	int	31h

	mov	esi, offset VESAName
	mov	ax, vesaInfoSelector
	push es
	mov	es, ax
	xor	edi, edi
	mov	ecx, 4
	repe	cmpsb

	jz	VesaFound
	pop	es

	; free up our info and mode buffers
	mov	dx, vesaInfoSelector
	mov	ax, 101h
	int	31h

	mov dx, modeInfoSelector
	mov	ax, 101h
	int 31h

	mov	eax, -1
	ret

VesaFound:

	pop	es
	mov	esi, offset aRealStack
	pushf
	pop	ax
	mov	[esi].r_flags, ax	; setup flags to some legit value
	mov	ax, modeInfoSegment
	mov	[esi].r_es, ax
	mov	[esi].r_edi, 0
	mov	[esi].r_eax, 4f01h
	mov	[esi].r_ecx, 101h
	mov	ax, 300h
	mov	bl, 10h
	xor	bh, bh
	mov	cx, 0
	mov	edi, esi
	int	31h

	mov	[esi].r_eax, 4f02h
	mov	[esi].r_ebx, 101h
	mov	edi, esi
	int	31h				; set video mode 101

	; setup some variables
	mov	ax,	modeInfoSelector
	push	es
	mov	es, ax
	xor	esi, esi
	assume esi: ptr ModeInfo
	xor	eax, eax
	mov	ax, es:[esi].winSize
	mov	edx, 1024
	mul edx  ; get the granularity in bytes
	mov	bankSize, eax
	xor	eax, eax
	mov	ax, es:[esi].modeGranularity
	mov	edx, 1024
	mul edx  ; get the granularity in bytes
	mov	granularity, eax
	mov	al, es:[esi].winA
	.if !(al & 4)	; winA writeable (must be)
		pop	es
		; free up our info and mode buffers
		mov	dx, vesaInfoSelector
		mov	ax, 101h
		int	31h

		mov dx, modeInfoSelector
		mov	ax, 101h
		int 31h

		mov	eax, -1	; non-standard vesa - don't support it
		ret
	.endif


	xor eax, eax
	mov ax, es:[esi].winASegment
	shl	eax, 4		; make it an offset
	mov writeSegment, eax

	xor eax, eax
	mov ax, es:[esi].winBSegment
	shl	eax, 4		; make it an offset
	mov readSegment, eax


	; determine which window to use for reading from video memory
	mov	al, es:[esi].winB
	and	al, 2			; is it readable?
	.if zero?
		; ok, what about windowA
		mov	al, es:[esi].winA
		and al, 1		;is it readable?
		.if !zero?
			mov	readWindow, 0	; use window A
			mov eax, writeSegment	
			mov readSegment, eax
		.else	; NOT SUPPORTED or ERROR
			pop	es
			; free up our info and mode buffers
			mov	dx, vesaInfoSelector
			mov	ax, 101h
			int	31h

			mov dx, modeInfoSelector
			mov	ax, 101h
			int 31h

			mov	eax, -1		; non-standard vesa - don't support it
			ret				; an error - one or the other must be
		.endif
	.else
		mov	readWindow, 1	; use window B (more typical)
	.endif

	pop	es

	; free up our info and mode buffers
	mov	dx, vesaInfoSelector
	mov	ax, 101h
	int	31h

	mov dx, modeInfoSelector
	mov	ax, 101h
	int 31h

	mov	eax, bankSize
	xor	edx, edx
	mov	ebx, granularity
	div	ebx
	mov	grainsPerBank, eax

	ret

SetupVesa endp

;----------------------

; to allow interupt driven cursor to obtain the current window no.
GetVesaWindow proc public uses edi ebx ecx edx
	
	; must be called after SetVesa!
	mov	edi, offset aRealStack
	mov	[edi].r_eax, 4f05h
	mov	[edi].r_ebx, 0100h	; get window A No.
	mov	ax, 300h
	mov	bl, 10h
	xor	bh, bh
	mov	cx, 0
	int	31h
	
	xor	eax, eax
	mov	eax, edx	; return the window number in eax

	ret

GetVesaWindow endp

;----------------------

; this is a call to take the window # directly 
SetVesaWindow proc public uses edi ebx ecx edx, theWindowNo:dword
	
	; must be called after SetVesa!

	mov	edi, offset aRealStack
	mov	[edi].r_eax, 4f05h
	mov	[edi].r_ebx, 0 
	mov	eax, theWindowNo
	mov	[edi].r_edx, eax
	mov	ax, 300h
	mov	bl, 10h
	xor	bh, bh
	mov	cx, 0
	int	31h
	
	xor	eax, eax
	mov	eax, edx

	ret

SetVesaWindow endp



SetWindow proc public
	pushad
	mov	edi, offset aRealStack
	mov	[edi].r_eax, 4f05h
	mov	[edi].r_ebx, 0
	mov	eax, grainsPerBank
	xor	edx, edx
	mov	dx, windowNo
	mul	dx				; get the number of grains total
	mov	[edi].r_edx, eax
 
	mov	ax, 300h
	mov	bl, 10h
	xor	bh, bh
	mov	cx, 0
	int	31h

	popad
	ret

SetWindow endp

SetReadWindow proc public, theWindow:word
	pushad
	mov	edi, offset aRealStack
	mov	[edi].r_eax, 4f05h
	xor	eax, eax
	mov	al, readWindow	; normally WindowB
	mov	[edi].r_ebx, eax
	mov	eax, grainsPerBank
	xor	edx, edx
	mov	dx, theWindow
	mul	dx				; get the number of grains total
	mov	[edi].r_edx, eax
 
	mov	ax, 300h
	mov	bl, 10h
	xor	bh, bh
	mov	cx, 0
	int	31h

	popad
	ret

SetReadWindow endp


ShakeScreen proc public, shakeCount:dword, shakeDir:dword, ticksVar:dword

	pushad
	
	;Save the Start Address High and Low registers
	mov	dx,3d4h		   ;CRT controller register
	mov	al,0ch	   	;select Start Addr. High
	out	dx,al
	inc	dx
	mov	al,bh		      ;High byte of offset
	in		al,dx
	mov	ah,al
	dec	dx
	mov	al,0dh		   ;select Start Addr. Low
	out	dx,al
	inc	dx
	mov	al,bl		      ;Low byte of offset
	in		al,dx
	mov	stAddrSav,ax

	; set up di based on shakeDir
	xor	edi, edi
	test	shakeDir, 1	   ;down
	.if	!zero? 
		.if currentVideoMode == 80h
			add	edi, 640*2
		.else                 
			add	edi, 320*2
      .endif
	.endif
	test	shakeDir, 2	   ;right
	.if	!zero?
		add	edi, 2
	.endif

	xor	bx,bx		      ;clear bx

	; shake screen by adjusting offset
	mov	ecx, shakeCount
	.repeat
		xor	bx,di		;offset
		add	bx,stAddrSav	;just in case start addr != 0
		mov	dx,3d4h		;CRT controller register
		mov	al,0ch		;select Start Addr. High
		out	dx,al
		inc	dx
		mov	al,bh		   ;High byte of offset
		out	dx,al
		dec	dx
		mov	al,0dh		;select Start Addr. Low
		out	dx,al
		inc	dx
		mov	al,bl		   ;Low byte of offset
		out	dx,al
		sub	bx,stAddrSav

		; wait for four ticks
		push	ecx
		push	edi
		mov	ecx, 3

		; address ticks Variable
		mov	edi, ticksVar
		.repeat
			; get current ticks
			mov	eax, [edi]
			.repeat
				; wait for ticks to change
			.until eax != [edi]
		.untilcxz

		; restore shake count and offset
		pop	edi
		pop	ecx
	.untilcxz

	; restore to original offset
	mov	bx,stAddrSav
	mov	dx,3d4h		   ;CRT controller register
	mov	al,0ch		   ;select Start Addr. High
	out	dx,al
	inc	dx
	mov	al,bh		      ;High byte of offset
	out	dx,al
	dec	dx
	mov	al,0dh		   ;select Start Addr. Low
	out	dx,al
	inc	dx
	mov	al,bl		      ;Low byte of offset
	out	dx,al

	; all done
	sub	ax, ax
	sub	dx, dx

	popad

	ret

ShakeScreen endp

;---------------------------------------------------------------------------

VesaSupport proc public  uses ebx esi edi

	local result:dword

	; allocate two blocks of 256 bytes (16 paragraphs)
	mov	ax, 100h
	mov bx, 16
	int 31h
	mov	vesaInfoSegment, ax
	mov	vesaInfoSelector, dx

	; setup the real mode stack with appropriate register values
	mov	esi, offset aRealStack
	mov	[esi].r_eax, 4f00h

	mov	ax, vesaInfoSegment
	mov	[esi].r_es, ax

	xor	eax, eax
	mov	[esi].r_edi, eax	

	mov	ax, 300h
	mov	bl, 10h
	xor	bh, bh
	mov	cx, 0
	mov	edi, esi
	int	31h

	mov	esi, offset VESAName
	mov	ax, vesaInfoSelector
	push es
	mov	es, ax
	xor	edi, edi
	mov	ecx, 4
	repe	cmpsb

	.if zero?
		
		; now check to be sure mode 101 is supported

		xor	esi, esi
		assume esi:ptr VgaInfoBlock
		xor eax, eax
		mov ax, word ptr es:[esi].VideoModePtr + 2
		shl eax, 4			; convert the "segment" to an offset 
		mov	si, word ptr es:[esi].VideoModePtr
		add	esi, eax
		assume esi:nothing
		pop	es

		.while 1
			mov	ax, es:[esi]
			inc	si
			inc si

			.if ax == 101h
				mov result, 1
				.break
			.elseif ax == 0ffh
				mov result, 0
				.break
			.endif
		.endw

	.else
		pop	es
		mov	result, 0
	.endif

	; free up our info buffer
	mov	dx, vesaInfoSelector
	mov	ax, 101h
	int	31h


	mov	eax, result

	ret

VesaSupport endp

SetVMapWidth proc public, newWidth:dword

	mov eax, newWidth
	mov vmapWidth, eax
	ret

SetVMapWidth endp


	END
