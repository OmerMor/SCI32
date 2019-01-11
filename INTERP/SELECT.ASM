.386
ASSUME	cs:cseg
cseg	segment  USE32 public
		PUBLIC			NSelector;
GETLINEARBASEADDR		EQU	02508h
LINEARTOPHYSICALADDR	EQU	02509h
MAPPHYSICALMEMORY		EQU	0250Ah
GETMEMORYSTATISTICS	EQU	02520h
READWRITELDT			EQU	02531h
MINMAXMEMORYUSE		EQU	02536h
ALLOCATESEGMENT		EQU	048h
FREESEGMENT				EQU	049h
READLDT					EQU	0
WRITELDT					EQU	1
NOSETVMMSTATISTICS	EQU	0
RESETVMMSTATISTICS	EQU	1
MINIMIZEEXTENDED		EQU	0
MINIMIZECONVENTIONAL	EQU	1
MAXIMIZEEXTENDED		EQU	2
MAXIMIZECONVENTIONAL	EQU	3
;
; LDT offsets and equates
;
LIMIT0to15				equ	0
BASE0to15				equ	2
BASE16to23				equ	4
ACCESSRights			equ	5
LIMIT16to19				equ	6	   ;first nibble
GRANULARITY				equ	6	   ;second nibble
BASE24to31				equ	7
CLRGRANULARITY			equ	070h
SETGRANULARITY			equ	080h
CLRLIMIT16TO19			equ	0f0h
NSelector proc 
; Creates a selector in the LDT pointing to the address given.
;	 input:
;	   DWORD	starting address
;	   DWORD	limit
;	   WORD	data or exec selector
;				0 -> data selector
;				1 -> executable selector 
;	 output:
;	   WORD	selector (zero indicates an error)
;
startAddr	   equ	   8
limit		   	equ	   12
typeDC		   equ	   16
localBytes	   equ	   14
newSelector    equ	   -2		  	;size 2 bytes
linearAddr	   equ	   -6		  	;size 4 bytes
LDTentry	   	equ	   -14		;size 8 bytes

		push	ebp
		mov	ebp, esp
		sub	esp, localBytes
		push	ebx
		push	ecx
		push	edi
		push	es

		; create a selector with zero size
		mov	ax,  0000h
		mov	ecx,1
		int	31h
		mov	[newSelector+ebp], ax
		jc		error

		; get linear base address of selector
		mov	bx,  ds
		mov	ax, 00006h 
		int	31h 
		jc		error

		shl	ecx,16
		mov	cx,dx
		; calculate linear base address for new selector
		add	ecx, [startAddr+ebp]
		mov	[linearAddr+ebp], ecx

		; read in current memory selector
		mov	bx,  ds
		mov	es,bx
		lea	edi, [LDTentry+ebp]
		mov	ax,  000bh
		int	31h 
		jc		error

		; now alter it
		; put in the limit0to15
		lea	edx, [LDTentry+ebp]
		mov	eax, [limit+ebp]
		mov	[edx+LIMIT0to15], ax
		; put in the #limit16to19 and turn off granularity bits
		mov	ax,  [limit+ebp+2]
		and	al,  0fh
		mov	bl,  [edx+GRANULARITY]
		and	bl,  CLRGRANULARITY
		or		al,  bl
		mov	[edx+GRANULARITY], al
		; put in base0to15
		mov	ax, [linearAddr+ebp]
		mov	[edx+BASE0to15], ax
		; put in base16to23
		mov	ax,  [linearAddr+ebp+2]
		mov	[edx+BASE16to23], al
		; put in base24to31
		mov	[edx+BASE24to31], ah
		; do we make it executable?
		mov	ax, [typeDC+ebp]
		cmp	ax, 0
		je		writeit
		and	byte ptr [edx+ACCESSRights],0f1h
		or		byte ptr [edx+ACCESSRights],00Ah
writeit:
		; write the altered selector information to the new selector
		mov	bx,ds
		mov	es,bx
		mov	bx,  [newSelector+ebp]
		lea	edi, [LDTentry+ebp]
		mov	ax,  000ch
		int	31h 
		jc		error
		; successful return new selector
		mov	dx,  [newSelector+ebp]
		jmp	SHORT exit
error:
		xor	dx,  dx
exit:
		xor	eax, eax
		mov	ax,dx	 
		pop	es
		pop	edi
		pop	ecx
		pop	ebx
		add	esp, localBytes
		pop	ebp
		ret
NSelector endp
cseg	ENDS
		END
