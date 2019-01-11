//	debug.cpp

#ifdef DEBUG
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sol.hpp"

#include "breakpt.hpp"
#include "config.hpp"
#include "debug.hpp"
#include "debugmnu.hpp"
#include "debugdsp.hpp"
#include "dialog.hpp"
#include "event.hpp"
#include "hotkey.hpp"
#include "integ.hpp"
#include "kernel.hpp"
#include "msg.hpp"
#include "object.hpp"
#include "opcodes.hpp"
#include "pmachine.hpp"
#include "savegame.hpp"
#include "selector.hpp"
#include "sendstac.hpp"
#include "sound.hpp"
#include "string.hpp"
#include "textid.hpp"
#include "vocab.hpp"
#include "window.hpp"

Bool	debuggerIsOn;
Bool	debuggerWasOn;
int	kernelCallHistory[MaxKernelCallHistory];
int	kernelCallOldest;
int	kernelCallNewest;
int	kernelCallIntegChecks;
Bool	singleStep;

static void		CallKernel();
static void		CheckSourceLevelOpcode();
static void		EditAcc();
static void		EditMemory();
static void		EditProperty(ObjectID);
static void		Inspect();
static void		Inspect(MemID id, size_t offset = 0);
static void		InspectList(ObjectID);
static void		InspectMemID(MemID id, size_t offset = 0);
static Bool		InspectObject(ObjectID);
static void		InspectProperty(ObjectID);
static void		ShowHelp();
static void		ShowInspectObjectHelp();

///////////////////////////////////////////////////////////////////////////////

static DebugMenuCmd		displayObjectCmds[] = {
	DebugMenuCmd(cmDisplayObjects, 'o'),
	DebugMenuCmd(cmDisplayObjectsWithAddrs, 'O'),
	0
};
static DebugMenuItem	displayItems[] = {
	DebugMenuItem("k)ernels",		'k',	cmDisplayKernelCalls),
	DebugMenuItem("m)ode",			'm',	cmDisplayMode),
	DebugMenuItem("p)al",			'p',	cmDisplayPalette),
	DebugMenuItem("O/objs",			displayObjectCmds),
	DebugMenuItem("r)es",			'r',	cmDisplayResources),
	DebugMenuItem("s)ends",			's',	cmDisplaySendStack),
	DebugMenuItem("b)itmaps",		'b',	cmDisplayBitmaps),
	DebugMenuItem("t)exts",			't',	cmDisplayTexts),
	0
};
static DebugMenu			displayMenu("DISPLAY", displayItems);

static DebugMenuItem	editItems[] = {
	DebugMenuItem("a)cc",		'a',	cmEditAcc, -1),
	DebugMenuItem("g)lobal",	'g',	cmEditGlobal),
	DebugMenuItem("l)ocal",		'l',	cmEditLocal),
	DebugMenuItem("t)emp",		't',	cmEditTemp, -1),
	DebugMenuItem("p)arameter",'p',	cmEditParameter, -1),
	DebugMenuItem("m)emory",	'm',	cmEditMemory, 'e'),
	0
};
static DebugMenu			editMenu("EDIT", editItems);

static DebugMenuItem	memoryItems[] = {
	DebugMenuItem("c)heck",				'c',	cmMemoryCheckIntegrity,		-1),
	DebugMenuItem("d)isable check",	'd',	cmMemoryCheckIntegrityOff,	-1),
	DebugMenuItem("e)nable check",	'e',	cmMemoryCheckIntegrityOn,	-1),
	DebugMenuItem("g)raph",				'g',	cmMemoryGraph,		-1),
	DebugMenuItem("l)ist",				'l',	cmMemoryList,		-1),
	DebugMenuItem("s)ummary",			's',	cmMemorySummary,	-1),
	0
};
static DebugMenu			memoryMenu("MEMORY", memoryItems);

