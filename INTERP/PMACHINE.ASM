;; PMACHINE.ASM
;;
;; PMachine implementation
;;
;; Author: Bryan Waters
;; Author: Stephen Nichols
;;

.386
.model flat,syscall

	include pmachine.h		;; include the pmachine header
	include pmachine.i		;; include the pmachine macros

.data

 	include optbl.i			;; Load the opcode jump table

	pm		PMachine	<>
	pmStack		dd		5120 	dup(?)	;; Pmachine stack
	descriptorTbl	dd		?

.code

public	pm

extern	LoadPropOffsets:proc
extern	LoadClassTbl:proc
extern	GetDispatchEntry:proc
extern	InitLanguage:proc
extern	pmLock0:proc
extern	setupTossRet:proc
extern	asm_fatal:proc
extern	memMgr:dword

; ---------------------------------------------------------------------------
; -									    -
; - pmGameSetProp is used to set the language of the game.		    -
; -									    -
; - AX = the selector.							    -
; - DX = the value to store in that selector.				    -
; -									    -
; ---------------------------------------------------------------------------
pmGameSetProp	proc

	push	edi
	push	ebx
	push	ecx

	memIDGetPtr	pm.game		; ebx points to the object

	movzx	edi, (ObjectID ptr [ebx]).propDict
	memIDGetPtrDI
	add	edi, 4			; skip the nRefs
	push	edi
	movzx	ecx, (ObjectID ptr [ebx]).osize

	cld
	repne	scasw
	jne	notFound

	pop	eax
	sub	edi, eax
	mov	word ptr [edi + ebx - 2], dx

	pop	ecx
	pop	ebx
	pop	edi
	ret

notFound:
	mov	edx, 2
	jmp	asm_fatal

pmGameSetProp	endp

; ---------------------------------------------------------------------------
; -									    -
; - pmStackClear clears the stack to a special init value.		    -
; -									    -
; ---------------------------------------------------------------------------
pmStackClear	proc

	lea	edi, pmStack
	push	eax
	mov	eax, 1599554882
	mov	ecx, 5120
	rep	stosd
	pop	eax

	lea	edi, pmStack
	mov	pm.StackPtr, edi
	mov	pm.TempPtr, edi
	mov	pm.ParmPtr, edi

	ret

pmStackClear	endp

; ---------------------------------------------------------------------------
pmTossMemID	proc

	;;	a code handle has just become invalid.  if it's on the call stack
	;;	make sure its next instruction is a op_ret or an op_toss, op_ret
	;;	sequence and then replace it with the appropriate special code handle
	
	;;	a toss, ret sequence is permitted in disposed code because it is
	;;	generated at the end of a switch statement and it's safe to allow
	;;	the programmer to call DisposeScript in a switch clause
	
	;;	if the next instruction is op_lineNum, check that the next instruction
	;;	after that is a return (or a toss/ret), and replace it with just a return
	;; (or a toss/ret)

	push	ebx
	push	edx

	mov	edx, 0

__Check_all:
	cmp	[pm.ipStack + edx], cx
	je	__found_it

	add	edx, 6
	cmp	dx, pm.ipPtr
	jne	__Check_all

	pop	edx
	pop	ebx
	ret

__found_it:

	movzx	ebx, word ptr [pm.ipStack + edx]
	memIDGetPtrEBX		; ebx points to the object
	add	ebx, dword ptr [pm.ipStack + edx + 2]

	cmp	byte ptr [ebx], OPret
	je	__ret

	cmp	byte ptr [ebx], OPlineNum
	jne	__try_toss

	cmp	byte ptr [ebx + 3], OPret
	jne	__try_toss

__ret:
	mov	ax, pm.disposeTossRet
	mov	word ptr [pm.ipStack + edx], ax
	mov	dword ptr [pm.ipStack + edx + 2], 1

	jmp	__Check_all

__try_toss:
	cmp	byte ptr [ebx], OPtoss
	jne	__error

	cmp	byte ptr [ebx + 1], OPret
	je	__tossret

	cmp	byte ptr [ebx + 1], OPlineNum
	jne	__error

	cmp	byte ptr [ebx + 4], OPret
	jne	__error

__tossret:
	mov	ax, pm.disposeTossRet
	mov	word ptr [pm.ipStack + edx], ax
	mov	dword ptr [pm.ipStack + edx + 2], 0

	jmp	__Check_all

__error:
	mov	edx, 3
	jmp	asm_fatal

pmTossMemID	endp

; ---------------------------------------------------------------------------
pmQuickMessage	proc
	
;	EBX	=	obj
;	ECX	=	nArgChars

nArgs		EQU	<[ebp - 4]>
prevPMSP	EQU	<[ebp - 8]>
nextMsgParms	EQU	<[ebp - 12]>
selector	EQU	<[ebp - 16]>
obj		EQU	<[ebp - 20]>

	push	ebp
	mov	ebp, esp
	sub	esp, 20

	;	send a series of messages (method invocations and property accesses)
	;	to 'obj'

	mov	obj, ebx
	memIDGetPtrEBX		; ebx points to the object

	;	save important variables
	push	pm.Locals
	push	pm.ParmPtr
	push	pm.TempPtr
	push	pm.curScriptNum
	mov	pm.StackPtr, edi

	pushIP

	;	where stack ptr will point after all this object's messages have been
	;	popped
	mov	edx, edi
	sub	edx, ecx
	push	edx
	add	edx, 2

	;	where the parameters of the next message for this object start
	mov	nextMsgParms, edx

;	//	make sure this is an object
;	obj.AssureValid();

;IFDEF DEBUG
;	pm.sendStack->PushObject(obj);
;endif

	;	process all the messages for this object
__next_message:
	mov	pm.acc, eax
;	call	check_eax

	mov	esi, nextMsgParms
	mov	pm.ParmPtr, esi
	add	pm.ParmPtr, 2

	;	get selector
	movzx	edx, word ptr [esi]
	mov	selector, edx

	;	get parameters
	movzx	edx, word ptr [esi + 2]
	add	esi, 4

	shl	edx, 1

	;	adjust  known parameters by those added by &rest
	add	dx, pm.RestArgs
	mov	pm.RestArgs, 0

	;	decrement  args to process by this msg's parms +
	;	1 word for selector + 1 word for  args
	sub	ecx, edx
	sub	ecx, 4		

	;	save location of next msg's parms
	mov	nextMsgParms, esi
	add	nextMsgParms, edx

;IFDEF DEBUG
;		pm.sendStack->PushSelector(selector);
;endif

	push	ecx
	push	edi

	mov	eax, dword ptr selector

;;	memIDGetPtr	pm.game		; ebx points to the object

	movzx	edi, (ObjectID ptr [ebx]).propDict
	memIDGetPtrDI
	add	edi, 4			; skip the nRefs
	push	edi
	movzx	ecx, (ObjectID ptr [ebx]).osize

	cld
	repne	scasw
	jne	notFound

	test	edx, 0ffffh
	jnz	__setProp

__getProp:
	pop	eax
	sub	edi, eax
	movzx	eax, word ptr [edi + ebx - 2]

	pop	edi
	pop	ecx
	jecxz	__processed
	jmp	__next_message

__setProp:
	pop	eax
	sub	edi, eax
	mov	dx, word ptr [esi]
	mov	word ptr [edi + ebx - 2], dx

