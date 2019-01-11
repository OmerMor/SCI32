// winthunk.cpp

#define INCLUDE_MMSYSTEM_H
#include "sol.hpp"
#include "mbox.hpp"
#include "sciwin.hpp"
#include "winthunk.hpp"

#ifdef WIN32S
#include "winclass.hpp"
#endif

#ifndef WIN32S

FARPROC	fp;
HANDLE	hlib32;
HINDIR	hInitTimeMgr;
HINDIR	hTermTimeMgr;
HINDIR	hTickCount;
HINDIR	hTickSet;
HINDIR	hSetCheckTime;
HINDIR	hDoSoundA;
HINDIR	hDoSoundB;
HINDIR	hDoSoundC;
HINDIR	hDoSoundD;
HINDIR	hDoSoundE;
HINDIR	hDoSoundF;
HINDIR	hDoSoundG;
HINDIR	hSetMIDIType;
HINDIR	hOpenSnd;
HINDIR	hCloseSnd;
HINDIR	hWEP;

#else

PUTREGISTER		pUTRegister = NULL;
PUTUNREGISTER	pUTUnRegister = NULL;
UT32PROC			pfnUTProc = NULL;
HMODULE			hKernel32 = 0;

#endif


void InitThunk(void)
{
#ifndef WIN32S

  hlib32 = LoadLibrary( "scidll.dll" );
  fp = GetProcAddress( hlib32, "Win386LibEntry" );

  hWEP			=	GetIndirectFunctionHandle(fp, INDIR_WORD, INDIR_ENDLIST );
  hInitTimeMgr =  GetIndirectFunctionHandle(fp, INDIR_WORD, INDIR_CDECL, INDIR_ENDLIST );
  hTermTimeMgr =  GetIndirectFunctionHandle(fp, INDIR_WORD, INDIR_CDECL, INDIR_ENDLIST );
  hTickCount =    GetIndirectFunctionHandle(fp, INDIR_WORD, INDIR_CDECL, INDIR_ENDLIST );
  hTickSet =      GetIndirectFunctionHandle(fp, INDIR_DWORD, INDIR_WORD, INDIR_ENDLIST );
  hSetCheckTime = GetIndirectFunctionHandle(fp, INDIR_DWORD, INDIR_DWORD, INDIR_WORD, INDIR_WORD, INDIR_ENDLIST );
  hDoSoundA	= 		GetIndirectFunctionHandle(fp, INDIR_WORD,
													INDIR_WORD, INDIR_ENDLIST );
  hDoSoundB	= 		GetIndirectFunctionHandle(fp, INDIR_WORD,
													INDIR_WORD,
													INDIR_WORD, INDIR_ENDLIST );
  hDoSoundC	= 		GetIndirectFunctionHandle(fp, INDIR_WORD,
													INDIR_DWORD, INDIR_WORD,
													INDIR_WORD, INDIR_ENDLIST );
  hDoSoundD	= 		GetIndirectFunctionHandle(fp, INDIR_WORD,
													INDIR_DWORD, INDIR_WORD,
													INDIR_WORD,
													INDIR_WORD, INDIR_ENDLIST );
  hDoSoundE	= 		GetIndirectFunctionHandle(fp, INDIR_WORD,
													INDIR_DWORD, INDIR_WORD,
													INDIR_WORD,
													INDIR_WORD,
													INDIR_WORD, INDIR_ENDLIST );
  hDoSoundF	= 		GetIndirectFunctionHandle(fp, INDIR_WORD,
													INDIR_DWORD, INDIR_WORD,
													INDIR_WORD,
													INDIR_WORD,
													INDIR_WORD,
													INDIR_WORD, INDIR_ENDLIST );
  hDoSoundG	= 		GetIndirectFunctionHandle(fp, INDIR_WORD,
													INDIR_DWORD, INDIR_WORD,
													INDIR_DWORD, INDIR_WORD,
													INDIR_DWORD, INDIR_WORD,
													INDIR_DWORD, INDIR_WORD,
													INDIR_WORD, INDIR_ENDLIST );

  hSetMIDIType	= 	GetIndirectFunctionHandle(fp,
													INDIR_WORD,
													INDIR_WORD, INDIR_ENDLIST );
  hOpenSnd	= 		GetIndirectFunctionHandle(fp,
													INDIR_WORD, INDIR_ENDLIST );
  hCloseSnd	= 		GetIndirectFunctionHandle(fp,
													INDIR_WORD, INDIR_ENDLIST );

#else
BOOL	bResult;

	hKernel32 = LoadLibrary("Kernel32.dll");
	pUTRegister = (PUTREGISTER)GetProcAddress(hKernel32, "UTRegister");
	pUTUnRegister = (PUTUNREGISTER) GetProcAddress( hKernel32, "UTUnRegister" );
	bResult = (*pUTRegister)(Main::hInstance,
								"scidllws.dll",
								NULL,
								"DLLProc",
								&pfnUTProc,
								NULL, NULL);
	if (!bResult)
		MBox("DLL not initialized","");
#endif
}

void EndThunk(void)
{
#ifndef WIN32S
	InvokeIndirectFunction(hWEP, DLL_WEP);
	FreeLibrary( hlib32 );
#else
	(*pUTUnRegister) (Main::hInstance);
	FreeLibrary(hKernel32);
#endif
}