static DebugMenuItem	debugItems[] = {
	DebugMenuItem(0,				Kbd::F7,	cmStepSource),
	DebugMenuItem(0,				Kbd::F8,	cmStepOverSource),
	DebugMenuItem(0,				'\t',	cmStepOver),
	DebugMenuItem(0,				'\r',	cmStep),
	DebugMenuItem("a)cc",		'a',	cmAcc),
	DebugMenuItem("b)reak",		'b',	&breakpointMenu),
	DebugMenuItem("c)ur",		'c',	cmCur),
	DebugMenuItem("d)sp",		'd',	&displayMenu),
	DebugMenuItem("e)dit",		'e',	&editMenu),
	DebugMenuItem("i)nspect",	'i',	cmInspect),
	DebugMenuItem("k)ernel",	'k',	cmCallKernel),
	DebugMenuItem("m)em",		'm',	&memoryMenu),
	DebugMenuItem("ex)it",		'x',	cmExit),
	DebugMenuItem("q)uit",		'q',	cmQuit),
	DebugMenuItem("?",			'?',	cmHelp),
	0
};

static DebugMenu			debugMenu("DEBUG", debugItems);

///////////////////////////////////////////////////////////////////////////////

#define MakeEditVariable(n, T)\
static void \
n##EditVariable(char* name, T vars, int nVars)\
{\
	static char buf[20];\
	\
	char prompt[20];\
	\
	if (!nVars) {\
		msgMgr->Alert("There are no %ss", name);\
		return;\
	}\
\
	sprintf(prompt, "Edit %s#", name);\
\
	while (1) {\
		int num;\
		if (!InputValue(IV_Number, prompt, buf, num))\
			return;\
\
		if (nVars != -1 && num > nVars) {\
			sprintf(prompt, "Max %s: %d",  name, nVars);\
			continue;\
		}\
\
		char valBuf[20];\
		strcpy(valBuf, FormatNumber(vars[num], False));\
		int val;\
		strcpy(prompt, "Value");\
		if (InputValue(IV_Value, prompt, valBuf, val))\
			vars[num] = val;\
		break;\
	}\
}

MakeEditVariable(Global, VariableID);
MakeEditVariable(Local, VariableID);
MakeEditVariable(Parm, PMStack::Ptr);
MakeEditVariable(Temp, PMStack::Ptr);

///////////////////////////////////////////////////////////////////////////////
	
