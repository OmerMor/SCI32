//	debugdsp.cpp
#ifdef DEBUG

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sol.hpp"

#include "debug.hpp"
#include "debugdsp.hpp"
#include "dialog.hpp"
#include "info.hpp"
#include "kernel.hpp"
#include "list.hpp"
#include "msg.hpp"
#include "opcodes.hpp"
#include "pmachine.hpp"
#include "remap.hpp"
#include "resource.hpp"
#include "sendstac.hpp"
#include "textid.hpp"
#include "vocab.hpp"
#include "window.hpp"

EventWindow*	debugWindow;
SOL_Window*		sourceWindow;

enum DisplayMode {
	DM_First,
	DM_Auto = DM_First,
	DM_Hex,
	DM_Signed,
	DM_Unsigned,
	DM_Last = DM_Unsigned
} static displayMode;

static void ShowSourceLine();

void
ShowDebugInfo()
{
	//	display single step information
	TextID	buf;
	
	//	current object name
	buf.AddF("%-29s", pm.object.Name());
	
	//	remap status
	buf.AddF("%c\n", Remap::remapDepthOn ? 'D' : Remap::remapCount ? 'R' : ' ');

	//	opcode
	buf.AddF("%s\n", OpcodeStr());

	//	accumulator, IP and display mode
	buf.AddF("acc $%04x  ip %u/$%x  %c\n",
		(ushort) pm.acc, pm.curScriptNum, pm.ip.ScriptOffset(),
		"AHSU"[displayMode]);

	//	stack pointer, parameters pointer and temps
	buf.AddF("sp $%x parms $%x temps $%x\n",
		pm.stack.PtrOffset(),
		pm.stack.ParmsOffset(),
		pm.stack.TempsOffset());

	//	globals, locals and stack
	buf.AddF("stack $%x glob $%x locl $%x\n",
		(ushort) pm.stack.stack, (ushort) pm.globals, (ushort) pm.locals);
	
	buf.AddF("SCI %s / for menu\n", version);

	if (!debugWindow) {
		const width = 135;
		debugWindow =
			DebugWindow(*buf, 0, width, SOL_Point(319 - width + 1, 14));
	} else
		debugWindow->SetText(*buf);

	debugWindow->Show();
	
	ShowSourceLine();
	
	buf.Free();
}

char*
OpcodeStr()
{
	//	create the opcode line for the debug info box
	
	static char dest[50];

	//	get the opcode and determine if its argument is byte or word-sized
	uchar opcode = pm.ip.PeekChar();
	Bool	charArg = opcode & OP_BYTE;
	opcode &= ~OP_BYTE;

	//	read the opcode name and information
	char opcodeData[100];
	GetVocabStr(OPCODE_VOCAB, opcode / 2, opcodeData);
	char* opcodeStr = opcodeData + 2;

	//	no args
	if (!*(UInt16*) opcodeData & 1)
		strcpy(dest, opcodeStr);
	else {
		SCIUWord arg =
			SCIUWord(charArg ? pm.ip.PeekNextChar() : pm.ip.PeekNextUWord());

		switch (opcode) {
			case op_callk:
				//	display kernel call name
				sprintf(dest, "%s %s", opcodeStr, kernelCalls[arg].name);
				break;

			case op_bt:
			case op_bnt:
				//	display whether branch will be taken
				sprintf(dest, "%s $%x will %sbranch", opcodeStr, arg, 
					pm.acc == (opcode == op_bt) ? "" : "not ");
				break;

			default:
				//	just display argument
				sprintf(dest, "%s $%x", opcodeStr, arg);
				break;
		}
	}
	
	return dest;
}

