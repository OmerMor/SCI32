//	msgw.cpp

#include "sol.hpp"
#include "config.hpp"
#include "mbox.hpp"
#include "msgw.hpp"

MsgMgrWin::~MsgMgrWin()
{
	if (*fatalStrs)
		MBox(fatalStrs, "Panic");
	else if (*quitStr)
		MBox(quitStr, "Quit");
}

void
MsgMgrWin::MonoOutput(char* str)
{
	SciDisplay(str);
}

void
MsgMgrWin::MonoPosition(int, int)
{
}