IFDEF NETWORK
	cmp	di, 0056h + 2
	ja	__not_Critical
	cmp	di, 0030h + 2
	jb	__not_Critical
ELSE
	cmp	di, 0058h + 2
	ja	__not_Critical
	cmp	di, 0034h + 2
	jb	__not_Critical
ENDIF

	or	word ptr [ebx + 14], 0008h

__not_Critical:
	pop	edi
	pop	ecx

	mov	eax, pm.acc
;	call	check_eax

	jecxz	__processed
	jmp	__next_message

__processed:
	;;	restore variables
	pop	edi

	popIP

	pop	pm.curScriptNum
	pop	pm.TempPtr
	pop	pm.ParmPtr
	pop	pm.Locals


;IFDEF DEBUG
;	pm.sendStack->Pop();
;endif

	mov	esp, ebp
	pop	ebp

	ret

notFound:
	pop	edi
	pop	edi

	shr	edx, 1
;	mov	edx, selector


__while_loop:
	push	ebx
	movzx	ebx, (ObjectID ptr [ebx]).script
	memIDGetPtrEBX
	mov	bx, (ScriptID ptr [ebx]).key
	mov	pm.curScriptNum, bx
	pop	ebx

	push	ebx
	movzx	ebx, (ObjectID ptr [ebx]).methDict
	memIDGetPtrEBX
	add	ebx, 4			; skip the nRefs
	movzx	ecx, word ptr [ebx]

	add	ebx, 2

	jecxz	__no_methods

__check_all_methods:
	cmp	ax, word ptr [ebx]
	je	__found_the_method

	add	ebx, 4
	loop	__check_all_methods

__no_methods:
	pop	ebx

	movzx	ebx, (ObjectID ptr [ebx]).super
	test	ebx, 0ffffffffh
	jz	__halt

;	curClass.AssureValid();

	memIDGetPtrEBX

;error	pop	ecx

	jmp	__while_loop

__halt:
	mov	edx, 1
	jmp	asm_fatal

__found_the_method:
	mov	[esi - 2], dx
	mov	eax, pm.acc

	movzx	edx, word ptr [ebx + 2]

	pop	ebx
	push	esi

	mov	esi, edx
	
	movzx	ebx, (ObjectID ptr [ebx]).script

	memIDGetPtrEBX

	push	pm.ipMemID
	push	pm.ip

	push	ebx

	movzx	ebx, (ScriptID ptr [ebx]).code
	mov	pm.ipMemID, bx
	memIDGetPtrEBX
	mov	pm.ip, ebx
	add	esi, ebx

	pop	ebx
	
	movzx	ebx, (ScriptID ptr [ebx]).vars
	mov	pm.Locals, bx

;IFDEF DEBUG
;	PushDebugInfo();
;endif

	;; get next opcode
	movzx	ebx, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	call	opTbl[ebx*4]

;	call	ExecuteCode   *** optimized out

;IFDEF DEBUG
;	PopDebugInfo();
;endif

	pop	pm.ip
	pop	pm.ipMemID

	mov	pm.acc, eax

					; re-deref, in case memory moved
	memIDGetPtr	obj		; ebx points to the object

	pop	esi
	pop	ecx

	jecxz	__processed2
	jmp	__next_message

__processed2:
	;;	restore variables
	pop	edi

	popIP

	pop	pm.curScriptNum
	pop	pm.TempPtr
	pop	pm.ParmPtr
	pop	pm.Locals


;IFDEF DEBUG
;	pm.sendStack->Pop();
;endif

	mov	esp, ebp
	pop	ebp

	ret

pmQuickMessage	endp

; ---------------------------------------------------------------------------
invokeMethod	proc

;	obj		=	EBX
;	selector	=	EDX
;	nArgs		=	ECX

	;	invoke a method or access a property from the kernel
	push	pm.object

	;	set new current object
	mov	pm.object, bx

;error	; add 1 word for selector and 1 word for  args
;error	add	ecx, 4

	;	push the selector
	ppush	dx

	;	push the number of chars of arguments
	ppush	cx

	;	save the count & the obj
	push	ecx
	push	ebx

	;	push the arguments
;error	sub	ecx, 4
	mov	ebx, esp

	mov	pm.acc, eax

	jecxz	__pushed_all

__push_all:
	mov	dx, word ptr [ebx]
	add	ebx, 4
	ppush	dx

	loop	__push_all

__pushed_all:
	;	call the method
;	call	check_eax
	pop	ebx
	pop	ecx
	add	ecx, 4
	call	pmQuickMessage

	;	restore current object
	pop	pm.object

	mov	eax, pm.acc

	ret

invokeMethod	endp

; ---------------------------------------------------------------------------
pmRun	proc

startMethod	EQU	<[ebp - 4]>		;; Local variable
pscript		EQU	<[ebp - 8]>		;; Local variable

	push	ebp
	mov	ebp, esp
	sub	esp, 8

	lea	ebx, pm.restart
	mov	[ebx], esi
	mov	[ebx + 4], edi
	mov	[ebx + 8], esp
	mov	[ebx + 12], ebp
	jmp	short __skipRestart

pmRestartBuf label proc
public pmRestartBuf

	lea	ebx, pm.restart
	mov	esi, [ebx]
	mov	edi, [ebx + 4]
	mov	esp, [ebx + 8]
	mov	ebp, [ebx + 12]

__skipRestart:
	mov	word ptr startMethod, s_replay

	test	pm.game, 0ffffffffh
	jnz	__already_inited
						;; this code can't be
	call	LoadPropOffsets			;; executed twice unless we
	call	LoadClassTbl			;; implement restart

	call	setupTossRet
	mov	pm.RestArgs, 0

;IFDEF DEBUG
;	sendStack = New SendStackDebug;
;endif

	mov	word ptr startMethod, s_play

__already_inited:
	call	pmStackClear

;IFDEF DEBUG
;	sendStack->Clear();
;	ResetDebugInfo();
;endif

	lea	eax, pscript
	push	eax
	push	0
	push	0
	call	GetDispatchEntry

	add	esp, 12	       	; remove junk

	mov	pm.object, ax
	mov	pm.game, ax

	movzx	ebx, word ptr pscript
	memIDGetPtrEBX

	movzx	edx, (ScriptID ptr [ebx]).vars
	mov	pm.Globals, dx
	call	pmLock0
	mov	pm.GlobalPtr, eax

	call	InitLanguage

;IFDEF DEBUG
;	if (configMgr->Get(configMgr->IntegrityChecking)) {
;		int mask = CHECK_ALL;
;		if (configMgr->Arg(configMgr->IntegrityChecking))
;			mask = configMgr->Val(configMgr->IntegrityChecking);
;		IntegrityChecking(mask,"-i");
;	}
;//  REMOVE THESE LINES when interpreter is 'solid'...
;	// Enable integrity checking by default
;	else
;		IntegrityChecking(CHECK_ALL);
;//  ...end of 'REMOVE THESE LINES'
;	CheckIntegrity("pm start");
;endif

	;; setup the accumulator
	mov	eax, 0
	mov	pm.acc, eax

	mov	ecx, 0
	movzx	edx, word ptr startMethod
	movzx	ebx, pm.game
	call	invokeMethod

