;*************************************************************************
;*
;* MIDI Parser
;*
;*
;*************************************************************************
.386

ifdef WINDOWS
DoSound	equ	<_nDoSound_>			;DoSound now in windll.c
SoundServer equ	<SoundServer_>
endif

_DATA    segment byte public use32 'DATA'
assume   ds:_DATA        

MIDI_S		EQU		1
		include	midi.i

;RECMID          EQU             1        ;Used as Mtest32 flag
;If not compiling for MTEST remove this flag
Cparam		EQU		2	 ;Stack offset for C params
LISTSIZE	EQU		16	 ;PlayList size (3 - 16)
SEL_LDT	equ 0004h		; table indicator; 0 ==> GDT, 1 ==> LDT
SS_DATA		equ	0010h or SEL_LDT ; User data segment

;--------------------------------------------------------------------------
;				Tables


public PlayList
PlayList	df 16 dup (0)	;Prioritized list of ptrs to current sound nodes
SampleList	df 16 dup (0)
ChNodes		df 16 dup (0)

EndSnds		dw 32 dup (0)	;This is not used
KludgeHdr	dw 16 dup (0)

Holding         db  0
ChList		db 16 dup (255)	;The playlist position (high nibble) of the 
				;sound node and sequenced channel (low nibble)
				;that owns a particular slot (output channel).
ChBed		db 16 dup (0)	;"locked" channel flags
ChPri		db 16 dup (0)	;Channel priority corresponding to the channel list
ChVoice		db 16 dup (0)	;Number of voices used by this channel

ChNew		db 16 dup (255)	;Area where we create our new chList

ChBedCopy	db 16 dup (0)	;Buffers for saving the channel info while reording
ChPriCopy	db 16 dup (0)	;the channel list
ChVoiceCopy	db 16 dup (0)
ChNewCopy	db 16 dup (255)
ChOld		db 16 dup (15)
VolRequest	db 16 dup (255)
savMuteFlag	db 15 dup (0)	;Buffer to save mute flags during restore Sound

ifdef RECMID	 
	
MIDIbuffer	db 140 dup (255)

;Variables for MIDI receive interrupt - @INT

MIDIBSIZE       EQU 4096                        ;MIDI interrupt buffer size
MIDIbufbeg      db MIDIBSIZE dup (0)            ;MIDI interrupt buffer
MIDIbufend      EQU MIDIbufbeg+MIDIBSIZE-1      ;MIDI buffer end address
MIDIbcount      dw 0                            ;MIDI interrupt buffer byte count
MIDIbhead       dd 0                            ;MIDI buffer empty pointer
MIDIbtail       dd 0                            ;MIDI buffer fill pointer

imask           db      0                       ;Interrupt mask
SavIntPtr       dd      0                       ;Saved interrupt vector (offset)
SavIntPtrS      dw      0                       ;Saved interrupt vector (segment)
install         db      0                       ;Install flag

il_base         EQU 8                           ;int number for IRQ0
midi_int        EQU 2                           ;Interrupt 2 
midi_level      EQU il_base+midi_int            ;midi interrupt level IRQ2
piccmd          EQU 20h                         ;8259 Priority int controller
picmsk          EQU 21h                         ;8259 Priority int controller

RunningStat     db      0    

endif

;---------------------------------------------------------------------------
;			     Variables

ifndef WINDOWS
Driver		DF 0	      	;Far address of sound driver
endif
SaveDS          dw 0
;SaveFS          dw 0
ARG1		dd 0
ARG2		dd 0
ARG3		dd 0
ARG1DATA	dw 0
ARG2DATA	dw 0
ARG3DATA	dw 0
Carg1		dw 0	      	;
Carg2		dw 0	      	;
Carg3		dw 0	      	;C arguments to be passed
Carg4		dw 0	      	;
Carg5		dw 0	      	;
Carg6		dw 0	      	;
baseIndex	dd 0	      	;Base Index value for ParseNode PROC
currOffset	dd 0

processSnds	db 0	      	;ON\OFF switch for SoundServer
loChnl		db 0	      	;Lowest channel supported by driver
hiChnl		db 15	      	;Highest channel supported by driver
devID		db 0	      	;ID of the current music device
realChnl	db 255	        ;The channel# data is being sent on
ghostChnl	db 0		;Current channel is a ghost channel
numVoices	db 0		;Number of voices supported by driver
playPos		db 0		;Position of node on PlayList for ParseNode PROC
revDefault	db 0		;Default reverb mode
vLeftCopy	db 0		;Copy of voicesLeft
updateChnls	db 0		;Flag telling next interrupt to call DoChannelList
vRequest	db 0		;DoChangeVol flag
requestChnl	db 0		;Current VolRequest channel
oldReqChnl	db 0		;requestChnl end reference
oldRecCom	db 0		;Receiving mode running status
restoring	db 0		;Tells ParseNode not to send anything
testNode	db 0		;Flag signalling ParseNode to look at MIDI port
UARTmode	db 0		;Flag showing MPU in UART mode
priInfo		db 0

;**********************************************
;* This was formerly in MidiHnd.i
;* in:  Handle ID
;* out:	dereferenced ptr ebp
;**********************************************

Hnd2Ptr macro	id
	lfs	ebp,fword ptr id 
;	mov	SaveFS, fs
;	mov	ebp, dword ptr id
;	mov	fs, word ptr id+4
;	mov	ebp, fs:[ebp]
endm

;*********************************************************
;* CallDriver macro
;*********************************************************
CallDriver	MACRO	arg1
		push	ebp
                push    fs
                push    es      
		mov	ebp,arg1
;;                mov     fs,SaveFS
ifndef WINDOWS
;		call	pword ptr _DATA:Driver
else
		call	Driver
endif
		pop     es
                pop     fs
                pop	ebp
		ENDM
;*********************************************************
;* getb macro
;*********************************************************
getb		MACRO	arg1
		push	edi
		mov	edi, baseIndex
		add	edi, currOffset
		mov	arg1,fs:[edi]
		inc	currOffset
		shl	si,1
		inc	word ptr es:[ebx+esi].tIndex
		shr	si,1
		pop	edi
		ENDM


;*********************************************************
;* BackupList macro
;*********************************************************
BackupList	MACRO				     ;Backup ChNew, ChPri,
		push	ax			     ;ChVoice, ChBed, and
		mov	ax,word ptr ChNew	     ;voices left
		mov	word ptr ChNewCopy,ax
		mov	ax,word ptr ChNew[2]
		mov	word ptr ChNewCopy[2],ax
		mov	ax,word ptr ChNew[4]
		mov	word ptr ChNewCopy[4],ax
		mov	ax,word ptr ChNew[6]
		mov	word ptr ChNewCopy[6],ax
		mov	ax,word ptr ChNew[8]
		mov	word ptr ChNewCopy[8],ax
		mov	ax,word ptr ChNew[10]
		mov	word ptr ChNewCopy[10],ax
		mov	ax,word ptr ChNew[12]
		mov	word ptr ChNewCopy[12],ax
		mov	ax,word ptr ChNew[14]
		mov	word ptr ChNewCopy[14],ax
		mov	ax,word ptr ChPri
		mov	word ptr ChPriCopy,ax
		mov	ax,word ptr ChPri[2]
		mov	word ptr ChPriCopy[2],ax
		mov	ax,word ptr ChPri[4]
		mov	word ptr ChPriCopy[4],ax
		mov	ax,word ptr ChPri[6]
		mov	word ptr ChPriCopy[6],ax
		mov	ax,word ptr ChPri[8]
		mov	word ptr ChPriCopy[8],ax
		mov	ax,word ptr ChPri[10]
		mov	word ptr ChPriCopy[10],ax
		mov	ax,word ptr ChPri[12]
		mov	word ptr ChPriCopy[12],ax
		mov	ax,word ptr ChPri[14]
		mov	word ptr ChPriCopy[14],ax
		mov	ax,word ptr ChVoice
		mov	word ptr ChVoiceCopy,ax
		mov	ax,word ptr ChVoice[2]
		mov	word ptr ChVoiceCopy[2],ax
		mov	ax,word ptr ChVoice[4]
		mov	word ptr ChVoiceCopy[4],ax
		mov	ax,word ptr ChVoice[6]
		mov	word ptr ChVoiceCopy[6],ax
		mov	ax,word ptr ChVoice[8]
		mov	word ptr ChVoiceCopy[8],ax
		mov	ax,word ptr ChVoice[10]
		mov	word ptr ChVoiceCopy[10],ax
		mov	ax,word ptr ChVoice[12]
		mov	word ptr ChVoiceCopy[12],ax
		mov	ax,word ptr ChVoice[14]
		mov	word ptr ChVoiceCopy[14],ax
		mov	ax,word ptr ChBed
		mov	word ptr ChBedCopy,ax
		mov	ax,word ptr ChBed[2]
		mov	word ptr ChBedCopy[2],ax
		mov	ax,word ptr ChBed[4]
		mov	word ptr ChBedCopy[4],ax
		mov	ax,word ptr ChBed[6]
		mov	word ptr ChBedCopy[6],ax
		mov	ax,word ptr ChBed[8]
		mov	word ptr ChBedCopy[8],ax
		mov	ax,word ptr ChBed[10]
		mov	word ptr ChBedCopy[10],ax
		mov	ax,word ptr ChBed[12]
		mov	word ptr ChBedCopy[12],ax
		mov	ax,word ptr ChBed[14]
		mov	word ptr ChBedCopy[14],ax
		pop	ax
		mov	vLeftCopy,al
		ENDM

;***********************************************************
;* RestoreList macro
;***********************************************************
RestoreList	MACRO				  ;Restore ChNew, ChPri,
		push	ax			  ;ChVoice, ChBed, and
		mov	ax,word ptr ChNewCopy	  ;voices left
		mov	word ptr ChNew,ax
		mov	ax,word ptr ChNewCopy[2]
		mov	word ptr ChNew[2],ax
		mov	ax,word ptr ChNewCopy[4]
		mov	word ptr ChNew[4],ax
		mov	ax,word ptr ChNewCopy[6]
		mov	word ptr ChNew[6],ax
		mov	ax,word ptr ChNewCopy[8]
		mov	word ptr ChNew[8],ax
		mov	ax,word ptr ChNewCopy[10]
		mov	word ptr ChNew[10],ax
		mov	ax,word ptr ChNewCopy[12]
		mov	word ptr ChNew[12],ax
		mov	ax,word ptr ChNewCopy[14]
		mov	word ptr ChNew[14],ax
		mov	ax,word ptr ChPriCopy
		mov	word ptr ChPri,ax
		mov	ax,word ptr ChPriCopy[2]
		mov	word ptr ChPri[2],ax
		mov	ax,word ptr ChPriCopy[4]
		mov	word ptr ChPri[4],ax
		mov	ax,word ptr ChPriCopy[6]
		mov	word ptr ChPri[6],ax
		mov	ax,word ptr ChPriCopy[8]
		mov	word ptr ChPri[8],ax
		mov	ax,word ptr ChPriCopy[10]
		mov	word ptr ChPri[10],ax
		mov	ax,word ptr ChPriCopy[12]
		mov	word ptr ChPri[12],ax
		mov	ax,word ptr ChPriCopy[14]
		mov	word ptr ChPri[14],ax
		mov	ax,word ptr ChVoiceCopy
		mov	word ptr ChVoice,ax
		mov	ax,word ptr ChVoiceCopy[2]
		mov	word ptr ChVoice[2],ax
		mov	ax,word ptr ChVoiceCopy[4]
		mov	word ptr ChVoice[4],ax
		mov	ax,word ptr ChVoiceCopy[6]
		mov	word ptr ChVoice[6],ax
		mov	ax,word ptr ChVoiceCopy[8]
		mov	word ptr ChVoice[8],ax
		mov	ax,word ptr ChVoiceCopy[10]
		mov	word ptr ChVoice[10],ax
		mov	ax,word ptr ChVoiceCopy[12]
		mov	word ptr ChVoice[12],ax
		mov	ax,word ptr ChVoiceCopy[14]
		mov	word ptr ChVoice[14],ax
		mov	ax,word ptr ChBedCopy
		mov	word ptr ChBed,ax
		mov	ax,word ptr ChBedCopy[2]
		mov	word ptr ChBed[2],ax
		mov	ax,word ptr ChBedCopy[4]
		mov	word ptr ChBed[4],ax
		mov	ax,word ptr ChBedCopy[6]
		mov	word ptr ChBed[6],ax
		mov	ax,word ptr ChBedCopy[8]
		mov	word ptr ChBed[8],ax
		mov	ax,word ptr ChBedCopy[10]
		mov	word ptr ChBed[10],ax
		mov	ax,word ptr ChBedCopy[12]
		mov	word ptr ChBed[12],ax
		mov	ax,word ptr ChBedCopy[14]
		mov	word ptr ChBed[14],ax
		pop	ax
		mov	al,vLeftCopy
		ENDM

;***************************************************
;* PreemptChn1 macro
;* 	al = voices available
;*
;*	Returns dh = 0FFH if couldn't preempt anything
;*			  or
;*			  dh = channel that was preempted	
;*			  al = updated # of available voices
;***************************************************
PreemptChn1	MACRO			;Preempt a channel off of
					;the ChNew list
		push	edi				
		push	ecx
		push	eax

	;search the channel priority list for the lowest priority channel

		mov	dh,0ffh		;found a non-0 priority channel flag
		xor	ax,ax		;lowest priority found so far
		xor	edi,edi		;channel index
p11:
		cmp	al,ChPri[edi]	;is this channel's pri lower that last?
		jae	p12		;no-skip
		mov	al,ChPri[edi]	;yes-save new pri as lowest found so far
		mov	cx,di		;save low pri channel #
		mov	dh,cl		;in dh

p12:
		inc	di
		cmp	di,16		;done with 16 channels?
		jne	p11		;no, loop for more

		pop	eax		
		cmp	dh,0ffh		;did we locate a non-0 priority channel?
		je	p13		;no-skip out

		xor	cx,cx
		mov	cl,dh		;yes-get the channel to preempt
		mov	di,cx		;in di

		add	al,ChVoice[edi]	;add that channel's voices back into available
		mov	ChNew[edi],0ffh	;mark the channel as available
		mov	ChVoice[edi],0	;and reset the channel's properties
		mov	ChPri[edi],0
		mov	ChBed[edi],0
p13:	        pop	ecx
		pop	edi
		ENDM

;***************************************************
;* PreemptChn2 macro
;*
;*		This macro is IDENTICAL to PreemptChn1, I don't
;*		know of any reason for it to be duplicated.
;*
;* 	al = voices available
;*
;*	Returns dh = 0FFH if couldn't preempt anything
;*			  or
;*			  dh = channel that was preempted	
;*			  al = updated # of available voices
;***************************************************
PreemptChn2	MACRO			;Preempt a channel off of
		push	edi		;the ChNew list
		push	ecx
		push	eax
		mov	dh,0ffh
		xor	ax,ax
		xor	edi,edi
p21:	cmp	al,ChPri[edi]
		jae	p22
		mov	al,ChPri[edi]
		mov	cx,di
		mov	dh,cl
p22:	inc	di
		cmp	di,16
		jne	p21
		pop	eax
		cmp	dh,0ffh
		je		p23
		xor	cx,cx
		mov	cl,dh
		mov	di,cx
		add	al,ChVoice[edi]
		mov	ChNew[edi],0ffh
		mov	ChVoice[edi],0
		mov	ChPri[edi],0
		mov	ChBed[edi],0
p23:	        pop	ecx
		pop	edi
		ENDM

;***************************************
;* SwapChnls Macro - 
;*		Swap two channels on the ChNew list
;*		si = channel A
;*		di = channel B
;***************************************
SwapChnls	MACRO
		push	eax
		mov	al,ChNew[edi]	    ;swap the PlayList position/channel bytes
		mov	ah,ChNew[esi]
		mov	ChNew[edi],ah
		mov	ChNew[esi],al

		mov	al,ChPri[edi]	    ;and the channel priority bytes
		mov	ah,ChPri[esi]
		mov	ChPri[edi],ah
		mov	ChPri[esi],al

		mov	al,ChVoice[edi]	    ;and the voices used bytes
		mov	ah,ChVoice[esi]
		mov	ChVoice[edi],ah
		mov	ChVoice[esi],al

		mov	al,ChBed[edi]	    ;and the "locked channel" flags
		mov	ah,ChBed[esi]
		mov	ChBed[edi],ah
		mov	ChBed[esi],al
		pop	eax
		ENDM

;***************************************
;* UpdateChannel1 macro
;*		al = sequenced channel
;*		si = channel list channel to update
;***************************************
UpdateChannel1	MACRO		     	    ;Update a channel from node
		push	eax	     	    ;information
		push	edx
		push	esi

		xor	ah,ah	     	    ;get sequenced channel in si
		xchg	si,ax	     	    ;channel list channel in al
		and	esi,0ffffh
		mov	cx,(ALLNOFF*256)    ;mov ch,ALLNOFF: mov cl,0
		CallDriver DController	    ;send allnotes off to channel list channel

		mov	cl,es:[ebx+esi].cPriVoice ;get the channel's numNotes
		and	cl,0fh
		mov	ch,NUMNOTES
		CallDriver DController		  ;tell the driver about it.

		mov	cl,es:[ebx+esi].cProgram  ;send the channel's program
		CallDriver DProgramChange

		push	si
		mov	si,ax			  ;channel list channel in si
		mov	VolRequest[esi],0ffh	  ;clear any pending channel volume
		pop	si			  ;sequenced channel in si
		mov	cl,es:[ebx+esi].cVolume	  ;get channel's volume
		mov	dl,es:[ebx].sVolume	  ;and the node's volume
		call	ScaleVolume		  ;scale it
		mov	ch,VOLCTRL
		CallDriver DController		  ;and send it to the driver

		mov	ch,PANCTRL		  ;update the pan controller
		mov	cl,es:[ebx+esi].cPan
		CallDriver DController

		mov	ch,MODCTRL		  ;update the modulation controller
		mov	cl,es:[ebx+esi].cModulation
		CallDriver DController

		shl	si,1					  ;sequenced channel * 2
		mov	cx,(DAMPRCTRL*256)			  ;mov ch,DAMPRCTRL: mov cl,0
		cmp	byte ptr es:[ebx+esi +1].cDamprPbend,080h ;is damper bit on?
		jb	local1Upd1
		mov	cl,07fh					  ;yes-setup to send damper on
local1Upd1:
		CallDriver DController

		mov	cx,es:[ebx+esi].cDamprPbend ;get bender value
		shr	si,1			    ;sequenced channel * 1
		xchg	cl,ch			    ;get MSB in cl, LSB in ch
		shl	cl,1			    ;get 7 bits in each
		cmp	ch,080h			    ;if ch hi bit is set,
		jb	local2Upd1
		or	cl,1		    	    ;move it to cl low bit
local2Upd1:
		and	cx,7f7fh		    ;get rid of hi bits
		CallDriver DPitchBend		    ;send to driver

		mov	cl,es:[ebx+esi].cCurNote    ;send the current note
		mov	ch,CURNOTE		    ;(affects PC driver only)
		CallDriver DController

		pop	esi
		pop	edx
		pop	eax
		ENDM

;***************************************
;* UpdateChannel2 Macro
;***************************************
UpdateChannel2	MACRO				   ;Update a channel from node
		push	eax			   ;information
		push	edx
		push	esi

		xor	ah,ah
		xchg	si,ax
		and	esi,0ffffh

		mov	cx,(ALLNOFF*256)
		CallDriver DController
		mov	cl,es:[ebx+esi].cPriVoice
		and	cl,0fh
		mov	ch,NUMNOTES
		CallDriver DController
		mov	cl,es:[ebx+esi].cProgram
		CallDriver DProgramChange
		push	si
		mov	si,ax
		mov	VolRequest[esi],0ffh
		pop	si
		mov	cl,es:[ebx+esi].cVolume
		mov	dl,es:[ebx].sVolume
		call	ScaleVolume
		mov	ch,VOLCTRL
		CallDriver DController
		mov	ch,PANCTRL
		mov	cl,es:[ebx+esi].cPan
		CallDriver DController
		mov	ch,MODCTRL
		mov	cl,es:[ebx+esi].cModulation
		CallDriver DController
		shl	si,1
		mov	cx,(DAMPRCTRL*256)
		cmp	byte ptr es:[ebx+esi + 1].cDamprPbend,080h
		jb		local1Upd2
		mov	cl,07fh
local1Upd2:
		CallDriver DController
		mov	cx,es:[ebx+esi].cDamprPbend
		shr	si,1
		xchg	cl,ch
		shl	cl,1
		cmp	ch,080h
		jb		local2Upd2
		or		cl,1
local2Upd2:
		and	cx,7f7fh
		CallDriver DPitchBend
		mov	cl,es:[ebx+esi].cCurNote
		mov	ch,CURNOTE
		CallDriver DController

		pop	esi
		pop	edx
		pop	eax
		ENDM
