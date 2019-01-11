;*************************************************************************
;*
;* Windows MIDI Driver
;*
;*************************************************************************
.386

DRIVER_I equ 1			;for midi.i

PCSPEAK	equ 1			;include code for PC speaker

OPTION	PROC:PRIVATE

;.model	medium, os_dos
;	include	macro.i
;	include driver.i
       	include midi.i

;	.CODE
_DATA    segment byte public use32 'DATA'
assume   ds:_DATA        
;	.DATA

extrn	_devType:word		;MIDI device type
PCSPEAKER equ 0
BASESYNTH equ 1
HIGHSYNTH equ 2
BASEADLIB		 equ 3

;***************************** Synthesizer stuff ************************

hMidi		dw	?		;Midi out handle
mastVol		db 1,8,17,25,34,42,50,59,67,76,84,92,101,109,118,127
masterVol	db 15 		;Master Volume 0-15
masterVolS	db 127		;Master Volume scaled to 0-127
chVol		db 16 dup (127)	;channel volumes
chVolM		db 16 dup (127)	;channel volumes with master volume applied
soundOn		db 1  		;Sound ON/OFF flag
noteTable	dw 128 dup (0)	;table of notes playing; bit set = note on
				;   entry nbr = note; bit nbr = channel

public	_chProgram

_chProgram	db	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

MidiOpen	dw	0
retry		db 0

ifdef PCSPEAK
;***************************** PC speaker stuff ************************

;Frequencies for MIDI notes 24 to 119 (including 3 pitches between semitones)

freq_tab dw   37,  37,  38,  38,  39,  39,  40,  41,  41,  42,  42,  43
         dw   44,  44,  45,  46,  46,  47,  48,  48,  49,  50,  50,  51
         dw   52,  53,  53,  54,  55,  56,  57,  57,  58,  59,  60,  61
         dw   62,  63,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72
         dw   73,  74,  76,  77,  78,  79,  80,  81,  82,  84,  85,  86
         dw   87,  89,  90,  91,  92,  94,  95,  97,  98,  99, 101, 102
         dw  104, 105, 107, 108, 110, 112, 113, 115, 116, 118, 120, 122
         dw  123, 125, 127, 129, 131, 133, 135, 137, 139, 141, 143, 145
         dw  147, 149, 151, 153, 156, 158, 160, 162, 165, 167, 170, 172
         dw  175, 177, 180, 182, 185, 188, 190, 193, 196, 199, 202, 205
         dw  208, 211, 214, 217, 220, 223, 226, 230, 233, 236, 240, 243
         dw  247, 250, 254, 258, 262, 265, 269, 273, 277, 281, 285, 289
         dw  294, 298, 302, 307, 311, 316, 320, 325, 330, 334, 339, 344
         dw  349, 354, 359, 365, 370, 375, 381, 386, 392, 398, 403, 409
         dw  415, 421, 427, 434, 440, 446, 453, 459, 466, 473, 480, 487
         dw  494, 501, 508, 516, 523, 531, 539, 546, 554, 562, 571, 579
         dw  587, 596, 604, 613, 622, 631, 640, 650, 659, 669, 679, 688
         dw  698, 709, 719, 729, 740, 751, 762, 773, 784, 795, 807, 819
         dw  831, 843, 855, 867, 880, 893, 906, 919, 932, 946, 960, 974
         dw  988,1002,1017,1031,1046,1062,1077,1093,1109,1125,1141,1158
         dw 1175,1192,1209,1227,1244,1263,1281,1300,1318,1338,1357,1377
         dw 1397,1417,1438,1459,1480,1501,1523,1545,1568,1591,1614,1637
         dw 1661,1685,1710,1735,1760,1786,1812,1838,1865,1892,1919,1947
         dw 1975,2004,2033,2063,2093,2123,2154,2186,2217,2250,2282,2316
         dw 2349,2383,2418,2453,2489,2525,2562,2599,2637,2675,2714,2754
         dw 2794,2834,2876,2917,2960,3003,3047,3091,3136,3182,3228,3275
         dw 3322,3371,3420,3469,3520,3571,3623,3676,3729,3783,3839,3894
         dw 3951,4008,4067,4126,4186,4247,4309,4371,4435,4499,4565,4631
         dw 4699,4767,4836,4907,4978,5050,5124,5198,5274,5351,5429,5507
         dw 5588,5669,5751,5835,5920,6006,6093,6182,6272,6363,6456,6550
         dw 6645,6741,6840,6939,7040,7142,7246,7352,7459,7567,7677,7789
	 dw 7902,8016,8134,8252,8372,8494,8618,8742,8870

