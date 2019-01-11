// sound.cpp

/*	NOTE:  a Sound node is now stored in an object property and in the sound
	list as a SoundID, which is a small number which is a handle to the memory.
	This is dereferenced automatically simply by using it it a context where
	a Sound* is expected.  For example:
	
	  SoundID snID = sounds.First();
	  snID->sNumber = 1;

	However, for speed this id should be dereferenced at those points
	where it sure that memory can't move.  This is done simply by

	  Sound* sn = snID;
*/

#include "sol.hpp"

#include "audio.hpp"
#include	"config.hpp"
#include	"intrpt.hpp"
#include	"kernel.hpp"
#include	"list.hpp"
#include	"midi.hpp"
#include	"msg.hpp"
#include "object.hpp"
#include "pmachine.hpp"
#include	"resource.hpp"
#include "savegame.hpp"
#include "selector.hpp"
#include	"sound.hpp"
#include "debug.hpp"

Bool					kernelCall = False;
Bool					soundInstalled = False;
int					numberOfVoices;
static int			reverbDefault;
SOL_List				sounds;

struct {
	ushort	count;
	ushort	func;
	ushort	arg2;
	ushort	arg3;
	ushort	arg4;
	ushort	arg5;
	ushort	arg6;
	ushort	arg7;
} static audArgs;

void
TermSndDrv()
{
	if (soundInstalled) {
		KillAllSounds();
		DoSound(STerminate);
		soundInstalled = False;
	}
}

void
KillAllSounds()
{
	/* Stop and delete each node in the sound list
	 */

	while(!sounds.IsEmpty()) {
		SoundID snID = sounds.First();
		Sound* sn = *snID;
		if (sn->sSample) {
			audArgs.count = 3;
			audArgs.func = STOP;
			audArgs.arg2 = sn->sNumber;
			audArgs.arg3 = snID;
			KDoAudio((kArgs) &audArgs);
		} else {
			DoSound(SEnd, (char*) sn);
			MemID id;
			ObjectID object = (ObjectID) sn->Key();
			if (int(id = object.GetIndexedProperty(sndHandle))) {
				if ((SOL_Handle) id != 1 && memMgr->IsValid((SOL_Handle)id)) {
//					id.Critical(False);
//					id.Unlock();
					FreeMidiData(*snID);
					id.Free();
				}
			}
		}

		FreeSNodeData(*snID);
		snID.Unlock();
		sounds.Delete(snID);
		snID.Free();
	}

}

void
RestoreAllSounds()
{
	ObjectID	soundObject;
	MemID		sHandle;
	ResNum	soundNum;

	/* For every node on the sound list, load the resource in
	 *	the s_number property.  If the sState property of the node
	 *	is non-zero, restart the sound using the SRestore function
	 *	in MIDI.S
	 */

	SoundID snID = sounds.First();

	while ((int) snID) {
		Sound* sn = *snID;
		AllocSNodeData(sn);
		soundObject = (ObjectID) snID->Key();
		soundNum = soundObject.GetIndexedProperty(sndNumber);
		if(sn->sSample) {
			if(soundObject.GetIndexedProperty(sndLoop) == -1 &&
					soundObject.GetIndexedProperty(sndHandle) != 0) {
				soundObject.SetIndexedProperty(sndHandle,0);
				PlaySnd(soundObject);
			}
/* put sample stuff here */
		} else {
			if(soundNum) {
				resMgr->Get(MemResSound, soundNum);
			}
			if((NodeDataAddr(sn))->sState) {
				sHandle = resMgr->Get(MemResSound, soundNum);
				sHandle.Critical();
		  		sHandle.Lock();
				soundObject.SetIndexedProperty(sndHandle, (Property) sHandle);
				sn->sPointer = (char*) *sHandle;
				AllocMidiData(sn, sHandle.Size());
				snID.Lock();
				DoSound(SRestore,(char*) sn);
			#ifdef DEBUG
			memMgr->SetChecksum(snID->sPointer,1);
			#endif

				UpdateCues(soundObject);
			} else
				soundObject.SetIndexedProperty(sndHandle,0);
		}
		
		snID = snID->Next();
	}

	/* Reset the default reverb mode
	 */

	DoSound(SSetReverb, reverbDefault);
}


