//	time.hpp

#ifndef TIME_HPP
#define TIME_HPP

#ifndef SAVEABLE_HPP
#include "saveable.hpp"
#endif

struct TimeMgr : Saveable
{
	virtual			~TimeMgr() {}

	virtual ulong	GetTickCount() = 0;
	virtual ulong	GetMilliSec() = 0;
	virtual void	SetTickCount(ulong) = 0;
	uint				SysTime(int func);
	
	void				SaveRestore(SaveGame*, Bool save);

};
extern TimeMgr * timeMgr;

#endif

