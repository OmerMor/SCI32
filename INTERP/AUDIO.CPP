// audio.cpp
//		digital audio sample playback

#include <stdio.h>

#include "sol.hpp"

#include "audio.hpp"
#include "config.hpp"
#include "dos.hpp"
#include	"kernel.hpp"
#include "memid.hpp"
#include "memmgr.hpp"
#include "pmachine.hpp"
#include "resource.hpp"
#include	"time.hpp"

#ifdef DEBUG
#include "event.hpp"
#include	"kbdmgr.hpp"
#include	"msg.hpp"
#endif

#define	ALL_SAMPLES	-1
#define	NO_SAMPLES	-2
#define	NO_DAC		-1

union {
	void *data;
	int (*code)(int,void *);
} audioDriver;

union {
	_Packed struct {
		int	ofs;
		short	sel;
		int	ofsOrig;
	} adr;
	char far *ptr;
} DACBuffer;

const	unsigned DAC_ARGS_IN	= 5;
const unsigned MAXSAMPLES = 5;
const unsigned MAXVOLUME = 127;
#define AUDIO_TEMP MOVEABLE+TRANSITORY
#define AUDIO_PERM MOVEABLE+PERSIST

struct Sample {
	int		attrs;
	int		audFD;
	uchar		bits;
	MemID		buffID;
	uchar		bufferNumber;
	int		bufferSize;
	int		bytesBuffered;
	int		bytesRead;
	int		bytesToRead;
	uchar		channels;
	Bool		compressed;
	int		compVal16;
	uchar		compVal8;
	uchar		cond;
	int		distortion;
	Bool		do16Conv8;
	Bool		do8Conv16;
	Bool		do11KConv22K;
	Bool		do22KConv11K;
	Bool		doDecomp;
	int		fadeLoc;
	Bool		fadeEnd;
	int		fadeStep;
	int		fadeTicks;
	int		fadeVol;
	int		fd;
	uchar		fillToggle;
	MemID		id;
	int		length;
	Bool		loop;
	ResNum	module;
	uchar		noun;
	int		num;
	int		offset;
	int		pausedLoc;
	int		rate;
	Bool		robot;
	Bool		scrambled;
	uchar		sequ;
	int		start;
	int		startLoc;
	int		tag;
	int		ticks;
	uchar		verb;
	int		volume;
};
Sample sample[MAXSAMPLES];

RobotAudInfo rbot;

#define	ROBOT_RATE		22050
#define	ROBOT_BITS		16
#define	ROBOT_CHANNELS	1

int		activeSamples = 0;
Bool		audioInstalled = False;
int		audVolFD = -1;
MemID		convBuff = 0;
int		distortion = 0;
int		DACArgsIn[DAC_ARGS_IN];
int		DACHbufferSize;
int		intHbufferSize;
int		globalLoc;
Bool		lastDAC;
Bool		mixCheck = True;
int		pausedLoc = 0;
int		preLoad = 0;
int		rate = 0, bits = 0, channels = 0, volume = MAXVOLUME;
int		rateMax, bitsMax, channelsMax, volumeMax;
Bool		rateSet = False, bitsSet = False, channelsSet = False;
int		sfxVolFD = -1;

void		AudioBits(int);
void		AudioChannels(int);
void		AudioCheck();
void		AudioDistort(int,int);
Bool		AudioFade(int);
int		AudioLoc(int);
Bool		AudioPause(int);
void		AudioPlay();
void		AudioRate(int);
Bool		AudioResume(int);
int		AudioSelect(int);
void		AudioStop(int);
void		AudioUnLoad();
void		AudioVolume(int,int);
Bool		CallDAC(int,int);
void		CheckRawFormat(int);
Bool		CheckSOLFormat(int);
Bool		CheckWAVEFormat(int);
void		ConvBase36(char*,int,int);
void		DistortBuffer(int,char far *);
void		DropSample(int);
void		FillBuffer(int);
void		FillRbotBuffer8(int);
void		FillRbotBuffer16(int);
int		FindSampleN(argList,int,int tag = 0);
void		FreeAudioDrv(void);
void		GetConvBuff(int);
char		GetDigit36(int);
unsigned GetDACDefaultSize(void);
void*		LoadAudioDrv(void);
void		MergeBuffer(int,int);
Bool		MergeBuffers(void);
void		VolumeBuffer(int,char far *);
#ifdef DEBUG
void		DACPlay();
int		GetBase36(char*,int);
#endif

