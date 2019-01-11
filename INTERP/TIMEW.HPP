//	timew.hpp

#ifndef TIMEW_HPP
#define TIMEW_HPP

#ifndef TIME_HPP
#include "time.hpp"
#endif

#ifndef WIN32S
extern	int far *timeToCheck;
#else
extern	int	*timeToCheck;
#endif

class TimeMgrWin : public TimeMgr {
public:
	TimeMgrWin();
	~TimeMgrWin();
	ulong	GetTickCount();
	void	SetTickCount(ulong);
	ulong	GetMilliSec();
	void	EnableTimer();
	void	DisableTimer();
#ifdef WIN32S
	int far *SetCheckTime(int);
	void	EndCheckTime(int far *);
#else
	void	SetCheckTime(int, int far *);
#endif

protected:
#ifndef WIN32S
	HANDLE	hlib32;
	HINDIR	hTermTimeMgr;
	HINDIR	hTickCount;
	HINDIR	hTickSet;
	HINDIR	hSetCheckTime;
	FARPROC	lpTimerCallback;
	int		timerId;
#endif
};

#endif