MemType
GetSoundResType(ResNum resId)
/* Return AUDIO if we have a DAC and there is an audio sample for this
	resId, else return SOUND.
*/
{
	if (audioInstalled && /*!audNone &&*/
		(resMgr->Find(MemResAudio,resId) || resMgr->Find(MemResWAVE,resId) ||
		resMgr->Check(MemResAudio,resId) || resMgr->Check(MemResWAVE,resId)))
		return MemResAudio;
	else
		return MemResSound;
}



/*	Kernel Functions
 */
void KDoSound(argList)
{
	ObjectID soundObject = arg(2);

	switch(arg(1)) {

		case	MASTERVOL:
			if(argCount == 1)
				pm.acc = (Acc) DoSound(SMasterVol,255);
			else
				pm.acc = (Acc) DoSound(SMasterVol,arg(2));
			break;

		case	SOUNDON:
			if(argCount == 1)
				pm.acc = (Acc) DoSound(SSoundOn,255);
			else
				pm.acc = (Acc) DoSound(SSoundOn,arg(2));
			break;

		case	RESTORESND:
			break;

		case	NUMVOICES:
			pm.acc = (Acc) numberOfVoices;
			break;

		case	NUMDACS:
			audArgs.count = 1;
			audArgs.func = DACFOUND;
			KDoAudio ((kArgs) &audArgs);
			break;

		case	SUSPEND:
			SuspendSounds(arg(2));
			break;

		case	INITSOUND:
			InitSnd(soundObject);
			break;

		case	KILLSOUND:
			KillSnd(soundObject);
			break;

		case	PLAYSOUND:
			PlaySnd(soundObject);
			break;

		case	STOPSOUND:
			StopSnd(soundObject);
			break;

		case	PAUSESOUND:
			kernelCall = True;
			PauseSnd(soundObject,arg(3));
			kernelCall = False;
			break;

		case	FADESOUND:
			FadeSnd(soundObject,arg(3),arg(4),arg(5),arg(6));
			break;

		case	HOLDSOUND:
			HoldSnd(soundObject,arg(3));
			break;

		case	SETVOL:
			SetSndVol(soundObject,arg(3));
			break;

		case	SETPRI:
			SetSndPri(soundObject,arg(3));
			break;

		case	SETLOOP:
			SetSndLoop(soundObject,arg(3));
			break;

		case	UPDATECUES:
			UpdateCues(soundObject);
			break;

		case	MIDISEND:
			MidiSend(soundObject,arg(3),arg(4),arg(5),arg(6));
			break;

		case	SETREVERB:
			if(argCount == 1)
				pm.acc = (Acc) DoSound(SSetReverb,255);
			else
				pm.acc = (Acc) DoSound(SSetReverb,arg(2));
			break;


		/* This function is on it's way out (after KQ5 cd and
		 * Jones cd ship
		 */
		case	CHANGESNDSTATE:
			ChangeSndState(soundObject);
			break;

	}
}


void
SuspendSounds(Bool onOff)
{

	/* Use the SProcess function of MIDI.S to ignore
	 * or honor calls to SoundServer.  A True value in
	 * onOff will cause sounds to be suspended, while a
	 * False value will cause them to continue
	 */

	DoSound(SProcess,!onOff);
}


