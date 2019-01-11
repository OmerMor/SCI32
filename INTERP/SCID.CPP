// scid.cpp

#include <dos.h>
#include <string.h>

#include "sol.hpp"

#include "audio.hpp"
#include "config.hpp"
#include "criterrd.hpp"
#include "errormgr.hpp"
#include "eventd.hpp"
#include "fault.hpp"
#include "graphmd.hpp"
#include "intrpt.hpp"
#include "memmgrd.hpp"
#include "moused.hpp"
#include "msgd.hpp"
#include "scid.hpp"
#include "sound.hpp"
#include "timed.hpp"
#include "video.hpp"

Bool	WING = False;

static SCIDOS sciDOS;

void
AsyncEventCheck()
{
	AudioServer();
}

SCIDOS::~SCIDOS()
{
}

void
SCIDOS::InitAudio()
{
	InitAudioDriver();
	atexit(EndAudio);
}

void
SCIDOS::InitExceptions()
{
	SCIIBM::InitExceptions();
	_dos_setvect(0, PanicDiv0);
}

void
SCIDOS::InitInterrupts()
{
	SetInterrupts();
	atexit(ResetInterrupts);
}

void
SCIDOS::InitPalette()
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
SCIDOS::InitSound()
{
	InitSoundDriver();
	atexit(TermSndDrv);
}

ConfigMgr*
SCIDOS::MakeConfigMgr(int argc, char* argv[])
{
	return New ConfigMgr(argc, argv);
}  

CritErrHandler*
SCIDOS::MakeCritErrHandler()
{
	return New CritErrHandlerDOS;
}

EventMgr*
SCIDOS::MakeEventMgr()
{
	return New EventMgrDOS;
}

GraphicsMgr*
SCIDOS::MakeGraphicsMgr()
{
	Bool usePri = False;

#ifdef DEBUG
	usePri = configMgr->Get(configMgr->PriScreen);
#endif
	if (!VesaSupport())
		msgMgr->Alert("No VESA support detected!");
	if (*configMgr->Get("hires", 0))  {
		int xres = atoi(configMgr->Get("hires", 0));
		int yres = atoi(configMgr->Get("hires", 1));

		return New GraphicsMgrDOS(xres, yres, True, usePri);
	}
	return New GraphicsMgrDOS(640, 480, True, usePri);
}

MemoryMgr*
SCIDOS::MakeMemoryMgr()
{
	return New MemoryMgrDOS;
}

SOL_Mouse*
SCIDOS::MakeMouse()
{
	SOL_Mouse* mouse = New MouseDOS(True);
	assert(graphMgr);
	graphMgr->GCursor().SetDevice(*mouse);
	return mouse;
}

MsgMgr*
SCIDOS::MakeMsgMgr()
{
	return New MsgMgrDOS;
}

TimeMgr*
SCIDOS::MakeTimeMgr()
{
	return New TimeMgrDOS;
}

ErrorMgr*
SCIDOS::MakeErrorMgr()
{
	return New ErrorMgr();
}

/////////////////////////////////////////////////////////////////////////////

int
main(int argc, char* argv[])
{
	sci = &sciDOS;

	sci->Run(argc, argv);

	return 0;
}


