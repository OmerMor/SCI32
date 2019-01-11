//	integ.cpp

#include "sol.hpp"

#ifdef DEBUG

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debug.hpp"
#include "integ.hpp"
#include "kernel.hpp"
#include "pmachine.hpp"
#include "string.hpp"
#include "textid.hpp"

struct IntegrityMask {

	IntegrityMask(int which, char* what = 0)
		{mask = which; description = what;}
	int	mask;
	char*	description;

};
static IntegrityMask integrityMasks[] = {

	IntegrityMask(CHECK_SIGNATURES,	"Signatures"),
	IntegrityMask(CHECK_CHECKSUMS,	"Checksums"),
	IntegrityMask(CHECK_FREE,			"Free space"),
	IntegrityMask(CHECK_LISTS,			"Descriptor lists"),
	IntegrityMask(CHECK_HEAP,			"System heap"),
	IntegrityMask(CHECK_GLOBALS,		"Global pointers"),
	IntegrityMask(CHECK_MONO_OFF,		"(no Mono echo)"),
	IntegrityMask(CHECK_ALL,			"Everything ")
};

int	nIntegrityMasks = sizeof(integrityMasks) / sizeof(*integrityMasks) - 1;
int	checkIntegrity = 0;
Bool	checkIntegrityRequest = False;


void
KIntegrityChecking(argList)
{
	if (argCount >= 1) {
		char buf[10];
		char* cp = buf;
		if (argCount >= 2) {
			TextID id = (TextID)arg(2);
			if (id.IsValid() && IsPrintable(*id))
				cp = *id;
			else
				sprintf(cp,"%d",arg(2));
		}
		IntegrityChecking((int)arg(1) | (checkIntegrity & CHECK_MONO_OFF), cp);
	} else
		IntegrityChecking(CHECK_ALL | (checkIntegrity & CHECK_MONO_OFF));
}


void
IntegrityChecking(int mask, char* fmt, ...)
{
	char	buf[MsgMgr::MaxStr];
	
	if (fmt) {
		va_list args;
		va_start(args, fmt);
		vsprintf(&buf[1], fmt, args);
		va_end(args);
		buf[0] = '(';
		strcat(buf,")");
	} else
		buf[0] = '\0';

	Bool mono = !(mask & CHECK_MONO_OFF); 
	mask &= CHECK_ALL;

	if (!mask) {
		if (checkIntegrity) {
			checkIntegrity = 0;
			if (mono)
				msgMgr->Mono("%sIntegrity checking DISABLED\n",buf);
			if (strlen(buf))
				msgMgr->Alert("%sIntegrity checking DISABLED",buf);
#ifndef MACINTOSH
			memMgr->IntegrityChecking();
#endif
		}
		return;
	}

	if (mask == checkIntegrity)
		return;

	int len = strlen(buf);
	strcat(buf,"Integrity checking ENABLED: ");
	if (mask == CHECK_ALL)
		strcat(buf,integrityMasks[nIntegrityMasks].description);
	else {
		for (int i = 0; i < nIntegrityMasks; i++) {
			if (mask & integrityMasks[i].mask) {
				strcat(buf,integrityMasks[i].description);
				strcat(buf,",");
			}
		}
		buf[strlen(buf)-1] = '\0';
	}
	if (!mono) {
		strcat(buf," ");
		strcat(buf,integrityMasks[nIntegrityMasks - 1].description);
#ifndef WINDOWS
	} else {
		msgMgr->Mono("%s\n",buf);
#endif
	}
	if (len)
		msgMgr->Alert(buf);
	if (!mono)
		mask |= CHECK_MONO_OFF;
	checkIntegrity = mask;
#ifndef MACINTOSH
	memMgr->IntegrityChecking();
#endif
	return;
}

void
CheckIntegrityRequest()
{
	debuggerWasOn = debuggerIsOn;
	debuggerIsOn = True;
	checkIntegrityRequest = True;
}

void
KCheckIntegrity(argList)
{
	char	buf[15];
	char *cp = "";

	if (argCount) {
		TextID id = (TextID)arg(1);
		if (id.IsValid() && IsPrintable(*id))
			cp = *id;
		else {
			sprintf(buf,"%d",arg(1));
			cp = buf;
		}
	}
	CheckIntegrity(cp);
	if (argCount)
		pm.acc = arg(1);
}

Bool
CheckIntegrity(char* fmt, ...)
{
	if (!checkIntegrity)
		return True;

	char	buf[MsgMgr::MaxStr];
	if (fmt) {
		va_list args;
		va_start(args, fmt);
		vsprintf(&buf[1], fmt, args);
		va_end(args);
		buf[0] = '(';
		strcat(buf,")");
	} else
		buf[0] = '\0';

	Bool mono = !(checkIntegrity & CHECK_MONO_OFF); 
	Bool ok = True;

#ifndef MACINTOSH
	#ifndef WINDOWS
		if (mono)
			msgMgr->Mono("\n%sCheckIntegrity...", buf);
	#endif
		if ((checkIntegrity & CHECK_HEAP) && _heapchk() > _HEAPEMPTY) {
			if (!msgMgr->Alert("%s Heap Check Error", buf))
				EnterDebug();
			ok = 0;
		}
		if (checkIntegrity & CHECK_GLOBALS)
//BEW CHANGED			ok &= pm.CheckIntegrity(buf);
			ok &= pmCheckIntegrity(buf);
		ok &= memMgr->CheckIntegrity(buf);
	#ifndef WINDOWS
		if (mono)
			msgMgr->Mono("%s\n", ok? "OK" : "ERROR");
	#endif
#endif //MACINTOSH
	return ok;
}

#endif