void
ShowMemID(MemID id, size_t offset, EventWindow*& window, MemAttrs attrs)
{
	TextID title;
	title.AddF("%s", FormatNumber((int) (SOL_Handle) id, False));
	if (offset)
		title.AddF(".%u", offset);
	title.AddF("  %s", id.GetMemTypeStr());
	title.AddF("  Size %s", FormatNumber(id.Size(), False));
	title.AddF("  Attrs $%x", attrs);

	const BytesPerLine	= 16;
	const LinesPerAdd		= 20;
	const LineWidth		= 64;

	char		lineBuf[LineWidth * LinesPerAdd + 1];
	char*		end = (char*) *id + id.Size() + SIGNATURESIZE;
	int		nLines = 0;
	char*		bufP = lineBuf;
	TextID	buf;

	for (char* cp = (char*) *id + offset; cp < end; cp += BytesPerLine) {
		int i;
		sprintf(bufP, "%04x: ", cp - (char*) *id);
		for (i = 0; i < BytesPerLine / 2; i++) {
			if (cp + i * 2  + 1 < end)
				sprintf(bufP + strlen(bufP), "%04x ", ((ushort*) cp)[i]);
			else if (cp + i * 2 < end)
				sprintf(bufP + strlen(bufP), "%02x   ", ((ushort*) cp)[i] & 0xFF);
			else
				strcat(bufP, "     ");
#if 0
			if (i == BytesPerLine / 2 / 2 - 1)
				strcat(bufP, " ");
#endif
		}
		strcat(bufP, " ");
		for (i = 0; i < BytesPerLine; i++)
			if (cp + i < end)
				sprintf(bufP + strlen(bufP), "%c", isprint(cp[i]) ? cp[i] : '.');
			else
				strcat(bufP, " ");
		strcat(bufP, "\n");
		
		if (++nLines >= LinesPerAdd) {
			buf.Add(lineBuf);
			bufP = lineBuf;
			nLines = 0;
		} else
			bufP += LineWidth;
	}
	
	buf.Add(lineBuf);

	delete window;
	window = CenterXWindow(*buf, *title, 320, 25);

	title.Free();
	buf.Free();
}

void
ShowObject(ObjectID obj, EventWindow*& window, int level)
{
	TextID buf;

	//	properties
	PropDictID propDict = (PropDictID) obj->propDict;
	char name[100];
	for (int i = 0; i < obj->size; i++)
		buf.AddF("%s:%s  ", GetSelectorName(propDict[i].selector, name),
			FormatNumber(obj->operator[](i)));

	buf.Add('\n');
	
	TextID title;
	title.AddF("Inspect :: %s (%s)\r\n", obj.Name(), FormatNumber(obj, False));

	if (window)
		delete window;
	
	int y = (level * 10 + 25) % 100;
	window = CenterXWindow(*buf, *title, 310, y);

	buf.Free();
	title.Free();
}

void
CycleDisplayMode()
{
	displayMode = DisplayMode(displayMode + 1);
	if (displayMode > DM_Last)
		displayMode = DM_First;
		
	ShowDebugInfo(); 	
}

char*
FormatNumber(SCIWord val, Bool showName)
{
	static char	buf[50];

	switch (displayMode) {
		case DM_Auto:
			if (showName && ((ObjectID) val).IsObject())
				sprintf(buf, "%s", ((ObjectID) val).Name());
			else if (val > 1000 || val < -100) {
				sprintf(buf, "$%x", (uint) (SCIUWord) val);
			} else
				sprintf(buf, "%d", val);
			break;
			
		case DM_Signed:
			sprintf(buf, "%d", val);
			break;
			
		case DM_Unsigned:
			sprintf(buf, "%u", (uint) (SCIUWord) val);
			break;
			
		case DM_Hex:
			sprintf(buf, "$%x", (uint) (SCIUWord) val);
			break;
	}
	
	return buf;
}

Bool
InputPropertyName(ObjectID obj, Selector& selector, char* name)
{
	//	get a valid property name for this object from the user
	
	char* prompt = "Property";
		
	while (1) {
		if (!GetInput(name, prompt, 25))
			return False;

		if (FindSelector(obj, name, selector))
			return True;
			
		prompt = "Not a selector for this object";
	}
}

