//	memmgrw.cpp

#include "sciwin.hpp"
#include <i86.h>
#include <dos.h>
#include <string.h>
#ifndef WIN32S
#include <malloc.h>
#endif

#include "sol.hpp"

#include "config.hpp"
#include "memmgrw.hpp"
#include "mbox.hpp"
#include "msg.hpp"
#include "newroom.hpp"
#include "shiftpal.hpp"
#include "sound.hpp"


#ifdef WIN32S
#include "wingfunc.hpp"
#include "scidllws.h"
#define	W32SUT_32
#include "w32sut.h"
extern	UT32PROC	pfnUTProc;
static	DWORD		Args[5];
#endif

void	UnloadDll(void);
int	GetMemSize(int limit);

#define	DPMI_INT								0x31
#define	MAXIMUM_SYSTEM_MEMORY			8192 * 1024
#define	WINGSIZE								400  * 1024

extern	uchar *wAudioBuffAddr;
int	wAudioBuffSize;
int	cursorBuffSize;

extern	uchar *cursorBuffAddr;
extern	int	maxCursorSize;

HLOCAL	hLocal;
#ifdef WIN32S
HGLOBAL	hAudioBuff;
HGLOBAL	hCursorBuff;
#endif

// Midi page locking variables
extern	DWORD		smallMidiSize;
extern	DWORD		largeMidiSize;
extern	DWORD		numSoundNodes;
extern	char far *lockedPagePtr;

// Special Kludge for AVI to allow us to free it's Alias assignments
void	TermMovie(void);
int	PageLockableBytes(void);