_DATA    ENDS


		PUBLIC	DoSound		; C Functions
                                               
		PUBLIC	PatchReq	;
		PUBLIC	Init		;
		PUBLIC	Terminate	;
		PUBLIC	ProcessSounds	;
		PUBLIC	SoundOn		;
		PUBLIC	RestoreSound	;
		PUBLIC	MasterVol	;
		PUBLIC	SetReverb	;
		PUBLIC	PlaySound	; 
		PUBLIC	EndSound	; Assembly functions
		PUBLIC	PauseSound	;
		PUBLIC	FadeSound	;
		PUBLIC	HoldSound	;
		PUBLIC	MuteSound	;
		PUBLIC	ChangeVol	;
		PUBLIC	ChangePri	;
		PUBLIC	GetSignal	;
		PUBLIC	GetDataInc	;
		PUBLIC	GetSMPTE	;
		PUBLIC	SendNoteOff	;
		PUBLIC	SendNoteOn	;
		PUBLIC	SendPChange	;
		PUBLIC	SendContrlr	;
		PUBLIC	SendPBend	;
		PUBLIC	SoundServer	;
		PUBLIC	GetSignalRset	;

		PUBLIC	PlayList	;
		PUBLIC	ChList		; Public tables
		PUBLIC	SampleList	;
		PUBLIC	AskDriver       
ifdef RECMID
    
	
                PUBLIC  MIDIbufbeg
                PUBLIC  MIDIbufend
                PUBLIC  MIDIbhead
                PUBLIC  MIDIbtail
                PUBLIC  MIDIbcount


                PUBLIC  StashMIDI
                 
		EXTRN	UpdateLists:NEAR
endif

ifdef	WINDOWS
		EXTRN	Driver:NEAR		;driver is linked
endif

_TEXT    segment byte public use32 'CODE'
assume cs:_TEXT 

;---------------------------------------------------------------------------
;	MIDI.S interface to C
;---------------------------------------------------------------------------

DoSound		PROC		       	;C INTERFACE
		push	ebp
		mov	ebp,esp
                push    fs
                push	es
		push	ds
		push	edi
		push	esi
                push    ebx
                push    ecx
                push    edx

                mov     SaveDS,DS

	;If the function was SMasterVol, SProcesss,
	;SSetReverb, or SSoundOn, or STerminate, then the first argument
	;was an int value, not an address

		mov	cx,word ptr [ebp+6+Cparam]  	
		cmp	cl,SMasterVol		   	
		je	fixFirstArg	   	   	
		cmp	cl,SProcess		   	
		je	fixFirstArg	   	   	
		cmp	cl,SSoundOn		   	
		je	fixFirstArg                
		cmp	cl,SSetReverb
		je	fixFirstArg                
		cmp	cl,STerminate
                je      fixFirstArg       
		cmp	cl,SPatchReq
		je	ADDRP
		cmp	cl,SGetSMPTE
		je	ADDRP                	
         	

		;Otherwise, the first argument is an address

		les	eax,fword ptr [ebp+10+Cparam] 
		mov	dx,word ptr [ebp+18+Cparam] 
		mov	Carg1,dx
		mov	dx,word ptr [ebp+22+Cparam]
		mov	Carg2,dx
		mov	dx,word ptr [ebp+26+Cparam]
		mov	Carg3,dx
		mov	dx,word ptr [ebp+30+Cparam]
		mov	Carg4,dx
		mov	dx,word ptr [ebp+34+Cparam]
		mov	Carg5,dx
		mov	dx,word ptr [ebp+38+Cparam]
		mov	Carg6,dx
		jmp	doSoundCall
fixFirstArg:
		mov	dx,word ptr [ebp+10+Cparam]  
		mov	Carg1,dx
		jmp	doSoundCall

ADDRP:
		les	eax,fword ptr [ebp+18+Cparam] 
		mov	ARG1,eax
		mov	ARG1DATA,es
;;*		mov	SaveFS,es

      		les	eax,fword ptr [ebp+26+Cparam] 
		mov	ARG2,eax
		mov	ARG2DATA,es

      		les	eax,fword ptr [ebp+34+Cparam] 
		mov	ARG3,eax
		mov	ARG3DATA,es

		les	eax,fword ptr [ebp+10+Cparam]

		jmp	doSoundCall
;ADDRS:
;              	mov	eax,dword ptr [ebp+10+Cparam] 
;		mov	edx,dword ptr [ebp+14+Cparam] 
;		mov	ARG1,edx
;		mov	edx,dword ptr [ebp+18+Cparam]
;		mov	ARG2,edx
;		mov	edx,dword ptr [ebp+22+Cparam]
;		mov	ARG3,edx

doSoundCall:
		call	DispatchC
outTest:        pop     edx
                pop     ecx
                pop     ebx
		pop	esi
		pop	edi
		pop	ds
		pop	es
                pop     fs
		pop	ebp
		ret
DoSound		ENDP


;***************************************
;* DispatchC
;* Function # in cl, args in Cargx
;***************************************
DispatchC	PROC	NEAR
;		mov	fs,SaveFS
		cmp	cl,SPatchReq
		jne	notPatchReq

	;patch request, returns ax=patch num or -1 if none
	;			numVoices at carg1
	;			0 at carg3
	;			devID at carg5


		call	PatchReq
		xor	ah,ah
		cmp	al,0ffh
		jne	Patch
		mov	ah,al
;		ret		;make it like 16-bit version -TBM 5/13/94
Patch:
notNoPatch:
		xor	cx,cx
		mov	cl,numVoices
		mov	ebx,dword ptr ARG1
		mov	es,ARG1DATA
		mov	es:[ebx],cx
		mov	cl,0
		mov	ebx,dword ptr ARG2
		mov	es,ARG2DATA
		mov	es:[ebx],cx
		mov	cl,devID
		mov	ebx,dword ptr ARG3
		mov	es,ARG3DATA
		mov	es:[ebx],cx
		ret

notPatchReq:
		cmp	cl,SInit
		jne	notInit
                xor     ebx,ebx
		mov	bx,Carg1
		call	Init
		ret

notInit:
		cmp	cl,STerminate
		jne	notTerminate
		call	Terminate
		ret

notTerminate:
		cmp	cl,SProcess
		jne	notProcess
		mov	cx,Carg1
		call	ProcessSounds
		ret

notProcess:
		cmp	cl,SSoundOn
		jne	notSoundOn
		mov	cx,Carg1
		call	SoundOn
		ret

notSoundOn:
		cmp	cl,SRestore
		jne	notRestore
		call	RestoreSound
		ret

notRestore:
		cmp	cl,SMasterVol
		jne	notMasterVol
		mov	cx,Carg1
		call	MasterVol
		ret

notMasterVol:
		cmp	cl,SSetReverb
		jne	notSetReverb
		mov	cx,Carg1
		call	SetReverb
		ret

notSetReverb:
		cmp	cl,SPlay
		jne	notPlay
		mov	cx,Carg1	;Funky Bed sound flag
		call	PlaySound
		ret

notPlay:
		cmp	cl,SEnd
		jne	notEnd
		call	EndSound
		ret

notEnd:
		cmp	cl,SPause
		jne	notPause
		mov	cl,byte ptr Carg1
		call	PauseSound
		ret

notPause:
		cmp	cl,SFade
		jne	notFade
		mov	dl,byte ptr Carg1
		mov	cl,byte ptr Carg2
		mov	ch,byte ptr Carg3
		call	FadeSound
		ret

notFade:
		cmp	cl,SHold
		jne	notSHold
		mov	cl,byte ptr Carg1
		call	HoldSound
		ret

notSHold:
		cmp	cl,SMute
		jne	notSMute
		mov	cx,Carg1
		call	MuteSound
		ret

notSMute:                   	
		cmp	cl,SChangeVol
		jne	notChangeVol
		mov	cx,Carg1
		call	ChangeVol
		ret

notChangeVol:
		cmp	cl,SChangePri
		jne	notChangePri
		mov	cx,Carg1
		call	ChangePri
		ret			
notChangePri:
		cmp	cl,SGetDataInc
		jne	notGetData
		call	GetDataInc
		mov	ax,cx		 ;return datainc value in ax
		ret
notGetData:
		cmp	cl,SGetSignal
		jne	notGetSignal
		call	GetSignal
		mov	al,cl
		xor	ah,ah		 ;return signal in ax
		ret

notGetSignal:
		cmp	cl,SGetSignalRset
		jne	notGetSignalRset
		call	GetSignalRset
		mov	al,cl
		xor	ah,ah		 ;return signal in ax
		ret

notGetSignalRset:
		cmp	cl,SGetSMPTE
		jne	notGetSMPTE
		call	GetSMPTE
		xor	ah,ah
		mov     ebx,dword ptr ARG1	 ;return SMPTE codes at addresses
		mov	es,ARG1DATA
		mov	al,dl
		mov	es:[ebx],ax		 ;minutes
		mov	ebx,dword ptr ARG2
		mov	es,ARG2DATA	 
		mov	al,ch
		mov	es:[ebx],ax		 ;seconds
		mov	ebx,dword ptr ARG3
		mov	es,ARG3DATA
		mov	al,cl
		mov	es:[ebx],ax		 ;frames
                ret

notGetSMPTE:
		cmp	cl,SNoteOff
		jne	notSNoteOff
		mov	dl,byte ptr Carg1
		mov	ch,byte ptr Carg2
		mov	cl,byte ptr Carg3
		call	SendNoteOff
		ret

notSNoteOff:
		cmp	cl,SNoteOn
		jne	notSNoteOn
		mov	dl,byte ptr Carg1
		mov	ch,byte ptr Carg2
		mov	cl,byte ptr Carg3
		call	SendNoteOn
		ret

notSNoteOn:
		cmp	cl,SController
		jne	notSContrlr
		mov	dl,byte ptr Carg1
		mov	ch,byte ptr Carg2
		mov	cl,byte ptr Carg3
		call	SendContrlr
		ret

notSContrlr:
		cmp	cl,SPChange
		jne	notSPChange
		mov	dl,byte ptr Carg1
		mov	cl,byte ptr Carg2
		call	SendPChange
		ret

notSPChange:
		cmp	cl,SPBend
		jne	notSPbend
		mov	dl,byte ptr Carg1
		mov	cx,word ptr Carg2
		call	SendPBend
		ret

notSPbend:

ifdef RECMID
		cmp	cl,SAskDriver
		jne	notSAskDriver
		mov	ah,byte ptr Carg1
		mov	al,byte ptr Carg2
		mov	ch,byte ptr Carg3
		mov	cl,byte ptr Carg4
		call	AskDriver
notSAskDriver:
endif

		ret
DispatchC	ENDP

;---------------------------------------------------------------------------
;
;---------------------------------------------------------------------------


;*********************************************
;* PatchReq
;*		es:eax is address of Driver
;*	returns:
;*		Updates globals numVoices and devID and Driver
;*		al = patch number, 0FFH if none	
;*********************************************

PatchReq	PROC				;DETERMINE PATCH REQUIREMENTS
ifndef WINDOWS					;driver is linked for windows
		mov	dword ptr Driver,eax	;save Driver address
		mov	word ptr Driver[4],es
endif

; @INT - Set address of StashMIDI routine for driver to call
;	 This kludge is here because:
;	 1) We wanted to use MTEST to receive MIDI from a sequencer on
;	    one computer to drive another MIDI device through MIDI.S and
;	    our driver.
;	 2) The MPU MIDI interfaces require all received bytes be taken
;	    care of before it will let us send one. In normal game operation,
; 	    the MPU drivers just gobble the incoming bytes since we don't 
;	    need them.
;	 3) In order to allow MTEST to receive the incoming bytes, there is
;	    a special development MPU driver that stashes the incoming bytes
;	    in a buffer rather than just axing them
;	 4) So here, we pass the address of the StashMIDI routine for the
;	    development driver to call.

ifdef RECMID
                mov     ax,cs
                mov     es,ax
                mov     eax,offset _TEXT:StashMIDI
endif
                
	       	CallDriver DPatchReq
		mov	numVoices,cl
		mov	devID,ch
		ret
PatchReq	ENDP


;*****************************************
;* Init
;*****************************************
Init		PROC				 ;INITIALIZE SOUND DRIVER
		push	cx


	       	CallDriver DInit		 ;Call Init function of
		mov	loChnl,cl		 ;Driver and set loChnl &
		mov	hiChnl,ch		 ;hiChnl with return value

		push	ax
		xor	cl,cl	      
						 ;Reset reverb mode
	       	CallDriver DSetReverb
		pop	ax

		pop	cx
		ret
Init		ENDP


;******************************************
;* Terminate
;******************************************
Terminate	PROC			  	;TERMINATE DRIVER
		push	bx
		push	cx
		push	es
		push	si


		mov	cl,15			;Leave the devices volume
		CallDriver DMasterVol		;at maximum

		CallDriver DTerminate		;Terminate the driver
					
ifdef RECMID				   	;*** If UART was turned on,
		call	ResetMPU	   	;Reset the MPU
                call    Drop_M_Interrupt   	;Remove MIDI interrupt - @INT 
endif

outTerminate:
		pop	si
		pop	es
		pop	cx
		pop	bx
		ret
Terminate	ENDP


;******************************************************
;* ProcessSounds - set or reset the processSounds Flag
;*		cx = TRUE or FALSE
;* The processSounds "Flag" is actually a count of how
;* may times the server has been disabled.
;******************************************************
ProcessSounds	PROC
		cmp	cx,0
		jne	processTrue	;if FALSE...
		inc	processSnds	;inc the "flag"
		ret

processTrue:				;if TRUE
		cmp	processSnds,0	;if the flag is already 0
		je	processExit	;then do nothing
		dec	processSnds	;otherwise, dec the count
processExit:
		ret
ProcessSounds	ENDP


;***********************************************************
;*	SoundOn - turns the sound on or off
;*			cx = 255	;request current state
;*			   = 0		;turn sound off
;*			   = 1		;turn sound on			
;***********************************************************
SoundOn		PROC
		CallDriver DSoundOn
		ret
SoundOn		ENDP


;*****************************************************************
;*	RestoreSound - this is called when restoring from a save game
;*		       Basically, it "plays" through the sound (as fast
;*		       as it can without actually creating any sound)
;*		       until it reaches the current timer value.	
;*
;*	es:ax = pointer to the sound node to restore
;*****************************************************************
RestoreSound	PROC				;PUT SOUND BACK ON PLAYLIST
		pushf
		;cli
		push	esi
		push	eax
		push	ebx
		push	ecx
		push	edx

		mov	ebx,eax			;pointer to the sound

		xor	ch,ch			;** That funky bed sound thing
		mov	cl,es:[ebx].sState
		dec	cl

		mov	esi,14
loopSaveNode:					;Save the values of the node
		mov	dl,es:[ebx+esi].cMute	;which will be overwritten
		and	dl,0f0h			;by the restore, and that
		mov	savMuteFlag[esi],dl	;must remain intact
		dec	si			;for 15 channels
		jns	loopSaveNode
		mov	dl,es:[ebx].sHold			;which will be overwritten
                mov     Holding,dl

		mov	restoring,1		;set flag to tell PlaySound not to make noise
		call	PlaySound		;Put it on the PlayList

		mov	cx,  es:[ebx].sTimer	   ;get the timer value to restore to in cx
		mov	word ptr es:[ebx].sTimer,0 ;and zero out the timer
		mov	al,es:[ebx].sLoop	   ;save the current loop property in al
		mov	byte ptr es:[ebx].sLoop,1  ;and set the node to loop

		cmp	cx,0			;are we restoring to the start of the sound?
		je	noRestore		;yes-so we're already done-skip out
loopRest:
		mov	dx,es:[ebx].sTimer	;Fast forward through sound
		call	ParseNode		;to the point where it was
		cmp	dx,es:[ebx].sTimer	;saved
		jb	noWrap			;if current timer > last timer then skip
		je	noRestore		;if current timer = last timer

		sub	dx,es:[ebx].sTimer	;??? otherwise timer value wrapped
		sub	cx,dx			;???
noWrap:
		cmp	cx, es:[ebx].sTimer	;if saved timer and current timer are
		jne	loopRest		;not the same, keep looping

noRestore:
		mov	es:[ebx].sLoop,al	;Put old loop value back, and
		mov	restoring,0		;clear restore flag

		mov	esi,14			
loopRestNode:					;Restore the values of the
		mov	dl,savMuteFlag[esi]	;node that were previously
		or	es:[ebx+esi].cMute,dl	;saved
		dec	si
		jns	loopRestNode

		mov   	dl,Holding
      		mov  	es:[ebx].sHold,dl


		call	DoChannelList		;stick this node's channels on the channel list

		pop	edx
		pop	ecx
		pop	ebx
		pop	eax
		pop	esi
		popf
		ret
RestoreSound	ENDP



;**********************************************************
;* MasterVol - CHANGE MASTER VOLUME
;*	       cl = 255  request current state
;*	       cl = 0-15 set master volume
;*	       in all cases returns current state
;**********************************************************
MasterVol	PROC
		cmp	cl,255		;If the volume is 255, then
		je	volOK		;we just want a return value
                                                        
		cmp	cl,15		;Insure that the volume is
		jbe	volOK		;15 or less
		mov	cl,15

volOK:
		CallDriver DMasterVol
		ret
MasterVol	ENDP


;*********************************************************
;* SetReverb - CHANGE REVERB MODE
;*	       --- this is no longer has any effect on any
;*	       	   current drivers 12/23/92 ---
;*
;*		cl = 255  	request	current state
;*		cl = 11-254	request default mode
;*		cl = 0-10 	set reverb mode
;*		in all cases returns current default mode
;*********************************************************
SetReverb	PROC				;CHANGE REVERB MODE
		cmp	cl,255			;If the mode is 255, then
		jne	notAskRevMode		;we just want a return value
		CallDriver DSetReverb
		ret

notAskRevMode:
		cmp	cl,10			;Insure that reverb mode is
		jbe	revOK			;10 or less.  If it's not,
		mov	al,revDefault		;then return the current
		xor	ah,ah			;default
		ret

revOK:
		mov	al,revDefault		;See if the first node on the
		push	es			;PlayList is set to default
		push	eax			;reverb.  If so, tell the
		push	ebx			;driver to change the reverb
		push	edx			;mode
		mov	revDefault,cl		;save new default mode

		les	ebx,PlayList		;get first address on Playlist
		mov	dx,es
		or	dx,bx
		jz	outSetReverb		;if it's null, there are no nodes, skip out

		cmp  	es:[ebx].sReverbMode,07fh
		jne	outSetReverb
		CallDriver DSetReverb

outSetReverb:
		pop	edx
		pop	ebx
		pop	eax
		pop	es
		ret
SetReverb	ENDP


;*******************************************************
;*	PlaySound - ADD SOUND TO PLAYLIST
;*	  es:ax = sound node address
;*	  cx = *** the funky bedsound flag
;* 	
;*******************************************************
PlaySound	PROC
		pushf
		;cli
;		inc	processSnds 		;temporarily disable the sound server

		push	ecx
                push    ebx

		push	ecx
		mov	cx,es			 	;If node es:ax is already
		xor	edi,edi			 	;on the PlayList, then
PfindNode:
		cmp	dword ptr PlayList[edi],eax	;do an End before adding
		jne	notThisNode		 	;it again
		cmp	word ptr PlayList[edi+4],cx
		jne	notThisNode
		call	DoEnd
		call	DoChannelList
		jmp	initNode
notThisNode:
		add	edi,6		       ;each playlist entry is 6 bytes long
		cmp	edi,(LISTSIZE * 6)
		jne	PfindNode

initNode:
		pop	ecx			;*** restore funky bed sound flag

		mov	ebx,eax			;bx=sound node ptr
		mov	es:[ebx].sState,1     	;Set the state property of
						;the node to TRUE (which
						;means the sound is playing)


		cmp	cx,0			;*** if it's a funky bed sound
		je	itsASFX			;***
		inc	es:[ebx].sState		;*** then bump the state (2=bed sound)
itsASFX:
		call	FixupHeader		;Set up the resource header

		;Initialize all index, rest, PriVoice, and channel parameters
		mov	ebx,eax			;bx=sound node ptr
		xor	esi,esi			;track/channel index
		xor	ecx,ecx			
		dec	ch			;ch=FF, cl=0, used as initializers
notTr16:
		shl	si,1			
		mov	word ptr es:[ebx+esi].tIndex,13
		mov	word ptr es:[ebx+esi].tLoopPoint,3
		mov	word ptr es:[ebx+esi].tRest,0
		mov	word ptr es:[ebx+esi].tLoopRest,0
		mov	word ptr es:[ebx+esi].cDamprPbend,2000h
		shr	si,1
		mov	es:[ebx+esi].tChannel,ch	;-1
		mov	es:[ebx+esi].tCommand,cl	;0
		mov	es:[ebx+esi].tLoopCommand,cl	;0
		mov	es:[ebx+esi].cPriVoice,ch	;-1
		mov	es:[ebx+esi].cModulation,cl	;0
		mov	es:[ebx+esi].cProgram,ch	;-1
		mov	es:[ebx+esi].cVolume,ch		;-1
		mov	es:[ebx+esi].cPan,ch		;-1
		mov	es:[ebx+esi].cCurNote,ch	;-1
		mov	es:[ebx+esi].cFlags,cl		;0
		mov	es:[ebx+esi].cMute,cl		;0
		inc	si

		cmp	si,15
		jne	notTr16
		mov	es:[ebx+esi].tChannel,ch	;-1
		mov	es:[ebx+esi].tCommand,cl        ;0
		mov	es:[ebx+esi].tLoopCommand,cl	;0
