// soundw.cpp
// Windows specific sound functions

#include "sciwin.hpp"

#include "sol.hpp"

#include	"config.hpp"
#include	"intrpt.hpp"
#include	"memmgrw.hpp"
#include	"midi.hpp"
#include	"resource.hpp"
#include	"sound.hpp"
#include	"soundwin.hpp"
#include "timew.hpp"
#include "mbox.hpp"

int	MidiType = 1;					/* default to base-level synth */
static int			devID;
static int			numberOfDACs;
extern SOL_List	sounds;

#define NUMSOUNDNODES	10
DWORD	smallMidiSize = 5000;
DWORD	largeMidiSize = 50000;
DWORD	numSoundNodes = NUMSOUNDNODES;
char far *midiCachePtr;
char far *smallBuffPtr;
char far *largeBuffPtr;
BOOL				smallBuffInUse = FALSE;
BOOL				largeBuffInUse = FALSE;
BOOL				nodeInUse[NUMSOUNDNODES];
SNodeData far *nodeBuffPtr[NUMSOUNDNODES];

void
InitSoundDriver()
{
	int		patchNum;
	MemID		patchID;
	/*	Load sound driver
	 */


	InitWinSound();

	SetMIDIType(MidiType);	//set to BASEADLIB

	patchNum = DoSound(SPatchReq,
							(short*) &numberOfVoices,	//dummy to make midi.asm happy
							(short*) &numberOfVoices,
							(short*) &numberOfDACs,
							(short*) &devID);
	if (DoSound(SInit, (short*)&devID, 0) == -1) {
		msgMgr->Fatal(SrcLoc, Msg_NoMusic);
	}

	soundInstalled = True;
	DoSound(SProcess,True);

	largeBuffPtr = midiCachePtr;
	smallBuffPtr = midiCachePtr + largeMidiSize + 10;
	for (int i = 0; i < NUMSOUNDNODES; i++) {
		nodeInUse[i] = FALSE;
		nodeBuffPtr[i] = (SNodeData far *)
								(smallBuffPtr + smallMidiSize + 10 +
								(i * sizeof(SNodeData)));
	}
}


void AllocSNodeData(Sound *sn)
{
#ifndef WIN32S

	for (int i = 0; i < NUMSOUNDNODES; i++)
		if (!nodeInUse[i]) {
			nodeInUse[i] = TRUE;
			sn->sDataPtr = nodeBuffPtr[i];
			_fmemcpy((SNodeData far *)nodeBuffPtr[i],
						(void __far *)&(sn->sData), sizeof(SNodeData));
			return;
		}

	_disable();
	((TimeMgrWin*)timeMgr)->DisableTimer();

	// Set pointer in the node and copy data to data area
	sn->sDataPtr = (SNodeData far *)AllocWinMem(sizeof(SNodeData), PAGELOCK);
	_fmemcpy(sn->sDataPtr, (void __far *)&(sn->sData), sizeof(SNodeData));

	((TimeMgrWin*)timeMgr)->EnableTimer();
	_enable();
#else

	_disable();
	((TimeMgrWin*)timeMgr)->DisableTimer();

	// Set pointer in the node and copy data to data area
	sn->sDataPtr = (SNodeData *)AllocWinMem(sizeof(SNodeData), PAGELOCK);
	memcpy((SNodeData *)(sn->sDataPtr), &(sn->sData), sizeof(SNodeData));

	((TimeMgrWin*)timeMgr)->EnableTimer();
	_enable();
#endif
}

#ifndef WIN32S
SNodeData far *NodeDataAddr(Sound *sn)
{
	return (SNodeData far *)sn->sDataPtr;
}
#else
SNodeData *NodeDataAddr(Sound *sn)
{
	return (SNodeData *)sn->sDataPtr;
}
#endif

void  FreeSNodeData(Sound *sn)
{
#ifndef WIN32S

	for (int i = 0; i < NUMSOUNDNODES; i++)
		if (sn->sDataPtr == nodeBuffPtr[i]) {
			nodeInUse[i] = FALSE;
			return;
		}

	_disable();
	((TimeMgrWin*)timeMgr)->DisableTimer();

	FreeWinMem((void far *)(sn->sDataPtr), PAGELOCK);

	((TimeMgrWin*)timeMgr)->EnableTimer();
	_enable();

#else

	_disable();
	((TimeMgrWin*)timeMgr)->DisableTimer();

	FreeWinMem((void *)(sn->sDataPtr), PAGELOCK);

	((TimeMgrWin*)timeMgr)->EnableTimer();
	_enable();
#endif
}

void	AllocMidiData(Sound *sn, int size)
{
#ifndef WIN32S
char far *lpMem;

	if ((size < smallMidiSize) && (!smallBuffInUse)) {
		smallBuffInUse = TRUE;
		(sn->sDataPtr)->sMidiPtr32 = smallBuffPtr;
		_fmemcpy(smallBuffPtr, (void __far *)(sn->sPointer), size);
		return;
	}

	if ((size < largeMidiSize) && (!largeBuffInUse)) {
		largeBuffInUse = TRUE;
		(sn->sDataPtr)->sMidiPtr32 = largeBuffPtr;
		_fmemcpy(largeBuffPtr, (void __far *)(sn->sPointer), size);
		return;
	}

	_disable();
	((TimeMgrWin*)timeMgr)->DisableTimer();

	lpMem = (char far *)AllocWinMem(size, PAGELOCK);

	(sn->sDataPtr)->sMidiPtr32 = (char far *)lpMem;
	_fmemcpy((char far *)lpMem, (void __far *)(sn->sPointer), size);

	((TimeMgrWin*)timeMgr)->EnableTimer();
	_enable();

#else

	_disable();
	((TimeMgrWin*)timeMgr)->DisableTimer();

	(sn->sDataPtr)->sMidiPtr32 = (char *)AllocWinMem(size, PAGELOCK);
	memcpy((sn->sDataPtr)->sMidiPtr32, sn->sPointer, size);

	((TimeMgrWin*)timeMgr)->EnableTimer();
	_enable();
#endif
}

void	FreeMidiData(Sound *sn)
{
#ifndef WIN32S
char far *lpMem;

	lpMem = (sn->sDataPtr)->sMidiPtr32;

	if (lpMem == largeBuffPtr) {	
		largeBuffInUse = FALSE;
		return;
	}

	if (lpMem == smallBuffPtr) {	
		smallBuffInUse = FALSE;
		return;
	}

	_disable();
	((TimeMgrWin*)timeMgr)->DisableTimer();

	FreeWinMem((void far *)lpMem, PAGELOCK);

	((TimeMgrWin*)timeMgr)->EnableTimer();
	_enable();

#else

	_disable();
	((TimeMgrWin*)timeMgr)->DisableTimer();

	FreeWinMem((void *)(sn->sDataPtr)->sMidiPtr32, PAGELOCK);

	((TimeMgrWin*)timeMgr)->EnableTimer();
	_enable();
#endif
}

void
SaveAllSounds()
{
	/* For every node on the sound list, copy the DLL's copy of the data
	 * back into the node.
	 */

	SoundID snID = sounds.First();

	while(snID) {
		Sound* sn = *snID;
		if (!sn->sSample)
#ifndef WIN32S
			_fmemcpy((void __far *)&(sn->sData), sn->sDataPtr, sizeof(SNodeData));
#else
			memcpy(&(sn->sData), sn->sDataPtr, sizeof(SNodeData));
#endif
		snID = snID->Next();
	}
}