size_t
MemoryMgrWin::GetSysMemAvail()
{
int sciMin, sciMax, availMem;
int totMem, allocMem;
int reserveMem;
int reserveMem4;
int reserveMem8;
int reserveMem16;
int totExternalBuffs;
DWORD		lockedPageSize;
char		errbuff[200];
int lockableSize;

#ifdef WIN32S
int wingMin;
int maxSize;
#else
int hiSize, loSize, currSize;
#endif
	
	memPtrWin = NULL;
	wAudioBuffAddr = NULL;
	lockedPagePtr = NULL;
	cursorBuffAddr = NULL;

	wAudioBuffSize = DACSize;
	reserveMem = reserveK * 1024;
	reserveMem4 = reserveK4 * 1024;
	reserveMem8 = reserveK8 * 1024;
	reserveMem16 = reserveK16 * 1024;
	sciMin = sciMinK * 1024;
	sciMax = sciMaxK * 1024;
	lockedPageSize = 0;
	cursorBuffSize = 0;
//#ifdef WIN32S
	lockableSize = PageLockableBytes();
	if (lockableSize < (4 * 1024 * 1024))
		reserveMem = reserveMem4;
	else if (lockableSize < (8 * 1024 * 1024))
		reserveMem = reserveMem8;
	else 		  
		reserveMem = reserveMem16;
	if (SHOWMEM) {
		sprintf(mbuff,"detected:%d reserving:%d",lockableSize,reserveMem);
		MBox(mbuff,"");
	}
//#endif

	totExternalBuffs = wAudioBuffSize + reserveMem;
	if (COLORCURSOR) {
		cursorBuffSize = 12 * maxCursorSize;
		totExternalBuffs += cursorBuffSize;
	}

#ifndef WIN32S
//************************** NOT WIN32S ***********************************
	lockedPageSize = smallMidiSize + largeMidiSize + 
							(numSoundNodes * sizeof(SNodeData)) + 30;
	lockedPagePtr = (char far *)AllocWinMem(lockedPageSize, PAGELOCK);

	totMem = PageLockableBytes();

	// Now see if we can actually allocate this much.  If not,
	// find out how much we can get.
	memPtrWin = (char*)_nmalloc(totMem);
	if (!memPtrWin) {
		hiSize = totMem;
		loSize = 0;

		while ((hiSize - loSize) > 500) {
			currSize = (hiSize + loSize) / 2;
			memPtrWin = (char *)_nmalloc(currSize);
			if (memPtrWin) {
				_nfree(memPtrWin);
				loSize = currSize;			
			} else {
				hiSize = currSize;
			}
		}
		_nheapmin();
		totMem = loSize;
	} else {
	 	_nfree(memPtrWin);
	}

	_nheapmin();

#else
//************************** WIN32S ***********************************

	wingMin = wingMinK * 1024;

	// How much do we want for the memory manager?
	if (sciMax > MAXIMUM_SYSTEM_MEMORY)
		maxSize = MAXIMUM_SYSTEM_MEMORY;
	else
		maxSize = sciMax;

	// Add in the external buffers
	maxSize += totExternalBuffs;

	if (WING) 
		// Add in WinG size
		maxSize += WINGSIZE;

	// find out how much we can get up to the limit
	totMem = GetMemSize(maxSize);

#endif


	// Subtract off the external buffers
	availMem = totMem - totExternalBuffs;

	allocMem = availMem;

#ifdef WIN32S
	if (WING) {
		// do we have enough memory to run WinG?
		if (wingMin > allocMem) {
			WING = FALSE;
			if (SHOWMEM)
				MBox("Not enough memory for WING","");
		} else {
			// load WinG
			if (!WingInit(640,480)) {
				WING = FALSE;
				if (SHOWMEM)
					MBox("Could not load WING","");
			} else {
				if (SHOWMEM) {
					sprintf(mbuff, "total:%d\n\n-reserve:%d\n-audio:%d\n-midi:%d\n-cursor:%d\n\navailable:%d",
						totMem, reserveMem, wAudioBuffSize, lockedPageSize, cursorBuffSize, allocMem);
					MBox(mbuff,"Before WING");
				}
				// reduce avail memory by amount WinG takes
//				totMem = GetMemSize(totMem);
				totMem -= WINGSIZE;
				allocMem = totMem - totExternalBuffs;
			}
		}	
	}

	if (SHOWMEM) {
		if (WING)
			MBox("Using WinG","");
		else
			MBox("Not using WinG","");
	}
#endif

	if (SHOWMEM) {
		sprintf(mbuff, "total:%d\n\n-reserve:%d\n-audio:%d\n-midi:%d\n-cursor:%d\n\navailable:%d",
					totMem, reserveMem, wAudioBuffSize, lockedPageSize, cursorBuffSize, allocMem);
		MBox(mbuff,"Memory");
	}

	if (sciMin > allocMem) {
		// If not enough memory, can't play.
		sprintf(errbuff, "You need %d more bytes of available memory to "
					"run under Windows.  If other applications are running, "
					"you might try terminating them.",
					sciMin - allocMem);
		MBox(errbuff,"");  
		exit(255);
	}

	return allocMem;
}

char *
MemoryMgrWin::GetSysMem(size_t theSize)
{
	if (SHOWMEM) {
		sprintf(mbuff,"asking:%d",theSize);
		MBox(mbuff,"");
	}

#ifndef WIN32S
	// allocate for the memory manager
	memPtrWin = (char*)_nmalloc(theSize + 100);
	if (!memPtrWin) {
		sprintf(mbuff,"main alloc error - size:%d", theSize);
		MBox(mbuff,"");
	}

	// allocate the audio buffers
	wAudioBuffAddr = (uchar*)_nmalloc(wAudioBuffSize);
	if (!wAudioBuffAddr) {
		sprintf(mbuff,"audio alloc error - size:%d", wAudioBuffSize);
		MBox(mbuff,"");
	}
	// allocate the cursor buffers
	if (COLORCURSOR) {
		cursorBuffAddr = (uchar*)_nmalloc(cursorBuffSize);
		if (!cursorBuffAddr) {
			sprintf(mbuff,"cursor alloc error - size:%d", cursorBuffSize);
			MBox(mbuff,"");
		}
	}

#else
	// allocate for the memory manager
//	memPtrWin = (char *)AllocWinMem(theSize, LOCKMAIN);
	hLocal = LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, (UINT)theSize);
	if (!hLocal) {
		MBox("main alloc error","");
		return (char *)0;
	}
	memPtrWin = (char *)LocalLock(hLocal);
	if (!memPtrWin) {
		MBox("mem alloc error","");
		return (char *)0;
	}

	// allocate the audio buffers
	wAudioBuffAddr = (uchar *)AllocWinMem(wAudioBuffSize, LOCKAUDIO);
	if (!wAudioBuffAddr) {
		MBox("audio alloc error","");
		return (char *)0;
	}

	// allocate the cursor buffers (local or global?)
	if (COLORCURSOR) {
		cursorBuffAddr = (unsigned char *)AllocWinMem(cursorBuffSize, LOCKMAIN);
		if (!cursorBuffAddr) {
			MBox("cursor alloc error","");
			return (char *)0;
		}
	}