void
KDoAudio(argList)
{
	int	n, saveCount, tag;
	Bool	pause = False;

	if (!audioInstalled || argCount < 1) {
		pm.acc = 0;
		return;
	}

	switch (arg(1)) {
		case DACFOUND:
			pm.acc = 1;
			break;
		case PRELOAD:
			if (argCount > 1)
				preLoad = arg(2);
			pm.acc = preLoad;
			break;
		case WPLAY:
			pause = True;
			// fall through to case PLAY:
		case PLAY:   /*  audio playback begins  */
			if (argCount < 2) {
				pm.acc = activeSamples;
				break;
			}
			if ((n = FindSampleN(args,0)) != NO_SAMPLES && sample[n].pausedLoc) {
				AudioResume(n);
				pm.acc = sample[n].ticks;
				break;
			}
			if (activeSamples == MAXSAMPLES) {
				pm.acc = 0;
				break;
			}
			n = activeSamples;
			if (argCount < 6) {
				sample[n].module = SFXMODNUM;
				sample[n].num = arg(2);
				if (argCount >= 3 && arg(3) != 1)
					sample[n].loop = arg(3);
				else
					sample[n].loop = False;
				if (argCount >= 4 && arg(4) >= 0 && arg(4) <= MAXVOLUME)
					sample[n].volume = arg(4);
				else
					sample[n].volume = MAXVOLUME;
				if (argCount == 5)
					sample[n].tag = arg(5);
				else
					sample[n].tag = 0;
			} else {
				sample[n].num = 0;
				sample[n].module = arg(2);
				sample[n].noun = (uchar)arg(3);
				sample[n].verb = (uchar)arg(4);
				sample[n].cond = (uchar)arg(5);
				sample[n].sequ = (uchar)arg(6);
				if (argCount >= 7 && arg(7) != 1)
					sample[n].loop = arg(7);
				else
					sample[n].loop = False;
				if (argCount >= 8 && arg(8) >= 0 && arg(8) <= MAXVOLUME)
					sample[n].volume = arg(8);
				else
					sample[n].volume = MAXVOLUME;
			}
			sample[n].robot = False;
			if (pm.acc = sample[n].ticks = AudioSelect(n)) {
				sample[n].pausedLoc = pause? 1 : 0;
				++activeSamples;
				AudioPlay();
				if (pause)
					sample[n].pausedLoc = sample[n].startLoc;					
			}
			break;
		case STOP:
			pm.acc = activeSamples;
			AudioStop(FindSampleN(args,0,argCount>2?arg(3):0));
			break;
		case PAUSE:
			pm.acc = AudioPause(FindSampleN(args,0,argCount>2?arg(3):0));
			break;
		case RESUME:
			pm.acc = AudioResume(FindSampleN(args,0,argCount>2?arg(3):0));
			break;
		case LOC:
			pm.acc = (Acc) AudioLoc(FindSampleN(args,0,argCount>2?arg(3):0));
			break;
		case RATE:
			if (argCount > 1) {
				if (arg(2)) {
					rateSet = True;
					AudioRate(arg(2));
				} else
					rateSet = False;
			}
			pm.acc = rate;
			break;
		case BITS:
			if (argCount > 1) {
				if (arg(2)) {
					bitsSet = True;
					AudioBits(arg(2));
				} else
					bitsSet = False;
			}
			pm.acc = bits;
			break;
		case CHANNELS:
			if (argCount > 1) {
				if (arg(2)) {
					channelsSet = True;
					AudioChannels(arg(2));
				} else
					channelsSet = False;
			}
			pm.acc = channels;
			break;
		case DACMIXNORMAL:
			if (argCount > 1)
				mixCheck = (Bool) arg(2);
			pm.acc = mixCheck;			
			break;
		case DISTORT:
			if (argCount > 1)
				AudioDistort(arg(2),FindSampleN(args,1));
			pm.acc = distortion;			
			break;
		case VOLUME:
			n = FindSampleN(args,1,argCount>3?arg(4):0);
			if (argCount >= 2 && arg(2) != -1)
				AudioVolume(arg(2),n);
			if (n >= 0 && n < activeSamples)
				pm.acc = sample[n].volume;			
			else
				pm.acc = volume;			
			break;
		case FADE:
			pm.acc = False;
			if (argCount < 5)
				break;		
			tag = argCount > 6? arg(7):0;
			saveCount = argCount;
			argCount = 2;
			n = FindSampleN(args,0,tag);
			argCount = saveCount;
			if (n < 0 || n >= activeSamples || sample[n].module != SFXMODNUM)
				break;
			if (sample[n].volume == arg(3))
				break;
			if (arg(5) == 0)
				AudioVolume(arg(3),n);
			else {
				sample[n].fadeVol = arg(3);
				sample[n].fadeTicks = arg(4);
				sample[n].fadeStep = arg(5);
				sample[n].fadeEnd = argCount >= 6? arg(6) : False;
				sample[n].fadeLoc = timeMgr->GetTickCount();
			}
			pm.acc = True;
			break;
		case FADE36:
			if (argCount < 9 && argCount != 6) {
				pm.acc = -1;
				break;		
			}
			n = FindSampleN(args,0);
			if (n < 0 || n >= activeSamples) {
				pm.acc = -1;
				break;
			}
			if (argCount > 6 && sample[n].volume == arg(7)) {
				pm.acc = -1;
				break;
			}
			if (argCount > 6 && arg(9) == 0) {
				AudioVolume(arg(9),n);
				pm.acc = 0;
				break;
			}
			if (argCount > 6) {
				sample[n].fadeVol = arg(7);
				sample[n].fadeTicks = arg(8);
				sample[n].fadeStep = arg(9);
				sample[n].fadeEnd = argCount >= 10? arg(10) : False;
			} else {
				sample[n].fadeVol = 0;
				sample[n].fadeTicks = 25;
				sample[n].fadeStep = 10;
				sample[n].fadeEnd = True;
			}
			sample[n].fadeLoc = timeMgr->GetTickCount();
			pm.acc = sample[n].fadeStep * sample[n].fadeTicks;
			break;
		default:
			msgMgr->Alert(SrcLoc, Msg_NoAudio);
			pm.acc = 0;
			break;   /*  unexpected audio driver function  */
	}
}

int
FindSampleN(argList,int argn,int tag)
{
	// argCount = # kernel args (subfunction takes one)
	// argn = # additional args prior to possible file number(s)
	int	n;

	// return the sample number if found; else return activeSamples

	if (argCount - 1 - argn <= 0)
		return ALL_SAMPLES;

	if (!activeSamples)
		return NO_SAMPLES;

	if (argCount - 1 - argn < 5) {
		for (n = 0; n < activeSamples; n++) {
			if (sample[n].num == arg(2+argn) && sample[n].module == SFXMODNUM) {
				if (tag && tag != sample[n].tag)
					continue;
				return n;
			}
		}
	} else for (n = 0; n < activeSamples; n++) {
		if (!sample[n].robot &&
				sample[n].num == 0 &&
				sample[n].module == (ResNum)arg(2+argn) &&
				sample[n].noun == (uchar)arg(3+argn) &&
				sample[n].verb == (uchar)arg(4+argn) &&
				sample[n].cond == (uchar)arg(5+argn) &&
				sample[n].sequ == (uchar)arg(6+argn))
			return n;
	}
	return NO_SAMPLES;
}

Bool
AudRobot(RobotAudInfo buff)
{
	int	n;

	if (!audioInstalled)
		return True;
	rbot.adr = buff.adr;
	rbot.len = buff.len;
	rbot.floc = buff.floc;
	rbot.ext = rbot.floc + rbot.len*4;
	rbot.channel = rbot.floc % 4 ? 1 : 0;
	if (rbot.floc == 0 && rbot.len != 0 && rbot.len != -1) {
		AudioStop(ALL_SAMPLES);
		rbot.hwm[0] = 0;
		rbot.hwm[1] = 2;
		if ((n=activeSamples) == MAXSAMPLES)
			return False;
		sample[n].module = sample[n].num = 0;
		sample[n].length = rbot.len;
		sample[n].start = 0;
		sample[n].loop = False;
		sample[n].volume = MAXVOLUME;
		sample[n].compressed = True;
		sample[n].robot = True;
		if (!AudioSelect(n))
			return False;
		rbot.id = sample[n].buffID;
		rbot.blen = rbot.id.Size();
		rbot.min = 0;
		rbot.max = rbot.blen * (16/bits);
		++activeSamples;
	} else {
		for (n = 0; n < activeSamples; n++)
			if (sample[n].robot)
				break;
		if (n == activeSamples)
			return False;
	}
	if (rbot.len == 0)
		AudioStop(n);
	else if (rbot.len == -1)
		sample[n].bytesToRead = 0;
	else {
		if (rbot.ext <= Max(rbot.min,rbot.hwm[rbot.channel]) ||
				rbot.max <= rbot.hwm[rbot.channel])
			return False;
		sample[n].bytesToRead = rbot.len;
		int ext = rbot.ext;
		int rmax = rbot.max;
		if (bits == 8)
			FillRbotBuffer8(n);
		else
			FillRbotBuffer16(n);
		if (ext > rmax)
			return False;
		if (rbot.floc == 2)
			AudioPlay();
	}
	return True;
}