void
Debug()
{
	//	monitor breakpoints and display debug information if single stepping
	
	if (checkIntegrityRequest) {
		checkIntegrityRequest = False;
		if (!checkIntegrity)
			IntegrityChecking(CHECK_ALL,"~");
		if (CheckIntegrity("~"))
			msgMgr->Alert("Integrity OK"); 
		if (!(debuggerIsOn = debuggerWasOn))
			return;
	}

	if (hotkeyRequest) {
		hotkeyRequest = False;
		Hotkey();
		if (!(debuggerIsOn = debuggerWasOn))
			return;
	}

	breakpoints.Test();
	
	CheckSourceLevelOpcode();

	if (!singleStep)
		return;
		
	PauseSnd(0, True);
	
	ShowDebugInfo();
	pm.sendStack->Show();
	
	if (!debugMenus.count)
		debugMenus.Add(&debugMenu);
	
	if (debugMenus.visible)
		debugMenus.Show();

	while (singleStep) {
		SOL_Event event = eventMgr->Wait(SOL_Event::All);
		//	eventMgr may have toggled off single stepping
		if (!singleStep)
			break;

if (event.type == SOL_Event::KeyDown && event.message == CtrlKey('D')) {
	static char gameName[50];
	char valBuf[10];
	*valBuf = 0;
	int val;
	if (InputValue(IV_String, "Game name prefix", gameName, val) &&
		 InputValue(IV_Number, "Game number", valBuf, val)) {
		saveGame->DumpFile(gameName, val);
		exit(1);
	}
}

		if (debugMenus.HandleEvent(event))
			continue;
			
		//	translate unclaimed mouse click to cmStep
		if (event.type == SOL_Event::MouseDown) {
			event.type = SOL_Event::Command;
			event.message = cmStep;
		}
			
		if (event.type != SOL_Event::Command)
			continue;
		
		if (breakpoints.HandleEvent(event.message))
			continue;

		switch (event.message) {
			case cmAcc:
				Inspect(pm.acc);
				break;

			case cmCur:
				Inspect(pm.object);
				break;

			case cmInspect:
				Inspect();
				break;

			case cmStep:
				eventMgr->Flush(SOL_Event::Mouse | SOL_Event::Key);
				return;
				
			case cmStepOver:
				breakpoints.StepOver();
				eventMgr->Flush(SOL_Event::Mouse | SOL_Event::Key);
				return;
				
			case cmStepOverSource:
				//	if there's no current source info, just step until
				//	next source line
				if (!pm.curSourceLineNum)
					breakpoints.StepSource();
				else
					breakpoints.StepOverSource();
				eventMgr->Flush(SOL_Event::Mouse | SOL_Event::Key);
				break;

			case cmStepSource:
				breakpoints.StepSource();
				eventMgr->Flush(SOL_Event::Mouse | SOL_Event::Key);
				break;

			case cmExit:
				singleStep = False;
				break;

			case cmQuit:
				exit(1);

			case cmDisplayMode:
				CycleDisplayMode();
				break;

			case cmDisplayObjects:
				ShowObjects();
				break;
	
			case cmDisplayResources:
				ShowResources();
				break;
	
			case cmDisplaySendStack:
				pm.sendStack->Toggle();
				break;
	
			case cmDisplayKernelCalls:
				ShowKernelCalls();
				break;
	
			case cmDisplayObjectsWithAddrs:
				ShowObjects(True);
				break;

			case cmDisplayPalette:
				PaletteWindow();
				break;

			case cmDisplayBitmaps:
				memMgr->ShowBitmaps();
				break;

			case cmDisplayTexts:
				ShowTexts();
				break;

			case cmEditAcc:
				EditAcc();
				break;
	
			case cmEditGlobal:
				GlobalEditVariable("global", pm.globals, -1);
				break;
	
			case cmEditLocal:
				LocalEditVariable("local", pm.locals, -1);
				break;
	
			case cmEditParameter:
				ParmEditVariable("parameter", pm.stack.parms, (int) pm.stack.parms[0]);
				break;
	
			case cmEditTemp:
				TempEditVariable("temp", pm.stack.temps, -1);
				break;
	
			case cmEditMemory:
				EditMemory();
				break;
				
			case cmCallKernel:
				CallKernel();
				break;
				
			case cmMemoryCheckIntegrity:
				if (!checkIntegrity)
					IntegrityChecking(CHECK_ALL,"debug");
				if (CheckIntegrity("debug"))
					msgMgr->Alert("Integrity OK"); 
				break;

			case cmMemoryCheckIntegrityOn:
				IntegrityChecking(CHECK_ALL,"debug");
				break;

			case cmMemoryCheckIntegrityOff:
				IntegrityChecking(0,"debug");
				break;

			case cmMemoryGraph:
				MemoryWindow();
				break;

			case cmMemoryList:
				ShowMemoryList();
				break;

			case cmMemorySummary:
				ShowMemoryStats();
				break;

			case cmHelp:
				ShowHelp();
				break;
		}
	}
	
	if (!singleStep) {
		debugWindow->Hide();
		sourceWindow->Hide();
		pm.sendStack->Hide();
		debugMenus.Delete();
		if (!breakpoints.AreAny())
			debuggerIsOn = False;
		PauseSnd(0, False);
	}
}

////////////////////////////////////////////////////////////////////////////////

static void
EditAcc()
{
	char	valBuf[20];
	int	val;

	strcpy(valBuf, FormatNumber(pm.acc, False));
	if (InputValue(IV_Value, "Edit accumulator", valBuf, val)) {
		pm.acc = val;
		ShowDebugInfo();
	}
}