;		mov	es:[ebx].sSample,cl		;0
		mov	es:[ebx].sHold,cl		;0
		mov	byte ptr es:[ebx].sReverbMode,07fh   ;set to default reverb mode
		shl	si,1
		mov	word ptr es:[ebx+esi].tIndex,13
		mov	word ptr es:[ebx+esi].tLoopPoint,3   ;init loop point past the
							     ;initial pgm, pan, and vol
							     ;or reverb if chan 16
		mov	word ptr es:[ebx+esi].tRest,0
		mov	word ptr es:[ebx].sLoopTime,0
		mov	word ptr es:[ebx].sVolume,127

		push	eax
		push	ebp
		push	ds
		xor	esi,esi

ifdef RECMID			      	       		;If the resource
		mov     ebp,dword ptr es:[ebx].sMidiPtr32 ;pointer is ffff:ffff,
		cmp	ebp,0ffffffffh			;should be taken from
		jne	playDeRef			;the MIDI port, rather
                                			;than from memory
		mov	oldRecCom,0
		cmp	UARTmode,0			;if we haven't yet inited the MPU
		jne	UARTon
		mov	UARTmode,1			;then do so now!
UARTon:
		push	ebx
		mov	bx,0ffh        			;reset MPU
		call	MPUcomm
		mov	bx,03fh				;then set it to UART mode
		call	MPUcomm
                call    Set_M_Interrupt     		;Install MIDI interrupt - @INT
		pop	ebx
		pop	ds
		pop	ebp
		pop	eax
		jmp	resetCritical
endif

playDeRef:
        	Hnd2Ptr es:[ebx].sMidiPtr32	;De-reference resource ptr to ebp

		;If there is priority information in the resource,
		;and the sFixedPri flag is not set, then set the
		;sPriority property to the priority in the resource

		cmp	byte ptr fs:[ebp+32],-1	;is there priority in the soundfile?
		je	initTheTracks		;no-skip around priority stuff
		cmp	es:[ebx].sFixedPri,0 
		jne	initTheTracks		;fixed pri is set so skip
		mov	cl,fs:[ebp+32] 		;otherwise stuff sound file
		mov	es:[ebx].sPriority,cl	;priority in the node

initTheTracks:

;Set channel values and rest
;for tracks. Set PriVoice,
;Volume and Pan for channels.

		mov	ecx,ebp	  ;edc		;save resource ptr top
		xor	edx,edx	  ;104
notTch16:
    		mov	dx,fs:[ebp]		;get 1st track offset
		cmp	dx,0			;if NULL then we are done
		je	endChnl 	
		mov	ebp,ecx	  ;edc		;restore resource ptr top
		add	ebp,edx	  ;edc?104		;point to the track
		mov	dl,byte ptr fs:[ebp]	;get the byte there


		mov	es:[ebx+esi].tChannel,dl	;if NOT a sample, save the track's channel in the node
		mov	dh,dl				;save the channel
		or	dh,0b0h				;init the command to "controller" since that's the first
		mov	es:[ebx+esi].tCommand,dh	;thing we'll be sending
		mov	dl,byte ptr fs:[ebp+12]		;get the first timing byte
		xor	dh,dh				;zero out the overflow bit (highest bit)
		cmp	dl,TIMINGOVER
		jne	notTOver
		mov	dl,0f0h				;if first timing is "timing overflow"
		mov	dh,080h				;set "overflow" bit and timing = 240

notTOver:
		shl	si,1				;track/channel index * 2
		mov	es:[ebx+esi].tRest,dx		;save the timing counter
		shr	si,1

		push	esi
		mov	dl,es:[ebx+esi].tChannel
		and	byte ptr es:[ebx+esi].tChannel,0fh ;make the track's channel <15
		test	dl,16				   ;if it's 16-31 then it's a ghost
		jz	notGhostTrack

		;it's a ghost track
		shl	si,1				   ;track/channel index * 2
		mov	word ptr es:[ebx+esi].tIndex,3	   ;init it past the initial pgm,pan, and vol
		mov	word ptr es:[ebx+esi].tRest,0	   ;init the rest counter
		xor	dh,dh
		mov	si,dx
		and	si,0fh				   ;get the channel in si
		or	byte ptr es:[ebx+esi].cFlags,2	   ;set the "ghost" channel flag
		jmp	nextInitChnl   			   ;removed SHORT

		;it's not a ghost track
notGhostTrack:
		xor	dh,dh
		mov	si,dx				;get the channel in si
		and	esi,0fh				;if it is 32-47 then it's locked
		test	dl,32
		jz	notLockedCh
		or	es:[ebx+esi].cFlags,1		;set the "locked" channel flag

notLockedCh:						
		test	dl,64				;if it's 64-79 then it's muted
		jz	notMutedCh
		mov	es:[ebx+esi].cMute,1		;set the channel "muted" property
notMutedCh:
		cmp	si,15				;if this is not channel 16 skip...
		jne	notInitCh16
		cmp	es:[ebx].sReverbMode,07fh	;if the node's reverb is not default
		jne	noPVol
		mov	al,fs:[ebp+8]			;then get the mode from the resource 
		mov	es:[ebx].sReverbMode,al		;and save it in the node
		jmp	nextInitChnl

notInitCh16:
		cmp	byte ptr es:[ebx+esi].cPriVoice,0ffh  ;if the channel's priority has not yet been set...
		jne	noPPri
		mov	al,byte ptr fs:[ebp+1]		      ;then grab the track's priority from the resource
		mov	es:[ebx+esi].cPriVoice,al	      ;and save it in the node
noPPri:
		cmp	byte ptr es:[ebx+esi].cProgram,0ffh   ;if the channel's pgm has not yet been set...
		jne	noPPrg				      ;then grab the pgm from the resource
		mov	al,fs:[ebp+4]			      ;and save it in the node
		mov	es:[ebx+esi].cProgram,al
noPPrg:
		cmp	byte ptr es:[ebx+esi].cVolume,0ffh    ;the channel's vol has not yet been set
		jne	noPVol
		mov	al,fs:[ebp+8]			      ;then grab it from the resource
		mov	es:[ebx+esi].cVolume,al		      ;and save it in the node
noPVol:
		cmp	byte ptr es:[ebx+esi].cPan,0ffh	      ;if the channel's pan has not yet been set
		jne	nextInitChnl
		mov	al,fs:[ebp+11]			      ;then grab it from the resource
		mov	es:[ebx+esi].cPan,al		      ;and save it in the node
nextInitChnl:
		pop	esi				      ;restore track/channel index
nextInitChnl2:
		inc	si		;next channel/track
		shl	si,1		;* 2
		mov	ebp,ecx		;restore resource top to bp
		add	ebp,esi		;and point bp to the next track ptr
		shr	si,1
		cmp	si,16		;done with 16 tracks?
endChnlje:
		je	endChnl
		jmp	notTch16	;no, go back for more..
endChnl:
		pop	ds
		pop	ebp
		pop	eax

resetCritical:
		cmp	es:[ebx].sState,2	;if this is a "funky bed" sound...
		jne	notFunkyBed
		mov	edi,14			;for all 15 channels...
loopFunkyBed:
		or	es:[ebx+edi].cFlags,1	;set the "locked" flag
		dec	di
		jns	loopFunkyBed

;Find a spot on the PlayList to append or
;insert the new sound node.	If there is no available
;spot, and all sounds on the PlayList are of a
;higher priority, then exit the procedure

notFunkyBed:
		mov	eax,ebx			  ;save sound node pointer
		mov	dl,es:[ebx].sPriority	  ;get node's priority in dl
		push	es
		xor	edi,edi			  ;init Playlist ptr
PfindPri:
		cmp	dword ptr PlayList[edi],0 ;is this slot open? (test segment only?)
						  ;CBG ??? do we need a better test here?
		jne	notEmpty
		cmp	word ptr PlayList[edi+4],0

		je	placeHere 		  ;yes-go insert it here
notEmpty:
		les	ebx,PlayList[edi]	  ;no-get ptr to the node that's here
		cmp	es:[ebx].sPriority,dl	  ;and compare the priority
		jbe	insertHere		  ;if our's is <= this one then go insert it here
		add	edi,6			  ;otherwise point to the next Playlist entry
		cmp	edi,(LISTSIZE * 6)
		jne	PfindPri		  ;if we are not done, loop back for more
		pop	es
		jmp	outPlay                   ;didn't find a slot-skip out

;If the slot in the middle of the play list then move everyone down one
insertHere:
		xor	esi, esi
		mov	si,((LISTSIZE - 2) * 6)	  ;point si to the next to last slot
keepInsrting:
		mov	bx,si
		add	bx,6			  ;next PlayList entry
		cmp	bx,di			  ;When the destination slot = the slot
		je	placeHere		  ;we want for the new node, we are done

		mov	ebx,dword ptr PlayList[esi]    ;copy 6 byte pointer to next higher slot				
		mov	dword ptr PlayList[esi+6],ebx				
		mov	bx,word ptr PlayList[esi+4]
		mov	word ptr PlayList[esi+10],bx
		sub	si,6            	       ;work towards the front of the Playlist
		jmp	keepInsrting

;Place node on play list where di is pointing to
placeHere:
		pop	es
		mov	ebx,eax			            ;restore sound node ptr
		mov	ax,es
		mov	dword ptr PlayList[edi],ebx   ;install the node on the Playlist
		mov	word ptr PlayList[edi+4],ax

;If this sound is restoring, don't reset critical values
		cmp	restoring,0
		jnz	outPlay

;not restoring, so init some stuff
		xor	cx,cx
		mov	es:[ebx].sDataInc,cx
		mov	es:[ebx].sTimer,cx
		mov	es:[ebx].sSignal,cl
		mov	es:[ebx].sFadeDest,cl
		mov	es:[ebx].sFadeTicks,cl
		mov	es:[ebx].sFadeCount,cl
		mov	es:[ebx].sFadeSteps,cl
		mov	es:[ebx].sPause,cl

		call	DoChannelList		;Update ChList

outPlay:        pop     ebx
		pop	ecx
;		dec	processSnds 		;enable the sound server
		popf
		ret
PlaySound	ENDP

;*******************************************************************
;* EndSound	- stop a sound
;*		es:ax = sound node to stop
;*******************************************************************
EndSound	PROC
		pushf
		;cli

		call	DoEnd		       	;Take node off PlayList, and
		call	DoChannelList	       	;update ChList

		popf
		ret
EndSound	ENDP


;******************************************************************
;* PauseSound - Pause or UnPause a Node
;*		 es:ax has pointer to sound node to pause/unpause
;*		 if es:az = 0000:0000 then all sounds on the list are
;*		 paused/unpaused.
;*		 cl = 0 to unpause
;*		      1 to pause
;*
;*	NOTE: A nodes sPause property is a count of how many times the node
;*	      has been paused.  This allows programmers to globally pause
;*	      all sounds, then unpause them without unpausing nodes that
;*	      were already paused when he/she paused them.
;******************************************************************
PauseSound	PROC
		pushf
		;cli
		push	eax
		push	ebx
                push    edx
		push	esi

		;If the node pointer is set to 0000:0000, then pause all
		;sounds on the PlayList
		mov     dx,ax
                mov	bx,es
;		or	dx,bx
		cmp	eax, 0
		jne	pauseOne

		; added 4/19/94 TBM
		cmp	bx, 0
		jne	pauseOne

		;For every sound node on the PlayList, increment or
		;decrement the sPause	property
		xor	esi,esi
lookPause:
		les	ebx,PlayList[esi]  	;get the node address
		mov     ax,es
;                or	ax,bx			;CGB ??? test es also?
;		jne	goodPauseNode		;if it's not NULL then skip to process it

		cmp	ax, 0
		jne	goodPauseNode
		cmp	ebx, 0
		jne	goodPauseNode

		;it's NULL so we hit the end of the list
		cmp	si,0			;are we still looking at the first entry?
		jne	outPause		;no-all done, skip out
		mov	si,6			;*** ??? yes,so keep looking ??? ***
		jmp	lookPause
goodPauseNode:
		mov	al,es:[ebx].sPause	;get the current pause property
		cmp	cl,0			;if cl = 0 skip to unpause the nodes
		je	unPause
		inc	al			;otherwise increment the pause value
		jmp	storePause		;and skip
unPause:
		cmp	al,0			;if this node is not paused already, then do nothing
		je		storePause
		dec	al			;otherwise, decrement the pause value
storePause:
		mov	es:[ebx].sPause,al	;save the new pause value
		add	si,6
		cmp	si,(LISTSIZE * 6)
		jne	lookPause		;if we're not done with the list, loop back for more
		jmp	outPause

		;Here if we only want to pause/unpause a single node				
pauseOne:
		mov	ebx,eax			;get the node's ptr
		mov	al,es:[ebx].sPause	;get the current pause count
		cmp	cl,0
		je	unPauseOne		;skip to unpause
		inc	al			;inc pause count
		jmp	storeOnePause
unPauseOne:
		cmp	al,0			;if the pause count is already 0 do nothing
		je		storeOnePause
		dec	al			;dec pause count
storeOnePause:
		mov	es:[ebx].sPause,al	;save new pause count

outPause:
	     	call	DoChannelList		;update the Channel List
		pop	esi
                pop     edx
		pop	ebx
		pop	eax
		popf
		ret
PauseSound	ENDP


;******************************************************************
;* Fade Sound	FADE A SOUND TO GIVEN LEVEL
;*		es:ax = sound node to fade
;*		dl = carg1 = destination volume
;*		cl = carg2 = FadeTicks
;*		ch = carg3 = FadeSteps
;******************************************************************
FadeSound	PROC
		pushf
		;cli
		push	ebx

		mov	ebx,eax				;If the volume of the node
		cmp	es:[ebx].sVolume,dl		;is already there, then
		je	outFade				;don't bother

		mov	es:[ebx].sFadeDest,dl		;Set fade properties in the
		mov	es:[ebx].sFadeTicks,cl		;sound node
		mov	es:[ebx].sFadeSteps,ch			
		mov	byte ptr es:[ebx].sFadeCount,0	;init the fade counter

outFade:
		pop	ebx
		popf
		ret
FadeSound	ENDP


;***********************************************************
;*	HoldSound	HOLD/RELEASE A SOUND
;*			es:ax = ptr to sound node
;*			cl = hold point or 0 if none.
;***********************************************************
HoldSound	PROC 
		push	ebx

		mov	ebx,eax			;Set sHold property of node
		mov	es:[ebx].sHold,cl

		pop	ebx
		ret
HoldSound	ENDP


;*****************************************************************
;* MuteSound	MUTE/UNMUTE AN ENTIRE NODE
;*		es:ax = ptr to sound node
;*		cl    = TRUE or FALSE
;*****************************************************************
MuteSound	PROC
		push	ebx
		push	edx
		push	esi
		
		inc	processSnds 		;temporarily disable the sound server

		;Change the mute flag of every channel in the node
		mov	ebx,eax
		mov	esi,14			;for 15 channels
loopMute:
		mov	dl,es:[ebx+esi].cMute
		cmp	cx,0			;cx=0 to unmute, otherwise mute
		jne	muteInc			;skip to mute it

		;the "Programmer set" mute count in contained in the 
		;upper nibble of the cMute property.  The "musician set" mute count
		;is in the lower nibble.  This procedure only affect the 
		;"programmer set" mute.

		cmp	dl,00fh			;if the mute count is already 0 do nothing
		jbe	storeMute
		sub	dl,10h			;otherwise dec the upper nibble
		jmp	storeMute
muteInc:
		cmp	dl,0f0h			;if the mute count = 15
		jae	storeMute		;do nothing
		add	dl,10h			;otherwise inc the upper nibble
storeMute:
		mov	es:[ebx+esi].cMute,dl	;save new mute count
		dec	esi			;next channel
		jns	loopMute

		call	DoChannelList		;update the channel list

		dec	processSnds		;re-enable (maybe) the server

		pop	esi
		pop	edx
		pop	ebx
		ret
MuteSound	ENDP


;***************************************************************
;* ChangeVol	CHANGE SOUND NODE VOLUME
;*		es:ax = ptr to sound node
;*		cx    = new volume
;***************************************************************
ChangeVol	PROC
		pushf
		;cli
		push	ebx

		mov	ebx,eax
		call	FindNode
		xor	ch,ch
		call	DoChangeVol

		pop	ebx
		popf
		ret
ChangeVol	ENDP


;*****************************************************************
;* ChangePri	CHANGE SOUND NODE PRIORITY
;*		es:ax = node ptr
;*		cl    = new priority
;*****************************************************************
ChangePri	PROC
		pushf
		;cli
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi

		mov	ebx,eax				;If it's the same as before,
		cmp	es:[ebx].sPriority,cl		;then don't bother
		jne	doPri
		jmp	outChPri

doPri:
		mov	es:[ebx].sPriority,cl		;Store new priority

		call	FindNode			;If it's not on the PlayList
		cmp	si,255				;then exit the procedure
		jne	movePriNodes
		jmp	outChPri

movePriNodes:
		mov	dword ptr PlayList[esi],0	;Remove the node from the
		mov	word ptr PlayList[esi+4],0	;PlayList
		cmp	si,((LISTSIZE - 1) * 6)
		je	CPaddNode			;skip if this is the last slot in the playList

CPmoveNodes:
		mov	ecx,dword ptr PlayList[esi+6]	;copy the node following this slot
		mov	dword ptr PlayList[esi],ecx	;to this slot
		mov	cx,word ptr PlayList[esi+10]
		mov	word ptr PlayList[esi+4],cx
		add	si,6				;and repeat for all remaining nodes
		cmp	si,((LISTSIZE - 1) * 6)
		jne	CPmoveNodes
		mov	word ptr PlayList[esi],0	;zero out the last node
		mov	word ptr PlayList[esi+2],0
		mov	word ptr PlayList[esi+4],0

CPaddNode:						;Put node back on in order of priority
		mov	dl,es:[ebx].sPriority		;get the node's Priority	
		push	es
		xor	edi,edi								
CPfindPri:
		cmp	word ptr PlayList[edi+4],0	;if we hit the end of the list

;terry 3/27/94
		jne	CPnotEnd
		cmp	dword ptr PlayList[edi],0

		je	CPplaceHere			;skip to install node

CPnotEnd:
		les	ebx,PlayList[edi]		;else, get ptr to the node
		mov	es:[ebx].sPriority,dl		;compare priorities
		jbe	CPinsertHere			;if our's is <= theirs then skip to insert node
		add	di,6				;try next slot
		jmp	CPfindPri

		;move all nodes down to make room for this one
CPinsertHere:
		mov	esi,((LISTSIZE - 2) * 6)	;point to next to last Playlist slot
insertMore:
		mov	ebx,esi
		add	ebx,6
		cmp	ebx,edi				;when the destination slot = the slot
		je	CPplaceHere			;we want, skip to install the node

		mov	ebx,dword ptr PlayList[esi]	;copy node address from this slot			
		mov	dword ptr PlayList[esi+6],ebx	;to the next one...			
		mov	bx,word ptr PlayList[esi+4]
		mov	word ptr PlayList[esi+10],bx
		sub	si,6
		jmp	insertMore

CPplaceHere:
		pop	es
		mov	dword ptr PlayList[edi],eax	;install ourselves on the Playlist
		mov	cx,es
		mov	word ptr PlayList[edi+4],cx

		call	DoChannelList			;Update ChList

outChPri:
		pop	edi
		pop	esi
		pop	edx
		pop	ecx
		pop	ebx
		popf
		ret
ChangePri	ENDP


;*****************************************************************
;* GetSignal	GET SIGNAL VALUE OF A NODE
;*		es:ax = node address
;*		returns value in cl
;*****************************************************************
GetSignal	PROC
		push	ebx

		mov	ebx,eax			;Get signal and put it in cl						;Get signal and put it in cl
		mov	cl,es:[ebx].sSignal

		pop	ebx
		ret
GetSignal	ENDP


;*****************************************************************
;* GetSignalRset   GET SIGNAL VALUE OF A NODE and resets the signal
;*		   es:ax = node address
;*		   returns value in cl
;*****************************************************************
GetSignalRset	PROC
		push	ebx

		mov	ebx,eax		    ;Get signal and put it in cl						;Get signal and put it in cl
		xor	cl,cl		    ;and reset signal						;and reset signal
		xchg	cl,es:[ebx].sSignal

		pop	ebx
		ret
GetSignalRset	ENDP



;******************************************************************
;* GetDataInc	Returns DATAINC property of a node
;*					es:ax = node address
;*					Returns data inc value in cx
;******************************************************************
GetDataInc	PROC
		push	ebx

		mov	ebx,eax			   ;Get DataInc value and put it
		mov	cx,es:[ebx].sDataInc	   ;in cx

		pop	ebx
		ret
GetDataInc	ENDP


;*****************************************************************
;* GetSMPTE - return the smpte value of a node
;*		  es:ax = node pointer
;*	Returns:
;*		  dx - minutes
;*		  ch - seconds	
;*		  cl - frames
;*****************************************************************
GetSMPTE	PROC
		push	eax
		push	ebx

		mov	ebx,eax			;Put the sTimer value of the
		mov	ax,es:[ebx].sTimer	;node into ax

		;Convert to minutes
		xor	dx,dx 
		mov	cx,3600
		div	cx
		push	ax			;save minutes

		;Convert to seconds and frames
		mov	ax,dx			;get remainder in ax
		mov	cl,60
		div	cl			;get seconds in al

		mov	cl,ah			;get remainder in cl (ticks)
		shr	cl,1			;/2 = 30 frames per sec in cl

		mov	ch,al			;save seconds in ch
		pop	dx			;restore minutes into dx

		pop	ebx
		pop	eax
		ret
