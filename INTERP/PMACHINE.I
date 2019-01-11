;; PMACHINE.I
;;
;; PMachine macro definitions
;;
;; Author: Stephen Nichols
;;

ifdef DEBUG

;;
;; ppush: macro for pushing a value onto the pmachine stack
;;
ppush	macro 	val
	push	ebx
	lea	ebx, pmStack
	cmp	[ebx + 5119], 1599554882
	je	__ok

	halt


__ok:
	pop	ebx

	add	edi, 2
	mov	word ptr [edi], val

endm

else

;;
;; ppush: macro for pushing a value onto the pmachine stack
;;
ppush	macro 	val
	add	edi, 2
	mov	word ptr [edi], val

endm

endIf

ifdef DEBUG

;;
;; ppop: macro for popping a value off of the pmachine stack
;;
ppop	macro 	val
	mov	val, word ptr [edi]
	mov	word ptr [edi], 0
	sub	edi, 2
endm

else

;;
;; ppop: macro for popping a value off of the pmachine stack
;;
ppop	macro 	val
	mov	val, word ptr [edi]
	sub	edi, 2
endm

endIf

;;
;; toss: macro to toss the top stack value
;;
toss	macro
	sub	edi, 2
endm

;; 
;; peek: macro to peek at the top stack value
;;
peek	macro	val
	mov	val, word ptr [edi]
endm

;; 
;; savePMState: macro for saving the state of the pmachine
;;
savePMState	macro
	push	esi
	push	edi
endm

;; 
;; restorePMState: macro for restoring the state of the pmachine
;;
restorePMState	macro
	pop	edi
	pop	esi
endm

;; 
;; pushIP: macro for saving the ip
;;
pushIP	macro
	push	ebx
	push	edx
	push	esi

	movzx	ebx, pm.ipPtr
	sub	esi, pm.ip

	mov	dx, pm.ipMemID
	mov	word ptr [pm.ipStack + ebx], dx
	mov	dword ptr [pm.ipStack + ebx + 2], esi
	add	pm.ipPtr, 6

	pop	esi
	pop	edx
	pop	ebx

endm

;; 
;; popIP: macro for saving the ip
;;
popIP	macro
	push	ebx
	push	edx

	sub	pm.ipPtr, 6
	movzx	edx, pm.ipPtr
	movzx	ebx, word ptr [pm.ipStack + edx]
	mov	esi, dword ptr [pm.ipStack + edx + 2]

	memIDGetPtrEBX		; ebx points to the object
	add	esi, ebx

	pop	edx
	pop	ebx

endm

;; MemID access macros
;;
;; Author: Stephen Nichols
;;

;; 
;; define all of the junk that goes with MemID access -- taken from 
;; memmgr.hpp
;;
mLOCKED			equ	00000000h
mCRITICAL		equ	00000100h
mMOVEABLE		equ	00000200h
mDISCARDABLE	equ	00000400h
mFREEMEMORY		equ	00000800h
mMARKEDMEMORY	equ	00001000h
mSWAPMEMORY		equ	00002000h
mHANDLEMEMORY	equ	00004000h
mCHECKSUMMABLE	equ	00008000h
mINITMEMORY		equ	00010000h
mFREEHANDLE		equ	00020000h
mCONVMEMORY		equ	00040000h
mPERSIST			equ	00080000h
mTRANSITORY		equ	00100000h
mLOCKEDSAVE		equ	00200000h
mLOCKEDNOSAVE	equ	00400000h
mDISCARDLAST	equ	00800000h
mALIGNBYTES		equ	03000000h
mBLACKHOLE		equ	80000000h
mUNUSEDATTRS	equ	7C000000h

;;
;; memIDGetPtr: macro for getting the address for a descriptor
;;
memIDGet	macro	memID
	movzx	ebx, word ptr memID
	shl	ebx, 5
	push	edx
	mov	edx, memMgr
	add	ebx, (memoryMgr ptr [edx]).desc
	pop	edx
endm 

;;
;; memIDGetPtr: macro for getting the address for a descriptor
;;
memIDGetPtr	macro	memID
	movzx	ebx, word ptr memID
	shl	ebx, 5
	push	edx
	mov	edx, memMgr
	add	ebx, (memoryMgr ptr [edx]).desc
	pop	edx

	mov	ebx, (MemID ptr [ebx]).data
endm 

;;
;; memIDGetPtrEBX: macro for getting the address for a descriptor
;;
memIDGetPtrEBX	macro
	shl	ebx, 5
	push	edx
	mov	edx, memMgr
	add	ebx, (memoryMgr ptr [edx]).desc
	pop	edx

	mov	ebx, (MemID ptr [ebx]).data
endm 

;;
;; memIDGetPtrDI: macro for getting the address for a descriptor
;;
memIDGetPtrDI	macro
	shl	edi, 5
	push	edx
	mov	edx, memMgr
	add	edi, (memoryMgr ptr [edx]).desc
	pop	edx

	mov	edi, (MemID ptr [edi]).data
endm 
