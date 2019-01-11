//	breakpt.cpp
#ifdef DEBUG
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "sol.hpp"

#include "breakpt.hpp"
#include "debug.hpp"
#include "debugdsp.hpp"
#include "debugmnu.hpp"
#include "dialog.hpp"
#include "kernel.hpp"
#include "msg.hpp"
#include "object.hpp"
#include "opcodes.hpp"
#include "pmachine.hpp"
#include "sendstac.hpp"

Breakpoints	breakpoints;

static DebugMenuItem	opcodeItems[] = {
	DebugMenuItem("c)all",	'c', cmBreakpointCall, -1),
	DebugMenuItem("p)roc",	'p', cmBreakpointProc, -1),
	DebugMenuItem("k)ernel",'k', cmBreakpointKernelAny, -1),
	DebugMenuItem("s)end",	's', cmBreakpointSend, -1),
	0
};
static DebugMenu	opcodeMenu("OPCODE BP", opcodeItems);

static DebugMenuCmd		kernelCmds[] = {
	DebugMenuCmd(cmBreakpointKernel, 		'k'),
	DebugMenuCmd(cmBreakpointKernelSticky, 'K'),
	0
};
static DebugMenuCmd		memoryCmds[] = {
	DebugMenuCmd(cmBreakpointMemory, 		'm', 't'),
	DebugMenuCmd(cmBreakpointMemorySticky, 'M', 'T'),
	0
};
static DebugMenuCmd		methodCmds[] = {
	DebugMenuCmd(cmBreakpointMethod, 		'e', 'b'),
	DebugMenuCmd(cmBreakpointMethodSticky, 'E', 'B'),
	0
};
static DebugMenuCmd		scriptCmds[] = {
	DebugMenuCmd(cmBreakpointScript, 		's', 'n'),
	DebugMenuCmd(cmBreakpointScriptSticky, 'S', 'N'),
	0
};
static DebugMenuItem	items[] = {
	DebugMenuItem("C)lear",		'C', cmBreakpointClear),
	DebugMenuItem("K/k)ernel",	kernelCmds),
	DebugMenuItem("M/m)emory",	memoryCmds),
	DebugMenuItem("mE/e)thod",	methodCmds),
	DebugMenuItem("S/s)cript",	scriptCmds),
	DebugMenuItem("op)code",	'p', &opcodeMenu),
	DebugMenuItem("ou)t",		'u', cmBreakpointStepOut, 'R'),
	0
};
DebugMenu	breakpointMenu("BP", items);

///////////////////////////////////////////////////////////////////////////////

class Breakpoint {
public:
	virtual Bool	Set(Bool /* sticky */ = False) { return False; }
	virtual void	Clear();
   virtual void	Test();
   virtual Bool	Triggered() { return False; }
   
protected:
   Bool	sticky;
};

void
Breakpoint::Clear()
{
	breakpoints.Remove(*this);
}

void
Breakpoint::Test()
{
	if (Triggered()) {
		DebugOn();
		if (!sticky)
			Clear();
	}
}

//////////////////////////////////////////////////////////////////////////////

class OpcodeBreakpoint: public Breakpoint {
public:
	void	Set(uchar opcode, ...);
	Bool	Triggered();
	void	Clear();

protected:
	void	AddOpcode(uchar opcode);

	enum { Size = 10 };
	char	entries[Size];
	int	nEntries;

};
static OpcodeBreakpoint opcodeBreakpoint;

void
OpcodeBreakpoint::AddOpcode(uchar opcode)
{
	assert(nEntries < Size);
	entries[nEntries++] = opcode;
}

void
OpcodeBreakpoint::Clear()
{
	Breakpoint::Clear();
	nEntries = 0;
}

void
OpcodeBreakpoint::Set(uchar opcode, ...)
{
	nEntries = 0;

	va_list	args;
	va_start(args, opcode);
	uchar op = opcode;
	while (op) {
		AddOpcode(op);
		op = va_arg(args, uchar);
	}
	va_end(args);
}

Bool
OpcodeBreakpoint::Triggered()
{
	uchar opcode = pm.ip.PeekChar() & ~OP_BYTE;
	for (int i = 0; i < nEntries; i++)
		if (entries[i] == opcode)
			return True;
	return False;
}