void
FillRbotBuffer8(int n)
{
	int	ofs, ext, len;

	rbot.cofs = 0;
	Decomp8To16(*convBuff,rbot.adr,rbot.len,0);
	rbot.len *= 2;
	int loc = Max(rbot.min+rbot.channel,rbot.hwm[rbot.channel]);
	int mx = rbot.max+rbot.channel;
	if (rbot.floc < loc) {
		rbot.cofs = (loc - rbot.floc) / 2;
		rbot.len -= rbot.cofs;
		rbot.floc = loc;
	}
	if (rbot.floc > mx) {
		rbot.len += (rbot.floc - mx) / 2;
		rbot.floc = mx;
	}
	if (rbot.ext > mx) {
		rbot.len -= (rbot.ext - mx) / 2;
		rbot.ext = mx;
	}

	rbot.len /= 2;
	Conv16To8(&convBuff[rbot.cofs],&convBuff[rbot.cofs],rbot.len);

	int maxhwm = Max(rbot.hwm[0],rbot.hwm[1]);
	if (rbot.ext > maxhwm)
		sample[n].bytesBuffered += (rbot.ext - maxhwm) / 2;

	if (rbot.floc > rbot.hwm[rbot.channel]) {
		ext = rbot.floc % (rbot.blen * 2);
		if ((rbot.floc&0xfffffffc) > (rbot.hwm[1-rbot.channel]&0xfffffffc)) {
			ofs = rbot.hwm[1-rbot.channel] % (rbot.blen * 2);
			if (ofs >= ext) {
				len = rbot.blen * 2 - ofs;
				Silence((char far *)&rbot.id[ofs/2],len/2,bits);
				ofs = (1-rbot.channel)?2:0;
			}
			len = ext - ofs;
			if (len > 0)
				Silence((char far *)&rbot.id[ofs/2],len/2,bits);
		}
		ofs = rbot.hwm[rbot.channel] % (rbot.blen * 2);
		if (ofs >= ext) {
			len = rbot.blen * 2 - ofs;
			DupeChannel(&rbot.id[ofs/2],len/4,0,bits);
			ofs = rbot.channel?2:0;
		}
		len = ext - ofs;
		if (len > 0)
			DupeChannel(&rbot.id[ofs/2],len/4,0,bits);
	}
	if (rbot.len > 0) {
		ofs = rbot.floc % (rbot.blen * 2);
		ext = rbot.ext % (rbot.blen * 2);
		len = 0;
		if (ofs >= ext) {
			len = (rbot.blen * 2 - (ofs & 0xfffffffc))/2;
			CopyEOS(&rbot.id[ofs/2],&convBuff[rbot.cofs],len/2,bits);
			ofs = rbot.channel?2:0;
		}
		CopyEOS(&rbot.id[ofs/2],&convBuff[rbot.cofs+len/2],(ext-ofs)/4,bits);
	}
	rbot.hwm[rbot.channel] = rbot.ext;
}

void
FillRbotBuffer16(int n)
{
	int	ofs, ext, len;

	rbot.cofs = 0;
	Decomp8To16(*convBuff,rbot.adr,rbot.len,0);
	rbot.len *= 2;
	int loc = Max(rbot.min+rbot.channel*2,rbot.hwm[rbot.channel]);
	int mx = rbot.max+rbot.channel*2;
	if (rbot.floc < loc) {
		rbot.cofs = (loc - rbot.floc) / 2;
		rbot.len -= rbot.cofs;
		rbot.floc = loc;
	}
	if (rbot.floc > mx) {
		rbot.len += (rbot.floc - mx) / 2;
		rbot.floc = mx;
	}
	if (rbot.ext > mx) {
		rbot.len -= (rbot.ext - mx) / 2;
		rbot.ext = mx;
	}

	int maxhwm = Max(rbot.hwm[0],rbot.hwm[1]);
	if (rbot.ext > maxhwm)
		sample[n].bytesBuffered += rbot.ext - maxhwm;

	if (rbot.floc > rbot.hwm[rbot.channel]) {
		ext = rbot.floc % rbot.blen;
		if ((rbot.floc&0xfffffffc) > (rbot.hwm[1-rbot.channel]&0xfffffffc)) {
			ofs = rbot.hwm[1-rbot.channel] % rbot.blen;
			if (ofs >= ext) {
				len = rbot.blen - ofs;
				Silence((char far *)&rbot.id[ofs],len,bits);
				ofs = (1-rbot.channel)?2:0;
			}
			len = ext - ofs;
			if (len > 0)
				Silence((char far *)&rbot.id[ofs],len,bits);
		}
		ofs = rbot.hwm[rbot.channel] % rbot.blen;
		if (ofs >= ext) {
			len = rbot.blen - ofs;
			DupeChannel(&rbot.id[ofs],len/4,0,bits);
			ofs = rbot.channel?2:0;
		}
		len = ext - ofs;
		if (len > 0)
			DupeChannel(&rbot.id[ofs],len/4,0,bits);
	}
	if (rbot.len > 0) {
		ofs = rbot.floc % rbot.blen;
		ext = rbot.ext % rbot.blen;
		len = 0;
		if (ofs >= ext) {
			len = (rbot.blen - (ofs & 0xfffffffc))/2;
			CopyEOS(&rbot.id[ofs],&convBuff[rbot.cofs],len/2,bits);
			ofs = rbot.channel?2:0;
		}
		CopyEOS(&rbot.id[ofs],&convBuff[rbot.cofs+len],(ext-ofs)/4,bits);
	}
	rbot.hwm[rbot.channel] = rbot.ext;
}

