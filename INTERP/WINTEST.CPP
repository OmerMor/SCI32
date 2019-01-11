#include "sciwin.hpp"
#include "sol.hpp"

#include "audio.hpp"
#include "kernel.hpp"
#include "mouse.hpp"
#include "object.hpp"
#include "textid.hpp"
#include "timew.hpp"
#include "sound.hpp"
#include "soundwin.hpp"
#include "mbox.hpp"

#ifdef JUNK
struct {
	ushort	count;
	ushort	func;
	ushort	arg2;
	ushort	arg3;
	ushort	arg4;
} static audArgs;
#endif

#ifdef JUNK
struct {
	ushort	count;
	ushort	arg1;
	ushort	arg2;
	ushort	arg3;
	ushort	arg4;
	ushort	arg5;
} static krnArgs;

//static Bool	caged = False;
TextID buff;
#endif

OFSTRUCT	ofs;
HFILE		hFile;

void WinTest ()
{
	if (DOFASTGRAPH) {
		DOFASTGRAPH = FALSE;
		MBox("go slow","");
	} else {
		DOFASTGRAPH = TRUE;
		MBox("go fast","");
	}
#ifdef JUNK
	buff = "g:\\games\\fred-32\\prog\\martin\\wndsurf1.avi";
	krnArgs.count = 2;
	krnArgs.arg1 = 0;
	krnArgs.arg2 = (ushort)buff;
	KShowMovie((kArgs) &krnArgs);

	krnArgs.count = 3;
	krnArgs.arg1 = 1;
	krnArgs.arg2 = 20;
	krnArgs.arg3 = 20;
	krnArgs.arg4 = 0;
	krnArgs.arg5 = 0;
	KShowMovie((kArgs) &krnArgs);

	krnArgs.count = 2;
	krnArgs.arg1 = 2;
	krnArgs.arg2 = 2;
	KShowMovie((kArgs) &krnArgs);

	krnArgs.count = 1;
	krnArgs.arg1 = 6;
	KShowMovie((kArgs) &krnArgs);
#endif	
#ifdef JUNK
//	MBox("delete time","");
//	delete timeMgr;
//	MBox("make time","");
//	timeMgr = New TimeMgrWin;
//	MBox("done time","");
//	MBox("pause","");
				PauseSnd ((ObjectID)0, TRUE);
				SuspendSounds(TRUE);
				CloseSnd ();
//	MBox("unpause","");
				OpenSnd ();
				SuspendSounds(FALSE);
				PauseSnd ((ObjectID)0, FALSE);
//	MBox("done","");
//	sprintf(mbuff,"time:%d",timeMgr->GetTickCount());
//	MBox(mbuff,"");
#endif
#ifdef JUNK
	SetErrorMode(SEM_FAILCRITICALERRORS);
	hFile = OpenFile("a:myfile.txt",&ofs,OF_PARSE);
	sprintf(mbuff,"err:%d",ofs.nErrCode);
	MBox(mbuff,"");
#endif
#ifdef JUNK
	buff = "b:myfile.txt";
	krnArgs.count = 3;
	krnArgs.arg1 = 0;
	krnArgs.arg2 = (ushort)buff;
	krnArgs.arg3 = 1;
	KFileIO((kArgs) &krnArgs);
#endif
#ifdef JUNK
	if (!caged) {
		caged = True;
		krnArgs.count = 4;
		krnArgs.arg1 = 100;
		krnArgs.arg2 = 100;
		krnArgs.arg3 = 125;
		krnArgs.arg4 = 200;
		KSetCursor((kArgs) &krnArgs);
	} else {
		caged = False;
		krnArgs.count = 1;
		krnArgs.arg1 = -2;
		KSetCursor((kArgs) &krnArgs);
	}
#endif
#ifdef JUNK
			audArgs.count = 2;
			audArgs.func = PLAY;
			audArgs.arg2 = 100;
			KDoAudio((kArgs) &audArgs);
#endif
}

void StartTimingTest ()
{
	if (MessageBox(NULL,"Show graphics?","Timing test",MB_YESNO | MB_ICONQUESTION) == IDYES)
		TESTGRAPHICSON = TRUE;
	else
		TESTGRAPHICSON = FALSE;
	TIMINGTESTFLAG = TRUE;
	currReps = testReps;
	testStart = timeMgr->GetTickCount();
}
