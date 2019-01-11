//	memory.cpp

#include "sol.hpp"

#include "kernel.hpp"
#include "msg.hpp"
#include "pmachine.hpp"

enum {
	MemLargest
};

void
KMemoryInfo(argList)
{
	switch (arg(1)) {
		case MemLargest:
			pm.acc = (Acc) memMgr->FreeMemory() / 1024;
			break;
		
		default:
			msgMgr->Fatal(SrcLoc, Msg_InvalidMemInfoCall, arg(1));
	}
}