int
AudioSelect(int n)
{
	int	fd;
	char	pathName[MaxPath + 1];

	sample[n].id = 0;
	sample[n].attrs = 0;
	sample[n].offset = 0;
	sample[n].audFD = -1;
	sample[n].fadeStep = 0;

	if (sample[n].robot) {

		sample[n].fd = -1;
		sample[n].rate = ROBOT_RATE;
		sample[n].bits = ROBOT_BITS;
		sample[n].channels = ROBOT_CHANNELS;
		sample[n].scrambled = False;
		sample[n].compressed = True;

	} else if (sample[n].module == SFXMODNUM) {

		// Search for AUD/WAV resource:
		pathName[0] = '\0';

		if ((sample[n].id = resMgr->Find(MemResAudio,sample[n].num)) ||
			 (sample[n].id = resMgr->Find(MemResWAVE, sample[n].num))) {
			sample[n].attrs = sample[n].id.Attrs();
			memMgr->SetNotDiscardable(sample[n].id);
			sample[n].fd = sample[n].audFD = -1;
/*
		int	ndx;
		// FIRST: Search for .AUD file via 'patchDir=' dir list
		} else if ((ndx = FindPatchEntry(MemResAudio, sample[n].num)) != -1) {
			sprintf(pathName, "%s%s", patchDir[ndx], resMgr->MakeName(MemResAudio, sample[n].num));
			sample[n].fd = sample[n].audFD = Open(pathName, 0);

		// SECOND: Search for .WAV file via 'patchDir=' dir list
		} else if ((ndx = FindPatchEntry(MemResWAVE, sample[n].num)) != -1) {
			sprintf(pathName, "%s%s", patchDir[ndx], resMgr->MakeName(MemResWAVE, sample[n].num));
			sample[n].fd = sample[n].audFD = Open(pathName, 0);
*/

		// THIRD: Search for .AUD file via 'audio=' dir list
		} else if ((fd = resMgr->Open(MemResAudio, sample[n].num, pathName)) != -1 &&
				fd != sfxVolFD) {
			sample[n].fd = sample[n].audFD = fd;
			#ifdef DEBUG   
			if (n > 0 && configMgr->Get(configMgr->AudioMonitor))
				if (!msgMgr->Alert("Loading %d.AUD",sample[n].num))
					msgMgr->Fatal("User abort");
			#endif

		// FOURTH: Searh for .WAV file via 'wave=' dir list
		} else if ((fd = resMgr->Open(MemResWAVE, sample[n].num, pathName)) != -1 &&
				fd != sfxVolFD) {
			sample[n].fd = sample[n].audFD = fd;
			#ifdef DEBUG   
			if (n > 0 && configMgr->Get(configMgr->AudioMonitor))
				if (!msgMgr->Alert("Loading %d.WAV",sample[n].num))
					msgMgr->Fatal("User abort");
			#endif

		// LAST CHANCE: Search for AUD/WAV resource in SFX Resource Volume
		} else if ((sample[n].offset = FindAudEntry((ResNum)sample[n].num)) != -1) {
			sample[n].fd = sfxVolFD;
			#ifdef DEBUG   
			if (n > 0 && configMgr->Get(configMgr->AudioMonitor))
				if (!msgMgr->Alert("Loading %d.AUD",sample[n].num))
					msgMgr->Fatal("User abort");
			#endif

		// AUD/WAV resource NOT FOUND!
		} else
			return 0;

	} else {

		// Search for @ resource:

		// FIRST: Search for @ file via 'audio=' dir list
		MakeName36(MemResAudio, pathName, (ResNum)sample[n].module, sample[n].noun,
			sample[n].verb, sample[n].cond, sample[n].sequ);
		if ((fd = resMgr->Open(MemResAudio36, 0, pathName)) != -1 &&
				fd != audVolFD && fd != sfxVolFD) {
			sample[n].fd = sample[n].audFD = fd;
			#ifdef DEBUG   
			if (n > 0 && configMgr->Get(configMgr->AudioMonitor))
				if (!msgMgr->Alert("Loading %s",pathName))
					msgMgr->Fatal("User abort");
			#endif

		// LAST CHANCE: Search for @ resource in AUD Resource Volume */
		} else if ((sample[n].offset = FindAud36Entry((ResNum)sample[n].module,
						sample[n].noun,sample[n].verb,sample[n].cond,sample[n].sequ))
					!= -1) {
			sample[n].fd = audVolFD;
			#ifdef DEBUG   
			if (n > 0 && configMgr->Get(configMgr->AudioMonitor))
				if (!msgMgr->Alert("Loading %s",pathName))
					msgMgr->Fatal("User abort");
			#endif

		// @ resource NOT FOUND!
		} else
			return 0;
	}

	if (!sample[n].robot && !CheckSOLFormat(n) && !CheckWAVEFormat(n))
		CheckRawFormat(n);

	if (preLoad && n+1 >= preLoad && sample[n].fd > 0) {
		sample[n].id.Get(MemAudioBuffer, sample[n].length, AUDIO_TEMP);
		sample[n].attrs = sample[n].id.Attrs();
		memMgr->SetNotDiscardable(sample[n].id);
		sample[n].id.Read(sample[n].fd);
		sample[n].start = 0;
	}

	sample[n].offset = sample[n].start;
	sample[n].bytesToRead = sample[n].length;
	sample[n].bytesRead = 0;
	sample[n].bufferSize = intHbufferSize;
	sample[n].bytesBuffered = 0;
	sample[n].bufferNumber = 0;
	sample[n].fillToggle = 0;
	sample[n].distortion = 0;

	if (sample[n].id)
		sample[n].bytesRead = sample[n].start;
	if (n == 0) {
		if (!rateSet)
			AudioRate(sample[n].rate);
		if (!bitsSet)
			AudioBits(sample[n].bits);
		if (!channelsSet)
			AudioChannels(sample[n].channels);
	}

	if (sample[n].compressed) {
		sample[n].doDecomp = True;
		sample[n].compVal8 = 0x80;
		sample[n].compVal16 = 0;
		sample[n].bufferSize /= 2;
		GetConvBuff(n);
	} else 
		sample[n].doDecomp = False;

	if (sample[n].bits > bits) {
		sample[n].do16Conv8 = True;
		sample[n].bufferSize *= 2;
		if (sample[n].compressed)
			sample[n].bufferSize *= 2;
		GetConvBuff(n);
		if (sample[n].compressed)
			sample[n].bufferSize /= 2;
	} else
		sample[n].do16Conv8 = False;

	if (sample[n].bits < bits) {
		sample[n].do8Conv16 = True;
		sample[n].bufferSize /= 2;
	} else
		sample[n].do8Conv16 = False;

	if (sample[n].rate > rate) {
		sample[n].do22KConv11K = True;
		sample[n].bufferSize *= 2;
		if (sample[n].compressed)
			sample[n].bufferSize *= 2;
		GetConvBuff(n);
		if (sample[n].compressed)
			sample[n].bufferSize /= 2;
	} else
		sample[n].do22KConv11K = False;

	if (sample[n].rate < rate) {
		sample[n].do11KConv22K = True;
		sample[n].bufferSize /= 2;
	} else
		sample[n].do11KConv22K = False;

	sample[n].buffID.Get(MemAudioBuffer, intHbufferSize * 2, AUDIO_TEMP);
	FillBuffer(n);
	FillBuffer(n);

	return 1 +
			(sample[n].length * (sample[n].doDecomp?2:1) * 60) /
			(sample[n].bits/8 * sample[n].channels * (int)sample[n].rate);
}

void
GetConvBuff(int n)
{
	int size = sample[n].robot? intHbufferSize * 2 : sample[n].bufferSize;

	if (!convBuff)
		convBuff.Get(MemAudioBuffer, size, AudioConvBufHandle);
	else if (convBuff.Size() < size)
		convBuff.Realloc(size);
}

void
MakeName36(MemType type, char* fname, ResNum module, uchar noun,
			  uchar verb, uchar cond, uchar sequ)
{
	if (type == MemResSync)
		fname[0] = 'S';
	else
		fname[0] = 'A';
	ConvBase36(&fname[1],module,3);
	ConvBase36(&fname[4],(int)noun,2);
	ConvBase36(&fname[6],(int)verb,2);
	fname[8] = '.';
	ConvBase36(&fname[9],(int)cond,2);
	ConvBase36(&fname[11],(int)sequ,1);
	fname[12] = '\0';
}

void
ConvBase36(char *str, int num10, int digits)
{
	int	n, t;

	t = 0;
	if (digits >= 3) {
		str[t++] = GetDigit36(n = num10 / (36*36));
		num10 -= n * (36*36);
	}
	if (digits >= 2) {
		str[t++] = GetDigit36(n = num10 / 36);
		num10 %= 36;
	}
	str[t] = GetDigit36(n = num10);
}

char
GetDigit36(int n)
{
	if (n <=  9)
		return((char)('0' + n));
	else
		return((char)('A' + n - 10));
}

