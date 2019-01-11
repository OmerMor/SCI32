//	pmachine.cpp

#include <stdarg.h>

#include "sol.hpp"

#include "config.hpp"
#include "debug.hpp"
#include "dos.hpp"
#include "integ.hpp"
#include "language.hpp"
#include "msg.hpp"
#include "memmgr.hpp"
#include "newroom.hpp"
#include "object.hpp"
#include "opcodes.hpp"
#include "pmachine.hpp"
#include "savegame.hpp"
#include "script.hpp"
#include "selector.hpp"
#include "sendstac.hpp"

PMachine	pm;

static PMIP	GetDispatchCodeAddr(unsigned scriptNum, unsigned entryNum,
					ScriptID* script);

void
PMachine::Run()
{
	//	Load the class table, allocate the p-machine stack

	Selector startMethod;
	
	setjmp(restartBuf);

	rmResList->NewRoom(0);
	if (!game) {
		//	this code can't be executed twice unless we implement restart
		LoadPropOffsets();
		LoadClassTbl();
		ip.Init();
		stack.Init();
		restArgs = 0;
#ifdef DEBUG
		sendStack = New SendStackDebug;
#endif
		startMethod = s_play;
	} else
		startMethod = s_replay;

	stack.Clear();
#ifdef DEBUG
	sendStack->Clear();
	ResetDebugInfo();
#endif
	ip.Clear();

	ScriptID script;
	game = object = GetDispatchEntry(0, 0, &script).objectID;
	globals = script->vars;
	globalPtr = (Property*) globals.Lock();

	InitLanguage();

#ifdef DEBUG
	if (configMgr->Get(configMgr->IntegrityChecking)) {
		int mask = CHECK_ALL;
		if (configMgr->Arg(configMgr->IntegrityChecking))
			mask = configMgr->Val(configMgr->IntegrityChecking);
		IntegrityChecking(mask,"-i");
	}
//  REMOVE THESE LINES when interpreter is 'solid'...
	// Enable integrity checking by default
	else
		IntegrityChecking(CHECK_ALL);
//  ...end of 'REMOVE THESE LINES'
	CheckIntegrity("pm start");
#endif

	InvokeMethod(game, startMethod, 0);

#ifdef DEBUG
	CheckIntegrity("pm end");
#endif
}

void
PMachine::End()
{
	game = 0;
	
	delete sendStack;

	stack.End();
}

#pragma off(unreferenced)

Bool
PMachine::CheckIntegrity(char* note)
{
	//	if globals are unlocked (during save/restore), don't worry about the
	//	pointer
	if (globals.Attrs() & MOVEABLE)
		return True;

	if ((Property*) *globals == globalPtr)
		return True;
	
#ifdef DEBUG
	if (!msgMgr->Alert("%sSomething has stepped on globals pointer",note))
		EnterDebug();
#else
	msgMgr->Alert(SrcLoc,Msg_GlobalPointer);
#endif

	return False;
}
#pragma on(unreferenced)

Property&
PMachine::Global(GlobalNum num)
{
	//	access SCI global variables

	return globalPtr[num];
}

void
PMachine::Dispatch(unsigned scriptNum, unsigned entryNum, unsigned nArgChars)
{
	//	execute the procedure at this script/entry

	//	save the current script's variables
	VariableID	saveLocals			= pm.locals;
	unsigned		saveCurScriptNum	= pm.curScriptNum;

	pm.ip.Push();

	//	get the new ip for this script/entry
	ScriptID	script;
	PMIP		ip = GetDispatchCodeAddr(scriptNum, entryNum, &script);

	//	set the new script's variables
	pm.locals			= script->vars;
	pm.curScriptNum	= scriptNum;

	//	execute the procedure
	pm.Call(ip, nArgChars);

	//	restore script variables
	pm.locals			= saveLocals;
	pm.curScriptNum	= saveCurScriptNum;

	pm.ip.Pop();
}

void
PMachine::Call(PMIP ip, unsigned nArgChars)
{
	//	execute the subroutine at ip

	//	save current subroutine's variables
	PMStack::Ptr	saveParmVar	= pm.stack.parms;
	PMStack::Ptr	saveTempVar	= pm.stack.temps;

	//	set up new parameter base:  current stack pointer, minus known
	// arguments, minus restArgs
	pm.stack.parms	= pm.stack.ptr - (nArgChars + pm.restArgs) / sizeof(Property);

	//	adjust # args on stack by pm.restArgs
	*pm.stack.parms += pm.restArgs / sizeof(Property);
	pm.restArgs = 0;

	//	let 'er rip
	pm.ip = ip;
#ifdef DEBUG
	PushDebugInfo();
#endif
	ExecuteCode();
#ifdef DEBUG
	PopDebugInfo();
#endif

	//	pop the arguments off the pm.stack
	pm.stack.ptr = pm.stack.parms - 1;

	//	restore subroutine variables
	pm.stack.parms = saveParmVar;
	pm.stack.temps = saveTempVar;
}

Acc
PMachine::InvokeMethod(const ObjectID& obj, Selector selector, unsigned nArgs,
								...)
{
	//	invoke a method or access a property from the kernel

	ObjectID saveObject = pm.object;

	// add 1 for selector and 1 for # args
	int	nArgChars = (nArgs + 2) * sizeof(Property);

	//	set new current object
	pm.object = obj;

	//	push the selector
	pm.stack.Push(selector);

	//	push the number of chars of arguments
	pm.stack.Push(nArgs);

	//	push the arguments
	va_list	args;
	va_start(args, nArgs);
	while (nArgs--)
		pm.stack.Push(va_arg(args, Property));
	va_end(args);

	//	call the method
	QuickMessage(obj, nArgChars);

	//	restore current object
	pm.object = saveObject;

	return pm.acc;
}