static void
EditMemory()
{
	static char address[40];
	int			val;
	size_t		offset;
	
	if (!InputValue(IV_MemID | IV_MemIDOffset, "Memory address", address, val,
		 &offset))
		return;
		
	MemID id = val;

	id.Lock();
	SCIWord* ptr = (SCIWord*) &id[offset];

	static char input[40];
	strcpy(input, FormatNumber(*ptr, False));
	
	char prompt[30];
	sprintf(prompt, "Edit memory at %u.%u", val, offset);
	if (InputValue(IV_Value, prompt, input, val))
		*ptr = val;
	
	id.Unlock();
}

////////////////////////////////////////////////////////////////////////////////

static void
Inspect()
{
	static char buf[40];
	int			val;
	size_t		offset;
	
	if (InputValue(IV_Address, "Inspect", buf, val, &offset))
		Inspect(val, offset);
}

static void
Inspect(MemID id, size_t offset)
{
	if (((ObjectID) id).IsObject() && offset == 0)
		InspectObject(id);
	else if (IsDisplayable(id))
		InspectMemID(id, offset);
	else
		msgMgr->Alert("%u is not a valid MemID", id);
}

static Bool
InspectObject(ObjectID obj)
{
	enum {
		cmInspectObjEdit = 500,
		cmInspectObjInspect,
		cmInspectObjMode,
		cmInspectObjList,
		cmInspectObjHelp
	};

	static DebugMenuCmd	cmds[] = {
		DebugMenuCmd(cmBreakpointProperty,			'b', 't'),
		DebugMenuCmd(cmBreakpointPropertySticky,	'B', 'T'),
		0
	};
	static DebugMenuItem	items[] = {
		DebugMenuItem("B/b)reak",	cmds),
		DebugMenuItem("e)dit",		'e',			cmInspectObjEdit),
		DebugMenuItem("i)nspect",	'i',			cmInspectObjInspect),
		DebugMenuItem("m)ode",		'm',			cmInspectObjMode),
		DebugMenuItem("l)ist",		'l',			cmInspectObjList, 'c'),
		DebugMenuItem("?",			'?',			cmInspectObjHelp),
		DebugMenuItem("",				Kbd::Enter,	cmOK),
		0
	};
	static DebugMenu	menu("INSPECT OBJECT", items);
	
	static windowNestingLevel;
	
	obj.Lock();

	EventWindow* window = 0;
	ShowObject(obj, window, windowNestingLevel++);
	
	debugMenus.Add(&menu);
	
	Bool			done = False;
	SOL_Event	event;

	do {
		event = eventMgr->Wait(SOL_Event::All);
		
		if (window->HandleEvent(event))
			continue;
		
		if (debugMenus.HandleEvent(event))
			continue;
			
		if (event.type != SOL_Event::Command)
			continue;

		switch (event.message) {
			case cmBreakpointProperty:
				breakpoints.SetPropertyBreakpoint(obj);
				break;

			case cmBreakpointPropertySticky:
				breakpoints.SetPropertyBreakpoint(obj, True);
				break;
			
			case cmInspectObjEdit:
				EditProperty(obj);
				ShowObject(obj, window, windowNestingLevel - 1);
				break;
			
			case cmInspectObjInspect:
				InspectProperty(obj);
				break;
				
			case cmInspectObjMode:
				CycleDisplayMode();
				ShowObject(obj, window, windowNestingLevel - 1);
				break;
			
			case cmInspectObjList:
				InspectList(obj);
				break;
			
			case cmInspectObjHelp:
				ShowInspectObjectHelp();
				break;
			
			case 0:
			case cmOK:
				done = True;
				break;
		}
	} while (!done);
	
	obj.Unlock();
	
	--windowNestingLevel;
	delete window;

	debugMenus.Delete();
	
	return event.message == cmOK;
}