portB		EQU	61h			;Timer/Sound port

pbvalue		dw 2000h
speed		dd 0
pbend		db 0
pbdir		db 0
ptch		db 0
;masterVol	db 15		;duplicate
;soundOn	db 1		;duplicate
;chVol		db 0		;duplicate
XchVol		db 0
vChannel	db 255
reverbMode	db 0
endif	;PCSPEAK

;	.CODE
_DATA    ENDS
_TEXT	       segment  byte public use32 'CODE'
;cseg    segment byte public use32 'CODE'
;assume cs:cseg 

; Calls to the MIDIOUT routines must be FAR PASCAL calls, which means that
; parameters are pushed in reverse order and that the callee pops the stack.
; Also, the ES register must be saved before the call and restored after.
; This seems to be necessary only because this is itself a far procedure; I
; tested the same calls from a near procedure without saving ES, and it
; worked OK.

; NOTE 6/9/94:  It was found that in low-memory situations, the MIDI Mapper
; sometimes chokes when it receives a volume change command.  For this
; reason, I handle volume changes for the high-level synth the same way as
; for a base-level; by scaling the Note On velocities.

; NOTE 9/28/94: Mark Seibert requested that the above-referenced change be
; backed out.  Scaling velocities for high-level synth does not work.  If
; the low-memory problem with volume changes re-appears, we will have to find
; another solution.

; NOTE 1/27/95: Problem showed up again in Phantasm.  Have re-instated
; scaling velocities for high-level.
; To make it easier, this is now controlled with the following define.
; Comment it out to scale velocities.
; Un-comment it to allow volume changes.

USEVOLCHANGE	equ 1


extrn	MIDIOUTGETNUMDEVS:NEAR
extrn	MIDIOUTOPEN:NEAR
extrn	MIDIOUTCLOSE:NEAR
extrn	MIDIOUTSHORTMSG:NEAR


	org	0
				
PUBLIC	Driver
PUBLIC	OpenMIDI_
PUBLIC	CloseMIDI_
;public	wbreak


JumpTable:
	dd	offset _TEXT:PatchReq 	 	;_PatchReq
	dd	offset _TEXT:Init      		;_Init
	dd	offset _TEXT:Terminate 		;_Terminate
	dd	offset _TEXT:NoSupport 		;_Service
	dd	offset _TEXT:NoteOff   		;_NoteOff	
	dd	offset _TEXT:NoteOn    		;_NoteOn
	dd	offset _TEXT:NoSupport 		;_PolyAfterTch
	dd	offset _TEXT:Controller		;_Controller
	dd	offset _TEXT:ProgramChange 	;_ProgramChange
	dd	offset _TEXT:NoSupport 		;_ChnlAfterTch
	dd	offset _TEXT:PitchBend 		;_PitchBend
	dd	offset _TEXT:SetReverb 		;_SetReverb
	dd	offset _TEXT:MasterVol 		;_MasterVol
	dd	offset _TEXT:SoundOn 		;_SoundOn
	dd	offset _TEXT:NoSupport 		;_SamplePlay
	dd	offset _TEXT:NoSupport 		;_SampleEnd
	dd	offset _TEXT:NoSupport 		;_SampleCheck
	dd	offset _TEXT:AskDriver 		;_AskDriver

ifdef PCSPEAK
;	For PC speaker
JumpTableSpk:
	dd	offset _TEXT:XPatchReq		;_PatchReq
	dd	offset _TEXT:XInit		;_Init
	dd	offset _TEXT:XTerminate		;_Terminate
	dd	offset _TEXT:NoSupport		;_Service
	dd	offset _TEXT:XNoteOff		;_NoteOff	
	dd	offset _TEXT:XNoteOn		;_NoteOn
	dd	offset _TEXT:NoSupport		;_PolyAfterTch
	dd	offset _TEXT:XController	;_Controller
	dd	offset _TEXT:NoSupport		;_ProgramChange
	dd	offset _TEXT:NoSupport		;_ChnlAfterTch
	dd	offset _TEXT:XPitchBend		;_PitchBend
	dd	offset _TEXT:XSetReverb		;_SetReverb
	dd	offset _TEXT:XMasterVol		;_MasterVol
	dd	offset _TEXT:XSoundOn		;_SoundOn
	dd	offset _TEXT:NoSupport		;_SamplePlay
	dd	offset _TEXT:NoSupport		;_SampleEnd
	dd	offset _TEXT:NoSupport		;_SampleCheck
	dd	offset _TEXT:XAskDriver		;_AskDriver
