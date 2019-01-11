//	debugdsp.hpp
//		for use only by debugger

#ifdef DEBUG

#ifndef DEBUGDSP_HPP
#define DEBUGDSP_HPP

//	for MemAttrs
#ifndef MEMMGR_HPP
#include "memmgr.hpp"
#endif

//	types of InputValues
enum IV_Type {
	IV_None			= 0x00,
	IV_Number		= 0x01,
	IV_Object		= 0x02,
	IV_MemID			= 0x04,
	IV_MemIDOffset	= 0x08,
	IV_ScriptOffset= 0x10,
	IV_String		= 0x20,
	IV_ObjectMethod= 0x40,
	IV_Value			= IV_Number | IV_Object | IV_MemID,
	IV_Address		= IV_Object | IV_MemID | IV_MemIDOffset
};

class EventWindow;
class MemID;
class ObjectID;
class SOL_Window;

void		CycleDisplayMode();
char*		FormatNumber(SCIWord val, Bool showName = True);
Bool		InputPropertyName(ObjectID obj, Selector&, char* name);
IV_Type	InputValue(int mask, char* prompt, char* buf, int& val,
				size_t* ofs = 0);
void		ShowTexts();
void		ShowDebugInfo();
void		ShowFreeMemory();
void		ShowMemID(MemID, size_t, EventWindow*&, MemAttrs);
void		ShowMemoryList();
void		ShowMemoryStats();
void		ShowObject(ObjectID, EventWindow*&, int level);
void		ShowObjects(Bool showIDs = False);
void		ShowResources();
void		ShowKernelCalls();

extern EventWindow*	debugWindow;
extern SOL_Window*	sourceWindow;

#endif
#endif