GetSMPTE	ENDP


;---------------------------------------------------------
;SendNoteOff - Send a NoteOff to a given channel in
;              a given node
;
; in  - es:ax = far node ptr
;       ch    = note #
;       cl    = velocity
;       dl    = channel
;
; out - nothing
;---------------------------------------------------------
SendNoteOff	PROC	
		push	eax
		push	ebx
		push	edx
		push	esi


		;Disable the SoundServer

		inc	processSnds


		;Put the position of the node on the PlayList
		;into si.  If 255 is returned, then the node
		;is not on the PlayList at all

		mov	ebx,eax
		call	FindNode
                cmp     si,255				
                je      outSendNoff

			push	esi			 ;save playlist number
                        xor     esi,esi

			;Reset the CurNote for this channel
			;(PC speaker uses this to restore a playing note)

			xor	dh,dh
			mov	si,dx			 ;get channel in si
			mov	es:[ebx+esi].cCurNote,-1

			
ifdef RECMID
			push	es
			push	eax
			push	ecx
			
			mov	eax,es:[ebx].sMidiPtr32	 ;check for FFFF:FFFF receiver node
			or	eax,eax
                        cmp     eax,0FFFFFFFFh
                        jne     SNOFF_ELSE
				mov	si,dx
				pop	ecx
				pop	eax
				pop	es
				CallDriver DNoteOff	 ;if receiver, just pass event thru
			 	jmp	SHORT outSendNoff
SNOFF_ELSE:
			pop	ecx
			pop	eax
			pop	es
endif


			;Combine the PlayList position and the channel
			;number in dl (this byte will represent the
			;node/channel we are sending to)

			mov	ax,si		;channel in ax
			pop	esi
			mov	dx,si		;get Playlist index into high nibble
			shl	dl,1
			shl	dl,1
			or	dl,al		;and combine w/channel in dl


			;Search for an entry for this node/channel on
			;the ChList.  The position of this entry on
			;the list will tell us the physical channel that
			;this channel was mapped to.  If no such entry
			;exists, then this channel is not currently
			;being played (possibly preempted)

			xor	esi,esi

NOFFWHILE:		cmp     dl,ChList[esi]
                        je      NOFFEND
                        inc     si
                        cmp     si,16
                        je      outSendNoff	;didn't find it, skip out
                        jmp     NOFFWHILE

NOFFEND:
			;Set up the input registers and call the Sound
			;Driver's NoteOff function

			mov	ax,si		;ax=channel,ch=note#,cl=velocity
			CallDriver DNoteOff


outSendNoff:

		;Enable the SoundServer

		dec	processSnds


		pop	edx
		pop	ebx
		pop	eax
		pop	esi
		ret
SendNoteOff	ENDP



;---------------------------------------------------------
;SendNoteOn - Send a NoteOn to a given channel in
;             a given node
;
; in  - es:ax = far node ptr
;       ch    = note #
;       cl    = velocity
;       dl    = channel
;
; out - nothing
;---------------------------------------------------------
SendNoteOn	PROC	
		push	eax
		push	ebx
		push	edx
		push	esi


		;Disable the SoundServer

		inc	processSnds


		;Put the position of the node on the PlayList
		;into si.  If 255 is returned, then the node
		;is not on the PlayList at all

		mov	ebx,eax
		call	FindNode

                cmp     si,255
                je      outSendNon
			push	esi	    ;save Playlist position


			;Fill the node's CurNote field with
			;the note number specified

			xor	dh,dh
			mov	si,dx
			mov	es:[ebx+esi].cCurNote,ch

			
ifdef RECMID
			;if receiver then just send the event to the driver
			push	es
			push	eax
			push	ecx
			 
			mov	eax,es:[ebx].sMidiPtr32
			or		eax,eax
			cmp     eax,0FFFFFFFFh
                        jne     SNON_ELSE
                         	mov	si,dx
				pop	ecx
				pop	eax
				pop	es
				CallDriver DNoteOn
			 	jmp	SHORT outSendNon
SNON_ELSE:
			pop	ecx
			pop	eax
			pop	es
endif


			;Combine the PlayList position and the channel
			;number in dl (this byte will represent the
			;node/channel we are sending to)

			mov	ax,si
			pop	esi
			mov	dx,si
			shl	dl,1
			shl	dl,1
			or	dl,al


			;Search for an entry for this node/channel on
			;the ChList.  The position of this entry on
			;the list will tell us the physical channel that
			;this channel was mapped to.  If no such entry
			;exists, then this channel is not currently
			;being played (possibly preempted)

			xor	esi,esi
                        
NONWHILE:	      	cmp     dl,ChList[esi]
                        je      NONEND
                        inc     si
                        cmp     si,16
                        je      outSendNon
                        jmp     NONWHILE

NONEND:

			;Set up the input registers and call the Sound
			;Driver's NoteOff function

			mov	ax,si
			CallDriver DNoteOn	   ;ax = channel


outSendNon:

		;Enable the SoundServer

		dec	processSnds


		pop	edx
		pop	ebx
		pop	eax
		pop	esi
		ret
SendNoteOn	ENDP


;****************************************************************
;* SendContrlr	SEND CONTROLLER TO A NODE
;*		es:ax = node address
;*		dl = Channel
;*		ch = Controller #
;*		cl = Controller value
;*
;*  This routine also has code in it to handle sending a program
;*  change to a channel if ch=127.  I don't know if it is used though
;*  since there is a whole special routine below to handle that...
;****************************************************************
SendContrlr	PROC
		pushf
		;cli
		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi

		mov	ebx,eax			;If the node isn't on the
		call	FindNode		;PlayList, exit the procedure
		cmp	si,255
		jne	gotContrlrNode
		jmp	outSCtrlr

gotContrlrNode:
		push	esi			;save Playlist position
		xor	dh,dh
		mov	si,dx			;get channel in si

		;If it's one of the controllers we save, then					
		;save it before sending it
		cmp	ch,VOLCTRL
		jne	notSCvol
		mov	es:[ebx+esi].cVolume,cl	  ;save the channel volume
		mov	dl,es:[ebx].sVolume	  ;get the node volume
		call	ScaleVolume		  ;scale the channel volume
		jmp	sendContrlr               ;and send it
notSCvol:
		cmp	ch,PANCTRL
		jne	notSCPan
		mov	es:[ebx+esi].cPan,cl
		jmp	sendContrlr               ;SHORT
notSCPan:
		cmp	ch,MODCTRL
		jne	notSCMod
		mov	es:[ebx+esi].cModulation,cl
		jmp	sendContrlr               ;SHORT
notSCMod:
		cmp	ch,DAMPRCTRL
		jne	notSDamp
		shl	si,1
		mov	ax,es:[ebx+esi].cDamprPbend
		and	ah,7fh			  	;clear old damper bit
		cmp	cl,0
		je	SCdampUp
		or	ah,80h		  		;set damper bit
SCdampUp:
		mov	es:[ebx+esi].cDamprPbend,ax   	;save new damper/PB value
		shr	si,1
		jmp	sendContrlr               	;SHORT
notSDamp:
		cmp	ch,MUTECTRL
		jne	notSCMute
		push	edx
		mov	dl,es:[ebx+esi].cMute	;get the current mute count
		cmp	cl,0			;unmute?
		jne	incMuteFlag		;no-skip
		cmp	dl,00fh			;if it's already 0 then do nothing
		jbe	outMuteFlag
		sub	dl,010h			;otherwise dec the upper nibble
		jmp	storeMuteFlag           ;SHORT
incMuteFlag:
		cmp	dl,0f0h			;if mute is already 15 do nothing
		jae	outMuteFlag
		add	dl,010h			;inc the upper nibble
storeMuteFlag:
		mov	es:[ebx+esi].cMute,dl	;save the new mute flag
		call	DoChannelList		;update the channel list
outMuteFlag:
		pop	edx
		pop	esi
		jmp	outSCtrlr                 ;SHORT
notSCMute:
		cmp	ch,127			  ;controller 127 is really pgm change
		jne	sendContrlr
		mov	es:[ebx+esi].cProgram,cl  ;save cl as new program change


sendContrlr:	
                 ifdef	RECMID
		 push	es
		 push	eax
		 push	ecx
		 mov	eax,es:[ebx].sMidiPtr32
		 cmp	eax,0ffffffffh
		 jne	notRecContrlr
		 pop	ecx		  	;if it's a receiver node
		 pop	eax
		 pop	es
		 pop	esi			;restore the Playlist position
		 mov	si,dx			;????	get channel in si? BUG
						;DX was possibly destroyed by the
						;Volume controller scaling above.
						;It will no longer hold the channel
						
		 and	esi,0ffh		;???
		 jmp	SConList           	;SHORT
notRecContrlr:	 pop	ecx
		 pop	eax
		 pop	es
		endif


		mov	ax,si			;channel in ax
		pop	esi			;restore Playlist position
		mov	dx,si
		shl	dl,1
		shl	dl,1			;get Playlist position in dl upper nibble
		or	dl,al			;merge with channel

		xor	esi,esi			
SClookList:					;Look for the node/channel
		cmp	ChList[esi],dl		;on the ChList.  If it's not
		je	SConList		;there, then exit
		inc	si
		cmp	si,16
		jne	SClookList
		jmp	outSCtrlr          	;SHORT, didn't find it, skip out

SConList:
		mov	ax,si			;get channel in ax
		cmp	ch,127			;is it really a program change?
		jne	itsAContrlr
		CallDriver DProgramChange	;yes, send pgm change
		jmp	outSCtrlr          	;SHORT
itsAContrlr:
		CallDriver DController		;no-send controller

outSCtrlr:
		pop	esi
		pop	edx
		pop	ecx
		pop	ebx
		pop	eax
		popf
		ret
SendContrlr	ENDP


;---------------------------------------------------------
;SendPChange - Send a ProgramChange to a given channel in
;              a given node
;
; in  - es:ax = far node ptr
;       cl    = program #
;       dl    = channel
;
; out - nothing
;---------------------------------------------------------
SendPChange	PROC	
		push	eax
		push	ebx
		push	edx
		push	esi


		;Disable the SoundServer

		inc	processSnds


		;Put the position of the node on the PlayList
		;into si.  If 255 is returned, then the node
		;is not on the PlayList at all

		mov	ebx,eax
		call	FindNode
                cmp     si,255
                je      outSendPch

			push	esi			;save the Playlist position


			;Store the new program in the node

			xor	dh,dh
			mov	si,dx			;get channel in si
			mov	es:[ebx+esi].cProgram,cl

			
ifdef RECMID
			push	es
			push	eax
			push	ecx
			
			mov	eax,es:[ebx].sMidiPtr32
			or		eax,eax
			cmp     eax,0FFFFFFFFh
                        jne     SPC_ELSE
			 	mov	si,dx
				pop	ecx
				pop	eax
				pop	es		  ;if it's a receiver node then
				CallDriver DProgramChange ;just send the the event thru
			 	jmp	SHORT outSendPch
SPC_ELSE:
			pop	ecx
			pop	eax
			pop	es
endif


			;Combine the PlayList position and the channel
			;number in dl (this byte will represent the
			;node/channel we are sending to)

			mov	ax,si
			pop	esi		;restore the Playlist position
			mov	dx,si
			shl	dl,1		;shift it into the high nibble
			shl	dl,1
			or	dl,al		;and mix in the channel


			;Search for an entry for this node/channel on
			;the ChList.  The position of this entry on
			;the list will tell us the physical channel that
			;this channel was mapped to.  If no such entry
			;exists, then this channel is not currently
			;being played (possibly preempted)

			xor	esi,esi
PCHWHILE:               cmp     dl,ChList[esi]
                        je      PCHEND
                        inc     si
                        cmp     si,16
                        je      outSendPch
                        jmp     PCHWHILE
PCHEND:

			;Set up the input registers and call the Sound
			;Driver's ProgramChange function

			mov	ax,si			;get channel in ax
			CallDriver DProgramChange


outSendPch:
		;Enable the SoundServer

		dec	processSnds


		pop	edx
		pop	ebx
		pop	eax
		pop	esi
		ret
SendPChange	ENDP

;****************************************************************
;* SendPBend - sends a pitchbender to a channel on a node
; in  - es:ax = far node ptr
;	dl	= Channel
;	cx  = Bender value
; out - nothing
;****************************************************************
SendPBend	PROC
		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi

		mov	ebx,eax			;If the node isn't on the
		call	FindNode		;PlayList, then exit
		cmp	si,255
		je	outSPB

                xor     esi,esi
		push	esi			;save Playlist position
		xor	dh,dh
		mov	si,dx			;get channel in si
		shl	si,1			;channel * 2
		mov	ax,cx
		cmp	byte ptr es:[ebx+esi + 1].cDamprPbend,128 ;see if the damper bit is set
		jb	noSPdampr
		or	ah,80h				;yes, so add it into the bender value
noSPdampr:
		mov	es:[ebx+esi].cDamprPbend,ax  	;save new bender
		shr	si,1

		;get Playlist position/channel in dl
		mov	dx,si		      ;put channel in dl
		pop	esi		      ;restore Playlist position
		shl	si,1		      ;shift it into the upper nibble
		shl	si,1
		or	dx,si	      	      ;and mix it with the channel

		;If the node/channel isn't on the ChList, then exit
		xor	esi,esi
SPlookList:
		cmp	ChList[esi],dl
		je	SPonList
		inc	si
		cmp	si,16
		jne	SPlookList
		jmp	SHORT outSPB	      ;didn't find it-skip out

SPonList:
		;get 7 bits in ch and 7 in cl
		shl	ch,1
		cmp	cl,128		      ;if cl has hi bit on, move it to
		jb	noSPhiBit
		or	ch,1		      ;ch's low bit
noSPhiBit:
		and	cl,7fh		      ;mask out cl's hi bit
		xchg	ch,cl		      ;get the MSB in cl, LSB in ch
		mov	ax,si		      ;channel in ax
		CallDriver DPitchBend

outSPB:
		pop	esi
		pop	edx
		pop	ecx
		pop	ebx
		pop	eax
		ret
SendPBend	ENDP


;---------------------------------------------------------------------------
;			   Internal Functions

;**************************************************
;* DoEnd 	END A SOUND
;*	 	es:ax = ptr to node to shut down
;**************************************************
DoEnd		PROC	NEAR
		push	esi
		push	es
		push	eax
		push	ebx
		push	ds
		push	ebp

ifdef RECMID				       ;If it was a test node,
		push	es		       ;make sure any notes still
		push	eax		       ;playing are turned off
		mov	ebx,eax
		mov	eax,es:[ebx].sMidiPtr32

		cmp	eax,0ffffffffh
		jne	notEndRecv

		xor	esi,esi		       ;for 15 channels, send All notes off
loopEndRecv:
		mov	eax,esi
		mov	ch,ALLNOFF
		mov	cl,127
		CallDriver DController
		inc	si
		cmp	si,16
		jne	loopEndRecv
notEndRecv:
		pop	eax
		pop	es
endif


		;Look for node es:ax on	the PlayList.  If it is not
		;found, then exit procedure

		xor	esi,esi
		mov	cx,es
findESound:
		cmp	eax,dword ptr PlayList[esi]
		jne	notEThisNode
		cmp	cx,word ptr PlayList[esi+4]
		je	clearNode
notEThisNode:
		add	si,6
		cmp	si,(LISTSIZE * 6)
		jne	findESound
		jmp	outDoEnd	 		;didn't find it, skip out

clearNode:
		;found the node on the Playlist

		mov	dword ptr PlayList[esi],0
		mov	word ptr PlayList[esi+4],0    ;If the node was in the
		cmp	si,((LISTSIZE -1) * 6)	      ;last position, then there
		je	putSignal	      	      ;is no need to move other nodes

moveNodes:
		mov	ecx,dword ptr PlayList[esi+6] ;Move everybody up one
		mov	dword ptr PlayList[esi],ecx
		mov	cx,word ptr PlayList[esi+10]
		mov	word ptr PlayList[esi+4],cx
		add	si,6
		cmp	si,((LISTSIZE -1) * 6)
		jne	moveNodes
		mov	dword ptr PlayList[esi],0     ;zero out the last slot
		mov	word ptr PlayList[esi+4],0

putSignal:
		;set the sound done signal, and reset the state
		mov	ebx,eax
		mov	byte ptr es:[ebx].sSignal,255
		mov	byte ptr es:[ebx].sState,0

outDoEnd:

		pop	ebp
		pop	ds
		pop	ebx
		pop	eax
		pop	es
		pop	esi
		ret
DoEnd		ENDP


;***********************************************************
;* DoChannelList - updates the channel list
;*
;*		   This procedure, being the
;*		   most complex and the most
;*		   used, is written for speed,
;*		   not for compact size.
;***********************************************************
DoChannelList	PROC	NEAR
		push	eax
		push	ebx
		push	ecx 
		push	edx
		push	edi
		push	esi
		push	ebp
		push	es
	
		;disable the sound server
		inc	processSnds

		;reset the flag that got us here
		mov	updateChnls,0

		xor	ax,ax			   ;Clear channel tables
		mov	bx,-1
		mov	word ptr ChList,bx
		mov	word ptr ChList[2],bx
		mov	word ptr ChList[4],bx
		mov	word ptr ChList[6],bx
		mov	word ptr ChList[8],bx
		mov	word ptr ChList[10],bx
		mov	word ptr ChList[12],bx
		mov	word ptr ChList[14],bx
		mov	word ptr ChVoice,ax
		mov	word ptr ChVoice[2],ax
		mov	word ptr ChVoice[4],ax
		mov	word ptr ChVoice[6],ax
		mov	word ptr ChVoice[8],ax
		mov	word ptr ChVoice[10],ax
		mov	word ptr ChVoice[12],ax
		mov	word ptr ChVoice[14],ax
		mov	word ptr ChBed,ax
		mov	word ptr ChBed[2],ax
		mov	word ptr ChBed[4],ax
		mov	word ptr ChBed[6],ax
		mov	word ptr ChBed[8],ax
		mov	word ptr ChBed[10],ax
		mov	word ptr ChBed[12],ax
		mov	word ptr ChBed[14],ax
		mov	word ptr ChPri,ax
		mov	word ptr ChPri[2],ax
		mov	word ptr ChPri[4],ax
		mov	word ptr ChPri[6],ax
		mov	word ptr ChPri[8],ax
		mov	word ptr ChPri[10],ax
		mov	word ptr ChPri[12],ax
		mov	word ptr ChPri[14],ax
		mov	word ptr ChNew,bx
		mov	word ptr ChNew[2],bx
		mov	word ptr ChNew[4],bx
		mov	word ptr ChNew[6],bx
		mov	word ptr ChNew[8],bx
		mov	word ptr ChNew[10],bx
		mov	word ptr ChNew[12],bx
		mov	word ptr ChNew[14],bx

		mov	word ptr SampleList,ax		;** Sample Kludge!
		mov	word ptr SampleList[2],ax	;(until Simu-Drivers)
		mov	word ptr SampleList[4],ax	;(until Simu-Drivers)

		;get the address of first node on playlist
		les	ebx,PlayList
		mov	dx,es
		or	dx,bx
		jne	notEmptyPlList

		;if there isn't a 1st node, just init the channel list to empty
		mov	dx,0ffffh
		mov	word ptr ChList,dx
		mov	word ptr ChList[2],dx		
		mov	word ptr ChList[4],dx
		mov	word ptr ChList[6],dx
		mov	word ptr ChList[8],dx
		mov	word ptr ChList[10],dx
		mov	word ptr ChList[12],dx
		mov	word ptr ChList[14],dx
		jmp	cleanUpChnls

notEmptyPlList:
		;The are entries on the playlist so...

		;Set the reverb mode to the value in
		;the first node's sReverbMode value.
		;127 means default reverb mode which
		;can be set by the apps programmer.

		mov	cl,es:[ebx].sReverbMode
		cmp	cl,127
		jne	notRevDefault
		mov	cl,revDefault
notRevDefault:
		CallDriver DSetReverb


	;*** DoChannelList PASS 1 ***

	;Base priority.  Each channel within a node may have a priority
	;from 0-15.  Since the nodes are already prioritized on the 
	;play list we can obtain a "system wide" channel priority
	;with (playlist position * 16) + channel priority.  Here bp
	;is incremented by 16 for each node we process. 


		xor	ebp,ebp
		xor	esi,esi			;PlayList index
		mov	al,numVoices		;al has number of voices available
loopDoNodes:
		;Get next sound node off PlayList.  If its the last
		;one, jump out to Pass2

		les	ebx,PlayList[esi]	;node pointer in bx	
		mov	dx,es
		or	dx,bx
		jnz	notLastNode
		jmp	doPass2			;found the end of the Playlist-jump out

notLastNode:
		;If the sound is paused,don't allocate any channels for it
		les	ebx,PlayList[esi]		
		cmp	byte ptr es:[ebx].sPause,0
		je	nodeNotPaused
		jmp	nextNode