void
InitSnd(ObjectID soundObject)
{
	SoundID	snID;
	ResNum	soundNum;
	uchar		sample;

	/* Allocate a sound node for the object being initialized
	 *	(if there isn't already one), load the sound resource
	 *	specified in the s_number property, and set up the node
	 *	properties
	 */

	soundNum = soundObject.GetIndexedProperty(sndNumber);

	if (GetSoundResType(soundNum) == MemResAudio)
		sample = 1;		/* We have a DAC and there is a sample for this sound */
	else {
	  	/* No DAC or no sample.
			First, filter out "not found" condition in case there is no MIDI
			file but there is a sample file.
		*/
		if (!resMgr->Check(MemResSound,soundNum) &&
			  (resMgr->Check(MemResAudio,soundNum) || resMgr->Check(MemResWAVE,soundNum)))
			return;
		sample = 0;
	}

	if(!sample && soundNum)
		resMgr->Get(MemResSound,soundNum);

	if(!(soundObject.GetIndexedProperty(sndNodePtr))) {
		snID.Get();
		snID.Clear();
		AllocSNodeData(*snID);
		sounds.AddToEnd(snID, soundObject);
		soundObject.SetIndexedProperty(sndNodePtr, (Property) snID);
	}
	else {
		snID = (SoundID) soundObject.GetIndexedProperty(sndNodePtr);
		StopSnd(soundObject);
	}

	Sound* sn = *snID;

	sn->sSample		= sample;
	if (!sample) {
#ifndef WIN32S
		SNodeData far *snData = NodeDataAddr(sn);
#else
		SNodeData *snData = NodeDataAddr(sn);
#endif
		snData->sLoop		= 0;
		if ((char) soundObject.GetIndexedProperty(sndLoop) == (char) -1)
			snData->sLoop = 1;
		snData->sPriority	= (char) soundObject.GetIndexedProperty(sndPriority);
		snData->sVolume	= (char) soundObject.GetIndexedProperty(sndVol);
		snData->sSignal	= 0;
		snData->sDataInc	= 0;
	}
}


void
KillSnd(ObjectID soundObject)
{
	// Stop the sound and delete its node

	SoundID snID = (SoundID) soundObject.GetIndexedProperty(sndNodePtr);
	StopSnd(soundObject);

	if((int) snID) {
		FreeSNodeData(*snID);
		sounds.Delete(snID);
//		snID.Unlock();
		snID.Free();
	}

	soundObject.SetIndexedProperty(sndNodePtr, 0);
}

