

	.386
	.MODEL	FLAT,SYSCALL
	.CODE


SetWindow PROTO
extrn windowNo:word
extrn bankSize:dword
extrn granularity:dword
extrn writeSegment:dword
extrn readSegment:dword

xCel STRUC
	pageOffset	dd	?
	rleOffset	dd ?
	dataOffset	dd ?
	xOffset		dw	?
	yOffset		dw ?
	xSize			dw	?
	ySize			dw ?
	compressType db ?
xCel ends

MAXX	equ	320
MAXY	equ	200
PAGE0		equ	0
PAGE1		equ	80
PAGE2		equ	160
PAGE3		equ	240

BadPointer PROTO thePointer:DWORD, row:DWORD


.data

extrn modeInfoSelector:word
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

	extrn aRealStack:realStack
	extrn currentVideoMode:byte


maxPtr		dd	0
minPtr		dd 	0
currentPage	dw 0
currentMode	db	0
baseTable	dw	MAXY dup(0)
maskTable	db	1,2,4,8,15
xLocation		dd	?
currBank			dw ?
startBank	dw ?
endBank		db ?
fullXDim			dd ?
xStart			dw ?
remainingLines dw ?
lineCounter		dw ?
vPageOffset		dd ?
lineOffset		dd ?
noLines			dw ?
yStart			dw ?
bankCount		dd ?
activeBank	db	?
bankLengths		dw	4 dup (?)
bankStarts		dw 4 dup (?)
remainingPixels dd ?
linesToDo	dd	?




.code



FillVideoPage proc public uses esi, frame:near ptr, minData: near ptr, maxData:near ptr

	mov	eax, maxData
	mov maxPtr, eax
	mov	eax, minData
	mov	minPtr, eax
	mov	minPtr, eax
	mov	esi, frame

	.if currentVideoMode == 1
		call RectDisplayX
	.elseif currentVideoMode == 80h
		call RectDisplayVesa
	.endif

	ret;
FillVideoPage endp
	


RectDisplayX proc uses esi edi ebx

	; esi = xCel offset

local rleBufferStart:dword
local pixelDataStart:dword, xLinesToDo:sword

	; general initialization

	xor	ecx, ecx				; clear the extended register, never use

assume esi:ptr xCel

	; setup y parameters - straight forward
	mov	ax, [esi].yOffset
	mov	yStart, ax
	mov	dx, [esi].ySize
	mov	noLines, dx

	mov	bankCount, 0

	mov	ax, [esi].xOffset
	mov	xStart, ax

	mov	ebx, [esi].pageOffset
	mov	vPageOffset, ebx
	add	vPageOffset,0a0000h

	xor eax, eax
	mov	ax, [esi].xSize	
	mov	fullXDim, eax

	.if [esi].compressType == 0
		call	BlockOutputX
		ret
	.endif

	;ret

; starting point for difference

	; setup esi to point to the cel data buffer
	mov	ebx, [esi].dataOffset
	mov	pixelDataStart, ebx
	mov	edx, [esi].rleOffset
	mov	rleBufferStart, edx

; call	ClearPageMode

	mov	currBank, 0
	
	.while currBank < 4
	
		mov	ax, currBank
		call	SelectBank			; make the bank addressable in video memory

		mov	ebx, pixelDataStart  ; For each bank we start over 
		mov	esi, rleBufferStart  ; at the beginning of rle codes & pixel data

		mov	dx, noLines
		mov	remainingLines, dx
		mov	xLocation, 0
		mov	lineCounter, 0

		; for each new bank, initialize the line's offset 
		mov	ax, yStart
		shl	ax, 1
		xor	edi, edi
		mov	di, ax
		mov	di, baseTable[edi]		; get the 320 mode starting address
		shr	di, 1			; that offset/4 = ourOffset
		shr	di, 1
		add	edi, vPageOffset			; video memory pageOffset
		mov	lineOffset, edi

		.while sword ptr remainingLines > 0
  	
			call	GetRunLengthsX		; returns dx = # of lines, cx = # of pixels
			