endif	;PCSPEAK
	       


;***********************************************************************
Driver		PROC	NEAR
;_Driver		PROC	NEAR
Start:
		push	es
		push	esi
		push	ebx
		push	ebp

		movzx	ebp, bp
		shl	bp,2
ifdef PCSPEAK
		.if	_devType == PCSPEAKER
			mov	ebp,dword ptr cs:JumpTableSpk[ebp]
		.else
			mov	ebp,dword ptr cs:JumpTable[ebp]
		.endif
else
	  	mov	ebp,dword ptr cs:JumpTable[ebp]
endif
		call	ebp

		pop	ebp
		pop	ebx
		pop	esi
		pop	es
    		ret
Driver		ENDP
;***********************************************************************


NoSupport PROC	NEAR
	  ret
NoSupport ENDP



PatchReq PROC	NEAR		;DETERMINE PATCH REQUIREMENTS

; in:	none

; out:	al = patch file nbr (-1 for no patch)
;	ah = support flags
;	cl = nbr of voices
;	ch = device id

	mov	al, 0ffh	;no patch
	mov	ah, 01h		;MIDI support, no DACs
	.if	_devType == BASESYNTH
		mov	ch, 11
		mov	cl, 6
	.endif
	.if	_devType == HIGHSYNTH
		mov	ch, 7
;		mov	ch, 12		;"m"
		mov	cl, 16
	.endif
	.if	_devType == BASEADLIB
		mov	ch, 0
		mov	cl, 9
	.endif
	ret

PatchReq ENDP



Init	PROC	NEAR		;INITIALIZE THE DRIVER

; in:	es:ax = pointer to patch data

; out:	ax = new size of driver
;	cl = lowest MIDI channel
;	ch = highest MIDI channel


	mov	eax, offset hMidi
	push	eax    		;pointer to handle
	mov	eax, -1		;device nbr (MIDI Mapper)
	push	eax
	xor	eax, eax
	push	eax    		;callback is null
	push	eax    		;instance is null
	push	eax    		;flags are null
wbreak::
	call	MIDIOUTOPEN
	mov	MidiOpen, 1

	call	SetPitchBend

	.if	_devType == BASESYNTH
		mov	cx, 12		;low channel range
		mov	ax, 14		;high channel range
	.endif
	.if	_devType == HIGHSYNTH
		mov	cx, 0
		mov	ax, 8
	.endif
	.if	_devType == BASEADLIB
		mov	cx, 0
		mov	ax, 15
		mov	_devType, BASESYNTH		;now, treat as base-level
	.endif
	mov	ch, al
	mov	ax,offset _TEXT:Init	;Return new size of driver
	ret

Init	ENDP



Terminate PROC	NEAR		;END SONG

; in:	none
; out:	none

	.if	MidiOpen
		call	ClearAll
		xor	eax, eax
		mov	ax, hMidi
		push	eax
		call	MIDIOUTCLOSE
		mov	MidiOpen, 0
	.endif
	ret

Terminate ENDP



NoteOff	PROC	NEAR 		;TURN OFF NOTE

; in:	al = channel
;	ch = note number
;	cl = velocity

	push	ax
;	.if	_devType != HIGHSYNTH
		call	setNoteOff	;reset table entry
;	.endif
	mov	ah, NOTEOFF
	call	StdShortMsg
	pop	ax
	ret

NoteOff	ENDP



NoteOn 	PROC	NEAR		;TURN NOTE ON

; in:	al = channel
;	ch = note number
;	cl = velocity

; conditional commented out 6/9/94 in order to also scale for HIGHSYNTH
; conditional un-commented 9/28/94 per Mark S.
ifdef USEVOLCHANGE
	.if	_devType == BASESYNTH
		jmp	noteonbase
	.endif
else
	jmp	noteonbase
endif

;	high-level synth, no general midi
	push	ax
	call	setNoteOn	;set table entry
	mov	ah, NOTEON
	call	StdShortMsg
	pop	ax
	ret