IV_Type
InputValue(int mask, char* prompt, char* buf, int& val, size_t* offset)
{
	//	get a value from the user
	
	val = 0;
	if (offset)
		*offset = 0;

	while (1) {
		if (!GetInput(buf, prompt, 30))
			return IV_None;

		char* cp;

		//	object method
		if (mask == IV_ObjectMethod) {
			char str[100];
			strcpy(str, buf);
			cp = strrchr(str, ' ');
			if (cp) {
				*cp = 0;
				cp++;
			}
			
			if (isdigit(*str) || *str == '$') {
				if (isdigit(*str))
					val = atoi(str);
				else
					sscanf(str + 1, "%x", &val);
				
				if (!((ObjectID) val).IsObject()) {
					prompt = "Not an object";
					continue;
				}
			} else if (!(val = FindObject(str))) {
				prompt = "Not an object";
				continue;
			}
	
			Selector selector;
	
			if (!cp || !stricmp(cp, "any"))
				*offset = (Selector) -1;
				
			else if (!FindSelector(val, cp, selector)) {
				prompt = "Not selector for object";
				continue;

			} else
				*offset = selector;
			
			return IV_ObjectMethod;
		}
		
		//	memid and offset
		if ((cp = strchr(buf, '.')) && mask != IV_String) {
			if (!(mask & IV_MemIDOffset)) {
				prompt = "MemID.offset not allowed here";
				continue;
			}
			char idStr[40];
			strcpy(idStr, buf);
			cp = strchr(idStr, '.');
			*cp = 0;
			if (*idStr == '$')
				sscanf(idStr + 1, "%x", &val);
			else if (isdigit(*idStr))
				val = atoi(idStr);
			else {
				prompt = "ID must be positive number";
				continue;
			}
			
			if (!IsDisplayable(val)) {
				prompt = "Invalid MemID";
				continue;
			}
			
			char* offsetStr = buf + (cp - idStr) + 1;
			if (*offsetStr == '$')
				sscanf(offsetStr + 1, "%x", offset);
			else if (isdigit(*offsetStr))
				*offset = atoi(offsetStr);
			else {
				prompt = "Offset must be positive number";
				continue;
			}
			
			if (*offset >= ((MemID) val).Size()) {
				prompt = "Offset is > id size";
				continue;
			}
			
			return IV_MemIDOffset;
		
		//	script number and offset
		} else if ((cp = strchr(buf, '/')) && mask != IV_String) {
			if (!(mask & IV_ScriptOffset)) {
				prompt = "Script/offset not allowed here";
				continue;
			}
			char scriptNumStr[40];
			strcpy(scriptNumStr, buf);
			cp = strchr(scriptNumStr, '/');
			*cp = 0;
			if (*scriptNumStr == '$')
				sscanf(scriptNumStr + 1, "%x", &val);
			else if (isdigit(*scriptNumStr))
				val = atoi(scriptNumStr);
			else {
				prompt = "Script number must be positive number";
				continue;
			}
			
			char* offsetStr = buf + (cp - scriptNumStr) + 1;
			if (*offsetStr == '$')
				sscanf(offsetStr + 1, "%x", offset);
			else if (isdigit(*offsetStr))
				*offset = atoi(offsetStr);
			else {
				prompt = "Offset must be positive number";
				continue;
			}
			
			//	make sure the offset is in range
			ScriptID script = ScriptPtr(val);
			if (*offset < script->codeOffset ||
				 *offset - script->codeOffset > script->code.Size()) {
				prompt = "Invalid code offset";
				continue;
			}
			
			return IV_ScriptOffset;
		
		//	decimal number
		} else if ((isdigit(*buf) || *buf == '-') && mask != IV_String) {
			val = atoi(buf);
			if ((mask & (IV_Object | IV_MemID)) && *buf != '-' &&
					IsDisplayable(val))
				return ((ObjectID) val).IsObject() ? IV_Object : IV_MemID;
			else if (mask & IV_Number)
				return IV_Number;
			else
				prompt = "Invalid MemID";

		//	hex number
		} else if (*buf == '$' && mask != IV_String) {
			sscanf(buf + 1, "%x", &val);
			if ((mask & (IV_Object | IV_MemID)) && IsDisplayable(val))
				return ((ObjectID) val).IsObject() ? IV_Object : IV_MemID;
			else if (mask & IV_Number)
				return IV_Number;
			else
				prompt = "Invalid MemID";

		//	string
		} else {
			if (mask & IV_String)
				return IV_String;

			if (val = FindObject(buf))
				if (mask & IV_Object)
					return IV_Object;
				else
					prompt = "Object name not permitted here";

			if (mask & IV_Object)
				prompt = "Not an object";
			else
				prompt = "Value must be numeric";
		}
	}
}

void
ShowResources()
{
	TextID buf = resMgr->MakeDebugDisplayStr();
	CenterXWindow(*buf, "Resources", 300, 25, True);
	buf.Free();
}

void
ShowMemoryStats()
{
	size_t	totUsed = 0;
	size_t	totFree = 0;
	size_t	maxFree = 0;
	int		nUsed = 0;
	int		nFree = 0;

	TextID listBuf;

	_heapinfo heapInfo;
	heapInfo._pentry = 0;
	while (_heapwalk(&heapInfo) == _HEAPOK) {
		if (heapInfo._useflag == _USEDENTRY) {
			listBuf.AddF("%ld ", heapInfo._size);
			totUsed += heapInfo._size;
			nUsed++;
		} else {
			listBuf.AddF("(%ld) ", heapInfo._size);
			totFree += heapInfo._size;
			nFree++;
			maxFree = Max(maxFree, heapInfo._size);
		}
	}

	TextID buf;
	buf.Add("HEAP:\n");
	buf.AddF("Total free %lu bytes, %d blocks, max %lu\n", totFree, nFree,
		maxFree);
	buf.AddF("Total used %lu bytes, %d blocks\n", totUsed, nUsed);
	
	buf.Add("\nHUNK:\n");

	size_t conventionalMem;
	size_t extendedMem;
	memMgr->TotalMemory(&conventionalMem, &extendedMem);
	buf.AddF("Total: conventional %uK extended %uK\n\n", conventionalMem,
		extendedMem);

	TextID memStr = memMgr->MakeMemTypeDspStr();
	buf.Add(memStr);
	memStr.Free();

	buf.Add("\nHEAP ALLOCATIONS:\n");
	buf.Add(listBuf);
	listBuf.Free();

	CenterXWindow(*buf, "Memory Statistics", 300, 25, True);
	buf.Free();
}