;push	bx
;mov	bx, vPageOffset
;call DisplayPage
;pop	bx


			.if cx		; ie there is some data to put out
				.repeat			; for 1 for more

					call	ProcessPixels	; all or part of a line					
					mov	eax, xLocation
					.if eax >= fullXDim
						mov	xLocation, 0
						inc	lineCounter;
						dec	remainingLines
						dec	dx
				
						; for each new line, setup the line's offset ONCE
						xor	edi, edi
						mov	di, lineCounter		; get starting row address
						add	di, yStart
						shl	di, 1
						mov	di, baseTable[edi]		; get the 320 mode starting address
						shr	di, 1			; that offset/4 = ourOffset
						shr	di, 1
						add	edi, vPageOffset	; video memory pageOffset
						mov	lineOffset, edi

					.endif

				.until sword ptr dx <= 0

			.else		; it's skip 

				add	lineCounter, dx		; dx can be 0 
				sub	remainingLines, dx

				.if dx
					mov	xLocation, 0
					; for each new line, setup the line's offset ONCE
					xor	edi, edi
					mov	di, lineCounter		; get starting row address
					add	di, yStart
					shl	di, 1
					mov	di, baseTable[edi]		; get the 320 mode starting address
					shr	di, 1			; that offset/4 = ourOffset
					shr	di, 1
					add	edi, vPageOffset			; video memory  pageOffset
					mov	lineOffset, edi

				.endif

			.endif

		.endw

		inc	currBank

	.endw
			
	ret			
		
RectDisplayX endp



RectDisplayVesa proc uses esi edi ebx

	; esi = xCel offset

local rleBufferStart:dword
local pixelDataStart:dword

	; general initialization

	xor	ecx, ecx				; clear the extended register, never use
	xor	eax, eax


assume esi:ptr xCel

	; setup y parameters - straight forward
	mov	ax, [esi].yOffset
	mov	yStart, ax
	mov	ax, [esi].ySize
	mov	noLines, ax

	mov	ax, [esi].xOffset
	mov	xStart, ax

	mov	eax, writeSegment
	mov	vPageOffset,eax
	xor eax, eax

	mov	ax, [esi].xSize	
	mov	fullXDim, eax

	.if [esi].compressType == 0
		call	BlockOutputVesa
		ret
	.endif

