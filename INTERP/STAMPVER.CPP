//	stampver.cpp

#include "sol.hpp"

#include "msg.hpp"
#include "stampver.hpp"

#define VERSTAMPBIAS 0x1234

int mapStamp =	VERSTAMPBIAS - 1;

// The following data is in a structure in order to maintain order-dependency
struct {
	int value;
	short stampFlag1;
	short stampFlag2;
	short stampFlag3;
} interpStamp = {
	-VERSTAMPBIAS,
	0x5241,
	0x4552,
	0x5349
};


void
VerifyVerStamp()
{

// NB: This code must be compiled only for out-of-house interpreters.

//#if !defined(DEBUG) && !defined(WINDOWS)
#if !defined(DEBUG)
	interpStamp.value += VERSTAMPBIAS;
	mapStamp -= VERSTAMPBIAS;
	if (mapStamp != -1 && interpStamp.value != -1 &&
			(mapStamp == 0 || mapStamp != interpStamp.value))
	   if (!msgMgr->Alert(Msg_VerStampMismatch) || mapStamp != -1)
			exit(255);
#endif
}

int
GetVerStamp()
{
	return interpStamp.value + VERSTAMPBIAS;
}