void
PlaySnd(ObjectID soundObject)
{

	ResNum		soundNum;
	MemID			sHandle;
	SoundID		snID;
#ifndef WIN32S
	SNodeData far	*snData;
#else
	SNodeData *snData;
#endif

	/* Load the resource in the s_number property of the object,
	 *	lock the resource, and start the sound playing
	 */

	if(!soundObject.GetIndexedProperty(sndNodePtr))
		InitSnd(soundObject);

	if (int(snID = (SoundID) soundObject.GetIndexedProperty(sndNodePtr))) {
		/* why is the following not set in InitSnd? TM - 7/17/92 */
		if (GetSoundResType(snID->sNumber) == MemResAudio) {
			audArgs.count = 3;
			audArgs.func = STOP;
			audArgs.arg2 = snID->sNumber;
			audArgs.arg3 = snID;
			KDoAudio ((kArgs) &audArgs);
		}
		soundNum = soundObject.GetIndexedProperty(sndNumber);
		snID->sNumber = soundNum;

		if(soundObject.GetIndexedProperty(sndHandle))
			if (!snID->sSample)
				StopSnd(soundObject);

		if (GetSoundResType(soundNum) == MemResAudio)
			snID->sSample = 1;
		else
			snID->sSample = 0;

		if (!snID->sSample) {		
			if (!resMgr->Check(MemResSound,soundNum) &&
					(resMgr->Check(MemResAudio,soundNum) || resMgr->Check(MemResWAVE,soundNum))) {
				snID->sSample = 1;
				soundObject.SetIndexedProperty(sndSignal, -1);
				return;
			}
			if(!(sHandle = resMgr->Get(MemResSound,soundNum))) {
				return;
			}
			sHandle.Critical();
 			sHandle.Lock();
		} else {
			if (!(sHandle = resMgr->Find(MemResWAVE,soundNum)))
				if (!(sHandle = resMgr->Find(MemResAudio,soundNum)))
					if (soundObject.GetIndexedProperty(sndFlags) & mPRELOAD) {
						if (!(resMgr->Check(MemResWAVE,soundNum)) ||
						    !(sHandle = resMgr->Get(MemResWAVE,soundNum)))
							if (!(resMgr->Check(MemResAudio,soundNum)) ||
							    !(sHandle = resMgr->Get(MemResAudio,soundNum)))
								sHandle = (MemID)-1;
					} else
						sHandle = (MemID)-1;
		}

		soundObject.SetIndexedProperty(sndHandle, (Property) sHandle);
		soundObject.SetIndexedProperty(sndSignal, 0);
		soundObject.SetIndexedProperty(sndMin, 0);
		soundObject.SetIndexedProperty(sndSec, 0);
		soundObject.SetIndexedProperty(sndFrame, 0);

		if (!snID->sSample) {
			snData = NodeDataAddr(*snID);
			snData->sPriority = (char) soundObject.GetIndexedProperty(sndPriority);


		/* The following few lines should be removed after KQ5 cd
		 * and Jones cd ship....
		 */
			snData->sVolume = (char) soundObject.GetIndexedProperty(sndVol);
			if(soundObject.GetIndexedProperty(sndLoop) == -1) {
				snData->sLoop = True;
			}
			else {
				snData->sLoop = False;
			}
		/* In the future, the volume property will be set to
		 * a value that will be passed into this function
		 */
		}

		snID.Lock();
		//check for -1 before de-referencing
		if (sHandle == (MemID)-1)
			snID->sPointer = (char*)-1;
		else
			snID->sPointer = (char*) *sHandle;
		if (!snID->sSample) {		
			AllocMidiData(*snID, sHandle.Size());
			ChangeSndState(soundObject);
			DoSound(SPlay,*snID,0);
			#ifdef DEBUG
			memMgr->SetChecksum(snID->sPointer,1);
			#endif

		} else {
			audArgs.count = 5;
			audArgs.func = PLAY;
			audArgs.arg2 = soundObject.GetIndexedProperty(sndNumber);
			audArgs.arg3 = soundObject.GetIndexedProperty(sndLoop);
			audArgs.arg4 = soundObject.GetIndexedProperty(sndVol);
			audArgs.arg5 = snID;
			KDoAudio ((kArgs) &audArgs);
		}
		
		if (!snID->sSample)
			// Why is this necessary? Didn't we get the value from the same place? (TM 3/18/94)
			soundObject.SetIndexedProperty(sndPriority, (Property) snData->sPriority);
	} else
		soundObject.SetIndexedProperty(sndSignal, -1);
}


void
StopSnd(ObjectID soundObject)
{
	SoundID		snID, searchSnID;

	/* Search every sound in the sound list to make sure that
	 *	there are no other sounds playing off of the same resource
	 *	as the one we are now stopping.  If there is, then we
	 *	do not want to unlock the resource when we stop the sound
	 */

	if (int(snID = (SoundID) soundObject.GetIndexedProperty(sndNodePtr))) {
		Sound* sn = *snID;
		if(sn->sSample) {
			audArgs.count = 3;
			audArgs.func = STOP;
			audArgs.arg2 = sn->sNumber;
			audArgs.arg3 = snID;
			KDoAudio ((kArgs) &audArgs);
			soundObject.SetIndexedProperty(sndHandle, 0);
			soundObject.SetIndexedProperty(sndSignal, -1);
			return;
		}
		searchSnID = sounds.First();
		while((int) searchSnID) {
			if((searchSnID != snID) && (searchSnID->sPointer == snID->sPointer) &&
				((ObjectID) searchSnID->Key()).GetIndexedProperty(sndHandle)) {
				break;
			}
			searchSnID = searchSnID->Next();
		}
		
		DoSound(SEnd,*snID);

		if((!searchSnID) && soundObject.GetIndexedProperty(sndHandle)) {
			MemID	id;
			if (int(id = (MemID) soundObject.GetIndexedProperty(sndHandle))) {
				if ((SOL_Handle) id != 1) {
					id.Critical(False);
					id.Unlock();
//if (memMgr->IsValid(id))
					FreeMidiData(*snID);
//	  				id.Free();
				}
			}
		}
//		snID.Unlock();

	}

	soundObject.SetIndexedProperty(sndHandle, 0);
	soundObject.SetIndexedProperty(sndSignal, -1);
}

