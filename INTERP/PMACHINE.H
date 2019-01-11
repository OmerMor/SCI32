PMachine 	struct

	game					dw	0		;; Object ID
	object				dw	0		;; Object ID
	
 	acc					dd	0		;; Accumulator
 	prevAcc				dd	0		;; Previous Accumulator

 	StackPtr				dd	0
 	TempPtr				dd	0
 	GlobalPtr			dd	0
 	ParmPtr				dd	0

 	Locals				dw	0
	Globals				dw	0

	disposeTossRet		dw	0
 	RestArgs				dw	0

	curScriptNum		dw	0		;; Current Script Number
	curSourceFile		db 256 dup (0)
	curSourceLineNum	dw 0

	restart				dd	4 dup (0)

	ip						dd 0
	ipMemID				dw 0
	ipPtr					dw	0
	ipStack				dw 3*100 dup (0)
	
PMachine	ends

MemID 	struct
	memInfo		dd	?
	data			dd	?
	byteSize		dd	?
	client		dd	?
	next			dw	?
	prev			dw	?
	nextO			dw	?
	prevO			dw	?
	tempLink		dw	?
	resNumber	dw	?
	checksum		dw	?
	lockCount	db	?
	initChar		db	?
MemID	ends

memoryMgr	struct

	vfptr					dd	?
	maxDescriptors		dd	?
	desc					dd	?
	firstDesc			dw	?
	pad1					dw	?
	needMoreDescs		dd	?
	freeList				dw	?
	freeListEnd			dw	?
	mruList				dw	?
	mruListEnd			dw	?
	ordList				dw	?
	ordListEnd			dw	?
	pad2					dw	?
	totalMemory			dd	?
	memoryBase			dd	?
	blackHoleAddr		dd	?
	trashArray			dd	?
	moveToFront			dd	?
	checkVolatile		dd	?
	info					dd	?

memoryMgr	ends

ObjectID	struct

	id			dw ?		; 0 ; MEMID		; our id
	osize		dw ?		; 2 ; PROPERTY	; number of properties in the object
	propDict	dw ?		; 4 ; MEMID		; handle to property dictionary
	methDict	dw ?		; 6 ; MEMID		; handle to method dictionary
	classNum	dw ?		; 8 ; PROPERTY	; class number
	script	dw ?		; a ; MEMID		; handle to script node for the object
	super		dw ?		; c ; MEMID		; handle to super class of object
	info		dw ?		; e ; PROPERTY	; bit-mapped information

ObjectID ends

ScriptID	struct

	next			dw ?		;	Next script node
	prev			dw ?		;	Prev script node
	key			dw ?		;	script key
	pad1			dw	?		;	Pad out to DWORD alignment
	nRefs			dd ?		;	# of objects that point to script
	vars			dw ?		;	id of variables
	pad2			dw	?		;	Pad out to DWORD alignment
	dispatches	dw ?		;	id of dispatch table
	pad3			dw	?		;	Pad out to DWORD alignment
	nIDs			dd ?		;	# of objects and strings
	ids			dw ?		;	id of table of objects and strings
	pad4			dw	?		;	Pad out to DWORD alignment
	code			dw ?		;	id of procedure and method code
	codeFixups	dw ?		;	table of fixups for restoring savegame
	nCodeFixups dd ?		;	how many
	codeOffset	dd ?		;	offset of code from beginning of .SCR

ScriptID ends

s_play	EQU	51
s_replay	EQU	73