;IFDEF DEBUG
;	CheckIntegrity("pm end");
;endif

	mov	esp, ebp
	pop	ebp

	ret

pmRun	endp

; ---------------------------------------------------------------------------
pmEnd	proc

	mov	pm.game, 0
	
;IFDEF DEBUG
;	delete	sendStack
;endIf

	ret

pmEnd	endp

; ---------------------------------------------------------------------------
pmCheckIntegrity	proc

	memIDGet	pm.Globals
	mov	edx, (MemID ptr [ebx]).memInfo
	test	edx, mMOVEABLE
	jnz	__True

	mov	ebx, (MemID ptr [ebx]).data
	cmp	ebx, pm.GlobalPtr
	je	__True

;IFDEF DEBUG
;	if (!msgMgr->Alert("%sSomething has stepped on globals pointer",note))
;		EnterDebug();
;else
;	msgMgr->Alert(SrcLoc,Msg_GlobalPointer);
;endif

	mov	ax, 0
	ret

__True:
	mov	ax, 1
	ret

pmCheckIntegrity	endp

; ---------------------------------------------------------------------------
; -									    -
; - IN: 	DX contains the Global number to retreive.   		    -
; -									    -
; - OUT:	EAX contains the value of that global.			    -
; -									    -
; ---------------------------------------------------------------------------
pmGlobal	proc

	movzx	ebx, dx
	shl	ebx, 1
	add	ebx, pm.GlobalPtr
	movzx	eax, word ptr [ebx]
 
	ret

pmGlobal	endp

; ---------------------------------------------------------------------------
; -									    -
; - IN: 	DX contains the Selector.		   		    -
; -		CX contains the number of Arguments.			    -
; -		ESI contains a pointer to the list of arguments		    -
; -									    -
; - OUT:								    -
; -									    -
; ---------------------------------------------------------------------------
pmPushMany	proc

	push	edi
	mov	edi, pm.StackPtr

	ppush	dx
	ppush	cx

	jcxz	__no_parms

	add	edi, 2

	rep	movsw
 
	sub	edi, 2
	mov	pm.StackPtr, edi
	pop	edi

	ret

__no_parms:
	mov	pm.StackPtr, edi
	pop	edi

	ret

pmPushMany	endp

; ---------------------------------------------------------------------------
pmCall	proc

; nIP		=	EBX
; nArgChars	=	ECX

	;	execute the subroutine at ip

	;	save the parameter and temp pointers
	push	pm.ParmPtr
	push	pm.TempPtr

	;	set up new parameter base
	mov	edx, edi
	add	ecx, 2
	sub	edx, ecx
	movzx	ecx, pm.RestArgs
	sub	edx, ecx
	push	edx
	add	edx, 2
	mov	pm.ParmPtr, edx

	shr	cx, 1
	add	[edx], cx

	mov	pm.RestArgs, 0

	mov	esi, ebx

;IFDEF DEBUG
;	PushDebugInfo();
;endif

	;; get next opcode
	movzx	ebx, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	call	opTbl[ebx*4]

;	call	ExecuteCode  *** optimized out

;IFDEF DEBUG
;	PopDebugInfo();
;endif

	;	pop the arguments off the pmStack
	pop	edi

	pop	pm.TempPtr
	pop	pm.ParmPtr

	ret

pmCall	endp

; ---------------------------------------------------------------------------
pmDispatch	proc

;scriptNum	=	EBX
;entryNum	=	EDX
;nArgChars	=	ECX

nscript			EQU	<[ebp - 4]>

	push	ebp
	mov	ebp, esp
	sub	esp, 4

	;	execute the procedure at this script/entry

	;	save the current script's variables
	push	pm.Locals
	push	pm.curScriptNum

	mov	pm.curScriptNum, bx

	pushIP
	push	pm.ipMemID
	push	pm.ip

	;	get the new ip for this script/entry
	push	eax
	push	ecx

	lea	eax, nscript
	push	eax
	push	edx
	push	ebx
	call	GetDispatchEntry
	add	esp, 12

	movzx	eax, ax

	memIDGetPtr	nscript

	;	set the new script's variables
	movzx	ecx, (ScriptID ptr [ebx]).vars
	mov	pm.Locals, cx

	movzx	ebx, (ScriptID ptr [ebx]).code
	mov	pm.ipMemID, bx
	memIDGetPtrEBX
	mov	pm.ip, ebx
	add	ebx, eax

	pop	ecx
	pop	eax

	;	execute the procedure
	call	pmCall

	;	restore script variables
	pop	pm.ip
	pop	pm.ipMemID
	popIP

	pop	pm.curScriptNum
	pop	pm.Locals

	mov	esp, ebp
	pop	ebp
	
	ret

pmDispatch	endp

extern GetClass:proc
extern kernelCalls:dword

;;
;; ExecuteCode: This function is called to start interpretation of p-code
;; at the passed address and offset.
;;
;ExecuteCode proc public
;
;	;; get next opcode
;	movzx	ebx, byte ptr [esi]
;	inc	esi
;
;	;; offset to opcode index
;	jmp	opTbl[ebx*4]
;
;ExecuteCode endp

;; 
;; InvokeCode: ASM callable NextOpcode interface
;;
;InvokeCode proc public
;
;	;; get next opcode
;	movzx	ebx, byte ptr [esi]
;	inc	esi
;
;	;; offset to opcode index
;	jmp	opTbl[ebx*4]
;
;InvokeCode endp