#endif

	return memPtrWin;
}


MemoryMgrWin::~MemoryMgrWin()
{
	TermMovie();	//free any AVI Aliases

#ifndef WIN32S
	free(memPtrWin);
	free(wAudioBuffAddr);
	if (COLORCURSOR)
		free(cursorBuffAddr);
	FreeWinMem(lockedPagePtr, PAGELOCK);

#else
//	FreeWinMem(memPtrWin, LOCKMAIN);
	LocalUnlock(hLocal);
	LocalFree(hLocal);
	FreeWinMem(wAudioBuffAddr, LOCKAUDIO);
	if (COLORCURSOR)
		FreeWinMem(cursorBuffAddr, PAGELOCK);
#endif
	UnloadDll();
#ifdef DEBUG
	if (configMgr->Get(configMgr->RoomMinK)) {
		sprintf(mbuff,"Min bytes required:%d in Room# %d\n",gameMaxAloc,gameMaxRoom);
		MBox(mbuff,"");
	}
#endif
}


size_t
MemoryMgrWin::GetConvMemAvail()
{
	return 0;
}

char *
MemoryMgrWin::GetConvMem(size_t bytes)
{
	size_t x = bytes;
	return NULL;
}

void
MemoryMgrWin::FreeConvMem(char *base)
{
	char *x = base;
}

int	GetMemSize(int limit)
{
int hiSize, loSize, currSize, lockableSize;
//HLOCAL hGlobal;

	// do DPMI call to get number of bytes which are lockable
	lockableSize = PageLockableBytes();

	// subtract 16k from this number; it's always too large
	lockableSize -= 16384;

	// only go after what was requested
	if (lockableSize < limit)
		limit = lockableSize;

	if (SHOWMEM) {
		sprintf(mbuff,"start size:%d",limit);
		MBox(mbuff,"");
	}

	// try to get it
//	hGlobal = GlobalAlloc(GMEM_FIXED, (UINT)limit);
//	hGlobal = GlobalAlloc(GMEM_MOVEABLE, (UINT)limit);
	hLocal = LocalAlloc(LMEM_MOVEABLE, (UINT)limit);
//	if (!hGlobal) {
	if (!hLocal) {
		// if couldn't get it, do binary search
		hiSize = limit;
		loSize = 0;

		while ((hiSize - loSize) > 1000) {
			currSize = (hiSize + loSize) / 2;
//			hGlobal = GlobalAlloc(GMEM_FIXED, (UINT)currSize);
//			hGlobal = GlobalAlloc(GMEM_MOVEABLE, (UINT)currSize);
			hLocal = LocalAlloc(LMEM_MOVEABLE, (UINT)currSize);
//			if (hGlobal) {
			if (hLocal) {
//				GlobalFree(hGlobal);
				LocalFree(hLocal);
				loSize = currSize;			
			} else {
				hiSize = currSize;
			}
		}
		limit = loSize;
	} else {
//		GlobalFree(hGlobal);
		LocalFree(hLocal);
	}

	if (SHOWMEM) {
		sprintf(mbuff,"end size:%d",limit);
		MBox(mbuff,"");
	}
	return limit;
}