; starting point for difference

	mov	eax, 640
	sub	ax, [esi].xSize
	mov	lineOffset, eax	; offset from the end of 1 line to the starting x of the next

	; setup esi to point to the cel data buffer
	mov	ebx, [esi].dataOffset
	mov	pixelDataStart, ebx
	mov	edx, [esi].rleOffset
	mov	rleBufferStart, edx

	mov	esi, rleBufferStart  ; at the beginning of rle codes & pixel data
	mov	dx, noLines
	mov	remainingLines, dx
	mov	lineCounter, 0

	xor	eax, eax
	mov	ax, yStart
	mov	edx, 640
	mul edx
	xor	ebx, ebx
	mov	bx,  xStart
	add	eax, ebx
	xor	edx, edx
	div	bankSize
	mov	windowNo, ax
	call SetWindow
	mov	edi, edx
	mov	eax, bankSize
	sub	eax, edx
	mov	edx, eax		; the pixels remaining in the Window kept here

	mov	ebx, pixelDataStart  ; the actual pixel data (colors)
	add	edi, writeSegment
	mov	eax, fullXDim
	mov	remainingPixels, eax



	.while sword ptr remainingLines > 0
  	
		call	GetRunLengthsVesa	; cx = # of pixels
			
		.if !carry?		; ie there is some data to put out

			.repeat			; for 1 for more

				push ecx	; for line repeat
				sub	remainingPixels, ecx

				.while cx
					mov	al, [ebx]
					inc	ebx
					stosb
					dec	edx			; remaining in the bank
					.if zero?
						inc windowNo
						call SetWindow
						mov	edi, writeSegment
						mov	edx, bankSize
					.endif
					dec cx
				.endw

				.if remainingPixels == 0
					mov	eax, fullXDim
					mov remainingPixels, eax
					inc	lineCounter;
					dec	remainingLines
					dec	linesToDo

					add	edi, lineOffset
					sub	edx, lineOffset		; account for pixels bypassed by start of line
					.if zero? || carry? 
						inc	windowNo;
						call SetWindow
						mov	eax, edx
						add	edx, bankSize
						mov	edi, writeSegment
						neg	eax			; how far we went over is now our offset
						add	edi, eax
					.endif	
				.endif

				pop ecx		; for line repeat

			.until sword ptr linesToDo <= 0

		.else		; it's skip 

			add	edi, ecx		; account for the run of skip
			sub	edx, ecx		; run of skip
			.if zero? || carry?
				inc	windowNo
				call SetWindow
				mov	eax, edx
				add	edx, bankSize
				mov	edi, writeSegment
				neg	eax
				add	edi, eax
			.endif

			sub remainingPixels, ecx	
			.if remainingPixels == 0
				mov	eax, fullXDim
				mov remainingPixels, eax
				inc	lineCounter;
				dec	remainingLines
				dec	linesToDo

				add	edi, lineOffset	; to the start of the next line
				sub	edx, lineOffset	; pixels bypassed by end & start of line
				.if zero? || carry? 
					inc	windowNo;
					call SetWindow
					mov	eax, edx
					add	edx, bankSize
					mov	edi, writeSegment
					neg	eax			; how far we went over is now our offset
					add	edi, eax
					xor ecx, ecx
				.endif	
			.endif

			.if sword ptr linesToDo > 0		; skip to new line
				mov eax, linesToDo
				sub	remainingLines, ax
				add	lineCounter, ax
				mov	eax, fullXDim
				mov	remainingPixels, eax
				mov	ax, lineCounter		; get starting row address
				add	ax, yStart
				push ebx
				mov	ebx, 640
				imul eax, ebx
				add	ax, xStart
				xor	edx, edx
				mov	ebx, bankSize
				div	ebx
				.if ax != windowNo 
					mov	windowNo, ax
					call SetWindow
				.endif

				mov	edi, writeSegment
				add	edi, edx
				neg	edx
				add	edx, bankSize
				pop	ebx

			.endif


		.endif

	.endw

	ret			
		
RectDisplayVesa endp


;===========================================================================

; ModeX


BlockOutputX proc 

; NOTE: Now (01/30/93) works only for mode 13X VERTICAL Frames!

; local endBank:byte, startBank:word
local doStartAdjust:dword, adjustedRun:byte
local bufferStart:dword

	; the remainder of the x offset comes automatically from the bank 
	; which is the last 2 bits of the horizontal offset
	xor eax, eax
	mov	ax, [esi].xOffset
	and	ax, 3
	mov	startBank, ax
	mov	bankCount, 0

	mov	ax, [esi].xSize	
	mov	fullXDim, eax

	; setup the ending bank #
	xor	ebx, ebx
	mov	bx, startBank			; add in the starting bank as word
	add	ax, bx
	dec	ax							; offset to 0
	and	ax, 3
	mov	endBank, al

	; setup arrays identifying the starting offset byte and the count for 
	; each bank.

	mov	ax, [esi].xSize			; find the minimum run count
assume si:nothing
	add	ax, startBank
	dec	ax					 		; offset to 0
	shr	ax, 1						; = to length/4
	shr	ax, 1	
	xor	ecx, ecx	 
	xor	ebx, ebx

	.while cx < 4

		mov	bankLengths[ebx], ax	; minimum run length 

		; now check to see if we need to add one for the endBank

		.if cl <= endBank
			inc	bankLengths[ebx]	; add in 1 for a remainder bank
		.endif

		; adjust if the bank is not put out for the first byte location
		.if	cx < startBank
			dec	bankLengths[ebx]		; this bank does one less
			mov	bankStarts[ebx], 1 	; we need to start displaying 1 byte over
		.else
			mov	bankStarts[ebx], 0	; don't need to adjust the starting display location 
		.endif


		inc	bx			; point to the next word of the data arrays
		inc	bx
		inc	cx			; the bank counter

	.endw

	; setup si to point to the cel data buffer
