#if !defined(SCIWIN_HPP)
#define SCIWIN_HPP

#ifndef ___WIN386_INCLUDED__
#include "winodef.hpp"
#include <windows.h>
#endif

extern	HWND		hMyWnd;
extern	HDC		hMyDC;
extern	HANDLE	MyInstance;
extern	HPALETTE	hPal, hOldPal;
extern	HCURSOR	hMyCur;
extern	HCURSOR	hAltCur;
extern	HCURSOR	hCurCur;
extern	int		gameWidth, gameHeight;
extern	int		gameBorder;
extern	int		vmapWidth;
extern	int		vmapHeight;
extern	BOOL		LOCKMAIN;
extern	BOOL		LOCKAUDIO;
extern	BOOL		SHOWMEM;
extern	BOOL		SHOWNEWS;
extern	BOOL		USEAUDIO;
extern	BOOL		USEMIDI;
extern	BOOL		DOPPCTEST;
extern	BOOL		HIRES;
extern	BOOL		TRUECOLOR;
extern	BOOL		COLORCURSOR;
extern	int		forcePPC;
extern	int		reserveK;
extern	int		reserveK4;
extern	int		reserveK8;
extern	int		reserveK16;
extern	int		sciMinK;
extern	int		sciMaxK;
extern	int		DACSize;
extern	int		cursorViewAdd;
extern	int		maxCursorSize;
extern	int		asyncCheckFreq;
extern	char		szWinPatchDir[];

#ifndef WIN32S
extern	HINDIR	hTickCount;
#else
extern	int		wingMinK;
#endif

#endif