nodeNotPaused:
		;*** Sample Kludge! (until Simu-Drivers is implemented)

		;If the node is a sample, then don't put deal with it on the
		;channel list.

;		cmp	byte ptr es:[ebx].sSample,0
;		je	nodeNotSample
		jmp	nodeNotSample
		cmp	dword ptr SampleList,0
		je	okSamp1
		jmp	nextNode

okSamp1:
		cmp	word ptr SampleList[4],0
		je	okSamp2
		jmp	nextNode
okSamp2:
		mov	dword ptr SampleList,ebx
		mov	bx,es
		mov	word ptr SampleList[4],bx
		jmp	nextNode

nodeNotSample:
		BackupList				;Backup ChannelList

		;Get the channel of the next track from node (Tracks
		;should already be prioritized in the sound node)

		xor	edi,edi				;Track index
loopDoTracks:
		mov	cl,es:[ebx+edi].tChannel	;get channel in cl
		cmp	cl,0ffh				;The end of track marker is 0FFH
		jne	notLastTrack
		jmp	nextChTrack

notLastTrack:
		;If the channel value is 0feh, then it's a sample so skip it

		cmp	cl,0feh
		jne	notSample
		jmp	nextChTrack

notSample:
		;If it's channel 16, then it doesn't belong on the ChList.

		cmp	cl,15
		jne	notCh16
		jmp	nextChTrack

notCh16:
		;If bit 1 of the cFlags array is set, then it's a ghost channel
		;and doesn't need to be on the channel list

		push	edi
		mov	di,cx
		and	edi,0ffh
		test	es:[ebx+edi].cFlags,2
		jz	notGhostChnl
		pop	edi
		jmp	nextChTrack

notGhostChnl:
		;If the cMuted property is TRUE, then the channel is muted
		;and doesn't need to be on the channel list

		test	es:[ebx+edi].cMute,0ffh
		pop	edi
		jz	notMutedChnl
		jmp	nextChTrack

notMutedChnl:
		mov	dx,si			  ;get Playlist position into
	   
                push    ax
                mov     ax,dx
                mov     dl,6
                div     dl
                mov     dx,ax
                shl     dl,1
                shl     dl,1
                pop     ax

        	shl	dl,1			  ;dl high nibble
		shl	dl,1
		or	dl,cl		 	  ;get channel into dl low nibble

		push	edi
		mov	di,cx
		and	edi,0ffh		  ;channel in di
		mov	ah,es:[ebx+edi].cPriVoice
		and	ah,00fh			  ;get the channel's numNotes in ah
		mov	ch,es:[ebx+edi].cPriVoice
		pop	edi
		shr	ch,1			  ;get channel's priority in ch
		shr	ch,1
		shr	ch,1
		shr	ch,1
		jz	nonPreemptable		  ;if priority is zero (non-preemptable)
		push	dx			  ;leave it that way, otherwise invert
		mov	dx,16			  ;the priority so low priority is a
		sub	dl,ch			  ;low number, and high priority is a
		add	dx,bp			  ;high numberand add the base (node)
		mov	ch,dl			  ;priority to it.
		pop	dx
nonPreemptable:
		;If bit 0 of the flags is set, then it's a locked channel and must 
		;occupy the channel slot it was sequenced on.

		push	edi
		mov	di,cx
		and	edi,0ffh		  ;channel in di
		test	byte ptr es:[ebx+edi].cFlags,1
		jz	lookOpenChnl		  ;not locked, skip ahead

		;it's a locked channel, check if somebody else already owns it
		cmp	ChNew[edi],0ffh
		jne	lookOpenChnl		  ;somebody else owns it, search for open channel anyway
		pop	edi
		mov	dh,cl			  ;nobody owns it, put channel in dh
		jmp	checkVoices		  ;and go grab it for ourselves

lookOpenChnl:
		;Look for an open channel. While looking, make sure
		;that this node/channel is not already on the list

		pop	edi
		mov	dh,0ffh			;init dh to "didn't find free channel"
		push	ebx
		xor	ebx,ebx			;start looking at channel 0
loopLookChnl:				     
		cmp	ChNew[ebx],0ffh
		je	openChnl		;found a free channel-skip

		;if this channel is occupied by ourselves, then just skip the track.
		cmp	ChNew[ebx],dl		;dl has Playlist position/channel
		jne	nextLookChnl
		pop	ebx
		jmp	nextChTrack
openChnl:
		;check if the free channel we found falls between the hi and lo
		;channels the installed driver can handle, the driver returned
		;these numbers to us when it was initted.


		cmp	bl,loChnl
		jb	nextLookChnl
		cmp	bl,hiChnl
		ja	nextLookChnl
		mov	dh,bl			;OK, move found channel into dh

nextLookChnl:
		;Here we look for the next free channel regardless of whether
		;we found one already or not.  This means we will take the last
		;free one we find.  DH holds the last free channel we found

		inc	bl
		cmp	bl,16			;done with 16 channels?
		jne	loopLookChnl		;no-go look again

		pop	ebx
		cmp	dh,0ffh			;did we find a free channel?
		jne	checkVoices		;yes-skip to grab it

		;There were no channels	open.  If we're a preemptable channel, then
		;we can blow it off and	proceed to the next node.
		;If we're non-preemptable, then we have to preempt someone or back out

		cmp	ch,0
		je	gotToGetChnl		;is it nonpreemptable? (0 priority)
		jmp	nextNode		;no-so blow off this node, and check the next
gotToGetChnl:					   
		;We have a non-preemptable channel and we can't find a slot on the 
		;channel list for the channel we are working on.

		PreemptChn1			;try to preempt a channel
		cmp	dh,0ffh
		jne	checkVoices
		jmp	blewIt			;failed-jump to restore the old channel list
checkVoices:
		;Got a channel	;Check for enough voices. If enough,
		;jump ahead to put channel on the list	There wern't enough voices.
		;If we're preemptable, blow it off and go to the next track

		cmp	ah,al			;is channel's numnotes <= voices available?
		jbe	putChOnList		;yes-skip to grab the channel for ourselves
		cmp	ch,0			;no-if this a preemptable channel
		je	loopPreEmpt
		jmp	nextChTrack		;then blow it off and try the next channel

loopPreEmpt:
		;We have a non-preemptable channel and room for it on the channel
		;list, but not enough free voices to satisfy the channel's numNotes.

		;Keep preempting channels until there are enough 
		;voices.  If we run out of preemptable channels,
		;then we have to back out.

		PreemptChn2			 ;Try to preempt a channel
		cmp	dh,0ffh
		jne	gotSomeVoices
		jmp	blewIt			 ;failed, go restore the old channel list
gotSomeVoices:
		cmp	ah,al			 ;is channel's numnotes <= voices available?
		ja	loopPreEmpt	 	 ;no-go preempt some more

putChOnList:
 		;Ok, we've got room on the channel list and enough available voices
		;to handle this channel.  Install the channel on the channel list

		push	edi

		;get channel we are taking in di
		xchg	dl,dh
		mov	di,dx
		xchg	dl,dh
		and	edi,0ffh

		mov	ChNew[edi],dl		 ;mark channel as used by playlist position-seq channel
		mov	ChVoice[edi],ah		 ;save number of voices allocated to us
		sub	al,ah			 ;and sub that from voices available
		mov	ChPri[edi],ch		 ;save this channel's priority

		push	edi
		mov	di,cx			 ;get sequenced channel in di
		and	edi,0ffh
		test	byte ptr es:[ebx+edi].cFlags,1 ;is this a locked channel?
		pop	edi
		jne	checkRightChnl
		mov	ChBed[edi],0		 ;no-zero out bed ("locked") property
		pop	edi
		jmp	nextChTrack

checkRightChnl:
		;The channel we are installing is a "locked" channel so make
		;sure that we are on the channel we were sequenced on.

		mov	ChBed[edi],1		 ;mark us as a "bed" channel
		cmp	dh,cl			 ;are we on the right channel?
		jne	notRightChnl
		pop	edi
		jmp	nextChTrack		 ;yes-done w/this track, go again

notRightChnl:
		;We have room for the "locked" channel we are trying to install,
		;but it's not yet on the correct channel because someone else
		;currently owns the channel we need.

		;If the channel in our desired position is not a
		;bed channel, just swap with him.

		push	esi
		mov	si,cx
		and	esi,0ffh		 ;get sequenced channel in si
		cmp	ChBed[esi],0		 ;is that channel already locked?
		jne	whichBedWins
		SwapChnls			 ;no, swap channels
		pop	esi
		pop	edi
		jmp	SHORT nextChTrack

whichBedWins:
		;OK, the channel we need to install our "locked" channel is
		;already occupied by someone else's "locked", we need to
		;decide who has a higher priority.

		;If we're preemptable, we can take ourselves off the
		;list and go on to the next track

		cmp	ch,0			 ;are we preemptable?
		je	checkOtherBed		 ;no-go check the other guy
		mov	ChNew[edi],0ffh		 ;yes-release the free channel we found
		mov	ChPri[edi],0		 ;and reset that channel's properties
		mov	ChVoice[edi],0
		mov	ChBed[edi],0
		add	al,ah			 ;add the voices back into voices available
		pop	esi
		pop	edi
		jmp	SHORT nextChTrack	 ;and go try the next track

checkOtherBed:
		;If the other "locked" channel is not preemptable, then we
		;have to back out

		cmp	ChPri[esi],0		 ;is the other guy preemptable?
		jne	preemptBed
		pop	esi
		pop	edi		      
		jmp	SHORT blewIt		 ;no, go restore the old channel list

preemptBed:
		;The other bed channel was preemptable, so we can steal his channel

		add	al,ChVoice[esi]		 ;add his voices back into voices available
		mov	ChNew[edi],0ffh		 ;release his channel
		mov	ChVoice[edi],0		 ;and reset his properties
		mov	ChPri[edi],0
		mov	ChBed[edi],0

		mov	ChNew[esi],dl		 ;install our playlist position/channel as owner
		mov	ChPri[esi],ch		 ;and our priority
		mov	ChVoice[esi],ah		 ;and # of voices
		sub	al,ah			 ;sub our numNotes from voices available
		pop	esi
		pop	edi

nextChTrack:
		;Do the next track

		inc	di
		cmp	di,16
		jne	gotMoreTracks
		jmp	nextNode		 ;if done w/tracks go on to the next node
gotMoreTracks:
		jmp	loopDoTracks

blewIt:
		;Here if we couldn't find an open channel, couldn't find enough
		;voices to handle the channel, or couldn't negociate peace
		;between two "locked" channels.

		RestoreList			;Restore the lists
nextNode:					
		add	bp,16	     		;Increment the base priority
		add	si,6			;and do the next node
		cmp	si,(LISTSIZE*6)
		je	doPass2
		jmp	loopDoNodes



;**********************************************************
;*** DoChannelList PASS 2 ***
;**********************************************************
;* Copy "locked" channels and node/channels that already
;*	exist in the channel List from the chNew list (that Pass1
;* just made), to the proper positions in the channel List.
;*
;* This is primarily an attempt to keep existing channels
;* from jumping around on the channel list.
;**********************************************************

doPass2:
ifdef RECMID

;If the RECMID option is on
;(for linking with the tools), put the current
;ChPri list into ChPriCopy,	and clear the actual list
;so when we move channels, we can move their matching
;priorities to the same	position.

		mov	ax,word ptr ChPri
		mov	word ptr ChPriCopy,ax
		mov	ax,word ptr ChPri[2]
		mov	word ptr ChPriCopy[2],ax
		mov	ax,word ptr ChPri[4]
		mov	word ptr ChPriCopy[4],ax
		mov	ax,word ptr ChPri[6]
		mov	word ptr ChPriCopy[6],ax
		mov	ax,word ptr ChPri[8]
		mov	word ptr ChPriCopy[8],ax
		mov	ax,word ptr ChPri[10]
		mov	word ptr ChPriCopy[10],ax
		mov	ax,word ptr ChPri[12]
		mov	word ptr ChPriCopy[12],ax
		mov	ax,word ptr ChPri[14]
		mov	word ptr ChPriCopy[14],ax
		mov	word ptr ChPri,0
		mov	word ptr ChPri[2],0
		mov	word ptr ChPri[4],0
		mov	word ptr ChPri[6],0
		mov	word ptr ChPri[8],0
		mov	word ptr ChPri[10],0
		mov	word ptr ChPri[12],0
		mov	word ptr ChPri[14],0
endif

		xor	esi,esi			;start with channel 0
loopPass2:
		;if this channel is not being used, then skip to the next channel

		cmp	ChNew[esi],0ffh
		jne	notEmptyChNew
		jmp	nextPass2

notEmptyChNew:
		;Is this channel a "locked" channel?  If so, copy
		;it to ChList at the same position.

		cmp	ChBed[esi],0		 ;"locked" channel?
		jne	copyBedCh
		jmp	noCopyBedCh		 ;no-skip

copyBedCh:
		;This is a "locked" channel.  Put it on the channel list in the same
		;position as it is on the ChNew list

		xor	eax,eax
		mov	al,ChNew[esi]		 ;copy the channel's Playlist position/channel
		mov	ChNew[esi],0ffh
		mov	ChList[esi],al		 ;to the real channel list, wipe out the ChNew entry

;ifdef RECMID
		;IF the RECMID option is on, move the priority with it also.
		mov	bl,ChPriCopy[esi]
		mov	ChPri[esi],bl
;endif

	 	;See if the node/channel that was previously there was the
		;same.  If not, Update the channel.

		mov	di,ax			 ;get the index into the Playlist for this node
		and	edi,0f0h		 ;shift from the hi nibble to
		shr	di,1			 ;playlist entry * 4
		shr	di,1

                push    ax
                mov     ax,di
                shr     ax,1
                shr     ax,1
                add     di,ax
                add     di,ax
                pop     ax
		

                les	ebx,PlayList[edi]	 ;get the node address in bx
		and	al,0fh			 ;isolate the sequenced channel
		cmp	ChOld[esi],al		 ;is the old channel the same as this channel?
		jne	notSameBed		 ;no-skip

		mov	di,si			 ;copy channel to di
		shl	di,1			 ;channel * 4
		shl	di,1
		
                add     di,si
                add     di,si

                cmp	dword ptr ChNodes[edi],ebx  ;is old node address the same as this one?
		jne	notSameBed		    ;no-skip
		mov	cx,es
		cmp	word ptr ChNodes[edi+4],cx
		jne	notSameBed		    ;no-skip
		jmp	nextPass2		    ;it's the same node and channel so no
						    ;need to update the channel

notSameBed:
		;the "fixed" channel that we have for this channel is not
		;the same one that was on the channel list before so we need
		;to shut down the old one, and init the new one

		UpdateChannel1
		jmp	nextPass2		    ;next channel...

noCopyBedCh:
		;OK, it's not a "locked" channel so see if we can locate
		;this node/channel somewhere on the oldChannel list

		mov	al,ChNew[esi]		    ;node/channel for this channel
		mov	bl,al
		and	ax,00fh			    ;get sequenced channel in al
		and	ebx,00f0h		    ;and playlist index in bx
		shr	bl,1
		shr	bl,1
		


                shr     bl,1
                shr     bl,1
                push    ax
                mov     ax,6
                mul     bl
                mov     bl,al
                pop     ax

                les	ebx,PlayList[ebx]	    ;get node address in es:bx

		;get the 4 byte index into the ChNodes array in di, starting
		;with the lowest channel available for the current driver.

		mov	cl,loChnl
		xor	ch,ch
		mov	di,cx
		shl	di,1
		shl	di,1

                add     di,cx
                add     di,cx

loopSameNode:
		cmp	dword ptr ChNodes[edi],ebx  ;is the old node the same as the one we're working on?
		jne	nextFindSame		    ;no-skip
		mov	cx,es
		cmp	word ptr ChNodes[edi+4],cx
		jne	nextFindSame		    ;no-skip

		;Same node, Look at the channel number
		;in ChOld and see if it is the same



                push    ax
                push    bx
                mov     ax,di
                mov     bl,6
                div     bl
                mov     di,ax
                pop     bx
                pop     ax

		cmp	ChOld[edi],al
		je	sameNodeCh		    ;found a matching node/channel-skip

                push    ax
                push    bx
                mov     ax,6
                mov     bx,di
                mul     bl
                mov     di,ax
                pop     bx
                pop     ax
nextFindSame:
		add	di,6			;inc to next ChNodes index
		mov	cx,di
 

                push    ax
                push    bx
                mov     ax,cx
                mov     bl,6
                div     bl
                mov     cx,ax
                pop     bx
                pop     ax

		dec	cl
		cmp	hiChnl,cl		;did we just finish with the driver's
						;highest channel?
		jne	loopSameNode		;no-keep looking for a match
		jmp	SHORT nextPass2		;yes-no match found, pass 3 will handle it

sameNodeCh:
		;We found a matching node/channel already on the channel list
		;at entry # di

		;If this ChList position is going to be occupied by a bed channel,
		;then this channel won't be able to stay on the channel it was
		;on before. Leave it for Pass3 to sort out.

		cmp	ChBed[edi],0
		jne	nextPass2

		;Copy this node/channel to the ChList, at the position
		;it used to be at.
		mov	cl,ChNew[esi]		
		mov	ChList[edi],cl		
		mov	ChNew[esi],0ffh

ifdef RECMID
  		;If the RECMID option is on, move the priority with it also
		mov	cl,ChPriCopy[esi]
		mov	ChPri[edi],cl
endif

nextPass2:
		inc	si			;If there are more channels
		cmp	si,16			;to copy, keep looping
		je	doPass3
		jmp	loopPass2



;*****************************************************************
;*** DoChannelList PASS 3 ***
;*	Finds places on the channel List for the channels remaining on
;* the chNew list that didn't get placed by pass 2.
;*****************************************************************

doPass3:
		mov	al,hiChnl		;1 higher than the driver's
		inc	al			;highest channel (in di)
		xor	ah,ah
		mov	di,ax
		xor	al,al
		xor	esi,esi

loopPass3:
		;Find a channel on the ChNew list that hasn't been added
		;to the channelList yet.

		cmp	ChNew[esi],0ffh
		jne	gotPass3Ch
		jmp	nextPass3

gotPass3Ch:
		;si has index to channel that needs to be added to channel list

		;Locate the last open channel	on the ChList starting from
		;the driver's highest channel and working backward
		mov	bx,di
		and	ebx,0ffffh
findLastOpen:
		dec	bx
		cmp	ChList[ebx],0ffh
		jne	findLastOpen

		;Copy our ChNew channel to ChList, and update the channel.
		mov	di,bx
		mov	al,ChNew[esi]	       ;copy PlayList position/channel
		mov	ChList[edi],al	       ;to the channel list

ifdef RECMID
		;if the RECMID option is on, move the priority with it also.
		mov	bl,ChPriCopy[esi]
		mov	ChPri[edi],bl
endif

		mov	bl,al
		and	al,00fh		       ;get sequenced channel in al
		and	ebx,0f0h
		shr	bx,1		       ;Playlist index in bx
		shr	bx,1
       
                shr     bx,1
                shr     bx,1
                push    ax
                mov     ax,6
                mul     bl
                mov     bx,ax
                pop     ax

		les	ebx,PlayList[ebx]      ;get node address
		push	esi
		mov	si,di		       ;si = channel list channel
		UpdateChannel2		       ;init that channel
		pop	esi

nextPass3:
		inc	si			;Loop back to get the next
		cmp	si,16			;remaining channel
		je	cleanUpChnls
		jmp	loopPass3



;**************************************************************
;*** CleanUp Removed Channels ***
;**************************************************************

;If any channels were active before but are not now,
;Turn it's notes off and reset it's NUMNOTES

cleanUpChnls:
		mov	esi,15
loopCleanUp:
		cmp	ChOld[esi],0fh	       ;was the channel slot busy before?
		je	nextCleanUp	       ;no, skip

		cmp	ChList[esi],0ffh       ;is the channel slot still busy?
		jne	nextCleanUp	       ;yes-skip

		mov	ax,si		       ;get channel list channel in ax
		mov	cx,(DAMPRCTRL*256)     ;mov ch,DAMPRCTRL: mov cl,0
		CallDriver DController	       ;turn off damper pedal

		mov	cx,(ALLNOFF*256)       ;mov ch,ALLNOFF: mov cl,0
		CallDriver DController	       ;send all notes off

		mov	cx,(NUMNOTES*256)      ;mov ch,NUMNOTES: mov cl,0
		CallDriver DController	       ;tell driver to release voices

nextCleanUp:
		dec	si
		jns	loopCleanUp


;***************************************************************
;*** Make ChOld List ***
;***************************************************************
;Make the ChOld list from the ChList for use by the
;next call to this procudure

		mov	ax,word ptr ChList     ;get 2 playlist position/channel entries

		;Isolate the channels, the empty
		;flag, which was 0FFH will become 0FH
		;but that's OK because no sequenced
		;channel will ever be 0FH because that's
		;a control channel.

		and	ax,0f0fh
		mov	word ptr ChOld,ax      ;save 2 slots in ChOld

		mov	ax,word ptr ChList[2]
		and	ax,0f0fh
		mov	word ptr ChOld[2],ax

		mov	ax,word ptr ChList[4]
		and	ax,0f0fh
		mov	word ptr ChOld[4],ax

		mov	ax,word ptr ChList[6]
		and	ax,0f0fh
		mov	word ptr ChOld[6],ax

		mov	ax,word ptr ChList[8]
		and	ax,0f0fh
		mov	word ptr ChOld[8],ax

		mov	ax,word ptr ChList[10]
		and	ax,0f0fh
		mov	word ptr ChOld[10],ax

		mov	ax,word ptr ChList[12]
		and	ax,0f0fh
		mov	word ptr ChOld[12],ax

		mov	ax,word ptr ChList[14]
		and	ax,0f0fh
		mov	word ptr ChOld[14],ax