int
FindAudEntry(ResNum id)
{
	int				offset;
	MemID				map;
	ResAudEntry*	entry;

	if (sfxVolFD == -1)
		return -1;

	if (!resMgr->Check(MemResMap, SFXMODNUM))
		return -1;
	map = resMgr->Get(MemResMap, SFXMODNUM);

	offset = 0;
	for (entry = (ResAudEntry *)*map; entry->id != (ushort)-1; ++entry) {
		offset += ((ulong)entry->offsetMSB << 16) + (ulong)entry->offsetLSW;
		if	(entry->id == id)
			return offset;
	}
	return -1;
}

int
FindAud36Entry(ResNum module, uchar noun, uchar verb, uchar cond, uchar sequ)
{
	int				offset;
	MemID				map;
	char*				ptr36;
	ResAud36Entry*	entry36;

	if (audVolFD == -1)
		return(-1);

	if (!resMgr->Check(MemResMap, module))
		return(-1);
	map = resMgr->Get(MemResMap, module);

	ptr36 = (char *)*map;
	offset = *(int *)ptr36;
	ptr36 += 4;
	for (entry36 = (ResAud36Entry*)ptr36; entry36->flag.sequ != 255;
			entry36 = (ResAud36Entry*)ptr36) {
		offset += ((ulong)entry36->offsetMSB << 16) + (ulong)entry36->offsetLSW;
		if	(entry36->noun == noun && entry36->verb == verb &&
			 entry36->cond == cond && (entry36->flag.sequ & SEQUMASK) == sequ) {
			if (entry36->flag.sync & SYNCMASK) {
				offset += entry36->syncLen;
				if (entry36->flag.rave & RAVEMASK)
					offset += entry36->raveLen;
			}
			return(offset);
		}
		ptr36 += sizeof(ResAud36Entry);
		if (!(entry36->flag.sync & SYNCMASK))
			ptr36 -= sizeof(entry36->syncLen);
		if (!(entry36->flag.rave & RAVEMASK))
			ptr36 -= sizeof(entry36->raveLen);
	}
	return -1;
}

void
AudioPlay()
{
	AudioCheck();
	if (activeSamples == 1) {
		DACBuffer.adr.ofs = DACBuffer.adr.ofsOrig;
		MergeBuffers();
		DACArgsIn[1] = MergeBuffers();
		CallDAC(DAC_PLAY,1);  // begin playing...
	} else if (lastDAC) {
		lastDAC = False;
		// continue playing (i.e. ignore prior "LAST BUFFER" message)
		CallDAC(DAC_CONTINUE,0);
	}
	sample[activeSamples-1].startLoc = timeMgr->GetTickCount();
	if (activeSamples == 1)
		globalLoc = sample[0].startLoc;
}

void
AudioStop(int n)
{
	int	t;

	if (n == NO_SAMPLES || !activeSamples)
		return;
	if (n == ALL_SAMPLES) {
		// Stop all samples
		for (t = 0; t < activeSamples; t++)
			DropSample(t);
		activeSamples = 0;
	} else {
		DropSample(n);
		--activeSamples;
		for (t = n; t < activeSamples; t++)
			sample[t] = sample[t+1];
	}
	if (!activeSamples) {
		// No samples remain for playback...
		CallDAC(DAC_STOP,0);
		if (convBuff) {
			convBuff.Free();
			convBuff = 0;
		}
	}
}

void
DropSample(int n)
{
	if ((int)sample[n].id && (sample[n].attrs & DISCARDABLE))
		memMgr->SetDiscardable(sample[n].id);
	if (sample[n].audFD != -1) {
		Close(sample[n].audFD);
		sample[n].audFD = -1;
	}
	sample[n].buffID.Free();
}

Bool
AudioPause(int n)
{
	int	ticks = timeMgr->GetTickCount();

	if (n == NO_SAMPLES)
		return False;
	if (n == ALL_SAMPLES) {
		if (!pausedLoc) {
			pausedLoc = ticks;
			return True;
		}
	} else if (!sample[n].pausedLoc) {
		sample[n].pausedLoc = ticks;
		return True;
	}
	return False;
}

Bool
AudioResume(int n)
{
	int	t, ticks = timeMgr->GetTickCount();

	if (n == NO_SAMPLES)
		return False;
	if (n == ALL_SAMPLES) {
		if (pausedLoc) {
			for (t = 0; t < activeSamples; t++)
				if (!sample[t].pausedLoc)
					sample[t].startLoc += ticks - pausedLoc;
			globalLoc += ticks - pausedLoc;
			pausedLoc = 0;
			return True;
		}
	} else if (sample[n].pausedLoc) {
		sample[n].startLoc += ticks - sample[n].pausedLoc;
		sample[n].pausedLoc = 0;
		return True;
	}
	return False;
}

int
AudioLoc(int n)
{
/* Notes on LOC:
	A return value of -1 indicates that no audio is currently playing.
	A return value of 0 through 65534 indicates the time in sixtieths of
	a second that the current audio selection has been playing for.
*/
	int	ticks = timeMgr->GetTickCount();

	if (n == NO_SAMPLES || (!activeSamples && !CallDAC(DAC_LOC,0)))
		return -1;
	else if (n == ALL_SAMPLES)
		if (pausedLoc)
			return pausedLoc - globalLoc;
		else
			return ticks - globalLoc;
	else if (sample[n].pausedLoc)
		return sample[n].pausedLoc - sample[n].startLoc;
	else if (pausedLoc)
		return pausedLoc - sample[n].startLoc;
	else
		return ticks - sample[n].startLoc;
}

void
AudioRate(int request)
{
	DACArgsIn[1] = Min(request,rateMax);
	if (rate != DACArgsIn[1]) {
		rate = DACArgsIn[1];
		CallDAC(DAC_SET_RATE,1);
	}
}

void
AudioBits(int request)
{
	DACArgsIn[1] = Min(request,bitsMax);
	if (bits != DACArgsIn[1]) {
		bits = DACArgsIn[1];
		CallDAC(DAC_SET_BITS,1);
	}
}

void
AudioChannels(int request)
{
	DACArgsIn[1] = Min(request,channelsMax);
	if (channels != DACArgsIn[1]) {
		channels = DACArgsIn[1];
		CallDAC(DAC_SET_CHANNELS,1);	 
	}
}

void
AudioDistort(int request, int n)
{
	if (n == ALL_SAMPLES)
		distortion = request;
	else if (n != NO_SAMPLES)
		sample[n].distortion = request;
}

void
AudioVolume(int request, int n)
{
	if (n == ALL_SAMPLES)
		volume = Min(request,MAXVOLUME);
	else if (n != NO_SAMPLES)
		sample[n].volume = Min(request,MAXVOLUME);
}

Bool
MergeBuffers()
{
	int	n, cnt = 0;

	if (!activeSamples)
		return False;

	lastDAC = False;
	if (!pausedLoc)
		for (n = 0; n < activeSamples; n++)
			if (sample[n].bytesBuffered && !sample[n].pausedLoc)
				MergeBuffer(n,cnt++);
	if (cnt) {
		DistortBuffer(ALL_SAMPLES,DACBuffer.ptr);
		VolumeBuffer(ALL_SAMPLES,DACBuffer.ptr);
	} else
		Silence(DACBuffer.ptr,DACHbufferSize,bits);
	if (DACBuffer.adr.ofs == DACBuffer.adr.ofsOrig)
		DACBuffer.adr.ofs += DACHbufferSize;
	else
		DACBuffer.adr.ofs -= DACHbufferSize;
	if (cnt)
		return True;
	for (n = 0; n < activeSamples; n++)
		if (sample[n].bytesToRead || sample[n].pausedLoc)
			return True;
	lastDAC = True;
	return False;
}