assume esi:ptr xCel
	mov	eax, dword ptr [esi].rleOffset;
	mov	bufferStart, eax
assume esi:nothing

	xor	eax, eax
	mov	ax, startBank

BlockBankLoop:		; loop here for each video bank
  	
	mov	currBank, ax		; al initialized just above or at bottom of loop
	call	SelectBank			; make the bank addressable in video memory

	; get the count for this bank

	xor	ah, ah				; clear it (al = current bank)
	shl	al, 1					; the offset must be words
	xor	esi, esi
	mov	si, ax				; point to this bank's length
	mov	cx, bankLengths[esi]
	xor	eax, eax
	mov	ax, bankStarts[esi]
	mov	doStartAdjust, eax	; setup in handy word for adding (may be 0)

	mov	dx, noLines
	mov	bx, yStart		; the line #
	mov	esi, bufferStart

linesLoop:		; loop here for each line
	
	push	esi		; save the source start address
	add	esi, bankCount	; the pixel to start with
	
	xor	edi, edi
	mov	di, bx		; get starting row address
	shl	di, 1
	mov	di, baseTable[edi]		; get the 320 mode starting address
	add	di, xStart
	shr	di, 1			; that offset/4 = ourOffset
	shr	di, 1
	add	edi, vPageOffset			; video memory pageOffset
	add	edi, doStartAdjust

	push	cx

	.while cx
		mov	al, [esi]
		stosb
		add	esi, 4			; every 4th pixel
		dec cx
	.endw
	
	pop	cx				; our adjusted xDim
	pop	esi				; start of line address
	add	esi, fullXDim

	inc	bx				; the line number
	dec	dx				; line counter
	jnz	linesLoop	

	inc	bankCount	
	inc	currBank
	mov	ax, currBank
	.if	al == 4
		xor	ax, ax
	.endif

	.if bankCount < 4
		jmp	BlockBankLoop
	.endif

	ret

BlockOutputX endp




BlockOutputVesa proc 


local bufferStart:dword

	xor eax, eax
	xor	ecx, ecx	 
	xor	ebx, ebx

	mov	esi, [esi].rleOffset;
assume esi:nothing

	; Find the starting window #
	xor	eax, eax
	mov	ax, yStart		; the line #
	mov	edx, 640
	imul eax, edx
	add	ax, xStart
	xor	edx, edx
	div	bankSize
	mov	windowNo, ax
	call SetWindow
	mov	edi, edx			; the offset is the remainder
	add	edi, writeSegment
	mov	eax, bankSize
	sub	eax, edx			; how much of the bank is consumed at the start
	mov	edx, eax			; keep it here for quick access
	mov	bx, noLines
	mov	ecx, fullXDim

	mov	eax, 640
	sub	eax, fullXDim
	mov	lineOffset, eax
	
	xor	eax, eax

linesLoop:		; loop here for each line
	
	push	esi		; save the source start address

	.if edx > ecx
		sub	edx, ecx
		rep movsb
	.else
		xchg ecx, edx
		sub edx, ecx
		rep	movsb
		mov	ecx, edx
		inc windowNo
		call SetWindow
		mov edi, writeSegment
		mov edx, bankSize
		sub edx, ecx
		rep movsb

	.endif

	
	add	edi, lineOffset
	sub	edx, lineOffset
 	.if zero? || carry?
		inc windowNo
		call SetWindow
		mov	eax, edx
		add	edx, bankSize	; edx has zero or neg number
		mov	edi, writeSegment
		neg eax
		add	edi, eax
	.endif

	pop	esi				; start of line address
	add	esi, fullXDim
	mov ecx, fullXDim

	dec	bx				; line counter
	jnz	linesLoop	

	ret

BlockOutputVesa endp


;===========================================================================

