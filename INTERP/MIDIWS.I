;-------------------------------------------------------------------------
;-			       Structures
;-------------------------------------------------------------------------

ifndef MIDI_1 
;-------------------------------------------------------------------------
;-			   Sound Node Definition
;-------------------------------------------------------------------------

Sound		STRUC

; this word is only here because in C++, Sound is derived from SOL_Node
; and WATCOM 9.5 insists on adding an extraneous byte because of it
;SOLinfo	dw 0

;sNext		dw 0		;Node header
;sPrev		dw 0		;Node header
;sKey		dw 0		;Node header
;sNumber		dw 0		;SCI sound number
;sPointer	dd 0		;Far pointer to sound resource
;xPointer	dd 0		;Far pointer to sound resource
sMidiPtr32	dd 0		;32 bit pointer to midi data
sMidiPtr16	dd	0		;16:16  pointer to midi data
tIndex		dw 16 dup (0)	;Index for 16 tracks
tLoopPoint	dw 16 dup (0)	;Loop RPoint for 16 tracks
tRest			dw 16 dup (0)	;Rest count for 16 tracks
tLoopRest	dw 16 dup (0)	;Rest count at loop for 16 tracks
tChannel		db 16 dup (0)	;Channel #'s for 16 tracks
tCommand		db 16 dup (0)	;Last command for 16 tracks (running status) 
tLoopCommand	db 16 dup (0)	;Command at loop for 16 tracks
cDamprPbend	dw 15 dup (0)	;Damper pedal and pitch bend for 15 channels
cPriVoice	db 15 dup (0)	;Priority & voice allocation for 15 channels
cModulation	db 15 dup (0)	;Modulation values for 15 channels
cPan			db 15 dup (0)	;Pan values for 15 channels
cVolume		db 15 dup (0)	;Volume values for 15 channels
cProgram		db 15 dup (0)	;Program values for 15 channels
cCurNote		db 15 dup (0)	;Current note being played
cFlags		db 15 dup (0)	;Channel Flags (Locked, Ghost)
cMute			db 15 dup (0)	;Channel mute counter (0 = not muted)
sDataInc		dw 0		;Current Data Increment cue value
sTimer		dw 0		;Age of sound (in 60th secs)
sLoopTime	dw 0		;Timer loop point
sSignal		db 0		;Sound signal
sState		db 0		;Flag is set if sound is playing
sHold			db 0		;Sound hold/release flag
sFixedPri	db 0		;Flags not to use sound file priority
sPriority	db 0		;Sound priority
sLoop			db 0		;Sound loops
sVolume		db 0		;Sound volume
sReverbMode	db 0		;Reverb setting
sFadeDest	db 0		;Fade destination
sFadeTicks	db 0		;Clock ticks before next fade
sFadeCount	db 0		;Fade tick counter
sFadeSteps	db 0		;Fade steps
sPause		db 0		;Pause flag
Sound		ENDS


	endif

;-------------------------------------------------------------------------
;-			       Macros
;-------------------------------------------------------------------------

;	include	midihnd.i
	
Dispatch	MACRO				;DISPATCH TO DRIVER FUNCTION
		push	dx
		shl	bp,1
		mov	dx,word ptr cs:JumpTable[bp]
		call	dx
		pop	dx
		ENDM

entr		MACRO				;SAVE REGS
		pushf
		push	bp
		push	si
		push	di
		push	bx
		push	dx
		push	ds
		push	es
		ENDM


retp		MACRO				;RESTORE REGS
		pop	es
		pop	ds
		pop	dx
		pop	bx
		pop	di
		pop	si
		pop	bp
		popf
		ret
		ENDM
		
;-------------------------------------------------------------------------
;-			      Global Equates
;-------------------------------------------------------------------------

SOUND		EQU	84h
;MIDI Parser Function Definitions

SPatchReq	EQU 0
SInit		EQU 1
STerminate	EQU 2
SProcess	EQU 3
SSoundOn	EQU 4
SRestore	EQU 5
SMasterVol	EQU 6
SSetReverb	EQU 7
SPlay		EQU 8
SEnd		EQU 9
SPause		EQU 10
SFade		EQU 11
SHold		EQU 12
SMute		EQU 13
SChangeVol	EQU 14
SChangePri	EQU 15
SGetSignal	EQU 16
SGetDataInc	EQU 17
SGetSMPTE	EQU 18
SNoteOff	EQU 19
SNoteOn		EQU 20
SController	EQU 21
SPChange	EQU 22
SPBend		EQU 23
SAskDriver	EQU 24
SSndServer	EQU 24
SGetSignalRset	EQU 25


;Driver Function Definitions

DPatchReq	EQU 0
DInit		EQU 1
DTerminate	EQU 2
DService	EQU 3
DNoteOff	EQU 4
DNoteOn		EQU 5
DPolyAfterTch	EQU 6
DController	EQU 7
DProgramChange	EQU 8
DChnlAfterTch	EQU 9
DPitchBend	EQU 10
DSetReverb	EQU 11
DMasterVol	EQU 12
DSoundOn	EQU 13
DSamplePlay	EQU 14
DSampleEnd	EQU 15
DSampleCheck	EQU 16
DAskDriver	EQU 17



;MIDI Commands

NOTEOFF		EQU 080h	;Note off
NOTEON		EQU 090h	;Note on
POLYAFTER	EQU 0a0h	;Poly aftertouch
CONTROLLER	EQU 0b0h	;Controller
PCHANGE		EQU 0c0h	;Program change
CHNLAFTER	EQU 0d0h	;Channel aftertouch
PBEND		EQU 0e0h	;Pitch bend
SYSEX		EQU 0f0h	;System exclusive (start of message)
EOX		EQU 0f7h	;System exclusive (end of transmission)
TIMINGOVER	EQU 0f8h	;Timing overflow (counts as 240)
ENDTRK		EQU 0fch	;End of track



;Controllers

MODCTRL		EQU 1		;Modulation controller
VOLCTRL		EQU 7		;Volume controller
PANCTRL		EQU 10		;Pan controller
DAMPRCTRL	EQU 64		;Sustain pedal controller
NUMNOTES	EQU 75		;Maximum note reassignment controller
RESTARTFLAG	EQU 76		;Song "restart on pause" controller
CHPRIORITY	EQU 77		;Channel priority (Amiga/Mac)
CURNOTE		EQU 78		;Current single voice note
MUTECTRL	EQU 78		;Channel mute controller
CHNLSET		EQU 79		;Play track on specified channel
REVERBMODE	EQU 80		;Reverb setting controller (MT-32)
NOISECTRL	EQU 81		;SSG noise controller (NEC-9801 FM card)
ENDPOINT	EQU 82		;Loop end point
DATAINC		EQU 96		;Data increment (cues)
ALLNOFF		EQU 123		;All notes off controller



;MPU-401 commands

STATPORT	EQU 0331h	;Status port
DATAPORT	EQU 0330h	;Data port
DSR		EQU 080h	;Data set ready bit mask
DRR		EQU 040h	;Data receive ready bit mask
ACK		EQU 0feh	;Acknowledge signal
MPU_RESET	EQU 0ffh	;Reset command
DATA_WRT	EQU 0d0h	;Data write command
EXCL_WRT	EQU 0dfh	;System exclusive command
STARTREC	EQU 022h	;Start MPU recording
STOPREC		EQU 011h	;Stop MPU recording