void
MergeBuffer(int n, int cnt)
{

	char*	buffer = (char*)*sample[n].buffID +
		sample[n].bufferNumber * DACHbufferSize;

	if (sample[n].robot) {
		static int bufferNumber;
		if (rbot.min == 0)
			bufferNumber = 0;
		bufferNumber++;
		rbot.min += DACHbufferSize * (16/bits);
		rbot.max += DACHbufferSize * (16/bits);
		if (rbot.min > rbot.hwm[1]) {
			if (rbot.min > rbot.hwm[0]) {
				Silence((char far *)buffer,DACHbufferSize,bits);
				rbot.hwm[0] = bufferNumber * DACHbufferSize * (16/bits);
				rbot.hwm[1] = bufferNumber * DACHbufferSize * (16/bits) + 2;
			} else {
				DupeChannel(buffer,DACHbufferSize/(2*(bits/8)),1,bits);
				rbot.hwm[1] = bufferNumber * DACHbufferSize * (16/bits) + 2;
			}
		} else if (rbot.min > rbot.hwm[0]) {
			DupeChannel(buffer,DACHbufferSize/(2*(bits/8)),0,bits);
			rbot.hwm[0] = bufferNumber * DACHbufferSize * (16/bits);
		}
	}

	DistortBuffer(n,(char far *)buffer);
	VolumeBuffer(n,(char far *)buffer);
	if (!cnt)
		_fmemmove(DACBuffer.ptr,(char far *)buffer,DACHbufferSize);
	else
		Merge(DACBuffer.ptr,buffer,DACHbufferSize,bits,mixCheck);

	if (++sample[n].bufferNumber == intHbufferSize / DACHbufferSize * 2)
		sample[n].bufferNumber = 0;
	sample[n].bytesBuffered -= Min(DACHbufferSize,sample[n].bytesBuffered);
}

void
FillBuffer(int n)
{
	static int	loopOfs = 0;

	if (sample[n].robot || loopOfs == sample[n].bufferSize || !sample[n].bytesToRead) {
		sample[n].fillToggle = 1 - sample[n].fillToggle;
		return;
	}
		
	int	len;
	int	fillOfs = sample[n].fillToggle * intHbufferSize;
	int	t = sample[n].bufferSize -
		loopOfs * (sample[n].do16Conv8? 2:1) * (sample[n].do22KConv11K? 2:1) /
		((sample[n].do8Conv16? 2:1) * (sample[n].do11KConv22K? 2:1));
	if (sample[n].bytesToRead > t)
		len = t;
	else
		len = sample[n].bytesToRead;
	sample[n].bytesToRead -= len;

	if (sample[n].do16Conv8 && sample[n].doDecomp) {
		if (sample[n].id) {
			memcpy((char *)*convBuff+len,&sample[n].id[sample[n].bytesRead],len);
			sample[n].bytesRead += len;
		} else {
			LSeek(sample[n].fd,sample[n].offset,SEEK_SET);
			convBuff.Read(sample[n].fd,len,len);
			sample[n].offset += len;
		}
	} else if (sample[n].do16Conv8 || sample[n].doDecomp) {
		if (sample[n].id) {
			memcpy(*convBuff,&sample[n].id[sample[n].bytesRead],len);
			sample[n].bytesRead += len;
		} else {
			LSeek(sample[n].fd,sample[n].offset,SEEK_SET);
			convBuff.Read(sample[n].fd,0,len);
			sample[n].offset += len;
		}
	} else if (sample[n].id) {
		memcpy(&sample[n].buffID[fillOfs+loopOfs],&sample[n].id[sample[n].bytesRead],len);
		sample[n].bytesRead += len;
	} else {
		LSeek(sample[n].fd,sample[n].offset,SEEK_SET);
		sample[n].buffID.Read(sample[n].fd,fillOfs+loopOfs,len);
		sample[n].offset += len;
	}

	if (sample[n].doDecomp) {
		if (sample[n].bits == 16) {
			if (sample[n].do16Conv8)
				sample[n].compVal16 =
					Decomp8To16(*convBuff,(char *)*convBuff+len,len,sample[n].compVal16);
			else
				sample[n].compVal16 =
					Decomp8To16(&sample[n].buffID[fillOfs+loopOfs*2],*convBuff,len,sample[n].compVal16);
		} else {
			sample[n].compVal8 =
				Decomp4To8(&sample[n].buffID[fillOfs+loopOfs*2],*convBuff,len,sample[n].compVal8);
		}
		len *= 2;
	}

	if (sample[n].do16Conv8) {
		len /= 2;
		Conv16To8(&sample[n].buffID[fillOfs+loopOfs],*convBuff,len);
	}

	if (sample[n].do8Conv16) {
		Conv8To16(&sample[n].buffID[fillOfs+loopOfs],len);
		len *= 2;
	}

	if (sample[n].do11KConv22K) {
		Conv11KTo22K(&sample[n].buffID[fillOfs+loopOfs],len,bits);
		len *= 2;
	}

	if (!sample[n].bytesToRead && sample[n].loop) {
		sample[n].compVal8 = 0x80;
		sample[n].compVal16 = 0;
		sample[n].bytesToRead = sample[n].length;
		sample[n].offset = sample[n].start;
		if (sample[n].id)
			sample[n].bytesRead = sample[n].start;
		else
			sample[n].bytesRead = 0;
		sample[n].bytesBuffered += len;
		if (sample[n].bufferSize != (loopOfs += len))
			FillBuffer(n);
		else
			sample[n].fillToggle = 1 - sample[n].fillToggle;
		loopOfs = 0;
		return;
	}
	
	if (len < intHbufferSize && !loopOfs) {
		// pad this partial buffer to next DAC-buffer boundary and adjust len
		int newlen = (len + DACHbufferSize - 1) / DACHbufferSize * DACHbufferSize;
		Silence((char far *)&sample[n].buffID[fillOfs+loopOfs+len],newlen-len,bits);
		len = newlen;
	}
	sample[n].bytesBuffered += len;
	sample[n].fillToggle = 1 - sample[n].fillToggle;
	return;
}

void
DistortBuffer(int n, char far *buffer)
{
	int	distortMask = (n == ALL_SAMPLES)?
				~distortion : ~sample[n].distortion;

	if (distortMask != -1)
		Distort(buffer,DACHbufferSize,bits,distortMask);
}

void
VolumeBuffer(int n, char far *buffer)
{
	int	vol;

	if (n == ALL_SAMPLES) {
		vol = volume;
		if (volumeMax > 1) {
			DACArgsIn[1] = vol * (volumeMax-1) / MAXVOLUME;
			CallDAC(DAC_SET_VOLUME,1);
			return;
		}
	} else
		vol = sample[n].volume;
	if (vol == MAXVOLUME)
		return;
	if (!vol) {
		Silence(buffer,DACHbufferSize,bits);
		return;
	}
	Volume(buffer,DACHbufferSize,bits,vol/8);
}