;; reverse bits in accumulator
OP_bnot label proc
public OP_bnot
	not	eax

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; add value on top of stack to accumulator
OP_add label proc
public OP_add
	add	ax, word ptr [edi]
	sub	edi, 2

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; subtract accumulator from top of stack
OP_sub label proc
public OP_sub
	mov	cx, word ptr [edi]
	sub	edi, 2
	sub	cx, ax
	movsx	eax, cx

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; multiply accumulator by top of stack
OP_mul label proc
public OP_mul
	imul	word ptr [edi]
	sub	edi, 2

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; divide top of stack by accumulator
OP_div label proc
public OP_div
	mov	ecx, eax
	ppop	ax
	cwd
	idiv	cx

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; do modulo of top of stack by the accumulator
OP_mod label proc
public OP_mod
	mov	cx, ax
	ppop	ax
	mov	dx, 0
	idiv	cx
	movzx	eax, dx

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; shift top of stack to the right by the accumulator
OP_shr label proc
public OP_shr
	mov	ecx, eax
	movsx	eax, word ptr [edi]
	sub	edi, 2
	shr	eax, cl

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; shift top of stack to the left by the accumulator
OP_shl label proc
public OP_shl
	mov	ecx, eax
	movzx	eax, word ptr [edi]
	sub	edi, 2
	shl	eax, cl

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; XOR accumulator with top of stack
OP_xor label proc
public OP_xor
	xor	ax, word ptr [edi]
	sub	edi, 2

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; AND accumulator with top of stack
OP_and label proc
public OP_and
	and	ax, word ptr [edi]
	sub	edi, 2

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; OR accumulator with top of stack
OP_or label proc
public OP_or
	or	ax, word ptr [edi]
	sub	edi, 2

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; negate the accumulator
OP_neg label proc
public OP_neg
	neg	eax

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; do a logical not on the accumulator
OP_not label proc
public OP_not
	or	eax, eax
	setz	al
	movzx	eax, al

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; set accumulator to 1 if it is equal to the top of the stack
OP_eq label proc
public OP_eq
	ppop	cx

	mov	pm.prevAcc, eax
	cmp	ax, cx
	sete	al
	movzx	eax, al

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; set accumulator to 1 if it is not equal to the top of the stack
OP_ne label proc
public OP_ne
	ppop	cx

	mov	pm.prevAcc, eax
	cmp	ax, cx
	setne	al
	movzx	eax, al

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; set accumulator to 1 if it is less than the top of the stack
OP_gt label proc
public OP_gt
	ppop	cx

	mov	pm.prevAcc, eax
	cmp	ax, cx
	setl	al
	movzx	eax, al

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; set accumulator to 1 if it is less than or equal to the top of the stack
OP_ge label proc
public OP_ge
	ppop	cx

	mov	pm.prevAcc, eax
	cmp	ax, cx
	setle	al
	movzx	eax, al

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; set accumulator to 1 if it is greater than the top of the stack
OP_lt label proc
public OP_lt
	ppop	cx

	mov	pm.prevAcc, eax
	cmp	ax, cx
	setg	al
	movzx	eax, al

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; set accumulator to 1 if it is greater than or equal to the top of the stack
OP_le label proc
public OP_le
	ppop	cx

	mov	pm.prevAcc, eax
	cmp	ax, cx
	setge	al
	movzx	eax, al

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; set accumulator to 1 if it is less than the top of the stack (unsigned)
OP_ugt label proc
public OP_ugt
	ppop	cx

	mov	pm.prevAcc, eax
	cmp	ax, cx
	setb	al
	movzx	eax, al

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; set accumulator to 1 if it is less than or equal to the top of the stack (unsigned)
OP_uge label proc
public OP_uge
	ppop	cx

	mov	pm.prevAcc, eax
	cmp	ax, cx
	setbe	al
	movzx	eax, al

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; set accumulator to 1 if it is greater than the top of the stack (unsigned)
OP_ult label proc
public OP_ult
	ppop	cx

	mov	pm.prevAcc, eax
	cmp	ax, cx
	seta	al
	movzx	eax, al

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; set accumulator to 1 if it is greater than or equal to the top of the stack (unsigned)
OP_ule label proc
public OP_ule
	ppop	cx

	mov	pm.prevAcc, eax
	cmp	ax, cx
	setae	al
	movzx	eax, al

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; branch if the accumulator is nonzero
OP_bt_byte label proc
public OP_bt_byte
	cmp	ax, 0
	jz	OP_bt_byte_SKIP

	movsx	ebx, byte ptr [esi]
	add	esi, ebx

OP_bt_byte_SKIP:
	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_bt_word label proc
public OP_bt_word
	cmp	ax, 0
	jz	OP_bt_word_SKIP

	movsx	ebx, word ptr [esi]
	add	esi, ebx

OP_bt_word_SKIP:
	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; branch if the accumulator is zero
OP_bnt_byte label proc
public OP_bnt_byte
	cmp	ax, 0
	jnz	OP_bnt_byte_SKIP

	movsx	ebx, byte ptr [esi]
	add	esi, ebx

OP_bnt_byte_SKIP:
	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_bnt_word label proc
public OP_bnt_word
	cmp	ax, 0
	jnz	OP_bnt_word_SKIP

	movsx	ebx, word ptr [esi]
	add	esi, ebx

OP_bnt_word_SKIP:
	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; unconditional branch
OP_jmp_byte label proc
public OP_jmp_byte
	movsx	ebx, byte ptr [esi]
	add	esi, ebx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_jmp_word label proc
public OP_jmp_word
	movsx	ebx, word ptr [esi]
	add	esi, ebx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load an immediate into the accumulator
OP_loadi_byte label proc
public OP_loadi_byte
	movsx	eax, byte ptr [esi]

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_loadi_word label proc
public OP_loadi_word
	movsx	eax, word ptr [esi]

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; push the accumulator on the stack
OP_push label proc
public OP_push
	ppush	ax

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; push an immediate onto the stack
OP_pushi_byte label proc
public OP_pushi_byte
	movsx	cx, byte ptr [esi]
	ppush	cx

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_pushi_word label proc
public OP_pushi_word
	mov	cx, word ptr [esi]
	ppush	cx

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; toss the top value on the stack
OP_toss label proc
public OP_toss
	toss

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; duplicate the top value on the stack
OP_dup label proc
public OP_dup
	peek	cx
	ppush	cx

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; create space for temporary variables on the stack
OP_link_byte label proc
public OP_link_byte
	movzx	ecx, byte ptr [esi]
	shl	ecx, 1
	sub	ecx, 2

	add	edi, 2
	mov	pm.TempPtr, edi

	add	edi, ecx

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_link_word label proc
public OP_link_word
	movzx	ecx, word ptr [esi]
	shl	ecx, 1
	sub	ecx, 2

	add	edi, 2
	mov	pm.TempPtr, edi

	add	edi, ecx

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_call_byte label proc
public OP_call_byte
	movsx	ebx, byte ptr [esi]
	movsx	ecx, word ptr [esi + 1]
	add	esi, 3

	;; save the instruction pointer and point it to the instruction
	;; to call
	pushIP
	push	pm.ipMemID
	push	pm.ip

	add	ebx, esi
	
	call	pmCall

	;; restore the instruction pointer 
	pop	pm.ip
	pop	pm.ipMemID
	popIP

	;; get next opcode
	movzx	ebx, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; make a call to a procedure in the current module
OP_call_word label proc
public OP_call_word
	movsx	ebx, word ptr [esi]
	movsx	ecx, word ptr [esi + 2]
	add	esi, 4

	;; save the instruction pointer and point it to the instruction
	;; to call
	pushIP
	push	pm.ipMemID
	push	pm.ip

	add	ebx, esi
	
	call	pmCall

	;; restore the instruction pointer 
	pop	pm.ip
	pop	pm.ipMemID
	popIP

	;; get next opcode
	movzx	ebx, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; call a kernel routine
OP_callk_byte label proc
public OP_callk_byte
	;; point to the beginning of arguments
	mov	ebx, edi

	movsx	edx, pm.RestArgs

	movzx	ecx, word ptr [esi + 1]
	add	ecx, edx

	sub	ebx, ecx

	;; adjust number of arguments to include pmRestArgs
	shr	edx, 1
	add	word ptr [ebx], dx

	;; get the kernel call entry number
	movzx	edx, byte ptr [esi]
	add	esi, 3

	mov	ecx, edx
	shl	edx, 3
	shl	ecx, 2
	add	edx, ecx

	mov	pm.RestArgs, 0

	;; save the pmachine state
	mov	pm.acc, eax
	mov	pm.StackPtr, edi

	pushIP
	push	pm.ipMemID
	push	pm.ip

	push	ebx
	;; call the kernel function
	call	kernelCalls[edx]

	;; restore the pmachine stack as well -- nifty shortcut
	pop	edi
	sub	edi, 2

	;; restore the pmachine state
	pop	pm.ip
	pop	pm.ipMemID
	popIP

	;; now the accumulator
	mov	eax, pm.acc
	movsx	eax, ax
	
	;; get next opcode
	movzx	ebx, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_callk_word label proc
