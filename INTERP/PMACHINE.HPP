// pmachine.hpp
// 	interface to the pmachine

#ifndef PMACHINE_HPP
#define PMACHINE_HPP

#ifndef GLOBALS_HPP
#include "globals.hpp"
#endif

#ifndef MEMID_HPP
#include "memid.hpp"
#endif

#ifndef OBJECT_HPP
#include "object.hpp"
#endif

#ifndef SAVEABLE_HPP
#include	"saveable.hpp"
#endif

#ifndef SCRIPT_HPP
#include	"script.hpp"
#endif

typedef long	Acc;

extern "C" {

void			pmRun();
void			pmEnd();
 
void			pmRestartBuf();
void			pmTossMemID( unsigned, unsigned );
#pragma aux pmTossMemID parm [ECX] [EAX];

Property&	pmGameSetProp(Selector select, unsigned int value);
#pragma aux pmGameSetProp parm [EAX] [EDX];

Property&	pmGlobal(GlobalNum);
#pragma aux pmGlobal parm [EDX];

Bool			pmCheckIntegrity(char* note);

void			pmDispatch(unsigned scriptNum, unsigned entryNum, unsigned nArgChars);

Acc			invokeMethod(unsigned Obj, unsigned select, unsigned nArgChars, void * a);
#pragma aux invokeMethod parm [EBX] [EDX] [ECX] [EDI];

void			pmQuickMessage(unsigned Obj, unsigned nArgChars, void * a);
#pragma aux pmQuickMessage parm [EBX] [ECX] [EDI];

void			pmPushMany(Selector select, unsigned nArgs, short* ptrs);
#pragma aux pmPushMany parm [EDX] [ECX] [ESI];

void			pmSaveRestoreBefore(SaveGame*, Bool);
void			pmSaveRestoreAfter(SaveGame*, Bool);
}

//static void		Messager(const ObjectID&);

_Packed struct PMachine {
	Int16			game;
	Int16			object;
	
	Int32		 	acc;
	Int32		 	prevAcc;

	Property* 	StackPtr;
 	Property* 	TempPtr;
 	Property* 	GlobalPtr;
 	Property* 	ParmPtr;

 	VariableID 	Locals;
	VariableID	Globals;

	Int16			disposeTossRet;
 	unsigned		RestArgs;

	Int16			curScriptNum;
	char			curSourceFile[256];
	Int16			curSourceLineNum;
	Int32			restart[4];
};

const uchar	OP_BYTE		= 0x01;		// char operation if set, int otw

extern "C" PMachine pm;

#endif