Bool
CheckSOLFormat(int n)
{
	// If the sample is in SOL format, the file pointer will be left
	// pointing at the first 'playable' byte:

	uchar	header[256], flag;
	uchar*	sptr = 0;

	if (sample[n].id) {
		sptr = (uchar*)*sample[n].id;
		memcpy(header,sptr,6);
		sptr += 6;
	} else {
		LSeek(sample[n].fd,sample[n].offset,SEEK_SET);
		Read(sample[n].fd,header,6);	// 2-byte header + 4-byte signature
	}
	if ((header[0] != (uchar)(MemResAudio|0x80)) || strcmp((char*)&header[2],"SOL"))
		return False;
	if (sptr) {
		sample[n].rate = ((uint)*sptr) + ((uint)(*(sptr+1)) << 8);
		flag = *(sptr+2);
		sample[n].length = ((uint)*(sptr+3)) + ((uint)(*(sptr+4)) << 8) +
			((uint)(*(sptr+5)) << 16) + ((uint)(*(sptr+6)) << 24);
		sample[n].start = (int)header[1]+2;
	} else {
		Read(sample[n].fd,&sample[n].rate,2);
		Read(sample[n].fd,&flag,1);
		Read(sample[n].fd,&sample[n].length,4);
		LSeek(sample[n].fd,(int)header[1]-11,SEEK_CUR);
		sample[n].start = LSeek(sample[n].fd,0,SEEK_CUR);
	}
	sample[n].bits = flag & 4 ? 16 : 8;
	sample[n].channels = flag & 16 ? 2 : 1;
	sample[n].compressed = flag & 1;
	sample[n].scrambled = flag & 2;
	sample[n].length &= 0xFFFFFFFC;
	return True;
}

Bool
CheckWAVEFormat(int n)
{
	// If the sample is in WAVE format, the file pointer will be left
	// pointing at the first 'playable' byte:

	char*	sptr = 0;
	int slen;
	int extra, br;
	char	okRIFF, okWAVE, okFMT;
	_Packed struct {
		char	id[4];
		long	len;
	} chunk;
	_Packed struct {
		short	fmttag;
		short	channels;
		long	rate;
		long	bytespersec;
		short	blockalign;
		short	bits;	
	} wf;

	if (sample[n].id) {
		sptr = (char*)*sample[n].id;
		slen = sample[n].id.Size();
	} else {
		LSeek(sample[n].fd,sample[n].offset,SEEK_SET);
	}
	okRIFF = okWAVE = okFMT = 0;
	while(1) {
		if (sptr) {
			if (slen < sizeof(chunk))
				break;
			memcpy((char*)&chunk,sptr,sizeof(chunk));
			sptr += sizeof(chunk); 
			slen -= sizeof(chunk);
		} else {
			if (Read(sample[n].fd,(char*)&chunk,sizeof(chunk)) != sizeof(chunk))
				break;
		}

		if (!strncmp(chunk.id,"RIFF",4)) {
			okRIFF = 1;
			continue;
		}

		if (!strncmp(chunk.id,"WAVE",4)) {
			// WAVE chunk does not have a length dword:
			if (sptr) {
				sptr -= sizeof(chunk.len);
				slen += sizeof(chunk.len);
			} else {
				LSeek(sample[n].fd,-sizeof(chunk.len),SEEK_CUR);
			}
			okWAVE = 1;
			continue;
		}

		if (!strncmp(chunk.id,"fmt ",4)) {
			if (sptr) {
				if (slen < sizeof(wf))
					break;
				memcpy((char *)&wf,sptr,sizeof(wf));
				sptr += sizeof(wf);
				slen -= sizeof(wf);
			} else {
				if (Read(sample[n].fd,(char *)&wf,sizeof(wf)) != sizeof(wf))
					break;
			}
			br = sizeof(wf);
			if (wf.fmttag == 2) {   // compressed wave file
				if (sptr) {
					if (slen < sizeof(int))
						break;
					memcpy(&extra,sptr,sizeof(int));
					sptr += sizeof(int) + extra;
					slen -= sizeof(int) + extra;
				} else {
					if (Read(sample[n].fd,&extra,sizeof(int)) != sizeof(int))
						break;
					LSeek(sample[n].fd,extra,SEEK_CUR);
				}
				br += sizeof(int) + extra;
				sample[n].compressed = True;
			} else
				sample[n].compressed = False;
			sample[n].rate = (int)wf.rate;
			sample[n].bits = wf.bits;
			if (sample[n].compressed  || (sample[n].bits != 8 && sample[n].bits != 16))
				msgMgr->Fatal(SrcLoc,"Can't play compressed WAVE audio");
			sample[n].channels = wf.channels;
			sample[n].scrambled = False;
			if (sptr) {
				sptr += chunk.len - br + (chunk.len & 1);
				slen -= chunk.len - br + (chunk.len & 1);
			} else {
				LSeek(sample[n].fd,chunk.len - br + (chunk.len & 1),SEEK_CUR);
			}
			okFMT = 1;
			continue;
		}

		if (!strncmp(chunk.id,"data",4)) {
			if (!okRIFF || !okWAVE || !okFMT)
				break;
			if (sample[n].id) {
				sample[n].start = sptr - (char*)*sample[n].id;
			} else {
				sample[n].start = LSeek(sample[n].fd,0,SEEK_CUR);
			}
			sample[n].length = chunk.len & 0xFFFFFFFC;
			return True;
		}

		// Unknown chunk type -- skip over it:
		if (sptr) {
			sptr += chunk.len + (chunk.len & 1);
			slen -= chunk.len + (chunk.len & 1);
		} else {
			LSeek(sample[n].fd,chunk.len + (chunk.len & 1),SEEK_CUR);
		}
	}
	return False;
}

void
CheckRawFormat(int n)
{

	if (sample[n].id) {
		sample[n].length = sample[n].id.Size();
	} else {
		LSeek(sample[n].fd,sample[n].offset,SEEK_SET);
		sample[n].length = FileLength(sample[n].fd);
	}
	sample[n].start = 0;
	sample[n].rate = rate;
	sample[n].bits = bits;
	sample[n].channels = channels;
	sample[n].scrambled = False;
	sample[n].compressed = False;
	return;
}