//////////////////////////////////////////////////////////////////////////////

class StepOverSourceBreakpoint: public OpcodeBreakpoint {
public:
	void	Set();
	Bool	Triggered();

protected:
	PMIP*	pmipStackPtr;

};
static StepOverSourceBreakpoint stepOverSourceBreakpoint;

void
StepOverSourceBreakpoint::Set()
{
	AddOpcode(op_lineNum);
	pmipStackPtr = pm.ip.StackPtr();
}

Bool
StepOverSourceBreakpoint::Triggered()
{
	return pm.ip.StackPtr() <= pmipStackPtr && OpcodeBreakpoint::Triggered();
}

//////////////////////////////////////////////////////////////////////////////

class PMIPStackBreakpoint: public Breakpoint {
public:
	void	Set(PMIP*);
	Bool	Triggered();

protected:
	PMIP*		stackPtr;

};
 static PMIPStackBreakpoint pmipStackBreakpoint;

void
PMIPStackBreakpoint::Set(PMIP* stackPtr)
{
	this->stackPtr = stackPtr;
}

Bool
PMIPStackBreakpoint::Triggered()
{
	return pm.ip.StackPtr() == stackPtr;
}

//////////////////////////////////////////////////////////////////////////////

class ScriptBreakpoint: public Breakpoint {
public:
	Bool	Set(Bool sticky = False);
	void	Set(uint scriptNum, size_t offset, Bool sticky = False);
	Bool	Triggered();

protected:
	uint		scriptNum;
	size_t	offset;

	static char	buf[];

};
static ScriptBreakpoint scriptBreakpoint;

char	ScriptBreakpoint::buf[20];

void
ScriptBreakpoint::Set(uint scriptNum, size_t offset, Bool sticky)
{
	this->scriptNum	= scriptNum;
	this->offset		= offset;
	this->sticky		= sticky;
}

Bool
ScriptBreakpoint::Set(Bool sticky)
{
	int		scriptNum;
	size_t	offset;
	if (!InputValue(IV_ScriptOffset, "Enter script/offset", buf, scriptNum,
		 &offset))
		return False;
	
	ScriptID script = ScriptPtr(scriptNum);
	Set(scriptNum, offset - script->codeOffset, sticky);

	return True;
}

Bool
ScriptBreakpoint::Triggered()
{
	return pm.curScriptNum == scriptNum && pm.ip.Offset() == offset;
}

///////////////////////////////////////////////////////////////////////////////

class MethodBreakpoint: public Breakpoint {
public:
	Bool	IsInMethod();
	Bool	Set(Bool sticky);
	void	Test();

protected:

	ObjectID	object;
	Selector	selector;
	Bool		skipCurrent;

	static char	buf[];

};
 static MethodBreakpoint methodBreakpoint;

char	MethodBreakpoint::buf[50];
	
Bool
MethodBreakpoint::IsInMethod()
{
	SendStack::Entry entry = *pm.sendStack->Top();
	return entry.object == object &&
		   (entry.selector == selector || selector == (Selector) -1);
}

Bool
MethodBreakpoint::Set(Bool sticky)
{
	int 		val;
	size_t	offset;
	if (!InputValue(IV_ObjectMethod, "Object method:", buf, val, &offset))
		return False;
		
	object = val;
	selector = offset;

	this->sticky = sticky;
	
	//	if we're starting out in the desired method, wait until we leave it
	//	and come back
	skipCurrent = IsInMethod();
	
	return True;
}

void
MethodBreakpoint::Test()
{
	//	are we at the desired object/method?

	Bool isInMethod = IsInMethod();
	if (isInMethod && !skipCurrent) {
		DebugOn();
		if (!sticky)
			Clear();
		else
			skipCurrent = True;

	} else if (skipCurrent && !isInMethod)
		skipCurrent = False;
}

//////////////////////////////////////////////////////////////////////////////

class MemoryBreakpoint: public Breakpoint {
public:
	Bool	Set(Bool sticky);
	void	Test();

protected:
	MemID		id;
	size_t	offset;
	SCIWord	val;
	
	static char buf[];

} ;
static MemoryBreakpoint memoryBreakpoint;

char	MemoryBreakpoint::buf[20];