static void
ShowInspectObjectHelp()
{
	char buf[] =
		"i - inspect selector\n"
		"e - edit selector\n"
		"t - set property breakpoint\n"
		"T - set sticky property breakpoint\n"
		"\n"
		"c - display collection\n"
		"m - change display mode\n"
		"\n"
		"? - see this\n"
	;

	CenterXWindow(buf, "Inspect Object Hotkeys", 210, 25, True);
}

static void
InspectProperty(ObjectID obj)
{
	static char	selectorName[40];

	Selector selector;

	if (!InputPropertyName(obj, selector, selectorName))
		return;
	
	Property property = obj.GetProperty(selector);
	
	if (((ObjectID) property).IsObject())
		InspectObject(property);
	else {
		TextID title;
		title.AddF("(%s %s?)", obj.Name(), selectorName);
		TextID buf;
		buf.AddF("%s\n", FormatNumber(property));
		if (IsDisplayable(property) && IsPrintable(*(TextID) property))
			buf.Add((TextID) property);
		CenterXWindow(*buf, *title, 150, 100, True);
		title.Free();
		buf.Free();
	}
}

void
InspectMemID(MemID id, size_t offset)
{
	enum {
		cmInspectNextLink	= 500,
		cmInspectPriorLink
	};
	static DebugMenuItem	items[] = {
		DebugMenuItem("n)ext link",	'n', cmInspectNextLink,	']'),
		DebugMenuItem("p)rior link",	'p', cmInspectPriorLink,'['),
		DebugMenuItem("Enter/Esc", 	Kbd::Enter, 0),
		0
	};
	static DebugMenu		menu("INSPECT MEMID", items);
	
	MemAttrs attrs = id.Attrs();

	id.Lock();
	
	EventWindow* window = 0;
	ShowMemID(id, offset, window, attrs);

	debugMenus.Add(&menu);
	
	MemID	newID;
	Bool	done = False;
	do {
		SOL_Event event = eventMgr->Wait(SOL_Event::All);
		
		if (debugMenus.HandleEvent(event))
			continue;
			
		if (window->HandleEvent(event))
			continue;
			
		if (event.type != SOL_Event::Command)
			continue;
		
		switch (event.message) {
			case 0:
				done = True;
				break;

			case cmInspectNextLink:
			case cmInspectPriorLink:
				newID = ((MemID*) *id)[event.message == cmInspectPriorLink];
				if (!newID)
					msgMgr->Alert("end");
				else if (!IsDisplayable(newID))
					msgMgr->Alert("link is not a valid MemID");
				else {
					id.Unlock();
					id = newID;
					attrs = id.Attrs();
					id.Lock();
					ShowMemID(id, 0, window, attrs);
				}
				break;
		}

	} while (!done);
	
	delete window;

	debugMenus.Delete();
	
	id.Unlock();
}

static void
EditProperty(ObjectID obj)
{
	Selector selector;
	char		selectorName[40];
	char		buf[40];

	*selectorName = 0;
	if (!InputPropertyName(obj, selector, selectorName))
		return;
	
	obj.Lock();

	strcpy(buf, FormatNumber(obj.GetProperty(selector), False));
	
	char* prompt = "Edit property";
	int val;
	if (InputValue(IV_Value, prompt, buf, val) != IV_None)
		obj.SetProperty(selector, val);
	
	obj.Unlock();
}

static void
InspectList(ObjectID list)
{
	if (!list.RespondsTo(s_elements)) {
		msgMgr->Alert("not a list");
		return;
	}
	
	SOL_ListID elements = list.GetProperty(s_elements);
	
	if (!elements) {
		msgMgr->Alert("empty");
		return;
	}
	
	for (KNodeID node = (SOL_ListID) elements->First();
		  node;
		  node = node->Next()) {
		ObjectID obj = node->val;
		if (!obj.IsObject())
			break;
		if (!InspectObject(obj))
			return;
	}
	msgMgr->Alert("end");
}

