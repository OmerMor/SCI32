// windac.cpp

#define INCLUDE_MMSYSTEM_H
#include "sciwin.hpp"
UINT __pascal _16waveOutPrepareHeader(HWAVEOUT hWaveOut, DWORD, UINT uSize);
UINT __pascal _16waveOutUnprepareHeader(HWAVEOUT hWaveOut, DWORD, UINT uSize);
UINT __pascal _16waveOutWrite(HWAVEOUT hWaveOut, DWORD, UINT uSize);

#include "sol.hpp"

#include "audio.hpp"
//#include "memid.hpp"
#include "mbox.hpp"

extern void AsyncEventCheck(void);

int DacCheck(void);
int DacInit(void);
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
void	writeWave (DWORD lpHdr);

uchar	*wAudioBuffAddr;
int	wAudioBuffSize;
int	writeCount = 0;
int	buffFilledCount;
int	buffPlayingCount;
Bool	closeFlag = False;
Bool	stopFlag = False;

static	WAVEOUTCAPS		woc;
static	PCMWAVEFORMAT	pwf;
static	HWAVEOUT			hWave;
static	WAVEHDR			wHdr1, wHdr2;
static	DWORD				wHdr1_alias, wHdr2_alias;
static	WORD				wError;
static	int				buffNum;
static	LPSTR				tBuf1;
static	LPSTR				tBuf2;
static	int				tBufSize;
static	Bool				playingFlag = False;
static	Bool				waveDevOpen = False;
static	int				maxBits, maxRate, maxChannels;

static int	*DACArgs;


int WinAudDrv (int func, int *DACArgsIN)
{

	DACArgs = DACArgsIN;

	// NOTE: The following was formerly a switch statement, but the Watcom
	// 9.5a compiler was generating bad code which caused the switch statement
	// to crash. TM - 10/31/93

	if (func == DAC_CHECK)
		return DacCheck();
	else if (func == DAC_INIT)
		return DacInit();
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
	else if (func == DAC_LOC) {
//		sprintf(mbuff,"loc:%d",buffPlayingCount);
//		SciDisplay(mbuff);
		return (buffPlayingCount);
		}
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
	else
		return 0;
}

int DacCheck()
{
	// toggle buffer number 0 or 1
	// arg 0
	// return number of buffer just filled

	// If arg count is zero, called from server waiting for outstanding
	// buffers to clear.
//	if (DACArgs[0] == 0) {
//		if (stopFlag)
//			if (writeCount == 0)
//				closeWaveDev();
//		return 0;
//	}

	// Called from audio.cpp while in preparation for a Play call
	// Fill the first buffer but don't start it.
	if (!playingFlag || stopFlag) {
		if (buffNum) {
			memcpy(tBuf1, (void *)DACArgs[1], tBufSize);
		} else {
			memcpy(tBuf2, (void *)DACArgs[1], tBufSize);
		}
		buffFilledCount++;
//	sprintf(mbuff,"init fill:%d",buffFilledCount);
//	SciDisplay(mbuff);
		buffNum = 1 - buffNum;
		return buffFilledCount;
	}
		
	// Normal call from server while play is in progress
	if (buffNum) {
		memcpy(tBuf1, (void *)DACArgs[1], tBufSize);
		writeWave(wHdr1_alias);
	} else {
		memcpy(tBuf2, (void *)DACArgs[1], tBufSize);
		writeWave(wHdr2_alias);
	}
	buffFilledCount++;

//	sprintf(mbuff,"play fill:%d",buffFilledCount);
//	SciDisplay(mbuff);

	buffNum = 1 - buffNum;
	return buffFilledCount;
}

