// windac.cpp

#define INCLUDE_MMSYSTEM_H
#include "sciwin.hpp"
#ifndef WIN32S
UINT __pascal _16waveOutPrepareHeader(HWAVEOUT hWaveOut, DWORD, UINT uSize);
UINT __pascal _16waveOutUnprepareHeader(HWAVEOUT hWaveOut, DWORD, UINT uSize);
UINT __pascal _16waveOutWrite(HWAVEOUT hWaveOut, DWORD, UINT uSize);
#endif

#include "sol.hpp"
#include "audio.hpp"
#include "mbox.hpp"

typedef	Bool	(*boolproc)();

extern void AsyncEventCheck(void);

int WinAudDrv(int func, int *DACArgsIn);
int DacInit(void);
int DacContinue(void);
int DacTerminate(void);
int DacPlay(void);
int DacStop(void);
int DacPause(void);
int DacResume(void);
int DacSetRate(void);
int DacSetBits(void);
int DacSetChannels(void);
int DacSetBuff(void);

void	DisplayError (WORD, char *);
void	openWaveDev (void);
int	openQuery (int,int,int);
void	closeWaveDev (void);
#ifndef WIN32S
void	writeWave (DWORD lpHdr);
#else
void	writeWave (LPWAVEHDR lpHdr);
#endif
void	AudioUpdate(void);

uchar	*wAudioBuffAddr;
int	writeCount = 0;

static	WAVEOUTCAPS		woc;
static	PCMWAVEFORMAT	pwf;
static	HWAVEOUT			hWave;
static	WAVEHDR			wHdr1, wHdr2;
#ifndef WIN32S
static	DWORD				wHdr1_alias, wHdr2_alias;
#endif
static	UINT				wError;
static	int				buffNum;
static	int				nextIn;
static	LPSTR				tBuf1;
static	LPSTR				tBuf2;
static	int				tBufSize;
static	Bool				waveDevOpen = False;
static	int				maxBits, maxRate, maxChannels;
static	Bool				(*AudCallBack)(void);
static	Bool				moreToPlay = False;
static	Bool				nowPlaying = False;
static	BOOL				audBuff1Out = FALSE;
static	BOOL				audBuff2Out = FALSE;

static int	*DACArgs;

unsigned
GetDACDefaultSize(void)
{
	return 32*1024;
}

void *
LoadAudioDrv(void)
{
//	if (USEAUDIO)
		return (&WinAudDrv);
//	else
//		return 0;
}

void
FreeAudioDrv(void)
{
}

int WinAudDrv (int func, int *DACArgsIN)
{

	DACArgs = DACArgsIN;

	// NOTE: The following was formerly a switch statement, but the Watcom
	// 9.5a compiler was generating bad code which caused the switch statement
	// to crash. TM - 10/31/93

	if (func == DAC_INIT)
		return DacInit();
	else if (func == DAC_CONTINUE)
		return DacContinue();
	else if (func == DAC_TERMINATE)
		return DacTerminate();
	else if (func == DAC_PLAY)
		return DacPlay();
	else if (func == DAC_STOP)
		return DacStop();
	else if (func == DAC_PAUSE)
		return DacPause();
	else if (func == DAC_RESUME)
		return DacResume();
	else if (func == DAC_LOC)
		return (writeCount);
	else if (func == DAC_SET_RATE)
		return DacSetRate();
	else if (func == DAC_SET_BITS)
		return DacSetBits();
	else if (func == DAC_SET_CHANNELS)
		return DacSetChannels();
	else if (func == DAC_SET_VOLUME)
		return 0;
	else if (func == DAC_SET_IRQ)
		return 0;
	else if (func == DAC_SET_DMA)
		return 0;
	else if (func == DAC_SET_BUFF)
		return DacSetBuff();
	else if (func == DAC_GET_ID)
		return 0;
	else if (func == DAC_GET_VER)
		return woc.vDriverVersion;
	else if (func == DAC_GET_REALOC)
		return 0;
	else if (func == DAC_GET_MAX_CHNLS)
		return woc.wChannels;
	else if (func == DAC_GET_MAX_BITS)
		return maxBits;
	else if (func == DAC_GET_MAX_RATE)
		return maxRate;
	else if (func == DAC_GET_VOL)
		return (woc.dwSupport & WAVECAPS_VOLUME);
	else if (func == DAC_GET_BUFF_SEL)
		return ((int)FP_SEG((int far *)&writeCount));
	else if (func == DAC_GET_BUFF_OFS)
		return ((int)wAudioBuffAddr);
	else
		return 0;
}