////////////////////////////////////////////////////////////////////////////////

ObjectID
FindObject(char* name)
{
	for (SOL_NodeID node = objects.Last(); node; node = node->Prev()) {
		ObjectID obj = node->Key();
		if (!strcmp(obj.Name(), name))
			return obj;
	}
	
	return 0;
}

Bool
FindSelector(ObjectID obj, char* name, Selector& num)
{
	char buf[40];
	for (num = 0; GetVocabStr(SELECTOR_VOCAB, num, buf); num++)
		if (!strcmp(name, buf))
			return obj.RespondsTo(num);
	return False;
}

///////////////////////////////////////////////////////////////////////////
//	turn debugger on or off

void
ToggleDebug()
{
	singleStep = !singleStep;
	debuggerIsOn = singleStep || breakpoints.AreAny();
}

void
EnterDebug()
{
	if (!pm.game)
		return;

	DebugOn();
	Debug();
}

void
DebugOn()
{
	singleStep = True;
	debuggerIsOn = debuggerWasOn = True;
}

void
ExitThroughDebug()
{
	EnterDebug();
	exit(1);
}

//////////////////////////////////////////////////////////////////////////////

void
KSetDebug(argList)
{
	singleStep = True;
	debuggerIsOn = True;
	if (argCount)
		pm.acc = arg(1);
}

void
KInspectObject(argList)
{
	InspectObject((ObjectID) arg(1));
	if (argCount == 2)
		pm.acc = arg(2);
}

/////////////////////////////////////////////////////////////////////////////

void
RegisterKernelCall(int entry)
{
	kernelCallHistory[kernelCallNewest] = entry;
	if (++kernelCallNewest == MaxKernelCallHistory)
		kernelCallNewest = 0;
	if (kernelCallNewest == kernelCallOldest)
		if (++kernelCallOldest == MaxKernelCallHistory)
			kernelCallOldest = 0;
	static int nks = 0;
	if (!kernelCallIntegChecks) {
		if (configMgr->Get(configMgr->KernelIntegCheck))
			kernelCallIntegChecks = atoi(configMgr->Str(configMgr->KernelIntegCheck));
		else
			kernelCallIntegChecks = 50000;
	}
	if (!(++nks%kernelCallIntegChecks))
		CheckIntegrity("%d kernel calls",nks);
}

/////////////////////////////////////////////////////////////////////////////

static void
ShowHelp()
{
	char buf[] =
		"q - quit\n"
		"` - toggle debug\n"
		"~ - check memory\n"
		"/ - toggle menu\n"
		"\n"
		"b - set breakpoint - object,method\n"
		"B - set breakpoint - object,method (sticky)\n"
		"n - set breakpoint - script,offset\n"
		"N - set breakpoint - script,offset (sticky)\n"
		"k - set breakpoint - kernel call\n"
		"K - set breakpoint - kernel call (sticky)\n"
		"t - set breakpoint - changed memory\n"
		"R - break out of current routine\n"
		"C - clear all breakpoints\n"
		"\n"
		"F7 - step by source line\n"
		"F8 - step across source line\n"
		"Enter - single step\n"
		"Tab - step across call\n"
		"m - change display mode\n"
		"\n"
		"a - inspect ID in accumulator\n"
		"c - inspect current object\n"
		"i - inspect object or memory\n"
		"\n"
		"f - show largest memory\n"
		"s - show sends\n"
		"r - show resources\n"
		"o - show objects\n"
		"O - show objects w/addresses\n"
		"p - show palette\n"
		"\n"
		"g - edit global var\n"
		"l - edit local var\n"
		"e - edit memory\n"
		"\n"
		"v - show version info\n"
		"? - see this\n"
	;

	CenterXWindow(buf, "Debugger Hotkeys", 210, 25, True);
}

//////////////////////////////////////////////////////////////////////////////

