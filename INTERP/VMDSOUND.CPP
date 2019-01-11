//***************************************************************************
//                                                                          *
//   Copyright (c)1995,M.D.O.   Programmeur:Mathieu___  Création:21/02/95   *
//                                                                          *
//   Portage :  [X]:PC  [_]:MAC  [_]:CDI  [_]:3DO  [_]:SATURN  [_]:PSX      *
//              [_]:Spécifique                                              *
//                                                                          *
//   Type de source :  [X]:Interne   [_]:Externe   [_]:Grand public         *
//                                                                          *
//   Programme rattaché : [X]:Librairies  [_]:Loader                        *
//                        [_]:Autre : ______________                        *
//                                                                          *
//***************************************************************************


#define INCLUDE_MMSYSTEM_H
#include "VMDMDO.HPP"
#include "VMD.HPP"
#include <stdlib.h>
#include <IO.H>
#include "audio.hpp"
#include "msg.hpp"
#include "mbox.hpp"

#ifdef WINDOWS
#ifndef WIN32S
UINT __pascal _16waveOutPrepareHeader(HWAVEOUT hWaveOut, DWORD, UINT uSize);
UINT __pascal _16waveOutUnprepareHeader(HWAVEOUT hWaveOut, DWORD, UINT uSize);
UINT __pascal _16waveOutWrite(HWAVEOUT hWaveOut, DWORD, UINT uSize);
UINT __pascal _16waveOutGetPosition(HWAVEOUT hWaveOut, DWORD, UINT uSize);
#endif
#endif

//**********************************************************************
//                                                                     *
//                                                                     *
//  Here is the sound manager...I don't think you have to change it    *
//  under Windows....But you have to recreate it under DOS...          *
//                                                                     *
//**********************************************************************


#ifdef WINDOWS
void	DisplayError (WORD, char *);
void	closeWaveDev(void);

static	MMTIME lpt;
static	PCMWAVEFORMAT Format;
static	HWAVEOUT	hSnd = NULL;
static	WAVEHDR		OneWaveout;
#ifndef WIN32S
static	DWORD			wHdr_alias;
static	DWORD			lpt_alias;
#endif
static	UINT		wError;
#else
void		MyCallback(void);
extern	int		DACArgsIn[];
extern	void		AudioRate(int);
extern	void		AudioBits(int);
#endif

#ifndef WINDOWS
static	uint		callBacks;
static	uint		DACSize;
#endif
static	int		RefFrequence;
static	LONG		TimerSoundBase;
static	LONG		CountSoundBase;
static	LONG		WinSoundBase;
static	LONG		WinSoundBase0;
int		RefSoundFrequence=22157;
BOOL		volatile SoundInProcess;

                                                                          
#pragma	argsused

void far * GXgetSoundMem(size_t size)
{
#ifdef WINDOWS
#ifdef WIN32S
HANDLE	hAudBuf;
void * 	lpt;

	hAudBuf = GlobalAlloc(GMEM_MOVEABLE, (DWORD)size);
	if (!hAudBuf)
		MessageBox (NULL, "snd alloc fail", "", MB_OK);
	lpt = (void *)GlobalLock(hAudBuf);
	if (!lpt)
		MessageBox (NULL, "snd lock fail", "", MB_OK);
	return lpt;

#else
HANDLE	hAudBuf;
LPVOID	lpMem16;

	hAudBuf = GlobalAlloc(GMEM_MOVEABLE, (DWORD)size);
	if (!hAudBuf)
		MessageBox (NULL, "snd alloc fail", "", MB_OK);
	lpMem16 = (LPVOID)GlobalLock(hAudBuf);
	if (!lpMem16)
		MessageBox (NULL, "snd lock fail", "", MB_OK);
	return ((void far *)MK_FP32(lpMem16));
#endif

#else
union {
	_Packed struct {
		int	ofs;
		short	sel;
	} adr;
	char far *ptr;
} DACBuffer;

	if (size > 65536) {
		msgMgr->Alert("Audio buff too big");
		return (void far *)0;
	}

	CallDAC(DAC_RELEASE_MEM,0);

	if ((size % 16) == 1)		//make sure we don't lose a byte
		size+=1;
	DACArgsIn[1] = (size)/2;
	DACArgsIn[2] = (int)MyCallback;
	if (CallDAC(DAC_SET_BUFF,2)) {
		msgMgr->Alert(SrcLoc, Msg_NoAudio);
		return (void far *)0;
	}
	DACSize = size/2;
	DACBuffer.adr.sel = CallDAC(DAC_GET_BUFF_SEL,0);
	DACBuffer.adr.ofs = CallDAC(DAC_GET_BUFF_OFS,0);
	return (void far *)DACBuffer.ptr;
#endif
}