Bool
MemoryBreakpoint::Set(Bool sticky)
{
	char*	prompt = "Memory id.offset:";

	while (1) {
		int i;
		if (!InputValue(IV_Address, prompt, buf, i, &offset))
			return False;
		id = i;
		val = (SCIWord&) id[offset];
		msgMgr->Alert("Memory breakpoint set at %d.%u: currently %s",
			(int) (short) id, offset, FormatNumber(val));

		break;
	}
	
	this->sticky = sticky;
	
	return True;
}

void
MemoryBreakpoint::Test()
{
	//	did the value change?
	
	SCIWord curVal = (SCIWord&) id[offset];
	if (curVal != val) {
		char curValStr[20];
		strcpy(curValStr, FormatNumber(curVal));
		msgMgr->Alert("Memory breakpoint triggered at %d.%u: was %s now %s",
			(int) (short) id, offset, FormatNumber(val), curValStr);
		
		DebugOn();
		if (!sticky)
			Clear();
		else
			val = curVal;
	}
}

//////////////////////////////////////////////////////////////////////////////

class PropertyBreakpoint: public MemoryBreakpoint {
public:
	Bool	Set(ObjectID obj, Bool sticky);
	void	Test();
	
protected:
	Selector	selector;

	static char buf[];

} ;
static PropertyBreakpoint propertyBreakpoint;

char	PropertyBreakpoint::buf[40];

Bool
PropertyBreakpoint::Set(ObjectID obj, Bool sticky)
{
	if (!InputPropertyName(obj, selector, buf))
		return False;
		
	id			= obj;
	offset	= obj.GetPropOffset(selector);
	val		= obj.GetPropAtOffset(offset);
	
	this->sticky = sticky;
	
	return True;
}

void
PropertyBreakpoint::Test()
{
	//	did the property change?
	
	SCIWord curVal = ((ObjectID) id).GetPropAtOffset(offset);
	if (curVal != val) {
		char curValStr[40];
		char selectorName[40];
		strcpy(curValStr, FormatNumber(curVal));
		msgMgr->Alert("Property breakpoint triggered: (%s %s?) was %s now %s",
			((ObjectID) id).Name(), GetSelectorName(selector, selectorName),
			FormatNumber(val), curValStr);
		
		DebugOn();
		if (!sticky)
			Clear();
		else
			val = curVal;
	}
}

////////////////////////////////////////////////////////////////////////////////

class KernelBreakpoint : public Breakpoint {
public:
	Bool	Set(Bool sticky);
   Bool	Triggered();

protected:
	int	num;
	
	static char buf[];

} ;
static KernelBreakpoint kernelBreakpoint;

char	KernelBreakpoint::buf[30];

Bool
KernelBreakpoint::Set(Bool sticky)
{
	char*	prompt = "Kernel call name or number:";

	while (1) {
		IV_Type iv;
		if (!(iv = InputValue(IV_Number | IV_String, prompt, buf, num)))
			return False;

		if (iv == IV_Number) {
			if (num > kernelMax)
				prompt = "Invalid kernel call number:";
			else
				break;

		} else {
			for (int i = 0; i < kernelMax; i++)
				if (!strcmp(buf, kernelCalls[i].name)) {
					num = i;
					break;
				}
			if (i >= kernelMax)
				prompt = "Invalid kernel call name:";
			else
				break;
		}
	}
	
	this->sticky = sticky;
	
	return True;
}

Bool
KernelBreakpoint::Triggered()
{
	uchar opcode = pm.ip.PeekChar();
	return (opcode == op_callk_Char && pm.ip.PeekNextChar() == num ||
		     opcode == op_callk_Word && pm.ip.PeekNextUWord() == num);
}

////////////////////////////////////////////////////////////////////////////////

Breakpoints::Breakpoints() : nEntries(0)
{
}

void
Breakpoints::Add(Breakpoint& bp)
{
	assert(nEntries < Size);
	//	only one of a kind, for now
	Remove(bp);
	entries[nEntries++] = &bp;
	debuggerIsOn = True;
}

void
Breakpoints::Clear()
{
	//	go in reverse order, so that removing entries won't affect iteration
	for (int i = nEntries - 1; i >= 0; i--)
		entries[i]->Clear();
}