;************************************************************
;*** Make ChNodes List ***
;************************************************************

;Construct the ChNodes list from the ChList and the PlayList for use in
;the next call to this procedure


		xor	esi,esi	   			;channelList index
		xor	edi,edi				;chNode index
loopChNodes:
		mov	bl,ChList[esi]			;get the node owner
		cmp	bl,0ffh
		jne	validChNode			;skip if not empty

		mov	word ptr ChNodes[edi],0		;zero out node address
		mov	word ptr ChNodes[edi+2],0
		mov	word ptr ChNodes[edi+4],0
		jmp	SHORT nextChNode

validChNode:
		and	ebx,0f0h				;get Playlist index in bx
		shr	bx,1
		shr	bx,1
	     
		shr	bx,1
		shr	bx,1
                push    ax
                mov     ax,6
                mul     bl
                mov     bx,ax
                pop     ax

        	mov	eax,dword ptr PlayList[ebx]	;get node address
		mov	dword ptr ChNodes[edi],eax	;save it
		mov	ax,word ptr PlayList[ebx+4]	;get node segment
		mov	word ptr ChNodes[edi+4],ax	;save it
nextChNode:
		add	di,6				;next ChNode entry
		inc	si				;next channelList entry
		cmp	si,16
		jne	loopChNodes			;loop till done w/16 channels

ifdef RECMID
		call	UpdateLists			;update MTEST'S screen
endif

		dec	processSnds			;re-enable the server

		pop	es
		pop	ebp
		pop	esi
		pop	edi
		pop	edx
		pop	ecx
		pop	ebx
		pop	eax
		ret
DoChannelList	ENDP


;****************************************************************
;* DoFade - process the fade properties
;*	    es:bx = ptr to node
;*	    si    = Playlist index for this node
;****************************************************************
DoFade		PROC	NEAR
		push	ebx
		push	ecx

		;If we still have some FadeCount left,	count it down and exit
		cmp	byte ptr es:[ebx].sFadeCount,0
		je	ticksOver
		dec	byte ptr es:[ebx].sFadeCount
		jmp	outDoFade

ticksOver:
		;copy fadeTicks to fadeCount to start another timing period
		mov	cl,es:[ebx].sFadeTicks
		mov	es:[ebx].sFadeCount,cl

		;If we're at the destination volume, stop the fade
		mov	cl,es:[ebx].sFadeDest
		and	cl,7fh			;mask off the "stop sound" bit
		cmp	cl,es:[ebx].sVolume
		je	endFade
		ja	fadeUp

		;Fade the sound down

		mov	cl,es:[ebx].sVolume
		mov	ch,es:[ebx].sFadeDest
		and	ch,7fh			;first, get the difference between where
		sub	cl,ch			;we are and where we want to be
		cmp	cl,es:[ebx].sFadeSteps
		ja	downSteps		;if the diff is > our step size, then skip

		mov	cl,es:[ebx].sFadeDest	;the diff is <= our step size, so just
		and	cl,7fh			;change volume to our destination vol.
		mov	ch,1			;set "volRequest" flag to not send the controllers
		call	DoChangeVol		;and change the node volume
		jmp	SHORT endFade

downSteps:
		mov	cl,es:[ebx].sVolume
		sub	cl,es:[ebx].sFadeSteps	;sub the fade steps from the current vol
		mov	ch,1			;set "volRequest" flag
		call	DoChangeVol		;and change the node volume
		jmp	SHORT outDoFade

fadeUp:
		;Fade the sound up

		mov	cl,es:[ebx].sFadeDest
		and	cl,7fh
		mov	ch,es:[ebx].sVolume	;get the diff between where we are and
		sub	cl,ch			;where we want to be
		cmp	cl,es:[ebx].sFadeSteps
		ja	upSteps			;if the diff is > our step size, skip

		mov	cl,es:[ebx].sFadeDest	;diff is <= step size so just change
		and	cl,7fh			;the volume to our destination vol.
		mov	ch,1			;set "requestVol" flag
		call	DoChangeVol
		jmp	SHORT endFade

upSteps:
		mov	cl,es:[ebx].sVolume
		add	cl,es:[ebx].sFadeSteps	;add the fade steps to our current volume.
		mov	ch,1			;set the "volRequest" flag
		call	DoChangeVol
		jmp	SHORT outDoFade

endFade:
		;set the signal to show the fade is done

		mov	byte ptr es:[ebx].sSignal,0feh

		;Stop the fade and check to see if we need to end the sound

		mov	byte ptr es:[ebx].sFadeSteps,0
		mov	cl,es:[ebx].sFadeDest
		and	cl,80h			;is the "stop sound" bit set?
		cmp	cl,0
		je	outDoFade
		mov	eax,ebx			;yes, stop the sound
		call	DoEnd
		mov	updateChnls,1		;signal next interrupt to doChannelList

outDoFade:
		pop	ecx
		pop	ebx
		ret
DoFade		ENDP


;****************************************************************
;* DoChangeVol	CHANGE SOUND NODE VOLUME
;*		es:bx = ptr to sound node
;*		si    = Playlist index for this node
;*		cl    = new volume to set node to
;*		ch    = "requestVol" flag
;****************************************************************
DoChangeVol	PROC	NEAR
		push	eax
		push	ebx
		push	ecx
		push	edx
		push	esi
		push	edi

		mov	vRequest,ch

		;If its the same volume as before, then don't bother
					
		cmp	cl,es:[ebx].sVolume
		jne	newVolNotSame
		jmp	outDoChVol

newVolNotSame:
		mov	es:[ebx].sVolume,cl	;Store the new volume

		cmp	si,255			;If the node isn't on the
		jne	volNodeOnList		;PlayList, then exit
		jmp	outDoChVol

volNodeOnList:
		;Change the volume of all real channels occupied by the sound

		mov	dx,si			;get Playlist Position in dl upper nibble
                push    ax
                mov     ax,si
		shl	dl,1
		shl	dl,1
                add     dl,al
                add     dl,al
                pop     ax
        
		xor	esi,esi			;use si as Channel List index
		xor	edi,edi
loopChnlVol:
		;Loop till we find an occupied channel

		mov	cl,ChList[esi]					
		cmp	cl,255							
		je	nextChEntry

		mov	ch,cl
		and	cl,0f0h		       ;get this channel's Playlist position in cl
		cmp	cl,dl		       ;are they the same?
		jne	nextChEntry

		mov	cl,ch		       ;yes, get sequenced channel
		and	cx,0fh
	        mov	di,cx		       ;in di
		mov	cl,es:[ebx+edi].cVolume;get the current channel volume in cl
		push	dx
		mov	dl,es:[ebx].sVolume    ;get the current node volume in dl
		call	ScaleVolume	       ;scale the volume

		cmp	vRequest,0	       ;is the "volRequest" flag set?
		je	sendNewVol
		mov	VolRequest[esi],cl     ;yes, don't actually send the new volume
		jmp	SHORT noVolSend	       ;save the vol in the vRequest array
sendNewVol:
		mov	ch,VOLCTRL
		mov	ax,si
		mov	VolRequest[esi],255    ;axe any pending volume send
		CallDriver DController	       ;send the new volume
noVolSend:
		pop	dx
nextChEntry:
		inc	si
		cmp	si,16		       ;do all 16 channels
		jne	loopChnlVol


		;If there are any ghost channels in the node, change
		;their volumes if no one else owns their channels

		xor	ch,ch
		xor	esi,esi		         ;track index
loopGhostVols:
		mov	cl,es:[ebx+esi].tChannel ;get the track's channel
		cmp	cl,0ffh			 ;if end of track list skip out
		je	outDoChVol

		mov	di,cx			 ;get channel in di
		test	es:[ebx+edi].cFlags,2	 ;check the "ghost channel" bit
		jz	noChGhostVol		 ;skip if not a ghost

		cmp	ChList[edi],255		 ;check if someone else owns the channel
		jne	noChGhostVol		 ;yes, so don't change volume (skip)

		mov	al,cl			 ;get channel in al
		mov	cl,es:[ebx+edi].cVolume	 ;channel volume in cl
		mov	dl,es:[ebx].sVolume	 ;node volume in dl
		call	ScaleVolume		 ;scale the channel volume

		cmp	vRequest,0		 ;should we send it now?
		je	sendNewGhstVol
		mov	VolRequest[edi],cl	 ;no, save the vol in vRequest array
		jmp	SHORT noChGhostVol

sendNewGhstVol:
		mov	ch,VOLCTRL		 ;channel in al
		mov	ax,di
		mov	VolRequest[edi],255
		CallDriver DController		 ;update the volume

noChGhostVol:
		inc	si
		cmp	si,16			 ;for 16 possible tracks
		jne	loopGhostVols
					
outDoChVol:
		pop	edi
		pop	esi
		pop	edx
		pop	ecx
		pop	ebx
		pop	eax
		ret
DoChangeVol	ENDP



;******************************************************************
;* PROCESS FADE VOLUME REQUESTS
;* Send the next two (or less) volume requests on the VolRequest list
;* 	We only process 2 at a time to avoid MIDI clog or a time delay
;*		during the interrupt.
;******************************************************************
DoVolRequests	PROC	NEAR 
                push    ebx
                push    esi

		xor	bl,bl		   	;# of requests we've processed
		mov	al,requestChnl		;the channel we should start processing
						;from this time around
		xor	ah,ah
		mov	si,ax			;get channel in si
nextVolReq:
		mov	cl,VolRequest[esi]	;get the volRequest value
		cmp	cl,255			;if 255, then there's no request for this channel
		je	noRequest

		mov	VolRequest[esi],255	;mark the request as fulfilled
		mov	ch,VOLCTRL
		mov	ax,si			;channel in al, cl has volume
		CallDriver DController		;update the channel volume
		inc	bl
		cmp	bl,2			;have we done 2 this time around?
		je	outVolReq		;yeah, skip out

noRequest:
		inc	si			;bump the channel #
		cmp	si,16
		jne	notWrapChnl
		xor	esi,esi			;if the channel=16, make it 0

notWrapChnl:
		mov	al,requestChnl		;get the channel we started with
		xor	ah,ah
		cmp	si,ax			;compare it to the one we just processed
		jne	nextVolReq		;if we havn't gone all the way around, go back for more

outVolReq:
		mov	ax,si
		mov	requestChnl,al		;save the channel we ending with
                pop     esi
                pop     ebx
		ret
DoVolRequests	ENDP


;***************************************************************
;* Sound Server  PARSE EACH NODE ON PLAYLIST
;***************************************************************
SoundServer	PROC

		;if the server is disabled then just exit

		cmp	processSnds,0
		je	switchOn
		ret

switchOn:
		pushf
		;cli
		push	esi
		push	edi
		push	fs
		push	es
		push	ds
		push	ebp
		push	edx
		push	ecx
		push	ebx
		push	eax

;		mov	fs,SaveFS
		;Update the channel list if anything happened in the
		;last interrupt to warrant it

		cmp	updateChnls,0
		je	stepNodes
		call	DoChannelList

stepNodes:
		;Fade and parse each node on the PlayList

		xor	esi,esi			       ;playlist index
		xor	edi,edi			       ;
nextNode2:
		les	ebx,PlayList[esi]	       ;get node address
		mov	ax,es
		or	ax,bx
		je	outServer		       ;if it's NULL, we're done with the list

		cmp	byte ptr es:[ebx].sPause,0     ;if it's paused, then skip it
		jne	nextActivate

		cmp	byte ptr es:[ebx].sFadeSteps,0
		je	noFade

;CGB *** replaced missing code
;		cmp	byte ptr es:[ebx].sSample,0    	;if it's a sample then skip it
;		jne	nextActivate

		cmp	byte ptr es:[ebx].sMidiPtr32,1	;???
		je	nextActivate

		cmp	byte ptr es:[ebx].sFadeSteps,0	;if a fade is in progress
		je	noFade
;CGB ***
		call	DoFade				;perform the fade					;perform the fade

		cmp	byte ptr es:[ebx].sSignal,255	;if fade ended this node...
		jne	noFade				;then the Playlist positions have
		sub	si,6				;shifted up, so go process this slot again
		jmp	SHORT nextActivate

noFade:
		call	ParseNode

		cmp	byte ptr es:[ebx].sSignal,255	;if ParseNode ended this node then the
		je	activate			;the playlist positions have shifted up
							;so go process this slot again

nextActivate:
		add	si,6				;??? Why aren't si and di the same ???
							;??? By the time si is at the end of the
							;list, di could be well off it...
activate:
		add	di,6
		cmp	si,(LISTSIZE * 6)
		jne	nextNode2			;loop back if not end of list

outServer:
		;Process volume requests:
		;During the process of parsing and fading, etc, multiple volume
		;changes may have been calculated.  To avoid sending each volume
		;change to the driver, we just put the calculated volume changes
		;in the volRequest array.  Then, when all is done, we can run
		;thru the volRequest array and get the last calculated volume
		;for each channel and just send that.		

		call	DoVolRequests

		;service the driver:
		;Some drivers perform envelopes and stuff on a 60 cycle timer
		;basis so here we call that routine.

ifndef WINDOWS
		CallDriver DService
endif


		pop	eax
		pop	ebx
		pop	ecx
		pop	edx
		pop	ebp
		pop	ds
		pop	es
		pop	fs
		pop	edi
		pop	esi
		popf
		ret

SoundServer	ENDP


				              
;***********************************************************
;* ParseNode - Parse the MIDI info in a node
;*	       es:bx = node ptr
;*	       di    = Playlist index	(??? wrong though)
;***********************************************************
ParseNode	PROC	NEAR
		push	esi									;node in es:bx
		push	edi
		push	ebp
		push	eax
		push	ebx
		push	ecx
		push	edx
		push	ds

ifdef RECMID
		mov	testNode,0
endif

		;Put the PlayList position of the node in the 4 hi-bits of playPos

                push    ax
                push    bx
                mov     ax,di
                mov     bl,6
                div     bl
                mov     di,ax
                pop     bx      
                pop     ax
                shl     di,1
                shl     di,1

		shl	di,1
		shl	di,1
		mov	cx,di
		mov	playPos,cl

		;Increment the timer
    ;		Hnd2Ptr es:[ebx].sMidiPtr32
   ;		cmp	ebp,0800000h
   ;		jge	outParse

		inc	word ptr es:[ebx].sTimer


ifdef RECMID
		;If the resource pointer is ffff:ffff,	then it's a test node,
		;and will be receiving data directly from the MPU.

		mov	ebp,dword ptr es:[ebx].sMidiPtr32  
		cmp	ebp,0ffffffffh
		je	maybeTest
		jmp	SHORT regularParse
maybeTest:
		cmp	ebp,0ffffffffh
		je	irregParse
		jmp	SHORT regularParse
irregParse:
		;it's a MIDI receiver node, get data from MIDI in

		call	DoTestNode
		cmp	si,0			;did we get anything from MIDI in?
		jne	notSi0
		jmp	endParse

notSi0:
		xor	esi,esi
		mov	ebp,offset MIDIbuffer  ;yes-so parse it like normal
		jmp	parseCommand
endif


regularParse:
		;Get the pointer to the sound data in bp

;		Hnd2Ptr	es:[ebx].sMidiPtr32
		lfs	esi,fword ptr es:[ebx].sMidiPtr32
		mov	baseIndex,esi

;Step through tracks	until a channel# of 0ffh
;is encountered.  Skip any sample tracks, marked with 0feh

		xor	esi,esi			;track index
loopParseTrk:
		mov	al,es:[ebx+esi].tChannel
		cmp	al,0ffh
		jne	notLastTrk
		jmp	outParse		;found end of tracks, exit

notLastTrk:
		cmp	al,0feh
		jne	goodTrack
		jmp	parseNext		;found a sample, skip it

goodTrack:
		;Check to see if the current node-channel is on the ChList,
		;or if the channel is a ghost channel

		mov	realChnl,255	        ;flag "output channel unknown"
		mov	ghostChnl,0		;and not ghost
		push	esi
		mov	si,ax
		and	esi,0ffh		;get channel in si
		test	es:[ebx+esi].cFlags,2	;is it a "ghost" channel?
		pop	esi
		jz	notGhostChnl1		;no-skip

		mov	realChnl,al		;mark as "fixed" channel
		mov	ghostChnl,1		;and ghost
		jmp	SHORT restorePtr

notGhostChnl1:
		and	al,0fh			;make sure channel is 0-15 (unneccesary?)
		mov	cl,al			;get channel in cl lower nibble
		or	cl,playPos		;get playlist position in cl upper nibble	

		xor	edi,edi			;channel list index
lookForCh:
		cmp	ChList[edi],cl		;is the node/channel we are parsing on the channel list?
		je	chOnList		;yes, skip to process it
		inc	di
		cmp	di,16
		jne	lookForCh		;keep looking...
		jmp	SHORT restorePtr	;nope, didn't find it, skip

chOnList:
		mov	dx,di
		mov	realChnl,dl		;save channel list index as output channel

restorePtr:     
		;Point ds:bp to the location in the track data where we last left off

		push	ebx
		mov	ebx,baseIndex
		shl	si,1			;track index * 2

;		mov	dx,fs:[ebp+esi]		;get offset to track data
;		add	bp,dx			;add to sound data base address to get track address
;		jnc	help2
;		add	ebp,010000h
;help2:		add	bp,es:[ebx+esi].tIndex	;then add our curent offset within that track
;     	    	jnc     help
;		add	ebp,010000h
;help:		cmp	word ptr es:[ebx+esi].tIndex,0	;are we already done with this track?

		xor	edx, edx
		mov	dx,fs:[ebx+esi]		;get offset to track data
		pop	ebx
		add	dx,es:[ebx+esi].tIndex	;then add our curent offset within that track
		mov	currOffset, edx		;and save it

		cmp	word ptr es:[ebx+esi].tIndex,0	;are we already done with this track?
		jne	notFrozenTrk
		shr	si,1
		jmp	parseNext		;yes, so skip it

notFrozenTrk:
		;Check for a rest count.  If there is, decrement it and
		;move on to the next track

		shr	si,1

		shl	si,1				 ;track index * 2
		cmp	word ptr es:[ebx+esi].tRest,0 	 ;any rest value to process?
		je	restOver			 ;no-skip

		dec	word ptr es:[ebx+esi].tRest	 ;dec the rest value

		;The high bit is set if we are processing a "timing overflow"
		;byte.  Here we check if we are done processing a "timing overflow" byte

		cmp	word ptr es:[ebx+esi].tRest,8000h
		jne	notDoneOver

		;We just finished a "timing overflow" byte so get the next
		;timing byte

		xor	dh,dh	  		;zero out the "overflow" bit
		shr	si,1			;track index * 1
		getb	dl			;get the next byte
		shl	si,1			;track index * 2
		cmp	dl,TIMINGOVER		;if the new byte is "timing overflow"
		jne	notTOAgain
		mov	dl,240			;set timing to 240
		mov	dh,80h			;and set "overflow" bit

notTOAgain:
		mov	es:[ebx+esi].tRest,dx	;save the new timing value
notDoneOver:
		;here if we're not done with a rest on this track
		;go do the next track

		shr	si,1
		jmp	parseNext		;track index * 1
				
restOver:
		;done with timing, process a command

		shr	si,1			;track index * 1

parseCommand:
		;Read next byte in MIDI	stream, and determine if it's running
		;status or not
		getb	dl
		cmp	dl,128
		jb	runningStat
		mov	es:[ebx+esi].tCommand,dl ;save new running status command
		jmp	SHORT parseIt

runningStat:
		;The new byte was a data byte so restore the running status
		;and back up the data pointer

		mov	dl,es:[ebx+esi].tCommand
;		dec	ebp
		dec	currOffset
		shl	si,1
		dec	word ptr es:[ebx+esi].tIndex
		shr	si,1

parseIt:
		mov	al,dl
		mov	ah,al
		and	ah,0f0h			;get command in ah
		and	al,0fh			;get channel in al

ifdef RECMID
		;If the node is a test node, set the realChnl (output channel)
		;to what the command byte said

		cmp	testNode,0
		je	notAutoChnl
		mov	realChnl,al
endif

notAutoChnl:                                    
		;If the end of the track has been reached, mark
		;this track as being done, and move on to the next track

		cmp	dl,ENDTRK
		jne	notEndTrk
		shl	si,1
		mov	word ptr es:[ebx+esi].tIndex,0	  ;0 tIndex signals "frozen track"
		shr	si,1
		jmp	parseNext							

notEndTrk:
		;If we're on channel 16, parse with the ControlCh procedure.
		;If we return with a 0 index, then we hit a "hold point"

		cmp	al,15
		jne	notControlCh
		call	ControlChnl		;parse control channel
		shl	si,1
		mov	dx,es:[ebx+esi].tIndex
		shr	si,1
		cmp	dx,0			;done with track?
		jne	nextComm		;no-get next command
		jmp	parseNext		;yes-do next track

