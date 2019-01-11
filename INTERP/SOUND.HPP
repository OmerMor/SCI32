// sound.hpp

#ifndef SOUND_HPP
#define SOUND_HPP

#ifndef LIST_HPP
#include "list.hpp"
#endif

#ifndef RESTYPE_HPP
#include "restype.hpp"
#endif

#ifndef SAVEABLE_HPP
#include "saveable.hpp"
#endif

extern	Bool		soundInstalled;
extern	int		numberOfVoices;

struct SoundMgr : Saveable {
	void	SaveRestoreBefore(SaveGame*, Bool save);
	void	SaveRestoreAfter(SaveGame*, Bool save);
	void	SaveRestoreDo(Saveable::Func, SaveGame*, Bool save);
} extern soundMgr;

class ObjectID;

struct SNodeData {
#ifndef WIN32S
	char far *	sMidiPtr32;				//far pointer to DLL's copy of MIDI data
#else
	char*			sMidiPtr32;				//far pointer to DLL's copy of MIDI data
#endif
	char*			sMidiPtr16;				//for use by WIN32S
	ushort 		tIndex[16];
	ushort		tLoopPoint[16];
	ushort		tRest[16];
	ushort		tLoopRest[16];
	uchar			tChannel[16];
	uchar			tCommand[16];
	uchar			tLoopCommand[16];
	ushort		cDamprPbend[15];
	uchar			cPriVoice[15];
	uchar			cModulation[15];
	uchar			cPan[15];
	uchar			cVolume[15];
	uchar			cProgram[15];
	uchar			cCurNote[15];  
	uchar			cFlags[15];  
	uchar			cMute[15];  
	ushort		sDataInc;
	ushort		sTimer;
	ushort		sLoopTime;
	uchar			sSignal;
	uchar			sState;
	uchar			sHold;
	uchar			sFixedPri;
	uchar			sPriority;
	uchar			sLoop;
	uchar			sVolume;
	uchar			sReverbMode;
	uchar			sFadeDest;
	uchar			sFadeTicks;
	uchar			sFadeCount;
	uchar			sFadeSteps;
	uchar			sPause;
};

struct Sound : SOL_Node {
	ResNum		sNumber;
	uchar			sSample;			//not MIDI-specific
	char*			sPointer;		//near pointer to MIDI data resource
	SNodeData far *sDataPtr;	//far pointer to DLL's node data
	SNodeData	sData;			//copy of DLL's node data for save/restore
};

enum {
	MASTERVOL,
	SOUNDON,
	RESTORESND,
	NUMVOICES,
	NUMDACS,
	SUSPEND,
	INITSOUND,
	KILLSOUND,
	PLAYSOUND,
	STOPSOUND,
	PAUSESOUND,
	FADESOUND,
	HOLDSOUND,
	MUTESOUND,
	SETVOL,
	SETPRI,
	SETLOOP,
	UPDATECUES,
	MIDISEND,
	SETREVERB,

	CHANGESNDSTATE
};

//	MIDI.S functions
enum {
	SPatchReq,
	SInit,
	STerminate,
	SProcess,
	SSoundOn,
	SRestore,
	SMasterVol,
	SSetReverb,
	SPlay,
	SEnd,
	SPause,
	SFade,
	SHold,
	SMute,
	SChangeVol,
	SChangePri,
	SGetSignal,
	SGetDataInc,
	SGetSYMPTE,
	SNoteOff,
	SNoteOn,
	SController,
	SPChange,
	SPBend,
	SAskDriver,
	SGetSignalRset
};

// MIDI commands
const uchar	NOTEOFF		= 0x80;
const uchar	NOTEON		= 0x90;
const uchar	POLYAFTER	= 0xa0;
const uchar	CONTROLLER	= 0xb0;
const uchar	PCHANGE		= 0xc0;
const uchar	CHNLAFTER	= 0xd0;
const uchar	PBEND			= 0xe0;

// defines for flags property of sound objects
const	mNOPAUSE		= 0x0001;
const	mFIXEDPRI	= 0x0002;
const	mPRELOAD		= 0x0004;

void		InitSoundDriver();
void		TermSndDrv();
void		KillAllSounds();
void		SaveAllSounds();
void		RestoreAllSounds();
MemType	GetSoundResType(ResNum);
void		SuspendSounds(Bool);
void		InitSnd(ObjectID);
void		KillSnd(ObjectID);
void		PlaySnd(ObjectID,int);
void		StopSnd(ObjectID);
void		PauseSnd(ObjectID,Bool);
void		FadeSnd(ObjectID,int,int,int,int);
void		HoldSnd(ObjectID,int);
void		SetSndVol(ObjectID,int);
void		SetSndPri(ObjectID,int);
void		SetSndLoop(ObjectID,int);
void		UpdateCues(ObjectID);
void		MidiSend(ObjectID,int,int,int,int);
void		ChangeSndState(ObjectID);
void		SampleCues(ObjectID);
void		PlaySamp(ObjectID,int);
void		InitSamp(ObjectID);

// Platform-specific functions
void	AllocSNodeData(Sound *);
void  FreeSNodeData(Sound *);
#ifndef WIN32S
SNodeData far *NodeDataAddr(Sound *);
#else
SNodeData *NodeDataAddr(Sound *);
#endif
void	AllocMidiData(Sound *, int);
void	FreeMidiData(Sound *);
void	SaveAllSounds(void);

#endif


