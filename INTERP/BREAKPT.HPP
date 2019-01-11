//	breakpt.hpp
#ifdef DEBUG
#ifndef BREAKPT_HPP
#define BREAKPT_HPP

class Breakpoint;
class DebugMenu;
class ObjectID;

struct Breakpoints {
	Breakpoints();
	Bool	AreAny() { return nEntries != 0; }
	void	Clear();
	Bool	HandleEvent(int cmd);
	void	SetPropertyBreakpoint(ObjectID obj, Bool sticky = False);
	void	StepOver();
	void	StepSource();
	void	StepOverSource();
	void	Test();

	void	Remove(Breakpoint&);		//	used only by Breakpoint::Clear()

protected:
	void	Add(Breakpoint&);
	void	Set(Breakpoint&, Bool sticky = False);
	void	StepOut();

	enum { Size = 20 };

	Breakpoint*	entries[Size];
	int			nEntries;
	
};

extern Breakpoints breakpoints;

extern DebugMenu	breakpointMenu;

#endif
#endif