public OP_callk_word
	;; point to the beginning of arguments
	mov	ebx, edi

	movsx	edx, pm.RestArgs

	movzx	ecx, word ptr [esi + 2]
	add	ecx, edx

	sub	ebx, ecx

	;; adjust number of arguments to include pmRestArgs
	shr	edx, 1
	add	word ptr [ebx], dx

	;; get the kernel call entry number
	movzx	edx, word ptr [esi]
	add	esi, 4

	mov	ecx, edx
	shl	edx, 3
	shl	ecx, 2
	add	edx, ecx

	mov	pm.RestArgs, 0

	;; save the pmachine state
	mov	pm.acc, eax
	mov	pm.StackPtr, edi

	pushIP
	push	pm.ipMemID
	push	pm.ip

	push	ebx
	;; call the kernel function
	call	kernelCalls[edx]

	;; restore the pmachine stack as well -- nifty shortcut
	pop	edi
	sub	edi, 2

	;; restore the pmachine state
	pop	pm.ip
	pop	pm.ipMemID
	popIP

	;; now the accumulator
	mov	eax, pm.acc
	movsx	eax, ax
;	call	check_eax
	
	;; get next opcode
	movzx	ebx, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; call a procedure in the base script
OP_callb_byte label proc
public OP_callb_byte
	mov	ebx, 0
	movzx	edx, byte ptr [esi]
	movzx	ecx, word ptr [esi + 1]
	add	esi, 3
	call	pmDispatch

	;; get next opcode
	movzx	ebx, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_callb_word label proc
public OP_callb_word
	mov	ebx, 0
	movzx	edx, word ptr [esi]
	movzx	ecx, word ptr [esi + 2]
	add	esi, 4
	call	pmDispatch

	;; get next opcode
	movzx	ebx, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; call a procedure in another script
OP_calle_byte label proc
public OP_calle_byte
	movzx	ebx, byte ptr [esi]
	movzx	edx, byte ptr [esi + 1]
	movzx	ecx, word ptr [esi + 2]
	add	esi, 4
	call	pmDispatch

	;; get next opcode
	movzx	ebx, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_calle_word label proc
public OP_calle_word
	movzx	ebx, word ptr [esi]
	movzx	edx, word ptr [esi + 2]
	movzx	ecx, word ptr [esi + 4]
	add	esi, 6
	call	pmDispatch

	;; get next opcode
	movzx	ebx, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; return from an call next opcode not a jump next opcode
OP_ret label proc
public OP_ret
	ret

;; send messages to object in accumulator
OP_send_byte label proc
public OP_send_byte
	push	pm.object
	mov	pm.object, ax
	
	mov	ebx, eax
	movzx	ecx, word ptr [esi]
	add	esi, 2
	add	cx, pm.RestArgs
	call	pmQuickMessage

	pop	pm.object

	;; get next opcode
	movzx	ebx, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load accumulator with pointer to class
OP_class_byte label proc
public OP_class_byte
	movzx	ecx, byte ptr [esi]

	push	ecx
	call	GetClass
	pop	ecx

	movzx	eax, ax

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load accumulator with pointer to class
OP_class_word label proc
public OP_class_word
	movzx	ecx, word ptr [esi]

	push	ecx
	call	GetClass
	pop	ecx

	movzx	eax, ax

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; send to the current object
OP_self label proc
public OP_self
	movsx	ebx, pm.object
	movzx	ecx, word ptr [esi]
	add	esi, 2
	add	cx, pm.RestArgs
	call	pmQuickMessage
	
	;; get next opcode
	movzx	ebx, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; send to a class
OP_super_byte label proc
public OP_super_byte
	movzx	ebx, byte ptr [esi]

	push	ebx
	call	GetClass
	pop	ebx

	movzx	eax, ax
	movzx	ebx, ax

	movzx	ecx, word ptr [esi + 1]
	add	esi, 3
	add	cx, pm.RestArgs
	call	pmQuickMessage

	;; get next opcode
	movzx	ebx, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; send to a class
OP_super_word label proc
public OP_super_word
	movzx	ebx, word ptr [esi]

	push	ebx
	call	GetClass
	pop	ebx

	movzx	eax, ax
	mov	ebx, eax

	movzx	ecx, word ptr [esi + 2]
	add	esi, 4
	add	cx, pm.RestArgs
	call	pmQuickMessage

	;; get next opcode
	movzx	ebx, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; add the rest of the parmeters
OP_rest_byte label proc
public OP_rest_byte
	mov	ebx, pm.ParmPtr
	movzx	ecx, word ptr [ebx]

	movzx	edx, byte ptr [esi]

	sub	ecx, edx
	inc	ecx
	shl	edx, 1

	cmp	ecx, 0
	jle	__OP_rest_loop

	add	pm.RestArgs, cx
	add	pm.RestArgs, cx

	add	edi, 2
	push	esi
	mov	esi, ebx
	add	esi, edx

	rep	movsw

	pop	esi
	sub	edi, 2

__OP_rest_loop:
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; no longer used in sci 32
OP_lea_byte:
	mov	eax, ebx
	mov	edx, 4
	jmp	asm_fatal

;; no longer used in sci 32
OP_lea_word:
	mov	eax, ebx
	mov	edx, 4
	jmp	asm_fatal

;; load accumulator with current object
OP_selfID label proc
public OP_selfID
	movsx	eax, word ptr pm.object

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; push the value of accumulator before the comparision
OP_pprev label proc
public OP_pprev
	mov	edx, pm.prevAcc
	ppush	dx

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_pToa_byte label proc
public OP_pToa_byte
	memIDGetPtr	pm.object
	movzx	ecx, byte ptr [esi]
	add	ebx, ecx
	movsx	eax, word ptr [ebx]
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_pToa_word label proc
public OP_pToa_word
	memIDGetPtr	pm.object
	movzx	ecx, word ptr [esi]
	add	ebx, ecx
	movsx	eax, word ptr [ebx]
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_aTop_byte label proc
public OP_aTop_byte
	memIDGetPtr	pm.object
	movzx	ecx, byte ptr [esi]
IFDEF NETWORK
	cmp	cx, 0056h
	ja	__not_Critical2
	cmp	cx, 0030h
	jb	__not_Critical2
ELSE
	cmp	cx, 0058h
	ja	__not_Critical2
	cmp	cx, 0034h
	jb	__not_Critical2
ENDIF

	or	word ptr [ebx + 14], 0008h

__not_Critical2:
	add	ebx, ecx
	mov	word ptr [ebx], ax
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_aTop_word label proc
public OP_aTop_word
	memIDGetPtr	pm.object
	movzx	ecx, word ptr [esi]
IFDEF NETWORK
	cmp	cx, 0056h
	ja	__not_Critical3
	cmp	cx, 0030h
	jb	__not_Critical3
ELSE
	cmp	cx, 0058h
	ja	__not_Critical3
	cmp	cx, 0034h
	jb	__not_Critical3
ENDIF

	or	word ptr [ebx + 14], 0008h

__not_Critical3:
	add	ebx, ecx
	mov	word ptr [ebx], ax
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_pTos_byte label proc
public OP_pTos_byte
	memIDGetPtr	pm.object
	movzx	ecx, byte ptr [esi]
	add	ebx, ecx
	mov	cx, word ptr [ebx]
	ppush	cx
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_pTos_word label proc
public OP_pTos_word
	memIDGetPtr	pm.object
	movzx	ecx, word ptr [esi]
	add	ebx, ecx
	mov	cx, word ptr [ebx]
	ppush	cx
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_sTop_byte label proc
public OP_sTop_byte
	memIDGetPtr	pm.object
	movzx	ecx, byte ptr [esi]
	ppop	dx