void
PauseSnd(ObjectID soundObject, Bool stopStart)
{
	/* If the Object parameter is 0, then we want to pause/unpause
	 *	every node.  If it is nonzero, pause/unpause only the node
	 *	belonging to that object
	 */

	if(!(soundObject)) {
		DoSound(SPause,(char*) 0, stopStart);
		if (kernelCall) {
			audArgs.count = 1;
			audArgs.func = stopStart? PAUSE : RESUME;
			KDoAudio((kArgs) &audArgs);
		}
	}
	else {
		SoundID snID;
		if(int(snID = (SoundID) soundObject.GetIndexedProperty(sndNodePtr))) {
			if (snID->sSample && kernelCall) {
				audArgs.count = 3;
				audArgs.arg2 = soundObject.GetIndexedProperty(sndNumber);
				audArgs.arg3 = snID;
				audArgs.func = stopStart? PAUSE : RESUME;
				KDoAudio((kArgs) &audArgs);
			} else
				DoSound(SPause,*snID, stopStart);
		}
	}
}


void
FadeSnd(ObjectID soundObject, int newVol, int fTicks, int fSteps, int fEnd)
{
	/* Fade the node belonging to the object specified using the
	 * fade function of MIDI.S
	 */

	SoundID snID;
	if (int(snID = (SoundID) soundObject.GetIndexedProperty(sndNodePtr))) {
		Sound* sn = *snID;
		if (sn->sSample) {
			audArgs.count = 7;
			audArgs.func = FADE;
			audArgs.arg2 = soundObject.GetIndexedProperty(sndNumber);
			audArgs.arg3 = newVol;
			audArgs.arg4 = fTicks;
			audArgs.arg5 = fSteps;
			audArgs.arg6 = fEnd;
			audArgs.arg7 = snID;
			KDoAudio((kArgs) &audArgs);
		} else {
			DoSound(SFade,*snID,newVol += fEnd? 128:0,fTicks,fSteps);
		}
	}
}


void
HoldSnd(ObjectID soundObject, int where)
{
	/* Set the hold property of the sound node belonging to the
	 *	specified object.  This is another type of loop setting,
	 *	in which sounds can be looped in the middle, and continue
	 *	on towards the end after being released
	 */

	SoundID snID;
	if (int(snID = (SoundID) soundObject.GetIndexedProperty(sndNodePtr))) {
		DoSound(SHold,*snID,where);
	}
}

void
SetSndVol(ObjectID soundObject, int newVol)
{

	/* Change the volume of the sound node and the sound object
	 * to the value passed in
	 */

	SoundID snID;
	if(int(snID = (SoundID) soundObject.GetIndexedProperty(sndNodePtr))) {
		if (snID->sSample) {
			audArgs.count = 4;
			audArgs.arg2 = newVol;
			audArgs.arg3 = soundObject.GetIndexedProperty(sndNumber);
			audArgs.arg4 = snID;
			audArgs.func = VOLUME;
			KDoAudio((kArgs) &audArgs);
		}
		if((NodeDataAddr(*snID))->sVolume != ((char) newVol)) {
			DoSound(SChangeVol,*snID,newVol);
			soundObject.SetIndexedProperty(sndVol, (Property) newVol);
		}
	}
}