int DacInit(void)
{
	if (!waveOutGetNumDevs())		/* any devices? */
		return -1;

	// Because of problems encountered with a Medivision driver, I have 
	// learned not to trust the return from DevCaps.  Instead, I now do
	// an open QUERY for each parameter.

	if (openQuery(8,44100,1))
		maxRate = 44100;
	else if (openQuery(8,22050,1))
		maxRate = 22050;
	else
		maxRate = 11025;

	if (openQuery(16,maxRate,1))
		maxBits = 16;
	else
		maxBits = 8;

	if (openQuery(maxBits,maxRate,2))
		maxChannels = 2;
	else
		maxChannels = 1;

	// set default values for the format
	pwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pwf.wf.nChannels  = 1;
	pwf.wf.nSamplesPerSec = 11025;
	pwf.wf.nAvgBytesPerSec = 11025;
	pwf.wf.nBlockAlign = 1;
	pwf.wBitsPerSample = (WORD)8;

	// set up buffer headers
	wHdr1.dwBytesRecorded = (DWORD)0;
	wHdr1.dwUser = (DWORD)0;
	wHdr1.dwFlags = (DWORD)0;
	wHdr1.dwLoops = (DWORD)0;

	wHdr2.dwBytesRecorded = (DWORD)0;
	wHdr2.dwUser = (DWORD)0;
	wHdr2.dwFlags = (DWORD)0;
	wHdr2.dwLoops = (DWORD)0;

#ifndef WIN32S
	wHdr1_alias = AllocAlias16(&wHdr1);
	wHdr2_alias = AllocAlias16(&wHdr2);
#endif
	return 0;
}

int DacTerminate(void)
{
	closeWaveDev();
#ifndef WIN32S
	FreeAlias16((DWORD)wHdr1.lpData);
	FreeAlias16((DWORD)wHdr2.lpData);
	FreeAlias16((DWORD)wHdr1_alias);
	FreeAlias16((DWORD)wHdr2_alias);
#endif
	return 0;
}

int DacPlay(void)
{
	openWaveDev();
#ifndef WIN32S
	writeWave(wHdr1_alias);
#else
	writeWave((LPWAVEHDR)&wHdr1);
#endif
	if (DACArgs[1]) {
#ifndef WIN32S
		writeWave(wHdr2_alias);
#else
		writeWave((LPWAVEHDR)&wHdr2);
#endif
		moreToPlay = True;
	} else {
		moreToPlay = False;
	}
	buffNum = 1;
	nextIn = 1;
	nowPlaying = True;
	return 0;
}

int DacContinue(void)
{
	moreToPlay = True;
	return 0;
}

int DacStop(void)
{
	closeWaveDev();
	moreToPlay = False;
	nowPlaying = False;
	return 0;
}

int DacPause(void)
{
	waveOutPause(hWave);
	return 0;
}

int DacResume(void)
{
	waveOutRestart(hWave);
	return 0;
}

int DacSetRate(void)
{
	pwf.wf.nSamplesPerSec = Min(DACArgs[1],maxRate);
	if (pwf.wBitsPerSample == 16) {
		pwf.wf.nAvgBytesPerSec = pwf.wf.nSamplesPerSec << 1;
		pwf.wf.nBlockAlign		= 2;
	} else {
		pwf.wf.nAvgBytesPerSec = pwf.wf.nSamplesPerSec;
		pwf.wf.nBlockAlign		= 1;
	}
	return 0;
}

