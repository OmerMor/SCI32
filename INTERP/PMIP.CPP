//	pmip.cpp

#include <stdio.h>

#include "sol.hpp"

#include "msg.hpp"
#include "opcodes.hpp"
#include "pmachine.hpp"
#include "pmip.hpp"

PMIP	PMIP::retPMIP;
PMIP	PMIP::tossRetPMIP;
PMIP  PMIP::stack[PMIP::MaxStack];
PMIP*	PMIP::stackPtr;

void
PMIP::Init()
{
	//	create some special pieces of code that can be substituted for
	//	code whose script has been disposed

	//	create a piece of code with just a return in it
	MemID retID;
	retID.Get(MemSpecialCode, 1, RetIPHandle);
	retID[0] = op_ret;
	retPMIP = PMIP(retID, (size_t) 0);

	//	create another with just 'toss, ret' in it
	MemID tossRetID;
	tossRetID.Get(MemSpecialCode, 2, TossRetIPHandle);
	tossRetID[0] = op_toss;
	tossRetID[1] = op_ret;
	tossRetPMIP = PMIP(tossRetID, (size_t) 0);
}

char*
PMIP::MakeStackDspStr(char* dest)
{
	*dest = 0;
	for (PMIP* entry = stack + 1; entry < stackPtr; entry++)
		sprintf(dest + strlen(dest), "%d: %u %p %u ", entry - stack, entry->id,
			entry->ptr, entry->ptr - (char*) *entry->id);
	return dest;
}

size_t
PMIP::ScriptOffset() const
{
	//	return the current offset as represented in the .SL listing file

	return Offset() + ScriptPtr(pm.curScriptNum)->codeOffset;
}

void
PMIP::TossMemID(MemID id, unsigned scriptNum)
{
	//	a code handle has just become invalid.  if it's on the call stack
	//	make sure its next instruction is a op_ret or an op_toss, op_ret
	//	sequence and then replace it with the appropriate special code handle
	
	//	a toss, ret sequence is permitted in disposed code because it is
	//	generated at the end of a switch statement and it's safe to allow
	//	the programmer to call DisposeScript in a switch clause
	
	//	if the next instruction is op_lineNum, check that the next instruction
	//	after that is a return (or a toss/ret), and replace it with just a return
	// (or a toss/ret)

	//	need to include current ip
	pm.ip.Push();

	//	first entry is 0
	for (PMIP* p = stack + 1; p < stackPtr; p++)
		if (p->id == id)
			if (**p == op_ret)
				*p = retPMIP;
			else if (**p == op_lineNum && (*p)[3] == op_ret)
				*p = retPMIP;
			else if (**p == op_toss && (*p)[1] == op_ret)
				*p = tossRetPMIP;
			else if (**p == op_toss && (*p)[1] == op_lineNum && (*p)[4] == op_ret)
				*p = tossRetPMIP;
			else
				msgMgr->Fatal(SrcLoc, Msg_DisposedScript, scriptNum, id);

	pm.ip.Pop();
}

void
PMIP::SetOffsetFromPtr()
{
	//	save current offset from beginning of id

	offset = Offset();
}

void
PMIP::SetPtrFromOffset()
{
	//	restore ptr from saved offset

	ptr = &id[offset];
}

void
PMIP::Unlock()
{
	//	unlock current ip if its locked and not persistent
	if (!(id.Attrs() & (MOVEABLE | PERSIST)))
		id.Unlock();
}

void
PMIP::Lock()
{
	//	lock current ip, if it's unlocked and not persistent
	if ((id.Attrs() & (MOVEABLE | PERSIST)) == MOVEABLE)
		id.Lock();
}

void
PMIP::SaveRestoreBefore(SaveGame*, Bool)
{
}

void	
PMIP::SaveRestoreAfter(SaveGame*, Bool)
{
}