void
PMachine::Messager(const ObjectID& obj)
{
	//	send messages to an object

	QuickMessage(obj, pm.ip.GetUWord() + pm.restArgs);
}


void
PMachine::QuickMessage(const ObjectID& obj, unsigned nArgChars)
{
	//	send a series of messages (method invocations and property accesses)
	//	to 'obj'

	Object *theObj = *obj;  // dereferenced object

	//	eventually this will be passed directly
	unsigned nArgs = nArgChars / sizeof(Property);

	//	save important variables
	VariableID		saveLocals			= pm.locals;
	PMStack::Ptr	saveParmVar			= pm.stack.parms;
	PMStack::Ptr	saveTempVar			= pm.stack.temps;
	unsigned			saveCurScriptNum	= pm.curScriptNum;

	pm.ip.Push();

	//	where stack ptr will point after all this object's messages have been
	//	popped
	PMStack::Ptr	prevPMSP		= pm.stack.ptr - nArgs;

	//	where the parameters of the next message for this object start
	PMStack::Ptr	nextMsgParms = prevPMSP + 1;

	//	make sure this is an object
	obj.AssureValid();

#ifdef DEBUG
	pm.sendStack->PushObject(obj);
#endif

	//	process all the messages for this object
	while (nArgs) {
		PMStack::Entry numArgs;

		PMStack::Ptr localParms = nextMsgParms;
		pm.stack.parms = nextMsgParms + 1;

		//	get selector
		Selector selector = *localParms++;
		numArgs = *localParms;

		//	adjust # known parameters by those added by &rest
		numArgs += pm.restArgs / sizeof(Property);
		pm.restArgs = 0;

		//	decrement # args to process by this msg's parms +
		//	1 for selector + 1 for # args
		nArgs -= numArgs + 2;

		//	save location of next msg's parms
		nextMsgParms = localParms + numArgs + 1;

#ifdef DEBUG
		pm.sendStack->PushSelector(selector);
#endif
		size_t offset = theObj->GetPropOffset(selector);

		if (offset != Object::BadPropOffset) {
			if (!numArgs)	//	no arguments -> property query
				pm.acc = theObj->GetPropAtOffset(offset);
			else					//	else set property to parm
				theObj->SetPropAtOffset(offset) = localParms[1];
		} else {
			//	search methods for this selector
			ObjectID curClass = obj;

			while (1) {
				Object* o = *curClass;
				Script* script = *(ScriptID) o->script;
				pm.curScriptNum = (SCIUWord) script->Key();
				int nEntries = ((MethDictID) o->methDict)->nEntries;
				MethDictEntry* methDictEntry = &((MethDictID) o->methDict)[0];

				for (int i = 0; i < nEntries; i++) {
					if (methDictEntry[i].selector == selector) {
						pm.ip = PMIP(script->code, methDictEntry[i].offset);
						pm.locals = script->vars;

						*localParms = numArgs;
#ifdef DEBUG
						PushDebugInfo();
#endif
						ExecuteCode();
#ifdef DEBUG
						PopDebugInfo();
#endif
						//	re-deref, in case memory moved
						theObj = *obj;
						break;
					}
				}

				if ( i != nEntries )
					break;

				//	look in superclass
				curClass = curClass->super;

				if (!curClass) {
					char selectorName[MaxSelectorName + 1];
					GetSelectorName(selector, selectorName);
					msgMgr->Fatal(Msg_BadSelector, selectorName,
						obj->Name() ? obj->Name() : "object");
					break;
				}
				curClass.AssureValid();
			}
		}
	}

	//	restore variables
	pm.curScriptNum	= saveCurScriptNum;
	pm.locals			= saveLocals;
	pm.stack.parms		= saveParmVar;
	pm.stack.temps		= saveTempVar;

	pm.ip.Pop();

	//	pop parameters off stack
	pm.stack.ptr		= prevPMSP;

#ifdef DEBUG
	pm.sendStack->Pop();
#endif
}

static PMIP
GetDispatchCodeAddr(unsigned scriptNum, unsigned entryNum, ScriptID* script)
{
	//	return the address of the code for this script/entry; also return
	//	pointer to handle to script node

	unsigned offset = GetDispatchEntry(scriptNum, entryNum, script).codeOffset;
	return PMIP((*script)->code, offset);
}

void
PMachine::SaveRestoreBefore(SaveGame* saveGame, Bool save)
{
	ip.SaveRestoreBefore(saveGame, save);
	globalPtr = (Property*) globals.Unlock();
}

void
PMachine::SaveRestoreDo(Saveable::Func func, SaveGame* saveGame, Bool save)
{
#ifdef DEBUG
	sendStack->SaveRestoreDo(func, saveGame, save);
#endif

	Saveable::SaveRestoreDo(func, saveGame, save);
}

void
PMachine::SaveRestoreAfter(SaveGame* saveGame, Bool save)
{
	ip.SaveRestoreAfter(saveGame, save);
	if (save)
		globalPtr = (Property*) globals.Lock();
}