;	base-level synth
noteonbase:
	push	ax
;	.if	_devType != HIGHSYNTH
		call	setNoteOn	;set table entry
;	.endif
	mov	ah, 0
	movzx	esi, ax	 	;si is channel index
	mov	al, chVolM[esi]	;get channel volume
	mul	cl		;apply it to velocity
	mov	cl, 7
	shr	ax, cl
	mov	cl, al
	pop	ax		;restore channel in al
	push	ax
	mov	ah, NOTEON
	call	StdShortMsg
	pop	ax
	ret

NoteOn 	ENDP



public Controller
Controller PROC	NEAR

; in:	al = channel
;	ch = controller number
;	cl = value

	push	ax
	push	cx

	.if	ch == 7			;channel volume?
		jmp	short ctlChanVol
	.endif

	.if	ch == 123		;clear channel?
		call	chanClear	;yes, turn off all notes
		jmp	endControl
	.endif

; Just send the controller message	
	mov	ah, CONTROLLER
	call	StdShortMsg
	jmp	endControl

; Controller 7 - set channel volume
ctlChanVol:

; no general midi re-mapping
	mov	ah, 0			;set channel volume
	movzx	esi, ax
	mov	chVol[esi], cl

	cmp	soundOn,0		;if sound off, that's it
	je	endControl

	mov	al, masterVolS		;apply master volume
	mul	cl
	mov	cl, 7
	shr	ax, cl
	mov	chVolM[esi], al		;set modified channel volume

; code commented out 6/9/94 in order to not send volume change for HIGHSYNTH
; code un-commented 9/28/94 per Mark S.
ifdef USEVOLCHANGE
	.if	_devType == HIGHSYNTH	;send modified volume to device
		pop	cx
		pop	ax
		push	ax
		push	cx
		mov	ah, CONTROLLER
		mov	cl, chVolM[esi]
		call	StdShortMsg
	.endif
endif

endControl:
	pop	cx
	pop	ax

	ret

Controller ENDP



ProgramChange	PROC	NEAR

; in:	al = channel
;	cl = program nbr

	push	ax
	movzx	esi,ax
	and	si,0ffh
	mov	_chProgram[esi],cl
	.if	al == 10	;map low-synth perc on chan 10 to chan 15
		mov	al, 15
	.endif
	or	al, PCHANGE 	;combine command and channel in byte 1
	mov	ah, cl		;program number
	xor	dx, dx
	call	SendShortMsg
	pop	ax
	ret

ProgramChange	ENDP



PitchBend PROC	NEAR	    	;PITCH BEND NOTE

; in:	al = channel
;	ch = low 7 bits
;	cl = high 7 bits

	push	ax

	mov	ah, PBEND
	call	StdShortMsg

	pop	ax

	ret

PitchBend ENDP



SetReverb PROC	NEAR

; in:	cl = new reverb mode value (255 is ignored)
;	ax = previous value

	ret

SetReverb ENDP



MasterVol PROC	NEAR

; in:	cl = new master volume (255 is ignored)
; out:	ax = previous volume


	push	cx

      	mov	al,masterVol		;previous value for return
      	xor	ah,ah

	.if	cl == 255		;info request only if 255
		jmp	short outMasterVol
	.endif

	mov	masterVol,cl
	cmp	soundOn,0
	je	outMasterVol
		
	push	ax

	mov	ch,0			;Scale master volume
	movzx	esi,cx
	mov	al,mastVol[esi]
	mov	masterVolS, al		;save it

; now apply the new master volume to all 16 channels

	mov	dl, al			;preserve scaled master volume
	mov	dh, 0
	mov	si, 16			;set channel count

chanloop:
	dec	si
	mov	cl, chVol[esi]		;get channel volume
	mov	ax, dx			;apply master volume
	mul	cl
	mov	cl, 7
	shr	ax, cl
	mov	chVolM[esi], al		;set modified channel volume

; conditional commented out 6/9/94 in order to also scale for HIGHSYNTH
; conditional un-commented 9/28/94 per Mark S.
ifdef USEVOLCHANGE
	.if	_devType == HIGHSYNTH	;send control msg
		push	ax
		push	cx
		push	dx
		push	si
		mov	ax, si		;channel
		mov	ah, CONTROLLER
		mov	ch, 7		;controller number
		mov	cl, chVolM[esi]	;modified volume
		call	StdShortMsg
		pop	si
		pop	dx
		pop	cx
		pop	ax
	.endif
