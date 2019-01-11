//	winthunk.hpp

#ifndef WINTHUNK_HPP
#define WINTHUNK_HPP

#ifndef WIN32S
#include "scidll.hpp"
#else
#define	W32SUT_32
#include "scidllws.h"
#include "w32sut.h"
#endif


void InitThunk(void);
void EndThunk(void);

#ifndef WIN32S

extern	FARPROC	fp;
extern	HANDLE	hlib32;
extern	HINDIR	hInitTimeMgr;
extern	HINDIR	hTermTimeMgr;
extern	HINDIR	hTickCount;
extern	HINDIR	hTickSet;
extern	HINDIR	hSetCheckTime;
extern	HINDIR	hDoSoundA;
extern	HINDIR	hDoSoundB;
extern	HINDIR	hDoSoundC;
extern	HINDIR	hDoSoundD;
extern	HINDIR	hDoSoundE;
extern	HINDIR	hDoSoundF;
extern	HINDIR	hDoSoundG;
extern	HINDIR	hSetMIDIType;
extern	HINDIR	hOpenSnd;
extern	HINDIR	hCloseSnd;

#else

typedef BOOL (WINAPI * PUTREGISTER) ( HANDLE     hModule,
                                      LPCSTR     lpsz16BitDLL,
                                      LPCSTR     lpszInitName,
                                      LPCSTR     lpszProcName,
                                      UT32PROC * ppfn32Thunk,
                                      FARPROC    pfnUT32Callback,
                                      LPVOID     lpBuff
                                    );

typedef VOID (WINAPI * PUTUNREGISTER) (HANDLE hModule);

extern	PUTREGISTER		pUTRegister;
extern	PUTUNREGISTER	pUTUnRegister;
extern	UT32PROC			pfnUTProc;
extern	HMODULE			hKernel32;

#endif

#endif