int DacSetBits(void)
{
	pwf.wBitsPerSample = Min((WORD)DACArgs[1],maxBits);
	if (pwf.wBitsPerSample == 16) {
		pwf.wf.nAvgBytesPerSec = pwf.wf.nSamplesPerSec << 1;
		pwf.wf.nBlockAlign		= 2;
	} else {
		pwf.wf.nAvgBytesPerSec = pwf.wf.nSamplesPerSec;
		pwf.wf.nBlockAlign		= 1;
	}
	return 0;
}

int DacSetChannels(void)
{
	pwf.wf.nChannels = Max(1,Min((WORD)DACArgs[1],maxChannels));
	return 0;
}

int DacSetBuff(void)
{
	// arg 1 - buffer size in bytes
	// arg 2 - call back address
	// return 0 if OK
	// allocate buffer of arg 1 K

	// NOTE: For reasons I cannot comprehend, allocating the audio buffers at
	// this point causes savegame to write a zero-length file.  This is the
	// case whether it is done via "New" or "_nmalloc".  Sometimes it's OK
	// if a small buffer is allocated (1K), but never with a big buffer (63K).
	// For this reason, the actual allocation is now done in memmgrw.cpp.
	// TM - 11/5/93


	tBufSize = DACArgs[1]/2;
	AudCallBack = (boolproc)DACArgs[2];
	wHdr1.dwBufferLength = (DWORD)tBufSize;
	wHdr2.dwBufferLength = (DWORD)tBufSize;

	tBuf1 = (LPSTR)wAudioBuffAddr;
	tBuf2 = (LPSTR)(tBuf1 + tBufSize);

	wHdr1.dwBufferLength = (DWORD)tBufSize;
	wHdr2.dwBufferLength = (DWORD)tBufSize;
#ifndef WIN32S
	wHdr1.lpData = (LPSTR)AllocAlias16(tBuf1);
	wHdr2.lpData = (LPSTR)AllocAlias16(tBuf2);
#else
	wHdr1.lpData = tBuf1;
	wHdr2.lpData = tBuf2;
#endif
	return 0;
}


int openQuery (int bits, int rate, int chan)
{
	pwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pwf.wf.nChannels  = chan;
	pwf.wf.nSamplesPerSec = rate;
	pwf.wBitsPerSample = (WORD)bits;

	if (pwf.wBitsPerSample == 16) {
		pwf.wf.nAvgBytesPerSec = pwf.wf.nSamplesPerSec << 1;
		pwf.wf.nBlockAlign		= 2;
	} else {
		pwf.wf.nAvgBytesPerSec = pwf.wf.nSamplesPerSec;
		pwf.wf.nBlockAlign		= 1;
	}

	wError = waveOutOpen ((LPHWAVEOUT)NULL,
									0,
									(LPWAVEFORMAT)&pwf.wf,
									(DWORD)NULL,
									(DWORD)0,
									WAVE_FORMAT_QUERY);
	return (!wError);
}


void openWaveDev (void)
{
	if (waveDevOpen)
		closeWaveDev();
	wError = waveOutOpen ((LPHWAVEOUT)&hWave,
									WAVE_MAPPER,
									(LPWAVEFORMAT)&pwf.wf,
									NULL,
									(DWORD)0,
									0);
	if (wError)
		DisplayError(wError, "waveOutOpen");
#ifndef WIN32S
	wError = _16waveOutPrepareHeader (hWave, wHdr1_alias, sizeof(WAVEHDR));
#else
	wError = waveOutPrepareHeader (hWave, (LPWAVEHDR)&wHdr1, sizeof(WAVEHDR));
#endif
	if (wError)
		DisplayError(wError, "waveOutPrepareHeader1");
#ifndef WIN32S
	wError = _16waveOutPrepareHeader (hWave, wHdr2_alias, sizeof(WAVEHDR));
#else
	wError = waveOutPrepareHeader (hWave, (LPWAVEHDR)&wHdr2, sizeof(WAVEHDR));
#endif
	if (wError)
		DisplayError(wError, "waveOutPrepareHeader2");
	waveDevOpen = True;
	writeCount = 0;
}