void GXreleaseSoundMem(void far * addr)
{
#ifdef WINDOWS
HANDLE	hAudBuf;

#ifdef WIN32S
	hAudBuf = (HGLOBAL)GlobalHandle((LPVOID)addr);
#else
LPVOID	lpMem16;

	lpMem16 = (LPVOID)MK_FP16((void far *)addr);
	hAudBuf = (HGLOBAL)GlobalHandle(SELECTOROF(lpMem16));
#endif
	GlobalUnlock(hAudBuf);
	GlobalFree(hAudBuf);
#else
	addr = addr;
	CallDAC(DAC_RELEASE_MEM,0);
	if (!GetDACBuffer())
		msgMgr->Alert("snd32 Get error");
#endif
}

#ifndef WINDOWS
void MyCallback()
{
  LONG T;

	++callBacks;
  T=ReadTimer();
  if (T-WinSoundBase>3000L)
  {
//    RefFrequence=1000L*callBacks*DACSize/(T-WinSoundBase0);
    RefFrequence=100L*callBacks*DACSize/((T-WinSoundBase0)/10);
    WinSoundBase=T;
  }
}
#endif

void GXstartSound(SOUND * Snd,WORD nbr,WORD freq)
{
#ifdef WINDOWS
#ifndef WIN32S
LPVOID	lpMem16;
#endif

//  GXstopSound(0);
  closeWaveDev();
  Format.wf.wFormatTag=WAVE_FORMAT_PCM;
  Format.wf.nChannels=1;
  Format.wf.nSamplesPerSec=freq;
  Format.wf.nAvgBytesPerSec=freq;
  Format.wf.nBlockAlign=1;
  Format.wBitsPerSample=8;
  wError = waveOutOpen(&hSnd,WAVE_MAPPER,(LPWAVEFORMAT)&Format,NULL,NULL,0);
	if (wError) {
		DisplayError(wError, "waveOutOpen");
		return;
	}
  if (abs(freq-RefFrequence)>1000)
    RefFrequence=freq;

  memset(&OneWaveout,0,sizeof(WAVEHDR));
#ifndef WIN32S
  lpMem16 = (LPVOID)MK_FP16((void far *)Snd->Start);
  OneWaveout.lpData=(char *)lpMem16;
#else
  OneWaveout.lpData=Snd->Start;
#endif
  OneWaveout.dwBufferLength=Snd->Size;
  OneWaveout.dwFlags=WHDR_BEGINLOOP+WHDR_ENDLOOP;
  if (!nbr)
    OneWaveout.dwLoops=-1;
  else
    OneWaveout.dwLoops=nbr;
#ifndef WIN32S
  wHdr_alias = AllocAlias16(&OneWaveout);
  lpt_alias = AllocAlias16(&lpt);
  wError = _16waveOutPrepareHeader (hSnd, wHdr_alias, sizeof(WAVEHDR));
#else
  wError = waveOutPrepareHeader(hSnd,&OneWaveout,sizeof(WAVEHDR));
#endif
	if (wError) {
		DisplayError(wError, "waveOutPrepareHeader");
		return;
	}
#ifndef WIN32S
  wError = _16waveOutWrite (hSnd, wHdr_alias, sizeof(WAVEHDR));
#else
  wError = waveOutWrite(hSnd,&OneWaveout,sizeof(WAVEHDR));
#endif
	if (wError) {
		DisplayError(wError, "waveOutWrite");
		return;
	}
  lpt.wType=TIME_BYTES;
#ifndef WIN32S
  wError = _16waveOutGetPosition(hSnd,lpt_alias,sizeof(lpt));
#else
  wError = waveOutGetPosition(hSnd,&lpt,sizeof(lpt));
#endif
	if (wError) {
		DisplayError(wError, "waveOutGetPosition1");
		return;
	}
  TimerSoundBase=ReadTimer();
  CountSoundBase=lpt.u.cb;
  WinSoundBase=TimerSoundBase;
  WinSoundBase-=CountSoundBase*1000L/(LONG)RefFrequence;
  WinSoundBase0=WinSoundBase;
  SoundInProcess=TRUE;
#else
	Snd = Snd;
	nbr = nbr;

  RefFrequence=freq;
  TimerSoundBase=ReadTimer();
  CountSoundBase=0;
  WinSoundBase=TimerSoundBase;
  WinSoundBase-=CountSoundBase*1000L/(LONG)RefFrequence;
  WinSoundBase0=WinSoundBase;

	callBacks = 0;
	AudioRate(freq);
	AudioBits(8);
	DACArgsIn[1] = (int)MyCallback;
	CallDAC(DAC_PLAY,1);
  SoundInProcess=TRUE;
#endif
}