static void
CallKernel()
{
	//	call a kernel function

	const InputLen = 50;
	static char input[InputLen + 1];
	char prompt[InputLen + 1];
	
	PMStack::Entry		stack[20];
	//	save the top entry for the number of entries
	PMStack::Entry*	ptr = stack + 1;

	strcpy(prompt, "Call kernel");
	
	while (1) {
		//	see what the user wants
		if (!GetInput(input, prompt, InputLen))
			return;
			
		//	make local copy so strtok() doesn't prevent redisplay
		char buf[InputLen + 1];
		strcpy(buf, input);

		//	get the kernel entry
		char* cp = strtok(buf, " ");
		if (!cp)
			return;

		int kernelEntry;
		for (int i = 0; i < kernelMax; i++)
			if (!strcmp(buf, kernelCalls[i].name)) {
				kernelEntry = i;
				break;
			}
		if (i >= kernelMax) {
			strcpy(prompt, "Not a kernel call:");
			continue;
		}
		
		Bool error = False;
		while (!error && (cp = strtok(0, " "))) {
			if (isdigit(*cp))
				*ptr++ = atoi(cp);
			else if (*cp == '$') {
				SCIWord val;
				sscanf(cp + 1, "%x", &val);
				*ptr++ = val;
			} else {
				SCIWord val = FindObject(cp);
				if (val)
					*ptr++ = val;
				else {
					error = True;
					sprintf(prompt, "%s is not an object", cp);
					break;
				}
			}
		}
		
		if (!error) {
			stack[0] = ptr - stack;
			kernelCalls[kernelEntry].func(stack);
			ShowDebugInfo();
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

static void
CheckSourceLevelOpcode()
{
	//	handle the source-level debugging opcodes specially while stepping,
	// since we use them to update our screen and we don't want them to ever
	// display
	
	while (1) {
		uchar opcode = pm.ip.PeekChar();
		if (opcode == op_lineNum) {
			pm.ip.GetChar();
			GetSourceLineNum();
		} else if (opcode == op_fileName) {
			pm.ip.GetChar();
			GetSourceModuleFileName();
		} else
			break;
	}
}

//	stack for debug info
struct {
	char	fileName[_MAX_PATH];
	int	lineNum;
} static debugInfo[100];
static int curDebugInfo;

void
ResetDebugInfo()
{
	// clear the debug info stack
	
	curDebugInfo = 0;
}

void
PushDebugInfo()
{
	//	we are about to execute code that is potentially in another module,
	//	so save where we are

	assert(curDebugInfo < NEls(debugInfo));	
	strcpy(debugInfo[curDebugInfo].fileName, pm.curSourceFile);
	debugInfo[curDebugInfo].lineNum = pm.curSourceLineNum;
	curDebugInfo++;
	
	// clear debug info, in case new module has none
	*pm.curSourceFile = 0;
	pm.curSourceLineNum = 0;
}

void
PopDebugInfo()
{
	assert(curDebugInfo);

	--curDebugInfo;
	strcpy(pm.curSourceFile, debugInfo[curDebugInfo].fileName);
	pm.curSourceLineNum = debugInfo[curDebugInfo].lineNum;
}

/////////////////////////////////////////////////////////////////////////////

//	dispose our stuff before saving or restoring, and recreate it after

Debugger * debugger;

Debugger::~Debugger()
{
	debugMenus.Hide();

	delete debugWindow;
	debugWindow = 0;

	delete sourceWindow;
	sourceWindow = 0;
}

void
Debugger::SaveRestoreBefore(SaveGame*, Bool save)
{
	if (singleStep) {
		debugMenus.Hide();

		delete debugWindow;
		debugWindow = 0;

		delete sourceWindow;
		sourceWindow = 0;
	}
	if (!save)
		breakpoints.Clear();
}

void
Debugger::SaveRestoreAfter(SaveGame*, Bool save)
{
	if (save && singleStep) {
		if (debugMenus.visible)
			debugMenus.Show();
		ShowDebugInfo();
	}
}
#endif