IFDEF NETWORK
	cmp	cx, 0056h
	ja	__not_Critical4
	cmp	cx, 0030h
	jb	__not_Critical4
ELSE
	cmp	cx, 0058h
	ja	__not_Critical4
	cmp	cx, 0034h
	jb	__not_Critical4
ENDIF

	or	word ptr [ebx + 14], 0008h

__not_Critical4:
	add	ebx, ecx
	mov	word ptr [ebx], dx
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_sTop_word label proc
public OP_sTop_word
	memIDGetPtr	pm.object
	ppop	dx
	movzx	ecx, word ptr [esi]
IFDEF NETWORK
	cmp	cx, 0056h
	ja	__not_Critical5
	cmp	cx, 0030h
	jb	__not_Critical5
ELSE
	cmp	cx, 0058h
	ja	__not_Critical5
	cmp	cx, 0034h
	jb	__not_Critical5
ENDIF

	or	word ptr [ebx + 14], 0008h

__not_Critical5:
	add	ebx, ecx
	mov	word ptr [ebx], dx
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_ipToa_byte label proc
public OP_ipToa_byte
	memIDGetPtr	pm.object
	movzx	ecx, byte ptr [esi]
IFDEF NETWORK
	cmp	cx, 0056h
	ja	__not_Critical6
	cmp	cx, 0030h
	jb	__not_Critical6
ELSE
	cmp	cx, 0058h
	ja	__not_Critical6
	cmp	cx, 0034h
	jb	__not_Critical6
ENDIF

	or	word ptr [ebx + 14], 0008h

__not_Critical6:
	add	ebx, ecx
	inc	word ptr [ebx]
	movsx	eax, word ptr [ebx]
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_ipToa_word label proc
public OP_ipToa_word
	memIDGetPtr	pm.object
	movzx	ecx, word ptr [esi]
IFDEF NETWORK
	cmp	cx, 0056h
	ja	__not_Critical7
	cmp	cx, 0030h
	jb	__not_Critical7
ELSE
	cmp	cx, 0058h
	ja	__not_Critical7
	cmp	cx, 0034h
	jb	__not_Critical7
ENDIF

	or	word ptr [ebx + 14], 0008h

__not_Critical7:
	add	ebx, ecx
	inc	word ptr [ebx]
	movsx	eax, word ptr [ebx]
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_dpToa_byte label proc
public OP_dpToa_byte
	memIDGetPtr	pm.object
	movzx	ecx, byte ptr [esi]
IFDEF NETWORK
	cmp	cx, 0056h
	ja	__not_Critical8
	cmp	cx, 0030h
	jb	__not_Critical8
ELSE
	cmp	cx, 0058h
	ja	__not_Critical8
	cmp	cx, 0034h
	jb	__not_Critical8
ENDIF

	or	word ptr [ebx + 14], 0008h

__not_Critical8:
	add	ebx, ecx
	dec	word ptr [ebx]
	movsx	eax, word ptr [ebx]
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_dpToa_word label proc
public OP_dpToa_word
	memIDGetPtr	pm.object
	movzx	ecx, word ptr [esi]
IFDEF NETWORK
	cmp	cx, 0056h
	ja	__not_Critical9
	cmp	cx, 0030h
	jb	__not_Critical9
ELSE
	cmp	cx, 0058h
	ja	__not_Critical9
	cmp	cx, 0034h
	jb	__not_Critical9
ENDIF

	or	word ptr [ebx + 14], 0008h

__not_Critical9:
	add	ebx, ecx
	dec	word ptr [ebx]
	movsx	eax, word ptr [ebx]
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_ipTos_byte label proc
public OP_ipTos_byte
	memIDGetPtr	pm.object
	movzx	ecx, byte ptr [esi]
IFDEF NETWORK
	cmp	cx, 0056h
	ja	__not_Critical10
	cmp	cx, 0030h
	jb	__not_Critical10
ELSE
	cmp	cx, 0058h
	ja	__not_Critical10
	cmp	cx, 0034h
	jb	__not_Critical10
ENDIF

	or	word ptr [ebx + 14], 0008h

__not_Critical10:
	add	ebx, ecx
	inc	word ptr [ebx]
	mov	cx, word ptr [ebx]
	ppush	cx
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_ipTos_word label proc
public OP_ipTos_word
	memIDGetPtr	pm.object
	movzx	ecx, word ptr [esi]
IFDEF NETWORK
	cmp	cx, 0056h
	ja	__not_Critical11
	cmp	cx, 0030h
	jb	__not_Critical11
ELSE
	cmp	cx, 0058h
	ja	__not_Critical11
	cmp	cx, 0034h
	jb	__not_Critical11
ENDIF

	or	word ptr [ebx + 14], 0008h

__not_Critical11:
	add	ebx, ecx
	inc	word ptr [ebx]
	mov	cx, word ptr [ebx]
	ppush	cx
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_dpTos_byte label proc
public OP_dpTos_byte
	memIDGetPtr	pm.object
	movzx	ecx, byte ptr [esi]
IFDEF NETWORK
	cmp	cx, 0056h
	ja	__not_Critical12
	cmp	cx, 0030h
	jb	__not_Critical12
ELSE
	cmp	cx, 0058h
	ja	__not_Critical12
	cmp	cx, 0034h
	jb	__not_Critical12
ENDIF

	or	word ptr [ebx + 14], 0008h

__not_Critical12:
	add	ebx, ecx
	dec	word ptr [ebx]
	mov	cx, word ptr [ebx]
	ppush	cx
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; 
OP_dpTos_word label proc
public OP_dpTos_word
	memIDGetPtr	pm.object
	movzx	ecx, word ptr [esi]
IFDEF NETWORK
	cmp	cx, 0056h
	ja	__not_Critical13
	cmp	cx, 0030h
	jb	__not_Critical13
ELSE
	cmp	cx, 0058h
	ja	__not_Critical13
	cmp	cx, 0034h
	jb	__not_Critical13
ENDIF

	or	word ptr [ebx + 14], 0008h

__not_Critical13:
	add	ebx, ecx
	dec	word ptr [ebx]
	mov	cx, word ptr [ebx]
	ppush	cx
	
	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load a memId to the accumulator
OP_lofsa label proc
public OP_lofsa
	movzx	eax, word ptr [esi]

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load a memId to the stack
OP_lofss label proc
public OP_lofss
	add	edi, 2
	mov	cx, word ptr [esi]
	mov	word ptr [edi], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; push 0 onto the stack
OP_push0 label proc
public OP_push0
	ppush	0

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; push 1 onto the stack
OP_push1 label proc
public OP_push1
	ppush	1

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; push 2 onto the stack
OP_push2 label proc
public OP_push2
	ppush	2

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; push the current object onto the stack
OP_pushSelf label proc
public OP_pushSelf
	mov	cx, pm.object
	ppush	cx

	;; get next opcode
	mov	bl, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with the value of a global