Bool
Breakpoints::HandleEvent(int cmd)
{
	switch (cmd) {
		case cmBreakpointClear:
			Clear();
			msgMgr->Alert("Breakpoints cleared");
			break;
		
		case cmBreakpointKernel:
			Set(kernelBreakpoint);
			break;

		case cmBreakpointKernelSticky:
			Set(kernelBreakpoint, True);
			break;

		case cmBreakpointMemory:
			Set(memoryBreakpoint);
			break;

		case cmBreakpointMemorySticky:
			Set(memoryBreakpoint, True);
			break;

		case cmBreakpointScript:
			Set(scriptBreakpoint);
			break;

		case cmBreakpointScriptSticky:
			Set(scriptBreakpoint, True);
			break;

		case cmBreakpointMethod:
			Set(methodBreakpoint);
			break;

		case cmBreakpointMethodSticky:
			Set(methodBreakpoint, True);
			break;

		case cmBreakpointStepOut:
			StepOut();
			break;

		case cmBreakpointCall:
			opcodeBreakpoint.Set(
				op_call, op_callb, op_calle,
				op_send, op_self, op_super,
				op_callk,
				0);
			Add(opcodeBreakpoint);
			singleStep = False;
			break;

		case cmBreakpointProc:
			opcodeBreakpoint.Set(op_call, op_callb, op_calle, 0);
			Add(opcodeBreakpoint);
			singleStep = False;
			break;

		case cmBreakpointKernelAny:
			opcodeBreakpoint.Set(op_callk, 0);
			Add(opcodeBreakpoint);
			singleStep = False;
			break;

		case cmBreakpointSend:
			opcodeBreakpoint.Set(op_send, op_self, op_super, 0);
			Add(opcodeBreakpoint);
			singleStep = False;
			break;

		default:
			return False;
	}
	
	return True;
}

void
Breakpoints::Remove(Breakpoint& bp)
{
	for (int i = 0; i < nEntries; i++)
		if (entries[i] == &bp) {
			memcpy(entries + i, entries + i + 1, nEntries - i - 1);
			debuggerIsOn = --nEntries > 0 || singleStep;
			return;
		}
}

void
Breakpoints::Set(Breakpoint& bp, Bool sticky)
{
	if (bp.Set(sticky))
		Add(bp);
}

void
Breakpoints::SetPropertyBreakpoint(ObjectID obj, Bool sticky)
{
	if (propertyBreakpoint.Set(obj, sticky))
		Add(propertyBreakpoint);
}

void
Breakpoints::StepOut()
{
	//	step out of a function or method call
	
	pmipStackBreakpoint.Set(pm.ip.StackPtr() - 1);
	Add(pmipStackBreakpoint);
	singleStep = False;
}

void
Breakpoints::StepSource()
{
	//	step into a source line by breaking on next line number opcode

	opcodeBreakpoint.Set(op_lineNum, 0);
	Add(opcodeBreakpoint);
	singleStep = False;
}

void
Breakpoints::StepOverSource()
{
	//	step over a source line
	
	stepOverSourceBreakpoint.Set();
	Add(stepOverSourceBreakpoint);
	singleStep = False;
}

void
Breakpoints::StepOver()
{
	//	step over a function or method call, if any

	uchar opcode = pm.ip.PeekChar();
	
	size_t offset;
	
	//	determine the offset of the next instruction following the call
	switch (opcode & ~OP_BYTE) {
		case op_self:
		case op_send:	
			offset = pm.ip.Offset() + 3;
			break;

		case op_call:
		case op_callb:
		case op_super:
			offset = pm.ip.Offset() + ((opcode & OP_BYTE) ? 4 : 5);
			break;

		case op_calle:
			offset = pm.ip.Offset() + ((opcode & OP_BYTE) ? 5 : 7);
			break;
		
		default:
			//	not a call--outa here
			return;
	}

	scriptBreakpoint.Set(pm.curScriptNum, offset);
	Add(scriptBreakpoint);
	
	singleStep = False;
}

void
Breakpoints::Test()
{
	//	go in reverse order, so that removing entries won't affect iteration
	for (int i = nEntries - 1; i >= 0; i--)
		entries[i]->Test();
}
#endif