void
SetSndPri(ObjectID soundObject, int newPri)
{

	/* Set the priority of the sound node to the value in newPri.
	 * If the value is -1, then simply clear the fixed priority
	 * flag in the sound node and in the flags property of the
	 * sound object.  If it is not -1, then set both of those
	 * flags
	 */

	SoundID snID;
	if(int(snID = (SoundID) soundObject.GetIndexedProperty(sndNodePtr))) {
#ifndef WIN32S
		SNodeData far* snData = NodeDataAddr(*snID);
#else
		SNodeData *snData = NodeDataAddr(*snID);
#endif
		if(newPri == -1) {
			snData->sFixedPri = False;
			soundObject.SetIndexedProperty(sndFlags, (Property)
				(soundObject.GetIndexedProperty(sndFlags) & (-1 - mFIXEDPRI)));
		}
		else {
			snData->sFixedPri = True;
			soundObject.SetIndexedProperty(sndFlags, (Property)
				(soundObject.GetIndexedProperty(sndFlags) | mFIXEDPRI));
			DoSound(SChangePri,*snID,newPri);
		}
	}
}


void
SetSndLoop(ObjectID soundObject, int newLoop)
{

	/* Set the loop property of the sound node and the sound
	 * object to the value passed in.
	 */
	 
	SoundID snID;

	if (int(snID = (SoundID) soundObject.GetIndexedProperty(sndNodePtr))) {
		if(newLoop == -1) {
			soundObject.SetIndexedProperty(sndLoop, -1);
			if (!snID->sSample)
				(NodeDataAddr(*snID))->sLoop = True;
		}
		else {
			soundObject.SetIndexedProperty(sndLoop, 1);
			if (!snID->sSample)
				(NodeDataAddr(*snID))->sLoop = False;
		}
	}
}


void
UpdateCues(ObjectID soundObject)
{
	int		min, sec, frame;
	int		time;
	short		signal;
	Acc		saveAcc;

	/* Copy the current cue, clock, and volume information from the
	 *	sound node to it's object.  This should be called every game
	 *	cycle (in the sounds check: method), or the time and cue
	 *	information in the sound object will be wrong
	 */

	SoundID	snID;
	if (int(snID = (SoundID) soundObject.GetIndexedProperty(sndNodePtr))) {
		if (snID->sSample) {
			saveAcc = pm.acc;
			audArgs.count = 3;
			audArgs.arg2 = soundObject.GetIndexedProperty(sndNumber);
			audArgs.arg3 = snID;
			audArgs.func = LOC;
			KDoAudio((kArgs) &audArgs);
			signal = (int)pm.acc;
			pm.acc = saveAcc;
			if (signal == -1)
				StopSnd(soundObject);
		} else {
#ifndef WIN32S
			SNodeData far* snData = NodeDataAddr(*snID);
#else
			SNodeData *snData = NodeDataAddr(*snID);
#endif
			soundObject.SetIndexedProperty(sndVol, (Property) snData->sVolume);
//			signal = DoSound(SGetSignalRset,*snID);
			signal = snData->sSignal;
			snData->sSignal = 0;

			switch(signal) {
				case 0xff:
					StopSnd(soundObject);
					break;
				case 0x00:
					if((ushort) soundObject.GetIndexedProperty(sndDataInc) !=
						 snData->sDataInc) {
						soundObject.SetIndexedProperty(sndDataInc,
							(Property) snData->sDataInc);
						soundObject.SetIndexedProperty(sndSignal,
							(Property) (snData->sDataInc + 127));
					}
					break;
				default:
					soundObject.SetIndexedProperty(sndSignal, (Property) signal);
			}


//			DoSound(SGetSYMPTE,*snID,(char*) &min,(char*) &sec,
//				(char*) &frame);
			time = snData->sTimer;
			min = time / 3600;
			time = time % 3600;
			sec = time / 60;
			time = time % 60;
			frame = time / 2;

			soundObject.SetIndexedProperty(sndMin, (Property) min);
			soundObject.SetIndexedProperty(sndSec, (Property) sec);
			soundObject.SetIndexedProperty(sndFrame, (Property) frame);


		}
	}
}