void closeWaveDev (void)
{

	if (!waveDevOpen)
		return;

	// If close is ever called with buffers outstanding, then do a Reset
	// and wait for buffers to clear.
	if (USEAUDIO) {
		if (writeCount) {
			wError = waveOutReset (hWave);
			if (wError)
				DisplayError(wError, "waveOutReset");
			while (writeCount > 0) {
				if (audBuff1Out && (wHdr1.dwFlags & WHDR_DONE)) {
					audBuff1Out = FALSE;
					writeCount--;
				}
				if (audBuff2Out && (wHdr2.dwFlags & WHDR_DONE)) {
					audBuff2Out = FALSE;
					writeCount--;
				}
			}
		}
	}

	if (wHdr1.dwFlags & WHDR_PREPARED)
		{
#ifndef WIN32S
		wError = _16waveOutUnprepareHeader (hWave, wHdr1_alias, sizeof(WAVEHDR));
#else
		wError = waveOutUnprepareHeader (hWave, (LPWAVEHDR)&wHdr1, sizeof(WAVEHDR));
#endif
		if (wError)
			DisplayError(wError, "waveOutUnprepareHeader1");	
		}
	if (wHdr2.dwFlags & WHDR_PREPARED)
		{
#ifndef WIN32S
		wError = _16waveOutUnprepareHeader (hWave, wHdr2_alias, sizeof(WAVEHDR));
#else
		wError = waveOutUnprepareHeader (hWave, (LPWAVEHDR)&wHdr2, sizeof(WAVEHDR));
#endif
		if (wError)
			DisplayError(wError, "waveOutUnprepareHeader2");	
		}
	wError = waveOutClose(hWave);
	if (wError)
		DisplayError(wError, "waveOutClose");
	waveDevOpen = FALSE;
	return;
}


#ifndef WIN32S
void writeWave (DWORD lpHdr)
{
	if (USEAUDIO) {
		wError = _16waveOutWrite (hWave, lpHdr, sizeof(WAVEHDR));
#else
void writeWave (LPWAVEHDR lpHdr)
{
	if (USEAUDIO) {
		wError = waveOutWrite (hWave, lpHdr, sizeof(WAVEHDR));
#endif
		if (wError)
			DisplayError(wError, "waveOutWrite");
		if (lpHdr == wHdr1_alias)
			audBuff1Out = TRUE;
		else
			audBuff2Out = TRUE;
	}
	writeCount++;
}


void DisplayError (WORD wError, char *title)
{
char	szErrorText[150];

if (!waveOutGetErrorText(wError, (LPSTR)szErrorText, sizeof(szErrorText)))
	MessageBox (NULL, szErrorText, title, MB_OK);
}

void
PollAudBuffs()
{
	if (!writeCount)
		return;

	if (nextIn) {
		if (audBuff1Out && (wHdr1.dwFlags & WHDR_DONE)) {
			audBuff1Out = FALSE;
			nextIn = 1 - nextIn;
			AudioUpdate();
		}
	} else {

		if (audBuff2Out && (wHdr2.dwFlags & WHDR_DONE)) {
			audBuff2Out = FALSE;
			nextIn = 1 - nextIn;
			AudioUpdate();
		}
	}
}

void
AudioUpdate()
{
	writeCount--;
	if ((writeCount == 0) && (!nowPlaying))
		closeWaveDev();
	if (moreToPlay) {
		moreToPlay = AudCallBack();
		if (moreToPlay) {
			if (buffNum)
#ifndef WIN32S
				writeWave(wHdr1_alias);
			else
				writeWave(wHdr2_alias);
#else
				writeWave((LPWAVEHDR)&wHdr1);
			else
				writeWave((LPWAVEHDR)&wHdr2);
#endif
			buffNum = 1 - buffNum;
		}
	}
}
