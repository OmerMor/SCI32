	.386
	.model	flat, SYSCALL



	.code


	PUBLIC	DecompressData


N		EQU	4096
LEND	EQU	15
F		EQU	(255+LEND+3)


DecompressData PROC

	PUSH	EBP
	MOV	EBP,ESP
	PUSH	ESI
	PUSH	EDI
	PUSH	EBX

	MOV	ESI,[EBP+8]
	MOV	EDI,[EBP+12]

	LODSD
	MOV	TailleOut,EAX
        MOV	Return,EAX

	MOV	EBX,4096-18
	MOV	Mode,100
	CMP     DWORD PTR [ESI],1450709556
	JNE	Lb1
	ADD	ESI,4
	MOV	EBX,F
	MOV	Mode,LEND+3
Lb1:
	PUSH	EDI
	LEA	EDI,BufferDec
	MOV	ECX,EBX
	MOV	EAX,020202020h
	SHR	ECX,1
	SHR	ECX,1
	REP	STOSD
	MOV	ECX,EBX
	AND	ECX,3
	REP	STOSB
L0:
	POP	EDI
	LEA	EBP,BufferDec

	MOV	DL,1
	JMP	Lb0

Lb4:
	LODSB
	CMP	AL,-1
	JNE	Lb4A

	MOV	ECX,8
	CMP	TailleOut,ECX
	JBE	Lb4A

	SUB	TailleOut,ECX
	SHR	ECX,1

	CMP	EBX,N-8
	JB	Lb4Y
Lb4Z:
	LODSW
	STOSW
	MOV	DS:[EBX+EBP],AL
	INC	EBX
	AND	EBX,0FFFh
	MOV	DS:[EBX+EBP],AH
	INC	EBX
	AND	EBX,0FFFh
	DEC	ECX
	JNE	Lb4Z
	JMP	SHORT Lb4

Lb4Y:
	LODSD
	STOSD
	MOV	DS:[EBP+EBX],EAX
	ADD	EBX,4
	LODSD
	STOSD
	MOV	DS:[EBP+EBX],EAX
	ADD	EBX,4
	JMP	SHORT Lb4

Lb4A:
	MOV	DL,8
	SHR	AL,1
	MOV	DH,AL
	JNC	Lb4b
Lb5:
	LODSB
	STOSB
	MOV	DS:[EBP+EBX],AL
	INC	EBX
	AND	EBX,0FFFh
	DEC	TailleOut
	JE	Lb3
Lb0:
	DEC	DL
	JE	Lb4
	SHR	DH,1
	JC	Lb5
Lb4b:
	LODSW
	MOV	CL,AH
	XOR	CH,CH
	AND	CL,0Fh
	SHR	AH,4
	XCHG	AX,CX
	ADD	AX,3
	CMP	AL,Mode
	JE	Lb7X
Lb7:
	XCHG	AX,CX
	MOV	Longueur,CX
	MOV	WORD PTR SaveBP,AX
	ADD	AX,CX
	ADD	CX,BX
	CMP	AX,N
	JAE	Lb8
	CMP	CX,N
	JAE	Lb8
	CMP	AX,BX
	JB	Lb9
	CMP	CX,WORD PTR SaveBP
	JAE	Lb10

Lb9:
	PUSH	ESI
	MOV	ESI,EBP
	ADD	ESI,SaveBP
	SUB	CX,BX
	SHR	CX,1
	JNC	L2
	LODSB
	STOSB
	MOV	DS:[EBP+EBX],AL
	INC	EBX
L2:
	LODSW
	STOSW
	MOV	DS:[EBP+EBX],AX
	INC	EBX
	INC	EBX
	DEC	ECX
	JE	L9
	LODSW
	STOSW
	MOV	DS:[EBP+EBX],AX
	INC	EBX
	INC	EBX
	DEC	ECX
	JE	L9
	LODSW
	STOSW
	MOV	DS:[EBP+EBX],AX
	INC	EBX
	INC	EBX
	DEC	ECX
	JNE	L2
L9:
	POP	ESI
	MOV	CX,Longueur
	SUB	TailleOut,ECX
	JA	Lb0
	JMP	Lb3
Lb7X:
	LODSB
	ADD	AX,18
	JMP	Lb7
Lb8:
	PUSH	EDX
	MOV	EDX,SaveBP
	SUB	CX,BX
Lb88:
	MOV	AL,DS:[EBP+EDX]
	STOSB
	MOV	DS:[EBP+EBX],AL
	INC	EBX
	AND	EBX,0FFFh
	INC	EDX
	AND    	EDX,0FFFh
	DEC	ECX
	JE	Lb89
	MOV	AL,DS:[EBP+EDX]
	STOSB
	MOV	DS:[EBP+EBX],AL
	INC	EBX
	AND	EBX,0FFFh
	INC	EDX
	AND    	EDX,0FFFh
	DEC	ECX
	JNE	Lb88
Lb89:
	POP	EDX
	MOV	CX,Longueur
	SUB	TailleOut,ECX
	JBE	Lb3
	JMP	Lb0
Lb10:
	PUSH	ESI
	MOV	ESI,EBP
	ADD	ESI,SaveBP
	SUB	CX,BX
Lk1:
	LODSB
	STOSB
	MOV	DS:[EBP+EBX],AL
	INC	EBX
	DEC	ECX
	JE	Lk9
	LODSB
	STOSB
	MOV	DS:[EBP+EBX],AL
	INC	EBX
	DEC	ECX
	JE	Lk9
	LODSB
	STOSB
	MOV	DS:[EBP+EBX],AL
	INC	EBX
	DEC	ECX
	JE	Lk9
	LODSB
	STOSB
	MOV	DS:[EBP+EBX],AL
	INC	EBX
	DEC	ECX
	JNE	Lk1
Lk9:
	POP	ESI
	MOV	CX,Longueur
	SUB	TailleOut,ECX
	JA	Lb0
Lb3:
	MOV	EAX,Return
	POP	EBX
	POP	EDI
	POP	ESI
	POP	EBP
	RET

DecompressData ENDP

MemcpyDBL proc public,ptrD:DWORD,ptrS:DWORD,s:DWORD,sx:DWORD
; arguments passed by register from C++	
;	mov	edi, ptrD	
;	mov	esi, ptrS
;	mov	ebx, sx
;	mov	ecx, s

	push 	edi
	push 	ecx

loopStart:
	lodsb	
 	mov	ah, al	
	stosw
	loop	loopStart

	pop  	ecx
	pop  	esi
	mov  	edi, esi
	add	edi, ebx	
	rep	movsw

	ret
MemcpyDBL endp

MemcpyBlack proc public,ptrD:DWORD,ptrS:DWORD,s:DWORD
; arguments passed by register from C++	
;	mov	edi, ptrD	
;	mov	esi, ptrS
;	mov	ecx, s

loopStart:
	lodsb	
 	mov	ah, al	
	stosw
	loop	loopStart

	ret
MemcpyBlack endp

	.data

	ALIGN	4
TailleOut	DD	0
Return		DD	0
SaveBP		DD	0
Longueur		DW	0
Mode			DB	0
BufferDec 	DB	(N+F+1+32) DUP (0)

	end