ProcessPixels proc uses edx 

; Output pixels for 1 plane, 1 run length to the screen.
; As we go through the loop, we compare a running count of data to the
; size of the data run.  Once we exceed that, we are done

; ecx = # of pixels to do (as if we were doing all 4 planes)
; ebx = the start of the data area for this run
		
	; setup the destination address
	

	xor	eax, eax
	xor	edx, edx
	mov	edi, lineOffset
	mov	eax, xLocation	; get the byte offset based on the screen x location
	add	ax, xStart
	shr	ax, 1
	shr	ax, 1
	add	edi, eax

	push	ebx
	push	cx

	mov	eax, xLocation	; find the starting bank based on the screen location
	add	ax, xStart
	and	ax, 3					; the starting bank for this run

	add	cx, ax				
	dec	cx
	shr	cx, 1	
	shr	cx, 1				; ecx is the starting x run count
	mov	dx, currBank

	.if	currBank < ax
		inc	edi
		mov	dx, currBank
		add	dx, 4
		sub	dx, ax
		add	ebx, edx	; now the starting source byte for THIS bank
		dec	cx
	.elseif currBank > ax
		mov	dx, currBank
		sub	dx, ax
		add	ebx, edx
	.endif


	; now check to see if we need to add one for the endBank
	; first, find the ending bank
	pop	dx			; the length originally from cx
	push	dx			; we'll need it later
	add	ax, dx
	dec	ax			; offset to 0
	and	ax, 3		; the ending bank for the run

	.if currBank <= ax
		inc	cx
	.endif


	.while cx > 0

		mov	al, [ebx]
		stosb
		add	ebx, 4			; every 4th pixel
		dec	cx

	.endw
	
	pop	cx
	add	xLocation, ecx
	pop	ebx					; get back the start location of this run
	add	ebx, ecx			; now pointing to the next run

	ret

ProcessPixels	endp
;===========================================================================

align 4
controlCodes dd offset controlError	; 0
	dd offset controlError				; 1
	dd offset pixelSkipWord				; 2
	dd	offset pixelDataWord				; 3
	dd offset pixelDataSkip				; 4
	dd offset controlError				; 5
	dd offset lineDataWord				; 6
	dd	offset lineSkipWord				; 7
	dd	offset lineDataSkip				; 8


GetRunLengthsX	proc uses ebx

	xor	edx, edx		; no "lines to do"
	xor	ecx, ecx
	xor	eax, eax

	lodsb						; get the rle code

	test	al, 80h
	.if !zero?
			
		; it's a byte code (doing only one line too)


		test	al, 40h
		.if !zero?

			; it's skip - bump xLocation 		

			xor	cx, cx			; won't be doin any data
			and al, 3fh	 			; how many skip to add in
			
			.if zero?		
				; skip to the end of the line
				mov	dx, 1
			.else
				add	xLocation, eax
			.endif

		.else		; it's data, not skip

			and	al, 3fh
			.if zero?
				; remainder of line is data
				mov	ecx, fullXDim
				sub	ecx, xLocation
;				mov	dx, 1
			.else
				mov	cx, ax			; run of bytes to do
			.endif

		.endif
		
		ret

	.else								; its a word rle code

		xor	ebx, ebx
		mov	bl, al
		shr	bl, 1		; code * 4 = table offset

		mov	ah, al
		lodsb			; get the next byte of the word code
		and	ax, 7ffh
		jmp	dword ptr cs:controlCodes[ebx]

	.endif

		pixelSkipWord::
			; Pixel Skip Word (will never complete a line)
			add	xLocation, eax
			xor	cx, cx
			jmp	runCodeDone

		pixelDataWord::
			; Pixel Data Word (will never complete a line)
			mov	cx, ax
			jmp	runCodeDone
	
		pixelDataSkip::
			
			mov	cx, ax
			jmp	runCodeDone

		lineDataWord::	
			
			; Line Data Word (only at start of line)
			mov	dx, ax
			.if zero?
				mov	dx, remainingLines
			.endif

			mov	ecx, fullXDim
			jmp	runCodeDone

		lineSkipWord::

			; Line Skip				; # lines to skip
			mov	dx, ax
			.if zero?
				mov	dx, remainingLines
			.endif
			xor	cx, cx		
			jmp	runCodeDone
	
		lineDataSkip::

			; Line Data Word With Skip (only at start of line)
			mov	dx, ax
			.if zero?
				mov	dx, remainingLines
			.endif

			mov	ecx, fullXDim
			jmp	runCodeDone

		controlError::

			xor	dx, dx
			xor	cx, cx