OP_lag_byte label proc
public OP_lag_byte
	movzx	edx, byte ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	movsx	eax, word ptr [edx]

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lag_word label proc
public OP_lag_word
	movzx	edx, word ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	movsx	eax, word ptr [edx]

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with the value of a local
OP_lal_byte label proc
public OP_lal_byte
	movzx	ecx, byte ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lal_word label proc
public OP_lal_word
	movzx	ecx, word ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with the value of a temp
OP_lat_byte label proc
public OP_lat_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lat_word label proc
public OP_lat_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with the value of a parameter
OP_lap_byte label proc
public OP_lap_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lap_word label proc
public OP_lap_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with the value of a global
OP_lsg_byte label proc
public OP_lsg_byte
	movzx	edx, byte ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	mov	cx, word ptr [edx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lsg_word label proc
public OP_lsg_word
	movzx	edx, word ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	mov	cx, word ptr [edx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with the value of a local
OP_lsl_byte label proc
public OP_lsl_byte
	movzx	ecx, byte ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lsl_word label proc
public OP_lsl_word
	movzx	ecx, word ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with the value of a temp
OP_lst_byte label proc
public OP_lst_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lst_word label proc
public OP_lst_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with the value of a parameter
OP_lsp_byte label proc
public OP_lsp_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lsp_word label proc
public OP_lsp_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with the value of an indexed global
OP_lagi_byte label proc
public OP_lagi_byte
	movzx	edx, byte ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	movsx	eax, word ptr [edx]

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lagi_word label proc
public OP_lagi_word
	movzx	edx, word ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	movsx	eax, word ptr [edx]

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with the value of an indexed local
OP_lali_byte label proc
public OP_lali_byte
	movzx	ecx, byte ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lali_word label proc
public OP_lali_word
	movzx	ecx, word ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with the value of an indexed temp
OP_lati_byte label proc
public OP_lati_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lati_word label proc
public OP_lati_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with the value of an indexed parameter
OP_lapi_byte label proc
public OP_lapi_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lapi_word label proc
public OP_lapi_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with the value of an indexed global
OP_lsgi_byte label proc
public OP_lsgi_byte
	movzx	edx, byte ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	mov	cx, word ptr [edx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lsgi_word label proc
public OP_lsgi_word
	movzx	edx, word ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	mov	cx, word ptr [edx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with the value of an indexed local
OP_lsli_byte label proc
public OP_lsli_byte
	movzx	ecx, byte ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lsli_word label proc
public OP_lsli_word
	movzx	ecx, word ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with the value of an indexed temp
OP_lsti_byte label proc
public OP_lsti_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lsti_word label proc
public OP_lsti_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with the value of an indexed parameter
OP_lspi_byte label proc
public OP_lspi_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_lspi_word label proc
public OP_lspi_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the accumulator into a global
OP_sag_byte label proc
public OP_sag_byte
	movzx	edx, byte ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	mov	word ptr [edx], ax

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_sag_word label proc
public OP_sag_word
	movzx	edx, word ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	mov	word ptr [edx], ax

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the accumulator into a local
OP_sal_byte label proc
public OP_sal_byte
	movzx	ecx, byte ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	mov	word ptr [ebx], ax

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_sal_word label proc
public OP_sal_word
	movzx	ecx, word ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	mov	word ptr [ebx], ax

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the accumulator into a temp
OP_sat_byte label proc
public OP_sat_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	mov	word ptr [ebx], ax

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_sat_word label proc
public OP_sat_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	mov	word ptr [ebx], ax

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the accumulator into a parameter
OP_sap_byte label proc
public OP_sap_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	mov	word ptr [ebx], ax

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_sap_word label proc
public OP_sap_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	mov	word ptr [ebx], ax

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the stack into a global
OP_ssg_byte label proc
public OP_ssg_byte
	movzx	edx, byte ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [edx], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_ssg_word label proc
public OP_ssg_word
	movzx	edx, word ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [edx], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the stack into a local
OP_ssl_byte label proc
public OP_ssl_byte
	movzx	ecx, byte ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_ssl_word label proc
public OP_ssl_word
	movzx	ecx, word ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the stack into a temp
OP_sst_byte label proc
public OP_sst_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_sst_word label proc
public OP_sst_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the stack into a parameter
OP_ssp_byte label proc
public OP_ssp_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_ssp_word label proc
public OP_ssp_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the accumulator into an indexed global
OP_sagi_byte label proc
public OP_sagi_byte
	movzx	edx, byte ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	movzx	eax, word ptr [edi]
	sub	edi, 2
	mov	word ptr [edx], ax

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_sagi_word label proc
public OP_sagi_word
	movzx	edx, word ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	movzx	eax, word ptr [edi]
	sub	edi, 2
	mov	word ptr [edx], ax

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the accumulator into an indexed local
OP_sali_byte label proc
public OP_sali_byte
	movzx	ecx, byte ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	movsx	eax, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], ax

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_sali_word label proc
public OP_sali_word
	movzx	ecx, word ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	movsx	eax, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], ax

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the accumulator into an indexed temp
OP_sati_byte label proc
public OP_sati_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	movsx	eax, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], ax

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_sati_word label proc
public OP_sati_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	movsx	eax, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], ax

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the accumulator into an indexed parameter
OP_sapi_byte label proc
public OP_sapi_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	movsx	eax, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], ax

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_sapi_word label proc
public OP_sapi_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	movsx	eax, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], ax

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the stack into an indexed global
OP_ssgi_byte label proc
public OP_ssgi_byte
	movzx	edx, byte ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [edx], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_ssgi_word label proc
public OP_ssgi_word
	movzx	edx, word ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [edx], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the stack into an indexed local
OP_ssli_byte label proc
public OP_ssli_byte
	movzx	ecx, byte ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_ssli_word label proc
public OP_ssli_word
	movzx	ecx, word ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the stack into an indexed temp
OP_ssti_byte label proc
public OP_ssti_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_ssti_word label proc
public OP_ssti_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; store the stack into an indexed parameter
OP_sspi_byte label proc
public OP_sspi_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_sspi_word label proc
public OP_sspi_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	mov	cx, word ptr [edi]
	sub	edi, 2
	mov	word ptr [ebx], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with an incremented global
OP_iag_byte label proc
public OP_iag_byte
	movzx	edx, byte ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	inc	word ptr [edx]
	movsx	eax, word ptr [edx]

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_iag_word label proc
public OP_iag_word
	movzx	edx, word ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	inc	word ptr [edx]
	movsx	eax, word ptr [edx]

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with an incremented local
OP_ial_byte label proc
public OP_ial_byte
	movzx	ecx, byte ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	inc	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_ial_word label proc
public OP_ial_word
	movzx	ecx, word ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	inc	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with an incremented temp