void
ShowFreeMemory()
{
	msgMgr->Alert("Largest allocation possible: %u", memMgr->FreeMemory());
}

void
ShowObjects(Bool showIDs)
{
	//	show the names of all objects, with ids, if desired
	
	TextID buf;
	
	int i = 10;
	int nObjects = 0;
	for (SOL_NodeID node = objects.First(); node; node = node->Next()) {
      i++;
      i = 10 + i%10;
		ObjectID obj = node->Key();
		if (obj->Info() & CLONEBIT)
			buf.Add("*");
		buf.Add(obj.Name());
		nObjects++;
		if (showIDs)
			buf.AddF(" $%x", (int) obj);
		buf.Add(" ");
	}
	
	TextID title;
	title.AddF("Objects : %d", nObjects);
	
	CenterXWindow(*buf, *title, 300, 25, True);

	buf.Free();
	title.Free();
}

void
ShowKernelCalls()
{
	//	show the last so many kernel calls made
	
	TextID buf;
	
	if (kernelCallNewest != kernelCallOldest) {
		int n = kernelCallNewest;
		do {
			if (n-- == 0)
				n = MaxKernelCallHistory - 1;
			buf.AddF("%s\n", kernelCalls[kernelCallHistory[n]].name);
		} while (n != kernelCallOldest);
	}
 
	if (kernelCallOldest)
		buf.Add("(etc.)");
	else if (kernelCallNewest)
		buf.Add("(end of list)");
	else
		buf.Add("(none)");

	CenterXWindow(*buf, "Kernel Call History", 120, 0, True);

	buf.Free();
}

void
ShowMemoryList()
{
	//	show each memory id
	
	int start = 1;
	static char buf[20] = "1";
	
	if (!InputValue(IV_Number, "Show MemIDs starting with:", buf, start))
		return;
	
	char title[40];
	const increment = 100;

	while (1) {
		int end;
		TextID buf = memMgr->MakeMemIDListStr(start, &end, increment);
		if (!buf) {
			msgMgr->Alert("End");
			break;
		}
		sprintf(title, "MemIDs %u - %u", start, end);
		Bool esc;
		DebugWindow(*buf, title, 200, SOL_Point(0, 0), True, &esc);
		buf.Free();
		if (esc)
			break;
		start += increment;
	}
}

void
ShowTexts()
{
	//	show text memory, within a range


	//	get the range, in the form 'start,end'
	
	static char buf[40];
	int dummy;
	if (!InputValue(IV_String, "Enter start,end MemIDs to display", buf, dummy))
		return;
		
	const char separators[] = " ,";
	int start;
	int end;
	
	char tmp[40];
	strcpy(tmp, buf);
	char* cp = strtok(tmp, separators);
	if (!cp)
		return;
	start = atoi(cp);
	
	cp = strtok(0, separators);
	if (!cp)
		end = 99999;
	else
		end = atoi(cp);
		
	memMgr->ShowText(start, end);
}

static void
ShowSourceLine()
{
	static char	file[_MAX_PATH];
	static int	lineNum = -1;
	static char	line[MaxSourceLineLen + 1];
	
	//	has the file or line number changed?
	if (strcmp(file, pm.curSourceFile) || lineNum != pm.curSourceLineNum) {
		strcpy(file, pm.curSourceFile);
		lineNum = pm.curSourceLineNum;

		if (!*file)
			strcpy(line, "No source information");
		else {
			char fullName[_MAX_PATH];
			_searchenv(file, "SINCLUDE", fullName);
			if (!*fullName)
				sprintf(line, "Can't find %s", file);
			else {
				FILE* fp = fopen(fullName, "rt");
				if (!fp)
					sprintf(line, "Can't open %s", fullName);
				else
					for (int i = 0; i < lineNum; i++)
						if (!fgets(line, sizeof line, fp)) {
							sprintf(line, "Can't find line %d in %s", lineNum,
								fullName);
							break;
						}
				fclose(fp);
			}
		}
	}

	if (!sourceWindow)
		sourceWindow = DebugWindow(line, 0, 320, SOL_Point(0, 0));
	else
		sourceWindow->SetText(line);
	sourceWindow->Show();
}

#endif