runCodeDone:

			ret


GetRunLengthsX	endp


align 4
controlCodesV dd offset controlErrorV	; 0
	dd offset controlErrorV				; 1
	dd offset pixelSkipWordV				; 2
	dd	offset pixelDataWordV				; 3
	dd offset pixelDataSkipV				; 4
	dd offset controlErrorV				; 5
	dd offset lineDataWordV				; 6
	dd	offset lineSkipWordV				; 7
	dd	offset lineDataSkipV				; 8

GetRunLengthsVesa	proc uses ebx

	mov	linesToDo, 0
	xor	ecx, ecx
	xor	eax, eax

	lodsb						; get the rle code

	test	al, 80h
	.if !zero?
			
		; it's a byte code (doing only one line too)


		test	al, 40h
		.if !zero?

			; it's skip -

			and al, 3fh	 			; how many skip to add in
			
			.if zero?		
				; skip to the end of the line
				mov	ecx, remainingPixels
				stc

			.else
				xor ecx, ecx
				mov	cl, al		
				stc			; indicate that it was a skip code

			.endif

		.else		; it's data, not skip

			and	al, 3fh
			.if zero?
				; remainder of line is data
				mov	ecx, remainingPixels
			.else
				mov	cx, ax			; run of bytes to do
			.endif

			clc		; not skip

		.endif
		
		ret

	.else								; its a word rle code

		xor	ebx, ebx
		mov	bl, al
		shr	bl, 1		; code * 4 = table offset

		mov	ah, al
		lodsb			; get the next byte of the word code
		and	ax, 7ffh
		jmp	dword ptr cs:controlCodesV[ebx]

	.endif

		pixelSkipWordV::
			; Pixel Skip Word (will never complete a line)
			mov	ecx, eax
			stc
			jmp	runCodeDoneV

		pixelDataWordV::
			; Pixel Data Word (will never complete a line)
			mov	ecx, eax
			clc
			jmp	runCodeDoneV
	
		pixelDataSkipV::
			
			mov	cx, ax
			clc
			jmp	runCodeDoneV

		lineDataWordV::	
			
			; Line Data Word (only at start of line)
			mov	linesToDo, eax
			.if zero?
				mov	ax, remainingLines
				mov	linesToDo, eax
			.endif

			mov	ecx, fullXDim
			clc
			jmp	runCodeDoneV

		lineSkipWordV::

			; Line Skip				; # lines to skip
			mov	linesToDo, eax
			.if zero?
				mov	ax, remainingLines
				mov	linesToDo, eax
			.endif

			mov	ecx, fullXDim
			stc
			jmp	runCodeDoneV
	
		lineDataSkipV::

			; Line Data Word With Skip (only at start of line)
			mov	linesToDo, eax
			.if zero?
				mov	ax, remainingLines
				mov	linesToDo, eax
			.endif

			mov	ecx, fullXDim
			clc
			jmp	runCodeDoneV

		controlErrorV::

			mov linesToDo, 0
			xor	cx, cx

runCodeDoneV:

			ret


GetRunLengthsVesa	endp



;===========================================================================

WaitForBlank proc public uses dx ax

; Wait for vertical blank to occur
; --------------------------------
	mov	dx, 03dah
wait1:
	in	al, dx
	and	al, 8
	jnz	wait1
wait2:
	in	al, dx
	and	al, 8
	jz	wait2
	ret

WaitForBlank endp
;===========================================================================

