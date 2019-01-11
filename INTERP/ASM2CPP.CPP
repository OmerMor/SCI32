#include "pmachine.hpp"

#include "asm2cpp.hpp"

void*
pmLock0( void ) {
	return( pm.Globals.Lock() );
}

void
setupTossRet( void ) {

	//	create some special pieces of code that can be substituted for
	//	code whose script has been disposed

	//	create another with just 'toss, ret' in it
	MemID tossRetID;
	tossRetID.Get(MemSpecialCode, 2, TossRetIPHandle);
	tossRetID[0] = 0x3a;  // op_toss;
	tossRetID[1] = 0x48;  // op_ret;
	pm.disposeTossRet = tossRetID;
}

#pragma off (unreferenced);

void
asm_fatal(int which, int parm1, int parm2, int parm3) {
	switch ( which ) {
		case 1:
			char selectorName[MaxSelectorName + 1];
			GetSelectorName(parm1, selectorName);
			msgMgr->Fatal(Msg_BadSelector, selectorName, "object");
			break;

		case 2:
			msgMgr->Fatal(Msg_InvalidProperty, parm1);
			break;

		case 3:
			msgMgr->Fatal(SrcLoc, Msg_DisposedScript, parm1, parm2);
			break;

		case 4:
			msgMgr->Fatal(SrcLoc, Msg_BadOpcode, parm1);
			break;
	}
}

void
pmSaveRestoreBefore(SaveGame* saveGame, Bool save)
{
	pm.GlobalPtr = (Property*) pm.Globals.Unlock();
}

void
pmSaveRestoreAfter(SaveGame* saveGame, Bool save)
{
	if (save)
		pm.GlobalPtr = (Property*) pm.Globals.Lock();
}

#pragma on (unreferenced);