notControlCh:
		;Get the real Channel #, then	Switch on the value of the
		;command in ah to any of the eight MIDI handling
		;procedures

		mov	al,realChnl
		cmp	ah,NOTEOFF
		jne	notNoteOff
		call	NoteOff
		jmp	nextComm

notNoteOff:
		cmp	ah,NOTEON
		jne	notNoteOn
		call	NoteOn
		jmp	SHORT nextComm
notNoteOn:
		cmp	ah,POLYAFTER
		jne	notPolyAfter
		call	PolyAfterTch
		jmp	SHORT nextComm
notPolyAfter:
		cmp	ah,CONTROLLER
		jne	notContrlr
		call	Controller
		jmp	SHORT nextComm
notContrlr:
		cmp	ah,PCHANGE
		jne	notPChange
		call	ProgramChange
		jmp	SHORT nextComm
notPChange:
		cmp	ah,CHNLAFTER
		jne	notChnlAfter
		call	ChnlAfterTch
		jmp	SHORT nextComm
notChnlAfter:
		cmp	ah,PBEND
		jne	notPBend
		call	PitchBend
		jmp	SHORT nextComm
notPBend:
		cmp	ah,SYSEX
		jne	notSysex
		call	SysEx
		jmp	SHORT nextComm

notSysex:
		;If the command wasn't any of the above, then
		;shut down the track before we do any serious damage.

		shl	si,1
		mov	word ptr es:[ebx+esi].tIndex,0
		shr	si,1
		jmp	SHORT parseNext

nextComm:
		;Get the next timing byte
						 
		getb	dl			 ;get the timing
		cmp	dl,0
		jne	endOfParse
		jmp	parseCommand		 ;if it was 0 then go parse it now

endOfParse:

ifdef RECMID					 ;If the node was a test node,
		cmp	testNode,0		 ;then exit
		je	notTestNode		
		jmp	endParse
endif

notTestNode:
		;Put the timing byte in	the track's tRest counter.
		;Subtract 1 because it will take one tick before the
		;next timing check

		;CB ***??? I don't see why we should subtract 1 from the new timing.
		;We decrement when we come into this routine, so what happens if
		;the new timing is a 1?  We dec it here (0) then we dec it again
		;next time around making it 0FFH...oops.

		shl	si,1
		cmp	dl,TIMINGOVER		;is the new byte a "timing overflow"?
		jne	regularTime
		mov	dl,239			;yes-make timing 240
		mov	dh,80h			;and set the "overflow" flag
		mov	es:[ebx+esi].tRest,dx	;save new rest
		shr	si,1
		jmp	SHORT parseNext		;do next track
regularTime:
		xor	dh,dh
		dec	dl
		mov	es:[ebx+esi].tRest,dx	;save new rest
		shr	si,1

parseNext:
		inc	si 			;Loop to next track
		cmp	si,16
		je	outParse
		jmp	loopParseTrk

outParse:
		;If the sound is over, either loop it or end it

		xor	esi,esi				   ;start w/track 0
checkForEnd:
		cmp	byte ptr es:[ebx+esi].tChannel,255 ;did we hit the end of the tracks?
		je	allFrozen			   ;yup, sound is done
		shl	si,1
		cmp	word ptr es:[ebx+esi].tIndex,0	   ;no, is this track frozen?
		jne	endParse			   ;no, sound is not done
		shr	si,1
nextEndChk:
		inc	si
		cmp	si,16				   ;for all 16 possible tracks
		jne	checkForEnd