void
MidiSend(ObjectID soundObject, int channel, int command, int value1, int value2)
{
	/* Send MIDI a command to any channel of the node belonging to
	 * the specified object
	 */

	channel--;

	if(command == PBEND) {
		if(value1 > 8191) {
			value1 = 8191;
		}
		if(value1 < -8192) {
			value1 = -8192;
		}
	}
	else {
		if(value1 > 127) {
			value1 = 127;
		}
		if(value1 < 0) {
			value1 = 0;
		}

		if(value2 > 127) {
			value2 = 127;
		}
		if(value2 < 0) {
			value2 = 0;
		}
	}

	SoundID	snID;
	if (int(snID = (SoundID) soundObject.GetIndexedProperty(sndNodePtr))) {
		switch(command) {
			case NOTEOFF:
				DoSound(SNoteOff,*snID,channel,value1,value2);
				break;
			case NOTEON:
				DoSound(SNoteOn,*snID,channel,value1,value2);
				break;
			case CONTROLLER:
				DoSound(SController,*snID,channel,value1,value2);
				break;
			case PCHANGE:
				DoSound(SPChange,*snID,channel,value1);
				break;
			case PBEND:
				DoSound(SPBend,*snID,channel,value1+8192);
				break;
		}
	}
}



/* This function is on it's way out (after KQ5 cd and
 * Jones cd ship)
 */

void
ChangeSndState(ObjectID soundObject)
{
	/* Update the sLoop, sVolume, and sPriority properties of
	 *	the sound node to what is currently in those properties of
	 *	object which the node belongs to
	 */

	SoundID	snID;
	if (int(snID = (SoundID) soundObject.GetIndexedProperty(sndNodePtr))) {
		if (!snID->sSample) {
#ifndef WIN32S
			SNodeData far* snData = NodeDataAddr(*snID);
#else
			SNodeData *snData = NodeDataAddr(*snID);
#endif
			snData->sLoop = 0;
			if(soundObject.GetIndexedProperty(sndLoop) == -1) {
				snData->sLoop = 1;
			}

			if(snData->sVolume != (char) soundObject.GetIndexedProperty(sndVol))	{
				DoSound(SChangeVol,*snID,soundObject.GetIndexedProperty(sndVol));
			}

			if(snData->sPriority != (char) soundObject.GetIndexedProperty(sndPriority)) {
				DoSound(SChangePri,*snID,soundObject.GetIndexedProperty(sndPriority));
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////

SoundMgr soundMgr;

void
SoundMgr::SaveRestoreBefore(SaveGame*, Bool save)
{
	//	pause sounds on save, so that SaveGame's checksum doesn't get
	//	screwed up
	//	turn off sound processing on restore, so no havoc ensues while we're
	//	reading in the sound list

	DoSound(SProcess, False);
	if (save) {
		PauseSnd(0, True);
		SaveAllSounds();
	}
	else
		KillAllSounds();
}

void
SoundMgr::SaveRestoreAfter(SaveGame*, Bool save)
{
	//	unpause sound after save
	//	after restore, reinitialize sounds

  	DoSound(SProcess, True);
	if (!save) 
		RestoreAllSounds();
		
	PauseSnd(0, False);
}

void
SoundMgr::SaveRestoreDo(Saveable::Func func, SaveGame* saveGame, Bool save)
{
	sounds.SaveRestoreDo(func, saveGame, save);

	Saveable::SaveRestoreDo(func, saveGame, save);
}
