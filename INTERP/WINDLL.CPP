// windll.cpp
//     Load and call a Windows 16-bit or 32-bit DLL.

#include "sciwin.hpp"
#include "sol.hpp"

#include	"kernel.hpp"
#include "pmachine.hpp"
#include "textid.hpp"
#include "mbox.hpp"

BOOL FindDll(LPSTR dllName);
BOOL FindSlot(void);

enum {
	WINDLL_LOAD,
	WINDLL_FREE,
	WINDLL_CALL
};

#define	MAXDLLS	10
#define	MAXNAME	30

static	char		tblName[MAXDLLS][MAXNAME];
static	HANDLE	tblHandle[MAXDLLS];
static	HINDIR	tblEntryPoint[MAXDLLS];
static	int		tblDllType[MAXDLLS];
static	BOOL		firstTime = TRUE;
static	int		currDll;


#pragma warning 579  9         //   Shut off cast warning for FP_OFF

void
KWinDLL(argList)
{
TextID	dllName;
MemID		dataBuff;
void far *dataPtr;
HANDLE	hLib;
FARPROC	fp;
HINDIR	hEntryPoint;
int		retVal, dllType;

	// initialize the DLL table if first call.
	if (firstTime) {
		firstTime = FALSE;
		for (currDll = 0; currDll < MAXDLLS; currDll++)
			strcpy(tblName[currDll],"");
			tblHandle[currDll] = NULL;
	}

	// get the DLL name and check its length
	dllName = (TextID)arg(2);
	dllName.Lock();
	if (lstrlen(*dllName) > MAXNAME) {
		sprintf(mbuff,"DLL name too long:%d\n%s",lstrlen(*dllName),*dllName);
		MBox(mbuff,"");
		dllName.Unlock();
		pm.acc = 0;
		return;
	}
	// default return to failure
	retVal = 0;

	switch (arg(1)) {

		case WINDLL_LOAD:
			// if already loaded, get out
			if (FindDll(*dllName)) {
				retVal = HINSTANCE_ERROR + 2;
				break;
			}

			// find an empty table entry
			if (!FindSlot()) {
				sprintf(mbuff,"DLL table overflow:%s",*dllName);
				MBox(mbuff,"");
				break;
			}

			// load the DLL
			hLib = LoadLibrary(*dllName);
			if (hLib < HINSTANCE_ERROR) {
				retVal = hLib;
				break;
			}

			// get the entry point address - try 16-bit DLL first
			fp = GetProcAddress( hLib, "SciDllEntry" );
			if (fp != NULL)
				dllType = 16;
			else {
				fp = GetProcAddress( hLib, "Win386LibEntry" );
				if (fp == NULL) {
					retVal = HINSTANCE_ERROR + 1;
					FreeLibrary(hLib);
					break;
				} else
					dllType = 32;
			}
			if (dllType == 32)
				hEntryPoint = GetIndirectFunctionHandle(fp, INDIR_DWORD,
																		  INDIR_WORD,
																		  INDIR_WORD,
																		  INDIR_ENDLIST);
			else
				hEntryPoint = GetIndirectFunctionHandle(fp, INDIR_PTR,
																		  INDIR_ENDLIST);
			if (hEntryPoint == NULL) {
				FreeLibrary(hLib);
				retVal = HINSTANCE_ERROR + 1;
				break;
			}

			// success; enter info in table
			strcpy(tblName[currDll],*dllName);
			tblHandle[currDll]		= hLib;
			tblEntryPoint[currDll]	= hEntryPoint;
			tblDllType[currDll] 		= dllType;
			retVal = HINSTANCE_ERROR + 2;
			break;

		case WINDLL_FREE:
			if (!FindDll(*dllName))
				break;
			FreeLibrary(tblHandle[currDll]);
			strcpy(tblName[currDll],"");
			tblHandle[currDll] = NULL;
			retVal = 1;
			break;

		case WINDLL_CALL:
			if (!FindDll(*dllName))
				break;
			// lock down the data buffer
			dataBuff = (MemID)arg(3);
			dataBuff.Lock();
			// get the buffer pointer
			dataPtr = (void far *)(*dataBuff);
			// call the DLL
			if (tblDllType[currDll] == 32)
				retVal = InvokeIndirectFunction(tblEntryPoint[currDll],
															(DWORD)FP_OFF(dataPtr),
															(WORD)FP_SEG(dataPtr),
															(WORD)1);
			else
				retVal = (DWORD)InvokeIndirectFunction(tblEntryPoint[currDll],
															*dataBuff);
			dataBuff.Unlock();
			break;

		default:
			break;
	}

	dllName.Unlock();
	pm.acc = retVal;
	return;
}
#pragma warning 579  4         //   Turn on cast warning

// This function is called when the interpreter terminates to free up
// any DLLs which the application forgot about.
void
FreeWinDlls(void)
{
	for (currDll = 0; currDll < MAXDLLS; currDll++) {
		if (tblHandle[currDll] != NULL)
			FreeLibrary(tblHandle[currDll]);
	}
}

// Find the requested DLL in the table.
BOOL
FindDll(LPSTR dllName)
{
	for (currDll = 0; currDll < MAXDLLS; currDll++) {
		if (!lstrcmpi(dllName,tblName[currDll]))
			return TRUE;
	}
	return FALSE;
}

// Find an empty table slot.
BOOL
FindSlot(void)
{
	for (currDll = 0; currDll < MAXDLLS; currDll++) {
		if (tblHandle[currDll] == NULL)
			return TRUE;
	}
	return FALSE;
}