OP_iat_byte label proc
public OP_iat_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	inc	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_iat_word label proc
public OP_iat_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	inc	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with an incremented parameter
OP_iap_byte label proc
public OP_iap_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	inc	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_iap_word label proc
public OP_iap_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	inc	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with an incremented global
OP_isg_byte label proc
public OP_isg_byte
	movzx	edx, byte ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	inc	word ptr [edx]
	mov	cx, word ptr [edx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_isg_word label proc
public OP_isg_word
	movzx	edx, word ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	inc	word ptr [edx]
	mov	cx, word ptr [edx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with an incremented local
OP_isl_byte label proc
public OP_isl_byte
	movzx	ecx, byte ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	inc	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_isl_word label proc
public OP_isl_word
	movzx	ecx, word ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	inc	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with an incremented temp
OP_ist_byte label proc
public OP_ist_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	inc	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_ist_word label proc
public OP_ist_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	inc	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with an incremented parameter
OP_isp_byte label proc
public OP_isp_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	inc	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_isp_word label proc
public OP_isp_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	inc	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with an incremented indexed global
OP_iagi_byte label proc
public OP_iagi_byte
	movzx	edx, byte ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	inc	word ptr [edx]
	movsx	eax, word ptr [edx]

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_iagi_word label proc
public OP_iagi_word
	movzx	edx, word ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	inc	word ptr [edx]
	movsx	eax, word ptr [edx]

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with an incremented indexed local
OP_iali_byte label proc
public OP_iali_byte
	movzx	ecx, byte ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	inc	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_iali_word label proc
public OP_iali_word
	movzx	ecx, word ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	inc	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with an incremented indexed temp
OP_iati_byte label proc
public OP_iati_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	inc	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_iati_word label proc
public OP_iati_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	inc	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with an incremented indexed parameter
OP_iapi_byte label proc
public OP_iapi_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	inc	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_iapi_word label proc
public OP_iapi_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	inc	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with an incremented indexed global
OP_isgi_byte label proc
public OP_isgi_byte
	movzx	edx, byte ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	inc	word ptr [edx]
	mov	cx, word ptr [edx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_isgi_word label proc
public OP_isgi_word
	movzx	edx, word ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	inc	word ptr [edx]
	mov	cx, word ptr [edx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with an incremented indexed local
OP_isli_byte label proc
public OP_isli_byte
	movzx	ecx, byte ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	inc	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_isli_word label proc
public OP_isli_word
	movzx	ecx, word ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	inc	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with an incremented indexed temp
OP_isti_byte label proc
public OP_isti_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	inc	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_isti_word label proc
public OP_isti_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	inc	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with an incremented indexed parameter
OP_ispi_byte label proc
public OP_ispi_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	inc	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_ispi_word label proc
public OP_ispi_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	inc	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with a decremented global
OP_dag_byte label proc
public OP_dag_byte
	movzx	edx, byte ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	dec	word ptr [edx]
	movsx	eax, word ptr [edx]

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dag_word label proc
public OP_dag_word
	movzx	edx, word ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	dec	word ptr [edx]
	movsx	eax, word ptr [edx]

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with a decremented local
OP_dal_byte label proc
public OP_dal_byte
	movzx	ecx, byte ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	dec	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dal_word label proc
public OP_dal_word
	movzx	ecx, word ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	dec	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with a decremented temp
OP_dat_byte label proc
public OP_dat_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	dec	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dat_word label proc
public OP_dat_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	dec	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with a decremented parameter
OP_dap_byte label proc
public OP_dap_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	dec	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dap_word label proc
public OP_dap_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	dec	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with a decremented global
OP_dsg_byte label proc
public OP_dsg_byte
	movzx	edx, byte ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	dec	word ptr [edx]
	mov	cx, word ptr [edx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dsg_word label proc
public OP_dsg_word
	movzx	edx, word ptr [esi]

	shl	edx, 1
	add	edx, pm.GlobalPtr
	dec	word ptr [edx]
	mov	cx, word ptr [edx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with a decremented local
OP_dsl_byte label proc
public OP_dsl_byte
	movzx	ecx, byte ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	dec	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dsl_word label proc
public OP_dsl_word
	movzx	ecx, word ptr [esi]

	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	dec	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with a decremented temp
OP_dst_byte label proc
public OP_dst_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	dec	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dst_word label proc
public OP_dst_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.TempPtr
	dec	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2] 
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with a decremented parameter
OP_dsp_byte label proc
public OP_dsp_byte
	movzx	ebx, byte ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	dec	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1] 
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dsp_word label proc
public OP_dsp_word
	movzx	ebx, word ptr [esi]

	shl	ebx, 1
	add	ebx, pm.ParmPtr
	dec	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with a decremented indexed global
OP_dagi_byte label proc
public OP_dagi_byte
	movzx	edx, byte ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	dec	word ptr [edx]
	movsx	eax, word ptr [edx]

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dagi_word label proc
public OP_dagi_word
	movzx	edx, word ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	dec	word ptr [edx]
	movsx	eax, word ptr [edx]

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with a decremented indexed local
OP_dali_byte label proc
public OP_dali_byte
	movzx	ecx, byte ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	dec	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dali_word label proc
public OP_dali_word
	movzx	ecx, word ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	dec	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with a decremented indexed temp
OP_dati_byte label proc
public OP_dati_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	dec	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dati_word label proc
public OP_dati_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	dec	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the accumulator with a decremented indexed parameter
OP_dapi_byte label proc
public OP_dapi_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	dec	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dapi_word label proc
public OP_dapi_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	dec	word ptr [ebx]
	movsx	eax, word ptr [ebx]

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with a decremented indexed global
OP_dsgi_byte label proc
public OP_dsgi_byte
	movzx	edx, byte ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	dec	word ptr [edx]
	mov	cx, word ptr [edx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dsgi_word label proc
public OP_dsgi_word
	movzx	edx, word ptr [esi]

	add	edx, eax
	shl	edx, 1
	add	edx, pm.GlobalPtr
	dec	word ptr [edx]
	mov	cx, word ptr [edx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	mov	bl, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with a decremented indexed local
OP_dsli_byte label proc
public OP_dsli_byte
	movzx	ecx, byte ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	dec	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dsli_word label proc
public OP_dsli_word
	movzx	ecx, word ptr [esi]

	add	ecx, eax
	shl	ecx, 1
	memIDGetPtr	pm.Locals
	add	ebx, ecx
	dec	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with a decremented indexed temp
OP_dsti_byte label proc
public OP_dsti_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	dec	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dsti_word label proc
public OP_dsti_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.TempPtr
	dec	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; load the stack with a decremented indexed parameter
OP_dspi_byte label proc
public OP_dspi_byte
	movzx	ebx, byte ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	dec	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 1]
	add	esi, 2

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_dspi_word label proc
public OP_dspi_word
	movzx	ebx, word ptr [esi]

	add	ebx, eax
	shl	ebx, 1
	add	ebx, pm.ParmPtr
	dec	word ptr [ebx]
	mov	cx, word ptr [ebx]
	add	edi, 2
	mov	word ptr [edi], cx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_filename label proc
public OP_filename

	mov	ebx, 0

__scan:
	mov	dl, [esi + ebx]
	mov	[pm.curSourceFile + ebx], dl
	inc	ebx
	test	byte ptr [esi + ebx], 255
	jnz	__scan

	add	esi, ebx
	inc	esi

	;; get next opcode
	movzx	ebx, byte ptr [esi]
	inc	esi

	;; offset to opcode index
	jmp	opTbl[ebx*4]

OP_linenum label proc
public OP_linenum
	mov	bx, word ptr [esi]
	mov	pm.curSourceLineNum, bx

	;; get next opcode
	movzx	ebx, byte ptr [esi + 2]
	add	esi, 3

	;; offset to opcode index
	jmp	opTbl[ebx*4]

;; Bad op code
BadOp label proc
public BadOp
	mov	eax, ebx
	mov	edx, 4
	jmp	asm_fatal

end