int	PageLockableBytes(void)
{
#ifndef WIN32S

//    This routine gets information
//    about free memory with DPMI call 0x0500 using Windows
//    as a DPMI host.  Note that only the first field of the
//    structure is guaranteed to contain a valid value; any
//    field that is not returned by the DPMI implementation
//    is set to -1 (0FFFFFFFFh).

_Packed struct meminfo {
    unsigned LargestBlockAvail;
    unsigned MaxUnlockedPage;
    unsigned LargestLockablePage;
    unsigned LinAddrSpace;
    unsigned NumFreePagesAvail;
    unsigned NumPhysicalPagesFree;
    unsigned TotalPhysicalPages;
    unsigned FreeLinAddrSpace;
    unsigned SizeOfPageFile;
    unsigned Reserved[3];
} MemInfoW;

union REGS regs;
DWORD mi_16;
unsigned a;
_Packed struct SREGS sregs;


	memset(&regs, 0, sizeof(union REGS));
	memset(&sregs, 0, sizeof(struct SREGS));
	regs.w.ax = 0x0500;
	mi_16 = AllocAlias16( &MemInfoW );
	sregs.es = HIWORD( mi_16 );
	regs.x.di = LOWORD( mi_16 );

	int86x( DPMI_INT, &regs, &regs, &sregs );
	FreeAlias16(mi_16);

	// to avoid compile warnings
	a = MemInfoW.LargestBlockAvail;
	a = MemInfoW.MaxUnlockedPage;
	a = MemInfoW.LargestLockablePage;
	a = MemInfoW.LinAddrSpace;
	a = MemInfoW.NumFreePagesAvail;
	a = MemInfoW.NumPhysicalPagesFree;
	a = MemInfoW.TotalPhysicalPages;
	a = MemInfoW.FreeLinAddrSpace;
	a = MemInfoW.SizeOfPageFile;
	a = MemInfoW.Reserved[0];

	return MemInfoW.LargestLockablePage * 4096;

#else

	// I would like to call GlobalMemoryStatus here, but it does not seem
	// to work under WIN32S; i.e. it does not return any information.

	return (int)((* pfnUTProc)(Args, (DWORD)DLL_LOCKABLE_PAGES, NULL) * 4096);
#endif
}

#ifndef WIN32S
void far * AllocWinMem(int size, int pageLock)
{
HGLOBAL	hGlobal;
LPVOID	lpMem16;

	hGlobal = GlobalAlloc(GPTR, size);
	if (!hGlobal)
		MBox("win alloc error","");
	lpMem16 = (LPVOID)GlobalLock(hGlobal);
	if (!lpMem16)
		MBox("win lock error","");

	if (pageLock == PAGELOCK) {
		if (!GlobalPageLock(SELECTOROF(lpMem16)))
			MBox("win page lock error","");
	}

	return ((void far *)MK_FP32(lpMem16));
}

void FreeWinMem(void far *lpMem, int pageLock)
{
HGLOBAL	hGlobal;
LPVOID	lpMem16;

	if (lpMem == NULL)
		return;
	lpMem16 = (LPVOID)MK_FP16((void far *)lpMem);
	if (pageLock == PAGELOCK)
		GlobalPageUnlock(SELECTOROF(lpMem16));
	hGlobal = (HGLOBAL)GlobalHandle(SELECTOROF(lpMem16));
	GlobalUnlock(hGlobal);
	if (GlobalFree(hGlobal))
		MBox("win free error","");
}
#else
void * AllocWinMem(int size, int pageLock)
{
	Args[0] = (DWORD)(size);
	Args[1] = (DWORD)(pageLock);
	return (void *)(* pfnUTProc)(Args, (DWORD)DLL_ALLOC_MEM, NULL);
}

void FreeWinMem(void *lpMem, int pageLock)
{
	if (lpMem == NULL)
		return;
	Args[0] = (DWORD)lpMem;
	Args[1] = (DWORD)(pageLock);
	(* pfnUTProc)(Args, (DWORD)DLL_FREE_MEM, NULL);
}
#endif

