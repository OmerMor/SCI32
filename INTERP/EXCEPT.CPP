//	except.cpp
//		set up signal and other exception handlers for IBM interpreter

#include <dos.h>
#include <signal.h>
#include <stdlib.h>

#include "sol.hpp"

#include "debug.hpp"
#include "scii.hpp"
#include "msg.hpp"

//	called from fault.asm
extern "C" {
	void AsmPanicReturn(int);
};
#pragma aux (PushArgs) AsmPanicReturn;

static void PanicIllegalIns(int signal);
static void PanicMemoryViolation(int signal);

void
SCIIBM::InitExceptions()
{
	// Setup misc error handlers
	signal(SIGILL, PanicIllegalIns);
	signal(SIGSEGV, PanicMemoryViolation);
}

void
AsmPanicReturn(int faultNo)
{
	switch(faultNo) {
		case 1:
			msgMgr->Alert(SrcLoc,Msg_DivideByZero,"Divide By Zero!");
			break;

		case 2:
			msgMgr->Fatal("General Protection Fault!");
			break;
	}
	_exit(1);
}


static void
PanicIllegalIns(int)
{
	msgMgr->Fatal("CPU Exception:  Illegal instruction");

#ifdef DEBUG
	EnterDebug();
#else
	exit(1);
#endif
}

static void
PanicMemoryViolation(int)
{
	msgMgr->Fatal("CPU Exception:  Illegal memory access");
#ifdef DEBUG
	EnterDebug();
#else
	exit(1);
#endif
}