int DacInit(void)
{
	// arg 0
	// return 0 - if OK
	// return -1 if no DAC

	if (!waveOutGetNumDevs())		/* any devices? */
		return -1;

#ifdef JUNK
	wError = waveOutGetDevCaps(0,
										(LPWAVEOUTCAPS)&woc,
										sizeof(WAVEOUTCAPS));
	if (wError) {
		DisplayError(wError, "DevCaps");
		return 1;
	}
	

	if (woc.dwFormats & (WAVE_FORMAT_1M16 |
								WAVE_FORMAT_1S16 |
								WAVE_FORMAT_2M16 |
								WAVE_FORMAT_2S16 |
								WAVE_FORMAT_4M16 |
								WAVE_FORMAT_4S16))
		maxBits = 16;
	else
		maxBits = 8;

	if (woc.dwFormats & (WAVE_FORMAT_4M08 |
								WAVE_FORMAT_4S08 |
								WAVE_FORMAT_4M16 |
								WAVE_FORMAT_4S16))
		maxRate = 44100;
	else if (woc.dwFormats & (WAVE_FORMAT_2M08 |
									  WAVE_FORMAT_2S08 |
								     WAVE_FORMAT_2M16 |
									  WAVE_FORMAT_2S16))
		maxRate = 22050;
	else
		maxRate = 11025;

	if (woc.dwFormats & (WAVE_FORMAT_1S16 |
								WAVE_FORMAT_2S16 |
								WAVE_FORMAT_4S16 |
								WAVE_FORMAT_1S08 |
								WAVE_FORMAT_2S08 |
								WAVE_FORMAT_4S08))
		maxChannels = 2;
	else
		maxChannels = 1;
	sprintf(mbuff,"bits:%d rate:%d chan:%d",maxBits,maxRate,maxChannels);
	MBox(mbuff,"");
#endif

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

//	sprintf(mbuff,"bits:%d rate:%d chan:%d",maxBits,maxRate,maxChannels);
//	MBox(mbuff,"");

	// set default values for the format
	pwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pwf.wf.nChannels  = 1;
	pwf.wf.nSamplesPerSec = 11025;
	pwf.wf.nAvgBytesPerSec = 11025;
	pwf.wf.nBlockAlign = 1;
	pwf.wBitsPerSample = (WORD)8;

	// set up buffer headers
	tBufSize = wAudioBuffSize / 2;
	tBuf1 = (LPSTR)wAudioBuffAddr;
	wHdr1.lpData = (LPSTR)AllocAlias16(tBuf1);
	wHdr1.dwBufferLength = (DWORD)tBufSize;
	tBuf2 = (LPSTR)(tBuf1 + tBufSize);
	wHdr2.lpData = (LPSTR)AllocAlias16(tBuf2);
	wHdr2.dwBufferLength = (DWORD)tBufSize;

	wHdr1.dwBytesRecorded = (DWORD)0;
	wHdr1.dwUser = (DWORD)0;
	wHdr1.dwFlags = (DWORD)0;
	wHdr1.dwLoops = (DWORD)0;

	wHdr2.dwBytesRecorded = (DWORD)0;
	wHdr2.dwUser = (DWORD)0;
	wHdr2.dwFlags = (DWORD)0;
	wHdr2.dwLoops = (DWORD)0;

	wHdr1_alias = AllocAlias16(&wHdr1);
	wHdr2_alias = AllocAlias16(&wHdr2);
	return 0;
}

int DacTerminate(void)
{
	// arg 0
	// no return
	closeWaveDev();
	FreeAlias16((DWORD)wHdr1.lpData);
	FreeAlias16((DWORD)wHdr2.lpData);
	FreeAlias16((DWORD)wHdr1_alias);
	FreeAlias16((DWORD)wHdr2_alias);
	return 0;
}

int DacPlay(void)
{
	// no return

//	sprintf(mbuff,"play bits:%d rate:%d",pwf.wBitsPerSample,pwf.wf.nSamplesPerSec);
//	SciDisplay(mbuff);
	openWaveDev();
	writeWave(wHdr1_alias);
	if (buffFilledCount > 1)
		writeWave(wHdr2_alias);
	playingFlag = True;
	stopFlag = False;
	return 0;
}

int DacStop(void)
{
	// arg 1 - 0 (Stop DAC immediately)
	//       - 1 (Stop DAC when current buffer completes)
	// no return
	if ((!DACArgs[0]) || (!DACArgs[1])) {
		if (waveDevOpen && writeCount) {
			wError = waveOutReset (hWave);
			if (wError)
				DisplayError(wError, "waveOutReset");
		}
		stopFlag = True;
		buffNum = 1;
		buffFilledCount = 0;
//		SciDisplay("stop immediate");
	}
	if ((DACArgs[0]) && (DACArgs[1] == 1)) {
		stopFlag = True;
//		SciDisplay("stop pending");
	}
	if ((DACArgs[0]) && (DACArgs[1] == -1)) {
		stopFlag = False;
//		SciDisplay("stop cancel");
	}
	return 0;
}

int DacPause(void)
{
	// arg 0
	// no return
	waveOutPause(hWave);
	return 0;
}

int DacResume(void)
{
	// arg 0
	// no return
	waveOutRestart(hWave);
	return 0;
}