endif

	cmp	si, 0			;finished all channels?
	jne	chanloop

	pop	ax

outMasterVol:
	pop	cx
	ret

MasterVol ENDP



SoundOn PROC	NEAR

; in:	cl = 0 for off, 1 for on, 255 is ignored
;	ax = previous state of on/off

	xor	ah,ah
	mov	al,soundOn
	cmp	cl,255
	jne	notSoReq
	ret

notSoReq:
	push	cx
	push	ax
	cmp	cl,0			;Set MasterVol to 0 to turn
	jne	turnSoundOn		;sound off
	mov	soundOn,0
	xor	ax, ax			;set volume to 0
	jmp	short setit

turnSoundOn:
	mov	soundOn,1		;sound back on
	mov	ah,0
	mov	al,masterVol
	movzx	esi,ax
	mov	al,mastVol[esi]

; now apply the new master volume to all 16 channels

setit:
	mov	dl, al			;preserve scaled master volume
	mov	dh, 0
	mov	si, 16			;set channel count

chanloop2:
	dec	si
	mov	cl, chVol[esi]		;get channel volume
	mov	ax, dx			;apply master volume
	mul	cl
	mov	cl, 7
	shr	ax, cl
	mov	chVolM[esi], al		;set modified channel volume

; conditional commented out 6/9/94 in order to also scale for HIGHSYNTH
; conditional un-commented 9/28/94 per Mark S.
ifdef USEVOLCHANGE
	.if	_devType == HIGHSYNTH	;send control msg
		push	ax
		push	cx
		push	dx
		push	si
		mov	ax, si		;channel
		mov	ah, CONTROLLER
		mov	ch, 7		;controller number
		mov	cl, chVolM[esi]	;modified volume
		call	StdShortMsg
		pop	si
		pop	dx
		pop	cx
		pop	ax
	.endif
endif

	cmp	si, 0			;finished all channels?
	jne	chanloop2

	pop	ax
	pop	cx
	ret

SoundOn ENDP



AskDriver PROC	NEAR

; in:	al = channel
;	ah = information selector
;	ch = additional selection data

; out:	ax = value of requested field

	ret

AskDriver ENDP



OpenMIDI_	PROC	NEAR

	.if	MidiOpen
		ret
	.endif
	mov	eax, offset hMidi
	push	eax    		;pointer to handle
	mov	eax, -1		;device nbr (MIDI Mapper)
	push	eax
	xor	eax, eax
	push	eax    		;callback is null
	push	eax    		;instance is null
	push	eax    		;flags are null
	call	MIDIOUTOPEN
	mov	MidiOpen, 1
	call	SetPitchBend
	ret

OpenMIDI_	ENDP



CloseMIDI_	PROC	NEAR

	.if	MidiOpen
		call	ClearAll
		xor	eax, eax
		mov	ax, hMidi
		push	eax
		call	MIDIOUTCLOSE
		mov	MidiOpen, 0
	.endif
	ret

CloseMIDI_	ENDP




StdShortMsg PROC NEAR
;***********************************************************
; Send MIDI short message in "standard form", i.e.

;	ah = command
;	al = channel
;	ch = byte 2
;	cl = byte 3
;***********************************************************

	push	ax
	push	bx
	push	cx
	push	dx

	.if	al == 10	;map low-synth perc on chan 10 to chan 15
		mov	al, 15
	.endif
	or	al, ah		;combine command and channel in byte 1
	mov	ah, ch		;byte 2
	xor	dx, dx
	mov	dl, cl		;byte 3
	call	SendShortMsg

	pop	dx
	pop	cx
	pop	bx
	pop	ax

	ret

StdShortMsg ENDP



SendShortMsg PROC NEAR
;***********************************************************
; Send a MIDI short message

; input: al = byte 1 (command and channel)
;        ah = byte 2
;        dl = byte 3
;        dh = byte 4 (usually 0)
;***********************************************************

	.if	!MidiOpen
		ret
	.endif

	mov	retry, 64	;set retry count
	mov	bx, ax		;need ax for result

