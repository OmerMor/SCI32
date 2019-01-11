// pmachine.hpp
// 	interface to the pmachine

#ifndef PMACHINE_HPP
#define PMACHINE_HPP

#include <setjmp.h>
#include <stdlib.h>

#ifndef GLOBALS_HPP
#include "globals.hpp"
#endif

#ifndef MEMID_HPP
#include "memid.hpp"
#endif

#ifndef OBJECT_HPP
#include "object.hpp"
#endif

#ifndef PMIP_HPP
#include "pmip.hpp"
#endif

#ifndef PMSTACK_HPP
#include "pmstack.hpp"
#endif

#ifndef SAVEABLE_HPP
#include	"saveable.hpp"
#endif

#ifndef SCRIPT_HPP
#include	"script.hpp"
#endif

typedef SCIWord	Acc;

class SendStack;

struct PMachine : Saveable {
	void	Run();
	void	End();

	Bool			CheckIntegrity(char* note);
	Property&	Global(GlobalNum);

	static void		Call(PMIP ip, unsigned nArgChars);
	static void		Dispatch(unsigned scriptNum, unsigned entryNum,
							unsigned nArgChars);
	static Acc		InvokeMethod(const ObjectID&, Selector,
							unsigned nArgChars = 0, ...);
	static void		QuickMessage(const ObjectID&, unsigned nArgChars);
	static void		Messager(const ObjectID&);

	void	SaveRestoreBefore(SaveGame*, Bool);
	void	SaveRestoreDo(Saveable::Func func, SaveGame* saveGame, Bool save);
	void	SaveRestoreAfter(SaveGame*, Bool);

	Acc			acc;
	unsigned		curScriptNum;
#ifdef MACINTOSH
	char			curSourceFile[32];
#else
	char			curSourceFile[_MAX_PATH];
#endif
	int			curSourceLineNum;
	ObjectID		game;
	VariableID	globals;
	Property*	globalPtr;
	VariableID	locals;
	PMIP			ip;
	ObjectID		object;
	Acc			prevAcc;
	int			restArgs;
	PMStack		stack;
	SendStack*	sendStack;

	jmp_buf		restartBuf;

protected:
	static void		ExecuteCode();

};
extern PMachine pm;

#endif