Bool
InitAudioDriver()
{
	unsigned	n, dSize, aSize;

	dSize = configMgr->GetNum("DACSize", 0, GetDACDefaultSize());
	if (!dSize)
		return False;

	aSize = configMgr->GetNum("audioSize", 0, INT_HBUFFSIZE*2);
	if (!aSize)
		return False;

	AdjustAudBuffSizes(&aSize,&dSize);

	intHbufferSize = aSize/2;
	DACHbufferSize = dSize/2;

	/* If audio is not requested -- vamboose
	 */

	audioDriver.data = LoadAudioDrv();
	if (!audioDriver.data)
		return False;

 	if ((DACArgsIn[1] = configMgr->GetNum("audioIRQ",0)))
		CallDAC(DAC_SET_IRQ,1);
	if ((DACArgsIn[1] = configMgr->GetNum("audioDMA",0)))
		CallDAC(DAC_SET_DMA,1);

	if ((n = CallDAC(DAC_INIT,0)) != 0) {
		if (n != NO_DAC)
			msgMgr->Alert(SrcLoc, Msg_NoAudio);
		FreeAudioDrv();
		return False;
	}

//	if ((n = CallDAC(DAC_GET_REALOC,0)))
//		audioDrv.Realloc(n);

	channelsMax = CallDAC(DAC_GET_MAX_CHNLS,0);
	bitsMax = CallDAC(DAC_GET_MAX_BITS,0);
	AudioBits(bitsMax);
	rateMax = CallDAC(DAC_GET_MAX_RATE,0);
	volumeMax = CallDAC(DAC_GET_VOL,0);

//	InstallServer(AudioServer, 1);

	if (bitsMax == 8)
		DACHbufferSize /= 2;

	if (!GetDACBuffer())
		return False;

	audioInstalled = True;
	
#ifdef DEBUG
//	if (configMgr->Get(configMgr->PlayAudio))
//		DACPlay();
#endif

	return True;
}

void
AdjustAudBuffSizes(unsigned *audioSize, unsigned *DACSize)
{

	*audioSize = 65536;

	*audioSize = ((*audioSize + 1023) / 1024) * 1024;
	*DACSize   = ((*DACSize + 1023) / 1024) * 1024;
	*DACSize   = *audioSize/((*audioSize/(*DACSize)+1)&0xfffffffe);
}

Bool
GetDACBuffer()
{
	DACArgsIn[1] = DACHbufferSize;
	DACArgsIn[2] = (int)MergeBuffers;
	if (CallDAC(DAC_SET_BUFF,2)) {
		msgMgr->Alert(SrcLoc, Msg_NoAudio);
		FreeAudioDrv();
		return False;
	}
	DACBuffer.adr.sel = CallDAC(DAC_GET_BUFF_SEL,0);
	DACBuffer.adr.ofsOrig = CallDAC(DAC_GET_BUFF_OFS,0);
	return True;
}

void
InitAudioVols()
{
	char		pathName[MaxPath + 1];
	char*		cp;

	/* Open optional Audio sound effects Volume */
	if (sfxVolFD != -1) {
		Close(sfxVolFD);
		resMgr->Release(MemResMap,SFXMODNUM);
	}
	strcpy(pathName, configMgr->Get("ressfx",0));
	if (strlen(pathName)) {
		cp = &pathName[strlen(pathName)-1];
		if (*cp != ':' && *cp != '\\')
			strcat(pathName,"\\");
	}
	strcat(pathName,SFXVOLNAME);
	sfxVolFD = Open(pathName, O_RDONLY);

	/* Open optional Base-36 Speech/Sync/Rave Volume */
	if (audVolFD != -1)
		Close(audVolFD);
	strcpy(pathName, configMgr->Get("resaud",0));
	if (strlen(pathName)) {
		cp = &pathName[strlen(pathName)-1];
		if (*cp != ':' && *cp != '\\')
			strcat(pathName,"\\");
	}
	strcat(pathName,AUDVOLNAME);
	audVolFD = Open(pathName, O_RDONLY);
}

#ifdef DEBUG
void
DACPlay()
{
	SOL_Event event;
	
	char* str = configMgr->Str(configMgr->PlayAudio);
	if (strlen(str) == 12 && (str[0] == 'A' || str[0] == 'a')) {
		sample[0].num = 0;
		sample[0].module = GetBase36(&str[1],3);
		sample[0].noun = GetBase36(&str[4],2);
		sample[0].verb = GetBase36(&str[6],2);
		sample[0].cond = GetBase36(&str[9],2);
		sample[0].sequ = GetBase36(&str[11],1);
	} else if (strlen(str)) {
		sample[0].num = atoi(str);
		sample[0].module = SFXMODNUM;
	}
	sample[0].distortion = 0;
	sample[0].volume = MAXVOLUME;
	sample[0].loop = False;
	printf("(%s)Press any key to stop audio playback...\n",str);
	while (1) {
		if (!AudioSelect(0)) {
			printf("Not found\n");
			while (!kbdMgr->Get(&event))
				;
			break;
		}
		++activeSamples;
		AudioPlay();
		while (activeSamples) {
			if (kbdMgr->Get(&event)) {
				AudioStop(ALL_SAMPLES);
				exit(1);
			}
			AudioServer();
		}
	}
	exit(1);
}

int
GetBase36(char* str36,int digits)
{
	uint	num36, n;

	num36 = 0;
	for (n = 0; n < digits; n++) {
		num36 *= 36;
		if (str36[n] < '0' || str36[n] > '9' && str36[n] < 'A' ||
				str36[n] > 'Z' && str36[n] < 'a' || str36[n] > 'z') {
			return 0;
		}
		if (str36[n] <= '9')
			num36 += str36[n] - '0';
		else if (str36[n] <= 'Z')
			num36 += str36[n] - 'A' + 10;
		else
			num36 += str36[n] - 'a' + 10;
	}
	return num36;
}
#endif

void
EndAudio()
{
	if (!audioInstalled)
		return;
	AudioStop(ALL_SAMPLES);
	CallDAC(DAC_TERMINATE,0);
	FreeAudioDrv();
	if (sfxVolFD != -1)
		Close(sfxVolFD);
	if (audVolFD != -1)
		Close(audVolFD);
	audioInstalled = False;
}

int
CallDAC(int func, int cnt)
{
	DACArgsIn[0] = cnt;
	return audioDriver.code(func,DACArgsIn);
}

void
AudioServer()
{
	int	n;

	AudioCheck();
	for (n = 0; n < activeSamples; n++) {
		if (sample[n].bytesBuffered <= intHbufferSize && sample[n].bytesToRead)
			FillBuffer(n);
		if (sample[n].fadeStep)
			if (AudioFade(n))
				AudioStop(n--);
	}
}

Bool
AudioFade(int n)
{
	if (sample[n].fadeLoc + sample[n].fadeTicks <= timeMgr->GetTickCount()) {
		if (!--sample[n].fadeStep) {
			if (sample[n].fadeEnd)
				return True;
			else
				AudioVolume(sample[n].fadeVol,n);
		} else{
			int volume = sample[n].volume -
				(sample[n].volume - sample[n].fadeVol) / (sample[n].fadeStep+1);
			if (volume == sample[n].fadeVol)
				sample[n].fadeStep = 1;
			AudioVolume(volume,n);
			sample[n].fadeLoc = timeMgr->GetTickCount();
		}
	}
	return False;
}

void
AudioCheck()
{
	int	n;

	for (n = 0; n < activeSamples; n++)
		if (!(sample[n].bytesToRead || sample[n].bytesBuffered))
			if (activeSamples > 1)
				AudioStop(n--);
			else
				if (!CallDAC(DAC_LOC,0))
					AudioStop(n);
}
