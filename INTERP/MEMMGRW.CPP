//	memmgrw.cpp

#include "sciwin.hpp"
#include <i86.h>
#include <dos.h>
#include <string.h>
#ifndef WIN32S
#include <malloc.h>
#endif

#include "sol.hpp"

#include "memmgrw.hpp"
#include "mbox.hpp"
#include "msg.hpp"
#include "newroom.hpp"
#include "sound.hpp"


#ifdef WIN32S
#include "scidllws.h"
#define	W32SUT_32
#include "w32sut.h"
extern	UT32PROC	pfnUTProc;
static	DWORD			Args[5];
static	PVOID			TransList[5];
#endif

void UnloadDll(void);

#define DPMI_INT	0x31
#define	MAXIMUM_SYSTEM_MEMORY			8192* 1024

extern	uchar *wAudioBuffAddr;
int	wAudioBuffSize;

#ifdef COLORCUR
extern	uchar *cursorBuffAddr;
extern	int	maxCursorSize;
int				cursorBuffSize;
#endif

#ifdef WIN32S
HLOCAL	hMem;
HGLOBAL	hAudioBuff;
#ifdef COLORCUR
HGLOBAL	hCursorBuff;
#endif
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
int hiSize, loSize, currSize;
int reserveMem, totMem, allocMem;
DWORD		lockedPageSize;
char		errbuff[200];

#ifdef WIN32S
int maxSize;
HLOCAL hGlobal;
#endif

	wAudioBuffSize = DACSize;
	reserveMem = reserveK * 1024;
	sciMin = sciMinK * 1024;
	sciMax = sciMaxK * 1024;
	lockedPageSize = 0;

#ifndef WIN32S
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

	maxSize = MAXIMUM_SYSTEM_MEMORY + wAudioBuffSize + reserveMem;

	totMem = PageLockableBytes();
	if (totMem > maxSize)
		totMem = maxSize;

	// Now see if we can actually allocate this much.  If not,
	// find out how much we can get.
	hGlobal = GlobalAlloc(GMEM_FIXED, (UINT)totMem);
	if (!hGlobal) {
		hiSize = totMem;
		loSize = 0;

		while ((hiSize - loSize) > 1000) {
			currSize = (hiSize + loSize) / 2;
			hGlobal = GlobalAlloc(GMEM_FIXED, (UINT)currSize);
			if (hGlobal) {
				GlobalFree(hGlobal);
				loSize = currSize;			
			} else {
				hiSize = currSize;
			}
		}
		totMem = loSize;
	} else {
		GlobalFree(hGlobal);
	}

#endif


	// Subtract off reserve and audio buffer
	availMem = totMem - reserveMem - wAudioBuffSize;
#ifdef COLORCUR
	cursorBuffSize = 12 * maxCursorSize;
	availMem -= (cursorBuffSize);
#endif

	// Get what we can up to sciMax.  After that, split the difference.
	if (sciMax > availMem)
		allocMem = availMem;
	else
		allocMem = sciMax + ((availMem - sciMax) / 2);

	if (SHOWMEM) {
#ifndef COLORCUR
		sprintf(mbuff, "alloc:%d\ntot:  %d\nres:  %d\naud:  %d\npage lock:  %d",
					allocMem, totMem, reserveMem, wAudioBuffSize, lockedPageSize);
#else
		sprintf(mbuff, "alloc:%d\ntot:  %d\nres:  %d\naud:  %d\ncur:  %d\npage lock:  %d",
					allocMem, totMem, reserveMem, wAudioBuffSize, cursorBuffSize, lockedPageSize);
#endif
		MBox(mbuff,"Memory");
	}

	if (sciMin > availMem) {
		// If not enough memory, can't play.
		sprintf(errbuff, "You need %d more bytes of available memory to "
					"run under Windows.  If other applications are running, "
					"you might try terminating them.",
					sciMin - availMem);
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
#ifdef COLORCUR
	// allocate the cursor buffers
	cursorBuffAddr = (uchar*)_nmalloc(cursorBuffSize);
	if (!cursorBuffAddr) {
		sprintf(mbuff,"cursor alloc error - size:%d", cursorBuffSize);
		MBox(mbuff,"");
	}
#endif

#else
	// allocate for the memory manager
	memPtrWin = (char *)AllocWinMem(theSize, LOCKMAIN);
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

#ifdef COLORCUR
	// allocate the cursor buffers (local or global?)
	cursorBuffAddr = (char *)AllocWinMem(cursorBuffSize, LOCKMAIN);
	if (!cursorBuffAddr) {
		MBox("cursor alloc error","");
		return (char *)0;
	}
#endif

#endif

	return memPtrWin;
}


MemoryMgrWin::~MemoryMgrWin()
{
	TermMovie();	//free any AVI Aliases

#ifndef WIN32S
	free(memPtrWin);
	free(wAudioBuffAddr);
#ifdef COLORCUR
	free(cursorBuffAddr);
#endif
	FreeWinMem(lockedPagePtr, PAGELOCK);

#else
	FreeWinMem(memPtrWin, PAGELOCK);
	FreeWinMem(wAudioBuffAddr, PAGELOCK);
#ifdef COLORCUR
	FreeWinMem(cursorBuffAddr, PAGELOCK);
#endif
#endif
	UnloadDll();
#ifdef DEBUG
	sprintf(mbuff,"Min bytes required:%d in Room# %d\n",gameMaxAloc,gameMaxRoom);
	MBox(mbuff,"");
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
	Args[0] = (DWORD)lpMem;
	Args[1] = (DWORD)(pageLock);
	Args[1] = (DWORD)(0);
	TransList[0] = &Args[0];
	TransList[1] = 0;
	(* pfnUTProc)(Args, (DWORD)DLL_FREE_MEM, TransList);
}
#endif