ClearMode13 proc  uses ax edi ecx

	mov	edi, 0a0000h
	xor	ax, ax
	mov	ecx, 32000
	rep	stosw
	ret

ClearMode13 endp

;===========================================================================

; clear all pages in paged graphics mode

ClearPageMode proc uses ax edi ecx


	mov	al, 4			; selects all 4 banks
	call	SelectBank
	mov	ecx, 8000
	mov	edi, 0a0000h
	xor	ax, ax
	rep	stosw

	ret

ClearPageMode endp
	

;===========================================================================

SC_INDEX equ	03c4h
CRTC_INDEX equ	03d4h
MISC_OUTPUT equ 3c2h


public SetVideoMode 
SetVideoMode proc uses edx edi ebx, newMode;
	mov	edx, newMode
	mov currentVideoMode, dl
	call SetMode

	mov	edi, offset baseTable
	xor	eax, eax
	mov	ecx, 200
	
	.repeat
		stosw
		add	ax, 320
	.untilcxz

	ret
SetVideoMode endp

SetMode proc 

	; dx = video mode
	; (0 = reset to mode 13)
	; (1 = Mode X, VERTICAL)
	; (2 = Mode X, HORIZONTAL)


	.if currentMode == 0
		call ClearMode13
	.endif

	call	WaitForBlank

	or	dx, dx
	jz	resetIt

	mov	currentMode, dl


	.if dx == 2
		; change the # of bytes between rows from 40 (default in mode 13h)
		;  to 160 (80 to 320 after * 2) for modeX horizontal format
		mov	dx, CRTC_INDEX
		mov	ax, 0a013h	; 160 + port index 13h	
		out	dx, ax

		mov	currentPage, PAGE1

	.endif
			
	; the following applys to both modeX vertical and horizontal

	mov	dx, SC_INDEX
	mov	ax, 0604h		; disable chain 4 mode
	out	dx, ax


	mov	dx, CRTC_INDEX
	mov	ax, 14h			; turn off dword mode
	out	dx, ax

	mov	ax, 0e317h		; set byte addressing mode	
	mov	dx, CRTC_INDEX
	out	dx, ax 

	; clear screen

	call	ClearPageMode

	.if currentMode == 2
		mov	bx, 80			; setup page '1'
		call	DisplayPage
	.endif

	ret

resetIt:

	; clear screen

	.if currentMode != 0
		call	ClearPageMode
	.endif

	mov	currentMode, 0

	mov	ax, 13h
	int	10h
	ret

SetMode endp


;===========================================================================

SelectBank proc

	pushad
	mov	activeBank, al
	xor	ebx, ebx
	mov	bl, al			; map #
	mov	ah, maskTable[ebx]
	mov	al, 2
	mov	dx, 3c4h
	out	dx, ax
	popad

	ret

SelectBank	endp




;===========================================================================

SelectReadBank proc uses ax dx

	; al = Page we wish to read

	; First, set Read Mode 0

	mov	ah, al		
	mov	al, 5
	mov	dx, 03ceh	; graphics reg/set mode
	out	dx, al
	mov	dx, 3cfh
	in		al, dx
	and	al, 0f7h		; mask out bit 3 for read mode 0
	out	dx, al

	; Now, Select the bank we want

	mov	dx, 3ceh
	mov	al, 4
	out	dx, ax
	ret

SelectReadBank endp


;===========================================================================

public ShowPage
ShowPage proc uses ebx, thePageOffset
	mov	ebx, thePageOffset
	call	DisplayPage
	ret
ShowPage endp




DisplayPage proc uses dx ax 

	; bx = page frame ADDRESS to display

	pushad
	call WaitForBlank

	mov	currentPage, bx
	mov	dx, 3d4h
	mov	al, 0ch
	out	dx, al		; select start address high

	mov	dx, 3d5h
	mov	al, bh		; the high address
	out	dx, al

	mov	dx, 3d4h
	mov	al, 0dh
	out	dx, al

	mov	dx, 3d5h
	mov	al, bl
	out	dx, al
	popad

	; call WaitForBlank

	ret

