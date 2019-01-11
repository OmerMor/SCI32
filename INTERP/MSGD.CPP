//	msgd.cpp

#include <stdio.h>

#include "sol.hpp"

#include "config.hpp"
#include "mono.hpp"
#include "msgd.hpp"

MsgMgrDOS::~MsgMgrDOS()
{
	if (*quitStr) {
		puts("\n");
		puts(quitStr);
	}
	if (*fatalStrs) {
		//	make sure someone's prompt doesn't obscure the message
		puts("\n");
		puts(fatalStrs);
		
		if (*sciFatalStr)
			puts(sciFatalStr);
	}
}

void
MsgMgrDOS::MonoOutput(char* str)
{
#ifdef DEBUG
	if (configMgr->Get(configMgr->WVIDEO))
		return;
#endif
	::MonoStr(str);
}

void
MsgMgrDOS::MonoPosition(int row, int col)
{
	//	position the mono monitor next char output
	
#ifdef DEBUG
	if (configMgr->Get(configMgr->WVIDEO))
		return;
#endif
	MonoSet(row,col);
}