tryagain:
	push	bx				;save in case we have to re-send
	push	dx

	xor	eax, eax
	mov	ax, hMidi
	push	eax
	push	dx
	push	bx
	call	MIDIOUTSHORTMSG

	pop	dx
	pop	bx
	dec	retry
	.if	(ax && retry)
		jmp	tryagain
	.endif
	ret

SendShortMsg ENDP


setNoteOn	PROC	NEAR

; Set note bit on in table
; in:	al = channel
;	ch = note number

	push	bx
	push	cx

	mov	ebx, 1 			;set mask bit to channel nbr
	mov	cl, al
	shl	bx, cl
	mov	cl, ch			;set index for note table
	mov	ch, 0
	xchg	bx, cx
	or	noteTable[ebx], cx	;set the bit

	pop	cx
	pop	bx

	ret

setNoteOn	ENDP


setNoteOff	PROC	NEAR

; Set note bit off in table
; in:	al = channel
;	ch = note number

	push	bx
	push	cx

	mov	ebx, 1 			;set mask bit to channel nbr
	mov	cl, al
	shl	bx, cl
	not	bx
	mov	cl, ch			;set index for note table
	mov	ch, 0
	xchg	bx, cx
	and	noteTable[ebx], cx	;reset the bit

	pop	cx
	pop	bx

	ret

setNoteOff	ENDP


chanClear	PROC	NEAR

; Turn off all notes on channel al

;	.if	_devType == HIGHSYNTH	;just send controller 123
;		push	ax
;		push	cx
;		mov	ah, CONTROLLER
;		mov	ch, 123
;		mov	cl, 0	
;		call	StdShortMsg
;		pop	cx
;		pop	ax
;		ret
;	.endif

	push	bx
	push	cx

	mov	ebx, 1 			;set mask bit to channel nbr
	mov	cl, al
	shl	bx, cl
	mov	cx, bx

	mov	bx, 0

nextnote:
	test	noteTable[ebx], cx	;note on?
	jz	bumpbx
	push	cx			;yes, turn it off
	mov	ch, bl
	mov	cl, 0
	call	NoteOff
	pop	cx

bumpbx:
	inc	bx     			;bump note index
	.if	bx < 128		;done?
		jmp short nextnote	;no, do next
	.endif

	pop	cx
	pop	bx
	ret

chanClear	ENDP


ClearAll PROC NEAR

; Turn off all notes on all channels

	push	ax

	mov	al, 0		;channel

nextchan:
	call	chanClear
	inc	al
	.if	al < 16
		jmp	short nextchan
	.endif

	pop	ax
	ret

ClearAll ENDP


SetPitchBend PROC NEAR

; Send pitchbend messages to all channels

	.if	_devType != HIGHSYNTH
		ret
	.endif

	push	ax

	mov	al, 0		;channel

nextpitch:
		mov	ch, 101
		mov	cl, 0	
		call	Controller
		mov	ch, 100
		mov	cl, 0	
		call	Controller
		mov	ch, 6
		mov	cl, 12	
		call	Controller

	inc	al
	.if	al < 16
		jmp	short nextpitch
	.endif

	pop	ax
	ret

SetPitchBend ENDP


ifdef PCSPEAK
;************************* routines for PC speaker *******************

XNoteOff	PROC	NEAR			;TURN A NOTE OFF
		cmp	ptch,ch
		jne	overLap
		call	SpeakerOff
overLap:	ret
XNoteOff	ENDP


XNoteOn		PROC	NEAR			;TURN A NOTE ON		
;wbreak1::
		cmp	vChannel,al
		je	gotAVoice
		ret

gotAVoice:	cmp	ch,0			;STD driver should just blow
		jne	notNote0		;off note 0
		ret

notNote0:	push	bx

		mov	bl,ch
		xor	bh,bh
		call	SpeakerOff
		call	SpeakerOn

		pop	bx
		ret
XNoteOn		ENDP


XController	PROC	NEAR			;PROCESS CONTROLLERS
		cmp	ch,ALLNOFF
		jne	notAllNoff
		call	SpeakerOff
		jmp	SHORT outContrlr

notAllNoff:	cmp	ch,NUMNOTES
		jne	notNumNotes
		cmp	cl,0
		je	releaseVoice
		cmp	vChannel,al
		je	outContrlr
		call	SpeakerOff
		mov	vChannel,al
		jmp	SHORT outContrlr
