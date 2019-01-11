//	debug.hpp
#ifdef DEBUG
#ifndef DEBUG_HPP
#define DEBUG_HPP

#ifndef SAVEABLE_HPP
#include "saveable.hpp"
#endif

#ifndef INTEG_HPP
#include "integ.hpp"
#endif

#ifndef OBJECT_HPP
#include "object.hpp"
#endif

class	ObjectID;

struct Debugger : Saveable {
	~Debugger();
	void	SaveRestoreBefore(SaveGame*, Bool save);
	void	SaveRestore(SaveGame*, Bool) {}
	void	SaveRestoreAfter(SaveGame*, Bool save);
} extern * debugger;

void	Debug();
void	EnterDebug();
void	ExitThroughDebug();
char*	OpcodeStr();
void	ResetDebugInfo();
void	PopDebugInfo();
void	PushDebugInfo();
void	RegisterKernelCall(int entry);
void	ToggleDebug();

const MaxSourceLineLen = 512;

//	internal functions used by other debugging modules
void		DebugOn();
ObjectID	FindObject(char* name);
Bool		FindSelector(ObjectID id, char* name, Selector& num);

inline Bool
IsDisplayable(MemID id)
{
	return (int) id != 1 && id.IsValid();
}

const int	MaxKernelCallHistory = 200;

extern Bool	debuggerIsOn;
extern Bool	debuggerWasOn;
extern int	kernelCallHistory[];
extern int	kernelCallOldest;
extern int	kernelCallNewest;
extern Bool	singleStep;

#endif
#endif