int DacSetRate(void)
{
	// arg 1 - rate
	// no return
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
	// 8 or 16
	// arg 1 - bits
	// no return
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
	// arg 1 - channels
	// no return
	pwf.wf.nChannels = Max(1,Min((WORD)DACArgs[1],maxChannels));
	return 0;
}

int DacSetBuff(void)
{
	// arg 1 - audBuffKSize
	// return 0 if OK
	// allocate buffer of arg 1 K

	// NOTE: For reasons I cannot comprehend, allocating the audio buffers at
	// this point causes savegame to write a zero-length file.  This is the
	// case whether it is done via "New" or "_nmalloc".  Sometimes it's OK
	// if a small buffer is allocated (1K), but never with a big buffer (63K).
	// For this reason, the actual allocation is now done in memmgrw.cpp.
	// TM - 11/5/93


//	sprintf(mbuff,"aud buff:%d", DACArgs[1]);
//	SciDisplay(mbuff);
//	MBox(mbuff,"");

	tBufSize = (DACArgs[1] * 1024) / 2;
	wHdr1.dwBufferLength = (DWORD)tBufSize;
	wHdr2.dwBufferLength = (DWORD)tBufSize;

//	tBufSize = wAudioBuffSize / 2;
//	tBuf1 = (LPSTR)wAudioBuffAddr;
//	wHdr1.lpData = (LPSTR)AllocAlias16(tBuf1);
//	wHdr1.dwBufferLength = (DWORD)tBufSize;
//	tBuf2 = (LPSTR)(tBuf1 + tBufSize);
//	wHdr2.lpData = (LPSTR)AllocAlias16(tBuf2);
//	wHdr2.dwBufferLength = (DWORD)tBufSize;
//	audioBuffer.ptr = (char far *)tBuf1;
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
//	SciDisplay("open");
	wError = waveOutOpen ((LPHWAVEOUT)&hWave,
									WAVE_MAPPER,
									(LPWAVEFORMAT)&pwf.wf,
									(DWORD)hMyWnd,
									(DWORD)0,
									CALLBACK_WINDOW);
	if (wError)
//		{
//		sprintf(mbuff,"bits:%d rate:%d chan:%d",pwf.wBitsPerSample,
//									pwf.wf.nSamplesPerSec, pwf.wf.nChannels);
//		MBox(mbuff,"open");
		DisplayError(wError, "waveOutOpen");
//		}
	wError = _16waveOutPrepareHeader (hWave, wHdr1_alias, sizeof(WAVEHDR));
	if (wError)
		DisplayError(wError, "waveOutPrepareHeader1");
	wError = _16waveOutPrepareHeader (hWave, wHdr2_alias, sizeof(WAVEHDR));
	if (wError)
		DisplayError(wError, "waveOutPrepareHeader2");
	waveDevOpen = True;
	buffPlayingCount = 0;
}


void closeWaveDev (void)
{
	if (!waveDevOpen)
		return;

	// If close is ever called with buffers outstanding, then do a Reset
	// and wait for buffers to clear.
	if (writeCount) {
		wError = waveOutReset (hWave);
		if (wError)
			DisplayError(wError, "waveOutReset");

		closeFlag = True;
		while (writeCount > 0)
			AsyncEventCheck();
		closeFlag = False;
	}

	stopFlag = False;
	playingFlag = False;
	buffPlayingCount = 0;

	if (wHdr1.dwFlags & WHDR_PREPARED)
		{
		wError = _16waveOutUnprepareHeader (hWave, wHdr1_alias, sizeof(WAVEHDR));
		if (wError)
			DisplayError(wError, "waveOutUnprepareHeader1");	
		}
	if (wHdr2.dwFlags & WHDR_PREPARED)
		{
		wError = _16waveOutUnprepareHeader (hWave, wHdr2_alias, sizeof(WAVEHDR));
		if (wError)
			DisplayError(wError, "waveOutUnprepareHeader2");	
		}
	wError = waveOutClose(hWave);
	if (wError)
		DisplayError(wError, "waveOutClose");
	waveDevOpen = FALSE;
	return;
}


void writeWave (DWORD lpHdr)
{
	wError = _16waveOutWrite (hWave, lpHdr, sizeof(WAVEHDR));
	if (wError)
		DisplayError(wError, "waveOutWrite");
	writeCount++;
	if (buffPlayingCount == 0)
		buffPlayingCount = 1;
}


void DisplayError (WORD wError, char *title)
{
char	szErrorText[150];

if (!waveOutGetErrorText(wError, (LPSTR)szErrorText, sizeof(szErrorText)))
	MessageBox (NULL, szErrorText, title, MB_OK);
}
