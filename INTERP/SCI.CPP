//	sci.cpp

#include "sol.hpp"

#include "config.hpp"
#include "criterr.hpp"
#include "debug.hpp"
#include "errormgr.hpp"
#include "except.hpp"
#include "event.hpp"
#include "font.hpp"
#include "graphmgr.hpp"
#include "heapleak.hpp"
#include "mouse.hpp"
#include "msg.hpp"
#include "plane.hpp"
#include "pmachine.hpp"
#include "resource.hpp"
#include "savegame.hpp"
#include "sci.hpp"
#include "stampver.hpp"
#include "time.hpp"
#include "test.hpp"

SCI* sci;

void
SCI::Init(int argc, char* argv[])
{
#if CHECKHEAPLEAKS
	heapLeakChecker= new HeapLeakChecker;
#endif

	critErrHandler	= MakeCritErrHandler();
	msgMgr 			= MakeMsgMgr();
	msgMgr->Init();        
	errorMgr 		= MakeErrorMgr();
	memMgr 			= MakeMemoryMgr();
	configMgr		= MakeConfigMgr(argc, argv);
	timeMgr			= MakeTimeMgr();
	InitInterrupts();

#ifdef DEBUG
	memMgr->Init(configMgr->Val(configMgr->MemAllocate));
#else
	memMgr->Init();
#endif

	resMgr			= MakeResourceMgr();
	eventMgr			= MakeEventMgr();
	graphMgr			= MakeGraphicsMgr();
	fontMgr			= MakeFontMgr();
	InitPalette();

	mouse				= MakeMouse();
	
	InitPlane();

	InitAudio();
	
	InitSound();
	
	InitExceptions();
	
	saveGame			= New SaveGame;
#ifdef DEBUG
	debugger			= New Debugger;
#endif

	VerifyVerStamp();
}

void
SCI::InitPlane()
{
	Plane* defaultPlane = New Plane(SOL_Rect(0,0,319,199));
	defaultPlane->SetPriority(0);
	assert(graphMgr);
	graphMgr->AddPlane(defaultPlane);
}

FontMgr*
SCI::MakeFontMgr()
{
	return New FontMgr();
}

ResourceMgr*
SCI::MakeResourceMgr()
{
	return New ResourceMgr;
}

void
SCI::Run(int argc, char* argv[])
{
	Init(argc, argv);
#ifdef DEBUG
	if (configMgr->Get(configMgr->ExtractResource))
		resMgr->ExtractResources();
	else
#endif
	Test();
}

SCI::~SCI()
{
	pm.End();  
	if (graphMgr)
		graphMgr->KillRobot();
#ifdef DEBUG
	delete debugger;
#endif

	delete saveGame;
	delete timeMgr;
	delete mouse;
	delete errorMgr;
	delete fontMgr;
	delete graphMgr;
	delete eventMgr;
	delete resMgr;
	delete configMgr;
	delete memMgr;
	delete msgMgr;
	delete critErrHandler;
	
#if CHECKHEAPLEAKS
	delete heapLeakChecker;
#endif
}