releaseVoice:	cmp	vChannel,al
		jne	outContrlr
		call	SpeakerOff
		mov	vChannel,255
		jmp	SHORT outContrlr

notNumNotes:	cmp	ch,CURNOTE
		jne	notCurNote
		cmp	cl,255
		je	outContrlr
		cmp	vChannel,al
		jne	outContrlr
		push	bx
		mov	bl,cl
		xor	bh,bh
		call	SpeakerOff
		call	SpeakerOn
		pop	bx
		jmp	SHORT outContrlr

notCurNote:	cmp	ch,VOLCTRL
		jne	outContrlr
		mov	XchVol,1
		cmp	cl,0
		jne	outContrlr
		call	SpeakerOff
		mov	XchVol,0

outContrlr:	ret
XController	ENDP


XPitchBend	PROC	NEAR
		push	ax
		push	bx
		push	cx
		push	dx

		push	cx			;Save pitch bend value in
		xchg	ch,cl			;pbvalue
		shr	ch,1
		jnc	noHiSaveBit
		or	cl,10000000b
noHiSaveBit:	mov	pbvalue,cx
		pop	cx

		cmp	vChannel,al		;Is the PC speaker allocated
		jne	noNote			;to this channel?
		
		mov	pbdir,0			;Determin distance from
		mov	ch,cl			;center
		mov	cl,0
		shr	ch,1
		jnc	noHiPitchBit
		mov	cl,10000000b
noHiPitchBit:	cmp	cx,2000h
		jne	notCenter
		mov	pbend,0
		jmp	short doBend
notCenter:	jbe	bendDown
		mov	ax,cx
		sub	ax,2000h
		mov	pbdir,1
		jmp	short calcBend
bendDown:	mov	ax,2000h
		sub	ax,cx

calcBend:	mov	bx,171			;Calculate bend value
		mov	dx,0
		div	bx
		mov	pbend,al

doBend:		cmp	ptch,0			;If there's a note on,
		je	noNote			;send new frequency
		mov	bh,0
		mov	bl,ptch
		call	SpeakerOn

noNote:		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret
XPitchBend	ENDP



SpeakerOff	PROC	NEAR			;TERMINATE SPEAKER
		cmp	ptch,0
		je	outSpkrOff
		push	ax
		ifdef	nec
		 push	dx
		 mov	ah,18h
		 int	18h
		 pop	dx
		else
		 in	al,portB
		 and	al,0fch
		 out	portB,al
		endif
		pop	ax
		mov	ptch,0

outSpkrOff:	ret
SpeakerOff	ENDP



SpeakerOn	PROC	NEAR			;GENERATE TONE
		push	ebx
		movzx	ebx, bx

		cmp	bl,24			;Insure that the pitch is
		jae	okPtch1			;in range
		jmp	short outSpkrOn
okPtch1:	cmp	bl,119
		jbe	okPtch2
		jmp	short outSpkrOn
okPtch2:	mov	ptch,bl

		sub	bl,24			;If there is a pitch bend
		shl	bx,1			;value, bend the note
		shl	bx,1
		cmp	pbend,0
		je	noPBend
		call	PBAdjust
noPBend:	cmp	bx,0ffffh
		jne	okPtch3
		jmp	short outSpkrOn
okPtch3:	shl	bx,1

		cmp	masterVol,0		;If Master volume is at
		je	outSpkrOn		;zero, Channel volume is at
		cmp	XchVol,0		;zero, or sound is off,
		je	outSpkrOn		;don't play any notes
		cmp	soundOn,0
		je	outSpkrOn

		ifdef nec
		 mov	al,076h
		 mov	dx,TIMER1_MODE
		 out	dx,al			;Write timer mode register

		 mov	dx,word ptr speed[2]	;Timer divisor =
		 mov	ax,word ptr speed[0]	;	speed/Frequency
		 mov	di,freq_tab[bx]	;Look up frequency in table
		 div	di
		 mov	dx,TIMER1_CNT
		 out	dx,al			;Write Timer 1 count low byte
		 mov	al,ah  
		 out	dx,al			;Write Timer 1 count high byte

		 mov	ah,17h			;Turn on speaker
		 int	18h
		else
		 mov	al,0b6h
		 out	43h,al			;Write timer mode register
		 mov	dx,14h			;Timer divisor =
		 mov	ax,4f38h		;	1331000/Frequency
		 mov	di,freq_tab[ebx]	;Look up frequency in table
		 div	di
		 out	42h,al			;Write Timer 2 count low byte
		 mov	al,ah
		 out	42h,al			;Write Timer 2 count high byte
		 in	al,portB		;Get current Port B setting
		 or	al,3			;Turn on speaker
		 out	portB,al
		endif