allFrozen:
		;All the tracks in this node have completed (they're frozen)
		;so the sound has completed.

		;if a hold point is set, all the track indexes were set to 0
		;to fake us into thinking the sound is done thus ending up here.
		;This must be the case if there a hold has been set, so loop back to the 
		;last loop point regardless of the loop property setting. 

		cmp	byte ptr es:[ebx].sHold,0
		jne	loopSound

		;if the loop property is set, then loop the sound
		cmp	byte ptr es:[ebx].sLoop,0
		jne	loopSound

findEndSpot:
		;end the sound

		mov	eax,ebx		       		;get node ptr in ax
		call	DoEnd
		mov	updateChnls,1			;signal that we need a doChannelList next
		jmp	SHORT endParse

loopSound:
		;we need to loop the sound

		mov	dx,es:[ebx].sLoopTime		;get the saved loop timer
		mov	es:[ebx].sTimer,dx		;stash it in the node's timer

		xor	esi,esi				;starting with track 0
loopLoopSnd:
		mov	dx,es:[ebx+esi].tLoopPoint	;copy the loop index
		mov	es:[ebx+esi].tIndex,dx		;to the track index

		mov	dx,es:[ebx+esi].tLoopRest	;copy the loop rest value
		mov	es:[ebx+esi].tRest,dx		;to the current rest value

		shr	si,1
		mov	dl,es:[ebx+esi].tLoopCommand	;copy the loop running status
		mov	es:[ebx+esi].tCommand,dl	;to the current running status

		shl	si,1
		add	si,2				;next track
		cmp	si,32
		jne	loopLoopSnd			;do all 16 tracks

endParse:
		pop	ds
		pop	edx
		pop	ecx
		pop	ebx
		pop	eax
		pop	ebp
		pop	edi
		pop	esi
		ret
ParseNode	ENDP


;****************************************************
;*	NoteOff
;*	es:bx = ptr to sound
;*	al    = output channel
;*	ah		= command
;****************************************************

NoteOff		PROC	NEAR
		push	esi

		getb	ch				;Put note # in ch, and its
		getb	cl				;velocity in cl

		push	eax
		mov	al,es:[ebx+esi].tChannel	;get sequenced channel
		mov	si,ax
		and	si,000000000fh			;in si
		pop	eax

		cmp	es:[ebx+esi].cCurNote,ch	   ;are we turning off the current note?
		jne	notCurNoteOff
		mov	byte ptr es:[ebx+esi].cCurNote,255 ;yes-reset the current note property

notCurNoteOff:
		cmp	al,255		    	;this channel is not on the channel list
		je	outNoteOff		;so skip it

		cmp	restoring,0		;are we restoring a saved game?
		jne	outNoteOff		;yeah so don't send the event

		and	al,0fh			;make output channel 0-15
	 	CallDriver DNoteOff		;send the event

outNoteOff:
		pop	esi
		ret
NoteOff		ENDP


;****************************************************
;*	NoteOn
;*	es:bx = ptr to sound
;*	al    = output channel
;*	ah    = command
;****************************************************

NoteOn		PROC	NEAR
		push	esi

		getb	ch			;Put note # in ch, and its 
		getb	cl			;velocity in cl


		push	eax		
		mov	al,es:[ebx+esi].tChannel ;get sequenced channel
		mov	si,ax
		and	esi,000000000fh		 ;in si
		pop	eax

		cmp	cl,0			 ;if the velocity is 0 then it's
		je	doNoteOff		 ;really a note off

		mov	es:[ebx+esi].cCurNote,ch ;save the note as current note

		cmp	al,255			;if the channel is not on the channel
		je	outNoteOn		;list, don't send the event

		cmp	restoring,0		;if we are restoring a saved game,
		jne	outNoteOn		;don't send the event

		and	al,0fh			;make channel 0-15
	  	CallDriver DNoteOn		;send note on
		jmp	SHORT outNoteOn

doNoteOff:
		cmp	es:[ebx+esi].cCurNote,ch           ;are we turning off the current note?
		jne	notCurNote
		mov	byte ptr es:[ebx+esi].cCurNote,255 ;yes-reset the curNote property

notCurNote:
		cmp	al,255			;if the channel is not on the channel list
		je	outNoteOn		;don't send the event

		cmp	restoring,0		;if we are restoring a saved game,
		jne	outNoteOn		;don't send the event

		and	al,0fh			;make channel 0-15
	   	CallDriver DNoteOff		;send note off

outNoteOn:
		pop	esi
		ret
NoteOn		ENDP


;***********************************************************
;* Poly Aftertouch
;*		es:bx = ptr to sound
;*		al    = output channel
;*		ah    = command
;***********************************************************
PolyAfterTch	PROC	NEAR
		getb	ch		 ;Put note # in ch and
		getb	cl		 ;pressure in cl

		cmp	al,255		 ;if the channel is not on the channel list
		je	noPolyAfter	 ;don't send the event

		cmp	restoring,0	 ;if we are restoring a saved game,
		jne	noPolyAfter	 ;don't send the event

					 ;***??? make channel 0-15 ???***
	     	CallDriver DPolyAfterTch
noPolyAfter:
		ret
PolyAfterTch	ENDP



;***********************************************************
;* Controller
;* es:bx = ptr to sound
;* al    = output channel
;* ah	 = command
;***********************************************************
Controller	PROC	NEAR
		push	esi

		getb	ch		 ;Put controller # in ch and
		getb	cl		 ;value in cl

		test	ghostChnl,0ffh
		jz	notGhostCont

		push	esi
		mov	si,ax
		and	esi,000000000fh	 ;get the output channel
		cmp	ChList[esi],255	 ;check if that channel is owned
		pop	esi
		je	notGhostCont	 ;yup, don't send the event
		jmp	outContrlr

notGhostCont:
		push	eax
		mov	al,es:[ebx+esi].tChannel
		mov	si,ax
		and	esi,000000000fh	 ;get sequenced channel in si
		pop	eax

		;Save applicable controller values in sound node

		cmp	ch,VOLCTRL
		jne	notVolCtrl
		mov	es:[ebx+esi].cVolume,cl	 ;save the channel volume
		mov	dl,es:[ebx].sVolume	 ;get the node volume
		call	ScaleVolume		 ;scale it


		;***??? this should test for 255 rather than < 32 ***???

		cmp	al,32			 ;if the channel is not on the channel list
		jb	clrVolRequest
		jmp	outContrlr		 ;don't send the event

clrVolRequest:
		;since we will be sending a volume update, reset
		;any pending volume updates from the volRequest array

		push	esi
		mov	si,ax			 ;output channel in si
		and	esi,0ffh
		mov	VolRequest[esi],255
		pop	esi
		jmp	doCtrlr

notVolCtrl:
		cmp	ch,PANCTRL
		jne	notPanCtrl
		mov	es:[ebx+esi].cPan,cl	 ;save channel pan
		jmp	doCtrlr

notPanCtrl:
		cmp	ch,MODCTRL
		jne	notModCtrl
		mov	es:[ebx+esi].cModulation,cl  ;save channel modulation
		jmp	doCtrlr

notModCtrl:
		cmp	ch,DAMPRCTRL
		jne	notDamprCtrl
		push	dx
		shl	si,1
		mov	dx,es:[ebx+esi].cDamprPbend
		or	dh,80h		     		;set the damper bit on
		cmp	cl,0				;if we are turning the damper off
		jne	notOffDampr
		and	dh,7fh				;then set the damper bit off
notOffDampr:
		mov	es:[ebx+esi].cDamprPbend,dx	;save new damper bit
		shr	si,1
		pop	dx
		jmp	SHORT doCtrlr

notDamprCtrl:
		cmp	ch,NUMNOTES
		jne	notNumNotes
		push	cx
		mov	ch,es:[ebx+esi].cPriVoice 	;get the current Priority/numnotes
		and	ch,0f0h				;isolate the priority
                and     cl,0fh				;Enforce NUMNOTES 0-15, @NUM
		or	ch,cl				;mix pri/new numnotes
		mov	es:[ebx+esi].cPriVoice,ch	;save it
		pop	cx
		mov	updateChnls,1			;flag "need doChannelList"
		jmp	SHORT doCtrlr

notNumNotes:
		cmp	ch,MUTECTRL
		jne	doCtrlr
		push	cx
		mov	ch,es:[ebx+esi].cMute	;get old mute byte
		and	ch,0f0h			;isolate "programmer" mutes
		test	cl,0ffh			;0 = off, else on
		je	muteOff
		mov	cl,1			;set musician mute flag
						;Unlike the programmer mute, this
						;is not a count.  It's either 1 or 0.

muteOff:
		or	ch,cl			;mix programmer/musician mute values
		mov	es:[ebx+esi].cMute,ch	;save new mute byte
		pop	cx
		mov	updateChnls,1		;flag "need doChannelList"

doCtrlr:
		;here we actually send the controller

		cmp	al,255			;if the channel is not on the channel list
		jae	outContrlr		;don't send the event

		cmp	restoring,0		;if we are restoring a saved game
		jne	outContrlr		;don't send the event

		and	al,0fh			;make channel 0-15
	      	CallDriver DController		;send the controller

outContrlr:
		pop	esi
		ret
Controller	ENDP



;***********************************************************
;* ProgramChange
;* 	es:bx = ptr to sound
;* 	al    = output channel
;* 	ah    = command
;***********************************************************
ProgramChange	PROC	NEAR
		push	esi

		getb	cl			;Put program # in cl

		test	ghostChnl,0ffh
		jz	notGhostPch

		;This is a ghost channel so don't send the program
		;unless nobody else owns the channel

		push	esi
		mov	si,ax
		and	esi,000000000fh		;output channel in si
		cmp	ChList[esi],255		;anybody own it?
		pop	esi
		je	notGhostPch
		jmp	SHORT noPChange		;yup, so skip it

notGhostPch:
		push	eax
		mov	al,es:[ebx+esi].tChannel
		mov	si,ax
		and	esi,000000000fh		  ;sequenced channel in si
		pop	eax

		mov	es:[ebx+esi].cProgram,cl  ;Save program in sound node

		cmp	al,255			  ;if channel is not on the channel list
		jae	noPChange		  ;don't send the event

		cmp	restoring,0		  ;if restoring a saved game
		jne	noPChange		  ;don't send the event

		and	al,0fh			  ;make channel 0-15
	       	CallDriver DProgramChange

noPChange:
		pop	esi
		ret
ProgramChange	ENDP

;***********************************************************
;* ChnlAfterTouch
;*		es:bx = ptr to sound
;*		al    = output channel
;*		ah    = command
;***********************************************************
ChnlAfterTch	PROC	NEAR
		getb	cl		     ;Put pressure in cl

		cmp	al,255		     ;if channel is not on the channel list
		jae	noChnlAfter	     ;don't send the event

		cmp	restoring,0	     ;if restoring a saved game
		jne	noChnlAfter	     ;don't send the event

	       	CallDriver DChnlAfterTch
noChnlAfter:
		ret
ChnlAfterTch	ENDP


;***********************************************************
;* PitchBend
;*	   es:bx = ptr to sound
;*	   al    = output channel
;*	   ah	 = command
;***********************************************************
PitchBend	PROC	NEAR
		push	esi

		getb	ch 			;Put lsb in ch and msb in cl
		getb	cl

		test	ghostChnl,0ffh
		jz	notGhostPbend

		;This is a ghost channel so don't send the bender unless
		;no one else owns the channel

		push	esi
		mov	si,ax
		and	esi,000000000fh		;output channel in si
		cmp	ChList[esi],255		;anyone own it?
		pop	esi
		je	notGhostPbend
		jmp	SHORT noPBend		;yup, don't send it

notGhostPbend:
		push	eax
		mov	al,es:[ebx+esi].tChannel	;get sequenced channel in si
		mov	si,ax
		and	esi,000000000fh
		pop	eax

		push	cx
		xchg	ch,cl			;lsb in cl, msb in ch
		shr	ch,1			;move msb low bit to lsb hi bit
		jnc	noHiPitchBit2
		or	cl,80h
noHiPitchBit2:
		shl	si,1
		cmp	word ptr es:[ebx+esi].cDamprPbend,8000h	;if the damper bit is on
		jb	noDamper
		or	ch,80h					;set it in the damper/bender word
noDamper:
		mov	es:[ebx+esi].cDamprPbend,cx		;save new damper/bender word
		shr	si,1
		pop	cx			;lsb in ch, msb in cl

		cmp	al,255			;if the channel is not on the channel list
		jae	noPBend			;don't send the event

		cmp	restoring,0		;if we are restoring a saved game
		jne	noPBend			;don't send the event

		and	al,0fh			;enforce channel to 0-15
	       	CallDriver DPitchBend

noPBend:
		pop	esi
		ret
PitchBend	ENDP

;***********************************************************
;* SysX		- should never be here!
;*		  es:bx = ptr to sound
;*		  al    = output channel
;*		  ah	= command
;***********************************************************
SysEx		PROC	NEAR
		call	SkipMidi	;We don't support it
		ret
SysEx		ENDP


;************************************************************
;* ControlChnl - process channel 16 (control channel)
;*		 si = track index (track number)
;************************************************************
ControlChnl	PROC	NEAR
		cmp	ah,PCHANGE
		je	doCue
		cmp	ah,CONTROLLER
		jne	notControlComm
		jmp	doContrlr
notControlComm:
		call	SkipMidi 		;skip it if it's not a valid control
		jmp	outConCh		;command.

doCue:
		;PGM CHANGE If it's a loop set, set the loop point.

		getb	ch			;get the value
		cmp	ch,127 			;is it loop set?
		jne	notLoopCue		;nope-skip

		push	dx
		getb	dl			;get the next timing value
		xor	dh,dh
		cmp	dl,TIMINGOVER		;if it's a timing overflow..
		jne	notLoopTOver
		mov	dh,80h			;set the overflow bit
		mov	dl,240			;and make the timing 240

notLoopTOver:
		shl	si,1
		mov	es:[ebx+esi].tRest,dx	;save the track's rest value
		shr	si,1
		pop	dx
		mov	byte ptr es:[ebx+esi].tCommand,0cfh ;and running status
		push	esi
		push	dx

		xor	esi,esi
loopLoopPoint:
		;for each track, set the loop index, rest value, and running status

		shl	si,1
		mov	dx,es:[ebx+esi].tIndex	    	;copy current track index to loopPoint
		mov	es:[ebx+esi].tLoopPoint,dx

		mov	dx,es:[ebx+esi].tRest		;copy current rest value to LoopRest
		mov	es:[ebx+esi].tLoopRest,dx

		shr	si,1
		mov	dl,es:[ebx+esi].tCommand	;copy current runningStatus to loopRunningStatus
		mov	es:[ebx+esi].tLoopCommand,dl

		inc	si				;next track
		cmp	si,16
		jne	loopLoopPoint

		mov	dx,es:[ebx].sTimer		;copy the node's current timer to the loop timer
		mov	es:[ebx].sLoopTime,dx

		pop	dx
		pop	esi				;restore control track index
		shl	si,1
		dec	word ptr es:[ebx+esi].tIndex	;back up the track ptr to the timing byte
;		dec	ebp
		dec	currOffset
		mov	word ptr es:[ebx+esi].tRest,0	;and pretend we never saw it...
		shr	si,1
		jmp	outConCh

notLoopCue:
	  	;If not loop then set the signal property of the sound node

		cmp	restoring,0			;if restoring a game, don't bother
		jne	outConCh
		mov	es:[ebx].sSignal,ch
		jmp	outConCh

doContrlr:
		;If it's reverb, set a new ReverbMode.

		getb	ch
		getb	cl
		cmp	ch,REVERBMODE
		jne	notReverb
		cmp	cl,07fh				;a reverb of 127 means "take the system
		jne	notNewRevDef			;default reverb setting".
		mov	cl,revDefault

notNewRevDef:
		mov	es:[ebx].sReverbMode,cl
		push	eax
	       	CallDriver DSetReverb			;set the node's new reverb
		pop	eax
		jmp	SHORT outConCh

notReverb:
		;If it's a DataInc cue, increment the value in the node.

		cmp	ch,DATAINC
		jne	notDataInc
		cmp	restoring,0		        ;if restoring a game, don't bother
		jne	outConCh
		inc	word ptr es:[ebx].sDataInc
		jmp	SHORT outConCh

notDataInc:
		;If it's an hold point, and sHold matches the value, 
		;loop to last loop point.

		cmp	ch,ENDPOINT
		jne	outConCh
		
		cmp	es:[ebx].sHold,cl	;compare the programmer's hold point
		jne	outConCh		;to this hold point, skip if !=

		push	esi
		xor	esi,esi
loopEndPoint:
		;found a hold point: for each track, set the event index to 0.
		;This will fool ParseNode into thinking the sound is done, then
		;looping the sound back to the loop point.

		mov	word ptr es:[ebx+esi].tIndex,0
		add	si,2
		cmp	si,32
		jne	loopEndPoint
		pop	esi

outConCh:
		ret
ControlChnl	ENDP

;*************************************************************
;*	SKIP NEXT MIDI COMMAND
;*		ah = MIDI command byte of event to skip
;*************************************************************
SkipMidi	PROC	NEAR
		cmp	ah,SYSEX
		jne	notSYSEX
keepSkipping:
		;skip until End-Off-SysX found (should never happen)

		getb	ch
		cmp	ch,EOX
		jne	keepSkipping
		ret

notSYSEX:
		;skip 1 or 2 data bytes

		cmp	ah,PCHANGE
		je	skipOnly1
		cmp	ah,CHNLAFTER
		je	skipOnly1
		getb	ch
skipOnly1:
		getb	ch
		ret
SkipMidi	ENDP


;***********************************************************
;* Scale channel volume controller to node master volume.
;*
;*  cVol	    x
;*  ---- = ----
;*  127	  sVol
;*
;*  dl = node volume
;*	 cl = channel volume
;***********************************************************
ScaleVolume	PROC	NEAR
		push	ax
		mov	al,cl		;get channel vol in ax
		xor	ah,ah

		inc	al		;make top vol 80H * 80H
		inc	dl

		mul	dl		;for a total of 4000H
		shl	ah,1		;make max total 8000H
		mov	cl,ah
		cmp	cl,0		;done if 0
		je	outScale
		dec	cl		;else make max total 7FH
outScale:
		pop	ax
		ret
ScaleVolume	ENDP


;******************************************************************
;* FindNode	FIND A NODE ON THE PLAYLIST
;*		es:bx = ptr to node
;*		Returns the index (*4) in si
;*		or si = 255 if not found
;******************************************************************
FindNode	PROC	NEAR
		push	ecx

		mov	cx,es 
		xor	esi,esi
lookForNode:
		cmp	dword ptr PlayList[esi],ebx ;index in si
		jne	notFoundNode
		cmp	word ptr PlayList[esi+4],cx
		jne	notFoundNode
		jmp	SHORT outFnode
notFoundNode:
		add	si,6
		cmp	si,(LISTSIZE * 6)
		jne	lookForNode
		mov	si,255

outFnode:       pop	ecx
		ret
FindNode	ENDP


;*******************************************************************
;*	FixupHeader - CREATE SOUND RESOURCE HEADER
;*		      ax = offset of node in playlist
;*******************************************************************
FixupHeader	PROC	NEAR
		push	ebx
		push	ecx
		push	edx
		push	ebp
		push	ds

		;if it's a receiver node, then don't bother
		mov	ebx,eax
		cmp	word ptr es:[ebx].sMidiPtr32,0ffffh
		jne	notReceiveNode
		cmp	word ptr es:[ebx+2 ].sMidiPtr32,0ffffh
		jne	notReceiveNode
		jmp	outFixup

notReceiveNode:
		Hnd2Ptr	es:[ebx].sMidiPtr32		;get the resource pointer in ebp

		cmp	byte ptr fs:[ebp+35],0feh 	;Make sure that this
		jne	doHeader		 	;resource hasn't already
		cmp	byte ptr fs:[ebp+34],0fdh 	;been fixed up
		jne	doHeader
		cmp	byte ptr fs:[ebp+33],0fch
		jne	doHeader
		jmp	outFixup

doHeader:
		push	ebp				;Clear the temp header buffer
		mov	esi,32
clearHeader:
		sub	si,2
		mov	KludgeHdr[esi],0
		cmp	si,0				;temp header buffer index
		jne	clearHeader

		;If there is a properties entry in the header,
		;pull the properties information out of it

		mov	priInfo,-1
		cmp	byte ptr fs:[ebp],0f0h
		jne	checkDevID
		mov	cl,fs:[ebp+1]			;get priority
		mov	priInfo,cl			;and save it
		add	ebp,8				;then skip the remainder of
							;the properties entry

;Sound file header format
;
;		struct devTracks {
;			byte deviceID;		;Device ID that the following trackInfo structures
;						;pertain to.  If it's -1 then there are no more 
;						;devTrack structures, and the track data will begin
;						;immediatly after on the next even byte.
;
;			struct trackInfo {
;				byte 	tLast;		;This will be -1 if there are no more trackInfo
;							;structures for this device.  The next devTracks
;							;structure will begin immediatly after this byte.
;
;				byte 	tUnused;	;This should just be 0
;
;				word	tOffset;	;16 bit offset from top of file (after the 0x84,0, 
;							;since they should be stripped.
;
;				word	tLength		;Length of the track
;			} tracks[??];			;There will be a variable number of trackInfo structures
;
;		}


		;Find our installed device ID in header
checkDevID:
		mov	cl,fs:[ebp]  		;get track device ID
		cmp	cl,devID		;is it our device?
		je	gotDevice		;yes-skip out

		cmp	cl,0ffh			;is it the end of the header?
		je	writeHeader		;yes, go write the new empty header


		;Here if the track's device is different from the one we want.

		inc	ebp			;get to the track info tLast field
findEndDev:
		mov	cl,fs:[ebp]		;get tLast
		inc	ebp			;inc to tUnused (or next device)
		cmp	cl,0ffh			;was tLast -1? (last track for this device)
		jne	notEndDev
		jmp	SHORT checkDevID	;yes-so bp go check next device ID
notEndDev:
		add	ebp,5			;point to next tLast and loop on trackInfo again
		jmp	SHORT findEndDev


;Construct the Kludged header

gotDevice:
		inc	ebp
findDevTracks:
		mov	cl,fs:[ebp]		;get tLast
		inc	ebp
		cmp	cl,0ffh			;if it's 0FFH then we are done
		je	writeHeader

		inc	ebp			;inc to tOffset
		mov	cx,fs:[ebp]		;get the track offset
		add	bp,4			;inc to next trackInfo structure
		jnc	help3
		add	ebp,010000h
  
help3:		mov	KludgeHdr[esi],cx	;save the offset in the tmp header buffer
		add	si,2			;next temp header index
		jmp	SHORT findDevTracks


;Write the header onto (over) the sound resource
;
; Internal header format
;
;		1 device only
;
;		word 	trackOffsets[16];
;		byte	priority;
;		byte	0FCH			;codes defining this as an already
;		byte	0FDH			;"converted"  header
;		byte	0FEH


writeHeader:
		pop	ebp			;restore resource pointer
		push	ebp
		xor	esi,esi			;temp header index
writeMoreHdr:
		mov	cx,KludgeHdr[esi]	;get first track index
		mov	fs:[ebp],cx		;save it over the resource
		add	si,2			;next temp header index
		add	bp,2			;next destination address in resource
		jnc	help5
		add	ebp,010000h
       
help5:		cmp	si,32
		jne	writeMoreHdr		;if not done, go back for more

		mov	cl,priInfo		;Put the priority information
		mov	fs:[ebp],cl		;into the new header

		pop	ebp			;restore resource ptr

		;Flag this resource as an already "fixed-up" resource

		mov	byte ptr fs:[ebp+33],0fch
		mov	byte ptr fs:[ebp+34],0fdh
		mov	byte ptr fs:[ebp+35],0feh

outFixup:
		pop	ds
		pop	ebp
		pop	edx
		pop	ecx
		pop	ebx
		ret
FixupHeader	ENDP




;************************************************************
;* AskDriver - get states from the driver
;*	       ah = MIDI command or 
;*	       		0 for MasterVolume
;*	       		1 for DefaultReverb
;*	       		2 for SoundOn state
;*					
;*	       ah = Carg1 (command)
;*	       al = Carg2 (channel)
;*	       ch = Carg3 (data byte 1)
;*	       cl = Carg4 (data byte 2)
;*
;************************************************************

AskDriver	PROC
		pushf
		;cli

		cmp	ah,0
		jne	notAskMVol
		mov	cl,255
		CallDriver DMasterVol
		jmp	SHORT outAsk
notAskMVol:
		cmp	ah,1
		jne	notAskRev
		mov	cl,255
		CallDriver DSetReverb
		jmp	SHORT outAsk
notAskRev:
		cmp	ah,2
		jne	notAskSndOn
		mov	cl,255
		CallDriver DSoundOn
		jmp	SHORT outAsk

notAskSndOn:
		CallDriver DAskDriver

outAsk:
			popf
		ret
AskDriver	ENDP


ifdef RECMID


;---------------------------------------------------------------------------
;---------------------------------------------------------------------------
;       Midi Receiving functions (for MTEST)


;**********************************************************
;* Get MIDI byte from MIDI receive interrupt buffer - @INT
;* Return it in al or 255 if no bytes available
;**********************************************************


MIDIget		PROC	NEAR
                push    ebx

                xor     ax,ax
receiveNext:
                cmp     MIDIbcount,0
                jne     Mg_ELSE
                 	;If no bytes return 255 in al
                        mov     al,255
                        jmp     outMIDIget
Mg_ELSE:
                dec     MIDIbcount              ;Decrement byte count
                mov     ebx,MIDIbhead           ;Buffer empty pointer
                mov     al,ds:[ebx]             ;Get character from buffer
                inc     ebx                      ;Bump buffer empty pointer

                cmp     ebx,offset ds:MIDIbufend ;Test for end of buffer
                jb      NotAtEnd                ;no, continue
                mov     ebx,offset ds:MIDIbufbeg ;Yes, set to beginning
NotAtEnd:
                mov     MIDIbhead,ebx           ;Update empty buff pointer

		cmp	ax,0f8h                 ;Timing clock, skip it
		je	receiveNext
		cmp	ax,0feh                 ;Active sensing, skip it
		je	receiveNext
		cmp	ax,0fch                 ;Stop message, skip it
		je	receiveNext      
		cmp	ax,0fah
		jb	outMIDIget              ;Got a byte, exit
		call	SendReset
		jmp	receiveNext

outMIDIget:
                pop     ebx
		ret
MIDIget		ENDP


;**************************************************
;* Put 1 byte back into MIDI receive buffer - @INT 
;**************************************************

MIDIback1       PROC    
                push    ebx


                inc     MIDIbcount               ;Increment byte count
                mov     ebx,MIDIbhead            ;Buffer empty pointer
                dec     ebx                       ;Dec buffer empty pointer

                cmp     ebx,offset ds:MIDIbufbeg ;Test for beg. of buffer
                jae     NotAtBeg                 ;no, continue
                mov     ebx,offset ds:MIDIbufend  ;Yes, set to end
                dec     ebx
NotAtBeg:
                mov     MIDIbhead,ebx            ;Update empty buff pointer


                pop     ebx
		ret
MIDIback1       ENDP



;***********************************************************
;* Stash MIDI bytes received in driver during output - @INT
;***********************************************************


StashMIDI       PROC    far
                push    ebx
		pushf
		;cli

                cmp     install,1
                jne     EndStash

                mov     ebx,MIDIbtail    	;Buffer fill pointer
                mov     ds:[ebx],al      	;Store character in buffer
                inc     ebx              	;Bump pointer

                cmp     ebx,offset ds:MIDIbufend ;Test for buffer end
                jb      NotEnd                   ;No, continue
                mov     ebx,offset ds:MIDIbufbeg ;offset of buffer start
NotEnd:         
                mov     MIDIbtail,ebx    	;Update buffer fill pointer
                inc     MIDIbcount      	;Increment count of bytes

EndStash:
                popf
                pop     ebx
                ret
StashMIDI       ENDP


;****************************************
;* MIDI receive interrupt handler - @INT
;****************************************

                PUBLIC MPU_int

MPU_int         PROC    far
                pushfd
                push    ds
                push    es
                push    ebp
                push    edi
                push    esi
                push    edx
                push    ecx
                push    ebx
                push    eax
                
                ;Test to see if MIDI interrupt

                mov     dx,STATPORT     ;Status port
                in      al,dx           ;Read status
                test    al,DSR          ;Data ready to receive?         
                jz      BytesWaiting    ;yes, get it

                ;Pass interrupt on

                pop     eax
                pop     ebx
                pop     ecx
                pop     edx
                pop     esi
                pop     edi
                pop     ebp
                pop     es
                pop     ds

                sub     esp,8
                push    ds
                push    eax
                mov     ax,SS_DATA
                mov     ds,ax
                mov     eax,SavIntPtr
                mov     [esp+8],eax
                movzx   eax,SavIntPtrS
                mov     [esp+12],eax
                pop     eax
                pop     ds
                iretd



BytesWaiting:

                mov     dx,DATAPORT     	;Data port
                in      al,dx           	;Read byte

                mov     ebx,MIDIbtail    	;Buffer fill pointer
                mov     ds:[ebx],al      	;Store character in buffer
                inc     ebx              	;Bump pointer

                cmp     ebx,offset ds:MIDIbufend ;Test for buffer end
                jb      NotEndM                   ;No, continue
                mov     ebx,offset ds:MIDIbufbeg ;offset of buffer start
NotEndM:         
                mov     MIDIbtail,ebx    	;Update buffer fill pointer
                inc     MIDIbcount      	;Increment count of bytes
OutInt:
                mov     al,00100000b    	;EOI for 8259
                out     piccmd,al      		;Reset 8259
     
                pop     eax
                pop     ebx
                pop     ecx
                pop     edx
                pop     esi
                pop     edi
                pop     ebp
                pop     es
                pop     ds
                popfd
                iretd
MPU_int         ENDP



;************************************
;* Set MIDI receive interrupt - @INT
;************************************

Set_M_Interrupt PROC    NEAR
                push    eax
                push    ebx      
                push    ecx
                push    edx

                ;Initialize buffer pointers

                mov     MIDIbcount,0            ;Set count to zero  
                mov     eax,offset ds:MIDIbufbeg
                mov     MIDIbhead,eax           ;Empty pointer
                mov     MIDIbtail,eax           ;Fill pointer

                ;Check if already installed

                cmp     install,1
                je      EndSetM
                
                mov     install,1              ;Set install flag
                
                ;generate pic mask - mask = (1 << intlevel) ^ 0xFF

                mov     cl,2
                mov     al,1
                shl     al,cl
                not     al
                mov     imask,al

                ;Get the current interrupt handler

                push    es
                mov     ax,2502h          ;Get vector
                mov     cl,midi_level   ;midi port interrupt level
                int     21h             ;Get current vector
                mov     SavIntPtr,ebx    ;Save old vector offset
        	mov     SavIntPtrS,es   ;Save old vector segment
                pop     es

                ;Establish new handler

                push    ds
                mov     edx,offset cs:MPU_int
                mov     ax,cs
                mov     ds,ax
                mov     ax,2506h
                mov     cl,midi_level
                int     21h
                pop     ds

                ;Enable interrupt

                in      al,picmsk       ;read 8259 interrupt mask
                and     al,imask        ;enable new interrupt
                out     picmsk,al       ;output new mask

EndSetM: 
                pop     edx
                pop     ecx
                pop     ebx
                pop     eax
                ret
Set_M_Interrupt ENDP


;*******************************************
;* Disconnect MIDI receive interrupt - @INT
;*******************************************

Drop_M_Interrupt PROC    NEAR
                push    eax
                push    ebx        

		 ;if we are not already installed, skip out

                cmp     install,0
                jne     DMI_ELSE
                        jmp     OutDrop
DMI_ELSE:
                mov     install,0              ;Reset install flag

                ;Disable interrupt

                ;cli
                in      al,picmsk       ;Get 8259 interrupt mask
                mov     bl,imask        ;Get interrupt mask byte
                not     bl              ;Invert it
                or      al,bl           ;Turn off interrupt
                out     picmsk,al       ;Output mask
                sti

                ;Restore original interrupt vector
                
                 push    ds
                 mov     ax,2504h
                 mov     cl,midi_level                   
                 mov     edx,ds:SavIntPtr
                 mov     ds,ds:SavIntPtrS
                 int     21h
                 pop     ds
                
OutDrop:       
                pop     ebx
                pop     eax
                ret
Drop_M_Interrupt ENDP






;**********************************************************
;* MPUcomm - send a command byte to the MPU
;**********************************************************
MPUcomm		PROC	NEAR
		push	eax
		push	ecx
		push	edx
		mov	cx,65535  	;retry count

		mov	dx,STATPORT

waitForMPU:
		in	al,dx		;Wait for hardware DRR							;Wait for hardware DRR
		test	al,DRR
		jz	sendComm
		dec	cx
		cmp	cx,0
		jne	waitForMPU
		jmp	SHORT outMTComm	;time out

sendComm:
		mov	al,bl		;Send command to MPU						;Send command to MPU
		out	dx,al

		mov	cx,65535	;retry count for ACK
waitForMPU2:
		in	al,dx
		rol	al,1
		jnb	data2Res
		dec	cx
		cmp	cx,0
		jne	waitForMPU2
					;just fall thru if we time out...

data2Res:
		mov	dx,DATAPORT

		in	al,dx									;Signal an error if
		cmp	al,ACK		;we don't signal any error or anything					;there is no response
		je	outMTComm	;if we didn't get an ACK.....

outMTComm:
		pop	edx
		pop	ecx
		pop	eax
		ret
MPUcomm		ENDP



;***********************************************************
;* ResetMPU
;***********************************************************
ResetMPU	PROC	NEAR
		pushf
		;cli
		push	eax
		push	ebx
		push	ecx
		push	edx

tryResAgain:
		mov	cx,65535     		;retry count
		mov	bl,0ffh			;command to send
		mov	dx,STATPORT

waitForMPUres:
		in	al,dx			;Wait for hardware DRR						;Wait for hardware DRR
		test	al,DRR
		jz	sendResComm
		dec	cx
		cmp	cx,0
		jne	waitForMPUres
		jmp	SHORT outMTResComm	;time out...

sendResComm:					
		mov	al,bl			;Send command to MPU					;Send command to MPU
		out	dx,al

		mov	cx,65535		;retry count for ACK
waitForMPU2res:
		in	al,dx
		rol	al,1
		jnb	datatoRes
		dec	cx
		cmp	cx,0
		jne	waitForMPU2res

		;we just fall thru if we time out

datatoRes:
		mov	dx,DATAPORT

		in	al,dx									;Signal an error if
		cmp	al,ACK			;if we don't get an ACK we don't				;there is no response
		je	outMTResComm		;do anything about it, so......
		jmp	SHORT tryResAgain

outMTResComm:
		pop	edx
		pop	ecx
		pop	ebx
		pop	eax
		popf
		ret
ResetMPU	ENDP




;*********************************************************
;*	DoTestNode - FILL MIDI RECEIVE BUFFER
;*********************************************************


DoTestNode	PROC	NEAR
                push	ebx
		push	es
                pushf
                ;cli

;clear the MIDIbuffer to all 255's


		mov	testNode,1
		xor	esi,esi

moreMidiBuff:
		call	MIDIget	     		;get a byte
		cmp	al,255
		je	outDoTest		;none there? skip

		cmp	al,0f0h			;if it's a SysX message...
		jne	notSysEx
		call	SkipSysEx		;throw away the sysx
		jmp	SHORT moreMidiBuff	;and go get some more

notSysEx:
		cmp	al,128			;is it a command?
		jae	noBuffRunn		;yeah, skip

                mov     RunningStat,1		;set signal that we need to use running status
		mov	MIDIbuffer[esi+1],al	;save data at byte 1

		mov	al,oldRecCom		;get the Midi in running status
		mov	MIDIbuffer[esi],al	;save it at byte 0
		add	si,2			;point to free location
		jmp	SHORT putInBuff

noBuffRunn:
		;we got a command

                mov     RunningStat,0		;not using running status
		mov	oldRecCom,al		;save new Midi In running status
		mov	MIDIbuffer[esi],al	;and at byte 0
		inc	si
		call	MIDIget      		;get data byte
                cmp     al,255
                jne     Gotit1			;got it, skip

		;If complete command is not ready put byte back in buffer @INT

                call    MIDIback1
                dec     si
                JMP     SHORT outDoTest

Gotit1:
		mov	MIDIbuffer[esi],al	;save data 1 in byte 1
		inc	si

putInBuff:
		mov	bl,oldRecCom		;get the status
		and	bl,0f0h			;isolate the command
		cmp	bl,CHNLAFTER		;channel after touch
		je	oneBuffByte		;and program change have only
		cmp	bl,PCHANGE		;one data byte
		je	oneBuffByte

		call	MIDIget			;get data 2
                cmp     al,255
                jne     Gotit2			;got it, skip

		;If complete command is not ready put 2 bytes back in buffer @INT
		;or 1 byte if running status

                call    MIDIback1
                cmp     RunningStat,0
                jne     DTN_ELSE
                  call    MIDIback1    
                  dec     si
DTN_ELSE:       dec     si
                JMP     SHORT outDoTest

Gotit2:
		mov	MIDIbuffer[esi],al	;save data2 at byte 2
		inc	si
oneBuffByte:
		mov	MIDIbuffer[esi],0	;set up the next events timing to 0
		inc	si
		jmp	moreMidiBuff

outDoTest:
		cmp	si,0			;did we collect any data?
		je	noEndBuff
		dec	si			;yeah, so set up the next event's
		mov	MIDIbuffer[esi],1	;timing to 1 so we come back here to
						;try again next tick.

noEndBuff:
                popf
		pop	es	
		pop	ebx
		ret					
DoTestNode	ENDP



;**************************************************************
;* ClearMidInfo - clear the Midi interrupt buffer
;**************************************************************
;ClearMidInfo	PROC	NEAR
;notEmptyYet:
;		call	MIDIget
;		cmp	al,255
;		jne	notEmptyYet
;		ret
;ClearMidInfo	ENDP


;**************************************************************
;* SkipSysX - read MIDI data until End-Of-Sysx is found
;**************************************************************
SkipSysEx	PROC	NEAR
loopSkipSysEx:
		call	MIDIget
		cmp	al,0f7h
		jne	loopSkipSysEx
		ret
SkipSysEx	ENDP


;**************************************************************
;* SendReset - for each of 16 channels, send a NumNotes of 0
;**************************************************************
SendReset	PROC	NEAR
		;ret
		push	eax
		push	ecx

		xor	al,al
loopSendReset:
		mov	ch,NUMNOTES
		xor	cl,cl
		CallDriver DController
		inc	al
		cmp	al,16
		jne	loopSendReset

		pop	ecx
		pop	eax
		ret
SendReset	ENDP


endif ;(RecMid)



_TEXT            ENDS
		END



