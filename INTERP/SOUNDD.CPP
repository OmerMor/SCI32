// soundd.cpp
// DOS specific sound initialization

#include "sol.hpp"

#include	"config.hpp"
#include	"intrpt.hpp"
#include	"midi.hpp"
#include	"resource.hpp"
#include	"select.hpp"
#include	"sound.hpp"

static int			devID;
static int			numberOfDACs;
static MemID		soundDrv;

void
InitSoundDriver()
{
	ushort		patchNum;
	MemID		patchID;
	char		*sndAddr;
	short		driverAddr;
	short		driverData;
	int		size;
	/*	Load sound driver
	 */

	char* fileName = configMgr->Get("soundDrv");
	if (!soundDrv.Load(MemDriver, fileName, PERSIST, SoundDriverHandle))
		msgMgr->Fatal(Msg_NoSoundDrv, fileName);

	sndAddr	= (char*) *soundDrv;
	size		= soundDrv.Size();
	driverAddr = Nselector(sndAddr,size,1);
	driverData = Nselector(sndAddr,size,0);

	/* Load patch (if one is needed)
	 * If bit 7 of the patch number is set, then the patch
	 * will need to be locked permanently in hunk
	 */
	patchNum = DoSound(SPatchReq, driverData, driverAddr,
		(short*) &numberOfVoices, (short*) &numberOfDACs,
		(short*) &devID);
			

	if(patchNum != 65535) {
		patchID = resMgr->Get(MemResPatch, patchNum & 0x7F, patchNum & 0x80);
#if 0
		if ((patchNum&0x7f) == 10)
			patchID = DoLoad(RES_PATCH,(patchNum & 0x7f),patchNum & 0x80);
		else			
			patchID = ResLoad(RES_PATCH,(patchNum & 0x7f),patchNum & 0x80);
#endif
	}

	/*	Initialize sound driver
	 */
	if  (patchNum != 65535) {
		if (DoSound(SInit, (char*) *patchID, 0) == -1) {
			soundDrv.Free();
			msgMgr->Fatal(SrcLoc, Msg_NoMusic);
		}
	}
	else 	{
		if (DoSound(SInit, (char*) 0, 0) == -1) {
			soundDrv.Free();
			msgMgr->Fatal(SrcLoc, Msg_NoMusic);
		}
	}

	InstallServer(SoundServer,1);
	soundInstalled = True;

	DoSound(SProcess,True);
}

void AllocSNodeData(Sound *sn)
{
	sn = sn;
}

SNodeData far *NodeDataAddr(Sound *sn)
{
	return (SNodeData far *)&(sn->sData);
}

void  FreeSNodeData(Sound *sn)
{
	sn = sn;
}

void	AllocMidiData(Sound *sn, int size)
{
	sn = sn;
	size = size;
}

void	FreeMidiData(Sound *sn)
{
	sn = sn;
}

void
SaveAllSounds()
{
}