DisplayPage endp

;===========================================================================

SetOutputPage proc 
	mov	currentPage, ax
	ret
SetOutputPage endp

;===========================================================================

GetOutputPage proc 
	mov	ax, currentPage
	ret
GetOutputPage endp


;===========================================================================



public CopyPage
CopyPage proc uses esi edi ebx, destPage, srcPage

	mov	edi, destPage
	add	edi, 0a0000h
	mov	esi, srcPage
	add	esi, 0a0000h

; edi = destPage
; esi = srcPage

	mov	dx, 3ceh
	mov	ax, 08h
	out	dx, ax

	mov	al, 4			; select all 4 banks
	call	SelectBank

	.if currentMode == 1
		mov	ecx, 16000
		rep	movsb
	.else

		; for mode 13XHorizontal need to do it a line at a time
		mov	ecx, MAXY
		
		.repeat

			push	ecx
			mov	ecx, 80	; 80 bytes (* 4 planes) per line
			push	esi
			push	edi
			rep	movsb		; copy 1 line
			pop	edi
			pop	esi
			add	esi, 320	; next line addresses
			add	edi, 320
		
			pop	ecx

		.untilcxz


	.endif

	mov	ax, 0ff08h
	out	dx, ax
	
	ret

CopyPage endp

;===========================================================================

Copy2VMAPX proto, vmapAddr:dword
Copy2VMAPVesa proto, vmapAddr:dword

Copy2VMAP proc public uses esi edi, vmapAddr:dword

	.if currentVideoMode == 1
		invoke Copy2VMAPX, vmapAddr
	.elseif
		invoke Copy2VMAPVesa, vmapAddr
	.endif

	ret

Copy2VMAP endp

Copy2VMAPX proc, vmapAddr:dword

	mov	ecx, 0

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

	.repeat

		mov	al, cl
		out	dx, al

		push	ecx
		mov edi, vmapAddr
		add	edi, ecx
		mov	esi, 0a0000h

		mov	ecx, 16000
		.repeat
			lodsb
			mov	[edi], al
			add	edi, 4
		.untilcxz	

		pop	ecx		; the bank #
		inc	ecx
	

	.until ecx >= 4

	mov	al, 3
	out	dx, al

	ret

Copy2VMAPX endp


Copy2VMAPVesa proc, vmapAddr:dword

	local readWindow:byte
	ret		; !!!!! because there's no 640x480 vmap now
	pushad
	; first, determine the window to write to
	mov	ax, modeInfoSelector
	mov	es, ax
	xor	esi, esi
	assume esi:ptr ModeInfo

	mov	al, [esi].winB
	and	al, 2			; is it readable? (normally is)
	.if zero?
		; ok, what about windowA
		mov	al, [esi].winA
		and al, 1		;is it readable?
		.if !zero?
			mov	readWindow, 0	; use window A
		.else
			popad
			ret				; an error - one or the other must be
		.endif
	.else
		mov	readWindow, 1	; use window B (more typical)
	.endif

	mov	ax, ds
	mov	es, ax		; put es back (always == ds)

	mov	edi, vmapAddr
	mov	esi, readSegment
	mov ecx, 153600		; for 640x480 vmap 640*480/2
	mov	edx, bankSize
	mov	windowNo, 0

	.repeat

		lodsw	; get 2 pixels
		stosw	; put 2 pixels
		dec	edx
		dec edx	; (banks are always even in size)

		.if zero?		; we ran out of bank
			pushad
			inc	windowNo
			mov	edi, offset aRealStack
			mov	[edi].r_eax, 4f05h
			xor	eax, eax
			mov	al, readWindow
			mov	[edi].r_ebx, eax
			mov	eax, bankSize
			xor	edx, edx
			mov	ebx, granularity
			div ebx				; how many grains per bank
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
			mov	esi, readSegment	; start over at zero offset
		.endif

	.untilcxz

	popad
	ret

Copy2VMAPVesa endp

end
