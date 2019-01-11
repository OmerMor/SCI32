//	sciw.cpp
// 	Windows startup routine for the pseudo machine.

#include "sciwin.hpp"
#include "sol.hpp"

#include "audio.hpp"
#include "config.hpp"
#include "criterrw.hpp"
#include "errormgr.hpp"
#include "eventw.hpp"
#include "graphmw.hpp"
#include "memmgrw.hpp"
#include "mousew.hpp"
#include "msgw.hpp"
#include "sci.hpp"
#include "sound.hpp"
#include "timew.hpp"

struct SCIWin : SCI {
	void					InitAudio();
	void					InitInterrupts();
	void					InitPalette();
	void					InitSound();

	ConfigMgr* 	  	   MakeConfigMgr(int argc, char* argv[]);
	CritErrHandler* 	MakeCritErrHandler();
	EventMgr*			MakeEventMgr();
	GraphicsMgr*		MakeGraphicsMgr();
	MemoryMgr*			MakeMemoryMgr();
	SOL_Mouse*			MakeMouse();
	MsgMgr*			 	MakeMsgMgr();
	TimeMgr*			 	MakeTimeMgr();
	ErrorMgr*       	MakeErrorMgr();
} static sciWin;

void
SCIWin::InitAudio()
{
	InitAudioDriver();
	atexit(EndAudio);
}

void
SCIWin::InitInterrupts()
{
}

void
SCIWin::InitPalette()
{
	// default palette
	SOLPalette* palette = New SOLPalette;
	(*palette)[255] = Rgb24F(255, 255, 255, 1);
	assert(graphMgr);
	graphMgr->GPalette().Submit(*palette);
	graphMgr->GPalette().UpdateForFrame();
	graphMgr->GPalette().UpdateHardware();
	delete palette;
}

void
SCIWin::InitSound()
{
	InitSoundDriver();
	atexit(TermSndDrv);
}

ConfigMgr*
SCIWin::MakeConfigMgr(int argc, char* argv[])
{
	ConfigMgr* configM = New ConfigMgr(argc, argv);

	// add the Windows patch directory so it will be searched first
	configM->AddTokenToFront("patchDir",szWinPatchDir);
	return configM;
}  

CritErrHandler*
SCIWin::MakeCritErrHandler()
{
	return New CritErrHandlerWin;
}

EventMgr*
SCIWin::MakeEventMgr()
{
	return New EventMgrWin;
}

GraphicsMgr*
SCIWin::MakeGraphicsMgr()
{
	GraphicsMgr* gMgr = New GraphicsMgrWin(gameWidth, gameHeight);
	if (DOPPCTEST)
		((GraphicsMgrWin*)gMgr)->TimingTest();
	return gMgr;
}

MemoryMgr*
SCIWin::MakeMemoryMgr()
{
	return New MemoryMgrWin;
}

SOL_Mouse*
SCIWin::MakeMouse()
{
	SOL_Mouse* mouse = New MouseWin();
#ifdef COLORCUR
	assert(graphMgr);
	graphMgr->GCursor().SetDevice(*mouse);
#endif
	return mouse;
}

MsgMgr*
SCIWin::MakeMsgMgr()
{
	return New MsgMgrWin;
}

TimeMgr*
SCIWin::MakeTimeMgr()
{
	return New TimeMgrWin;
}


// The font manager
ErrorMgr*   errorMgr;

ErrorMgr*
SCIWin::MakeErrorMgr()
{
//	return New ErrorMgr();
	return (ErrorMgr*)0;
}

//////////////////////////////////////////////////////////////////////////////

int
scimain(int argc, char* argv[])
{
	sci = &sciWin;

	sci->Run(argc, argv);

	return 0;
}