outSpkrOn:	pop	ebx
		ret
SpeakerOn	ENDP


PBAdjust	PROC	NEAR			;ADJUST FREQUENCY FOR PBEND
		push	cx
		
		mov	ch,0			;Bend the frequency
		mov	cl,pbend
		cmp	pbdir,0
		je	notPbGreat
		add	bx,cx
		jmp	short chkRange
notPbGreat:	sub	bx,cx

chkRange:	cmp	bx,24			;Insure that the new frequency
		jb	notPtchOk		;is in range
		cmp	bx,476
		ja	notPtchOk
		jmp	short outPbAdj
					
notPtchOk:	mov	bx,0ffffh

outPbAdj:	pop	cx
		ret
PBAdjust	ENDP


XTerminate	PROC	NEAR			;TERMINATE SOUND DRIVER
		call	SpeakerOff
		ret
XTerminate	ENDP


XMasterVol	PROC	NEAR			;CHANGE MASTER VOLUME
		mov	al,masterVol
		xor	ah,ah
		cmp	al,0
		je	chkJustRet
		mov	al,1	

chkJustRet:	cmp	cl,255
		jne	doMasterVol
		ret

doMasterVol:	mov	masterVol,cl		;If volume is zero, turn
		cmp	cl,0			;sound off
		jne	not0Vol
		call	SpeakerOff
not0Vol:	ret
XMasterVol	ENDP


XSetReverb	PROC	NEAR			;RETURN REVERB STATE
		mov	al,reverbMode
		xor	ah,ah
	
		cmp	cl,255
		jne	storeRev
		ret

storeRev:	mov	reverbMode,cl
		ret
XSetReverb	ENDP


XSoundOn	PROC	NEAR			;TURN SOUND ON/OFF
		mov	al,soundOn
		xor	ah,ah
	
		cmp	cl,255
		jne	doSoundOn
		ret

doSoundOn:	cmp	cl,0
		jne	XturnSoundOn
		mov	soundOn,0
		call	SpeakerOff
		ret

XturnSoundOn:	mov	soundOn,1
		ret
XSoundOn	ENDP


XAskDriver	PROC	NEAR			;RETURN CHANNEL VALUES
		cmp	ah,0e0h
		jne	notPbend
		mov	ax,pbvalue
		ret

notPbend:	cmp	ah,0b0h
		jne	outAsk
		cmp	ch,NUMNOTES
		jne	outAsk
		cmp	cl,255
		jne	outAsk
		cmp	al,vChannel
		jne	outNumNotes
		mov	ax,1
		ret
outNumNotes:	xor	ax,ax
		ret

outAsk:		mov	ax,0ffffh
		ret
XAskDriver	ENDP


XInit		PROC	NEAR			;INITIALIZE THE DRIVER
		ifdef nec
		 mov	dx,TIMER1_MODE
		 mov	al,76h
		 out	dx,al

		 xor	ax,ax
		 mov	es,ax
		 mov	bx,501h
		 mov	al,es:[bx]
		 test	al,10000000b
		 jnz	nec1
		 mov	word ptr speed[2],25h
		 mov	word ptr speed[0],8000h
		 mov	x,04cdh
		 jmp	nec2
nec1:		 mov	word ptr speed[2],1eh
		 mov	word ptr speed[0],7800h
		 mov	ax,03e6h
nec2:		 mov	dx,TIMER1_CNT
		 out	dx,al
		 mov	al,ah
		 out	dx,al
		endif
		mov	ax,offset _TEXT:Init	;Return new size of driver,
		mov	cl,0			;and channel range (1-16)
		mov	ch,15
		ret
XInit		ENDP


XPatchReq	PROC	NEAR			;DETERMINE PATCH REQUIREMENTS
		mov	ah,00000001b		;MIDI data only
		mov	al,0ffh			;No patch requirements
		mov	ch,18			;Device # 18
		mov	cl,1			;1 voice
		ret
XPatchReq	ENDP

endif	;PCSPEAK


;cseg            ENDS
_TEXT		ENDS
		END