void GXstopSound(WORD)
{
#ifdef WINDOWS
  HWAVEOUT hSnd2;

  SoundInProcess=FALSE;
  if (!hSnd)
	return;
  hSnd2=hSnd;
  hSnd=NULL;
  waveOutReset(hSnd2);
#ifndef WIN32S
  _16waveOutUnprepareHeader (hSnd2, wHdr_alias, sizeof(WAVEHDR));
#else
  waveOutUnprepareHeader(hSnd2,&OneWaveout,sizeof(WAVEHDR));
#endif
  waveOutClose(hSnd2);
#else
  SoundInProcess=FALSE;
	CallDAC(DAC_STOP,0);
#endif
}


LONG GetSoundPos(BOOL Mode)
{
#ifdef WINDOWS
  ULONG A;
  ULONG B;
  LONG T;
  LONG Cb,Tb;
#define BIAS 5000

  if (!hSnd)
	return 0;
  lpt.wType=TIME_BYTES;
#ifndef WIN32S
  wError = _16waveOutGetPosition(hSnd,lpt_alias,sizeof(lpt));
#else
  wError = waveOutGetPosition(hSnd,&lpt,sizeof(lpt));
#endif
	if (wError) {
		DisplayError(wError, "waveOutGetPosition1");
		return 0;
	}
  A=lpt.u.cb;
  if (!Mode)
//    return(A);
    return(A - BIAS);
  T=ReadTimer();
  if (T-WinSoundBase>3000L)
  {
//    RefFrequence=1000L*A/(T-WinSoundBase0);
    RefFrequence=100L*A/((T-WinSoundBase0)/10);
    WinSoundBase=T;
  }
  Cb=CountSoundBase;
  Tb=TimerSoundBase;

  B=Cb+((T-Tb)/10L)*(LONG)RefFrequence/100L;
  if (B>A)
//    return(B);
    return(B - BIAS);
  else
//    return(A);
    return(A - BIAS);
#else
  ULONG B;
  ULONG T;
  LONG Cb,Tb;

	Mode = Mode;

  T=ReadTimer();
  Cb=CountSoundBase;
  Tb=TimerSoundBase;

  B=Cb+((T-Tb)/10L)*(LONG)RefFrequence/100L;
  return(B);
#endif
}

#ifdef WINDOWS
void DisplayError (WORD wError, char *title)
{
char	szErrorText[150];

if (!waveOutGetErrorText(wError, (LPSTR)szErrorText, sizeof(szErrorText)))
	MessageBox (NULL, szErrorText, title, MB_OK);
}
#endif
