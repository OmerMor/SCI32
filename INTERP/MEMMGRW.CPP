//	memmgrw.cpp

#include "sciwin.hpp"
#include <i86.h>
#include <dos.h>
#include <string.h>
#include <conio.h>
#include "sol.hpp"
#include "config.hpp"
#include "memmgrw.hpp"
#include "mbox.hpp"
#include "neww.hpp"
#include "msg.hpp"
#include "newroom.hpp"
#include "shiftpal.hpp"
#include "sound.hpp"
#include "winthunk.hpp"

#ifndef WIN32S
#include <malloc.h>
#else
#include "wingfunc.hpp"
#endif


void	GetWindowsMemory(void);
int	GetMemSize(int limit);
int	PageLockableBytes(void);
int 	BaseMem(void);
int 	ExtendedMem(void);

#define	DPMI_INT								0x31
#define	WINGSIZE								400  * 1024

extern	uchar *wAudioBuffAddr;
extern	uchar *cursorBuffAddr;
extern	int	maxCursorSize;
extern	char far *midiCachePtr;
extern	void	FreeWinDlls(void);

int	sysMemAvail;
char	*sysMemAddr;
int	wAudioBuffSize;
int	newCacheSize;
int	cursorBuffSize;

#ifdef WIN32S
HLOCAL	hLocal;
HGLOBAL	hAudioBuff;
HGLOBAL	hCursorBuff;
static	DWORD		Args[5];
#endif


size_t
MemoryMgrWin::GetSysMemAvail()
{
	return sysMemAvail;
}

char *
MemoryMgrWin::GetSysMem(size_t theSize)
{
	if (theSize > sysMemAvail) {
		sprintf(mbuff,"asking:%d, have:%d",theSize,sysMemAvail);
		MBox(mbuff,"");
		exit(255);
	}
	return sysMemAddr;
}


MemoryMgrWin::~MemoryMgrWin()
{
#ifndef WIN32S
	free(sysMemAddr);
	free(wAudioBuffAddr);
	if (cursorBuffSize)
		free(cursorBuffAddr);
	FreeWinMem(midiCachePtr, PAGELOCK);

#else
//	FreeWinMem(sysMemAddr, LOCKMAIN);
	LocalUnlock(hLocal);
	LocalFree(hLocal);
	FreeWinMem(wAudioBuffAddr, LOCKAUDIO);
	if (cursorBuffSize)
		FreeWinMem(cursorBuffAddr, PAGELOCK);
#endif

#ifdef DEBUG
	if (configMgr->Get(configMgr->RoomMinK)) {
		sprintf(mbuff,"Min bytes required:%d in Room# %d\n",gameMaxAloc,gameMaxRoom);
		MBox(mbuff,"");
	}
#endif

	EndThunk();
	FreeWinDlls();
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

void	GetWindowsMemory(void)
{
int sciMin, sciMax;
int grossMemAvail, netMemAvail;
int reserveMem;
int wingMin;
int wingSize;
int physicalBytes;
int lockableBytes;
int useableBytes;
int totExternalBuffs;
DWORD		midiCacheSize;
char		errbuff[300];

	
	sysMemAddr		= NULL;
	wAudioBuffAddr	= NULL;
	midiCachePtr	= NULL;
	cursorBuffAddr	= NULL;

	// compute size in bytes of all required buffers
	wAudioBuffSize = DACSize;
	reserveMem		= reserveK * 1024;
	sciMin			= sciMinK * 1024;
	sciMax			= sciMaxK * 1024;
	newCacheSize	=	cacheSize1 * (sizeof(Bool) + sizeof(void *)) +
						 	cacheSize2 * (sizeof(Bool) + sizeof(void *)) +
						 	cacheSize3 * (sizeof(Bool) + sizeof(void *)) +
							reserveSize +
							cacheSize1 * (entrySize1 + HEADERSIZE) +
							cacheSize2 * (entrySize2 + HEADERSIZE) +
							cacheSize3 * (entrySize3 + HEADERSIZE) + 100;
	cursorBuffSize = 12 * maxCursorSize;
#ifndef WIN32S
	midiCacheSize = (NUMSOUNDNODES * sizeof(SNodeData) + 100);
	for (int i = 0; i < NUMMIDIBUFFS; i++)
		midiCacheSize += midiBuffSize[i];
						
	wingMin	= 0;
	wingSize	= 0;
#else
	midiCacheSize = 0;
	wingMin	= wingMinK * 1024;
	if (WING)
		wingSize	= WINGSIZE;
	else
		wingSize	= 0;
#endif

	totExternalBuffs = wAudioBuffSize
							+ newCacheSize
							+ midiCacheSize
							+ wingSize
							+ cursorBuffSize;

	// determine physical memory
	physicalBytes = (BaseMem() + ExtendedMem()) * 1024;

	// start with maximum for the memory manager
	netMemAvail = sciMax;

	// compute total required
	grossMemAvail = netMemAvail + totExternalBuffs;

	// how much is lockable?
	lockableBytes = PageLockableBytes();

	// deduct the reserve requirement
	useableBytes = lockableBytes - reserveMem;

	// don't try for more than useable
	if (grossMemAvail > useableBytes)
		grossMemAvail = useableBytes;

	// see how much we can actually allocate
	grossMemAvail = GetMemSize(grossMemAvail);

	// compute new net
	netMemAvail = grossMemAvail - totExternalBuffs;

#ifdef WIN32S
	if (WING) {
		// do we have enough memory to run WinG?
		if ((wingMin > netMemAvail) || (sciMin > (netMemAvail - WINGSIZE))) {
			WING = FALSE;
			netMemAvail += wingSize;
			wingSize = 0;
			if (SHOWMEM)
				MBox("Not enough memory for WING","");
		} else {
			// load WinG
			if (!WingInit(640,480)) {
				WING = FALSE;
				netMemAvail += wingSize;
				wingSize = 0;
				if (SHOWMEM)
					MBox("Could not load WING","");
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
		sprintf(errbuff, "physical:%10d"
							"\nlockable:%10d"
							"\n   reserve:%10d"
							"\nuseable:%10d"
							"\n\nused:"
							"\n--manager: %10d"
							"\n--new:          %10d"
							"\n--audio:        %10d"
							"\n--midi:          %10d"
							"\n--wing:            %10d"
							"\n--cursor:         %10d"
							"\n\ntot used:    %10d",
							physicalBytes,
							lockableBytes,
							reserveMem,
							useableBytes,
							netMemAvail,
							newCacheSize,
							wAudioBuffSize,
							midiCacheSize,							
							wingSize,
							cursorBuffSize,
								netMemAvail +
								newCacheSize +
								wAudioBuffSize +
								midiCacheSize +
								wingSize+
								cursorBuffSize
							);
		MBox(errbuff,"Memory");
	}

	if (sciMin > netMemAvail) {
		// If not enough memory, can't play.
		sprintf(errbuff, "You need %d more bytes of available memory to "
					"run under Windows.  If other applications are running, "
					"you might try terminating them.",
					sciMin - netMemAvail);
		MBox(errbuff,"");  
		exit(255);
	}

	// allocate for the MIDI cache
	if (midiCacheSize)
		midiCachePtr = (char far *)AllocWinMem(midiCacheSize, PAGELOCK);

	// allocate for the memory manager
#ifndef WIN32S
	sysMemAddr = (char*)_nmalloc(netMemAvail + 100);
	if (!sysMemAddr) {
		sprintf(mbuff,"main alloc error - size:%d", netMemAvail);
		MBox(mbuff,"");
		return;
	}
#else
	hLocal = LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, (UINT)netMemAvail);
	if (!hLocal) {
		MBox("main alloc error","");
		return;
	}
	sysMemAddr = (char *)LocalLock(hLocal);
	if (!sysMemAddr) {
		MBox("mem alloc error","");
		return;
	}
#endif

	// allocate for the "new" buffer
	if (newCacheSize) {
#ifndef WIN32S
		newCacheAddr = (void *)_nmalloc(newCacheSize);
		if (!newCacheAddr) {
			sprintf(mbuff,"cache alloc error - size:%d", newCacheSize);
			MBox(mbuff,"");
			return;
		}
#else
		newCacheAddr = (void *)AllocWinMem(newCacheSize, PAGELOCK);
#endif
		newState = NEW_MEM_CACHE_AVAIL;
	}

	// allocate the audio buffers
#ifndef WIN32S
	wAudioBuffAddr = (uchar*)_nmalloc(wAudioBuffSize);
#else
	wAudioBuffAddr = (uchar *)AllocWinMem(wAudioBuffSize, LOCKAUDIO);
#endif
	if (!wAudioBuffAddr) {
		sprintf(mbuff,"audio alloc error - size:%d", wAudioBuffSize);
		MBox(mbuff,"");
		return;
	}

	// allocate the cursor buffers (local or global?)
	if (cursorBuffSize) {
#ifndef WIN32S
		cursorBuffAddr = (uchar*)_nmalloc(cursorBuffSize);
#else
		cursorBuffAddr = (uchar *)AllocWinMem(cursorBuffSize, LOCKMAIN);
#endif
		if (!cursorBuffAddr) {
			sprintf(mbuff,"cursor alloc error - size:%d", cursorBuffSize);
			MBox(mbuff,"");
			return;
		}
	}

	sysMemAvail = netMemAvail;
}

int	GetMemSize(int limit)
{
int hiSize, loSize, currSize;
#ifndef WIN32S
char	*testAddr;
#else
HLOCAL	hTest;
#endif

#ifndef WIN32S
	testAddr = (char*)_nmalloc(limit);
	if (!testAddr) {
		hiSize = limit;
		loSize = 0;

		while ((hiSize - loSize) > 1000) {
			currSize = (hiSize + loSize) / 2;
			testAddr = (char *)_nmalloc(currSize);
			if (testAddr) {
				_nfree(testAddr);
				loSize = currSize;			
			} else {
				hiSize = currSize;
			}
		}
		_nheapmin();
		limit = loSize;
	} else {
	 	_nfree(testAddr);
	}

	_nheapmin();
#else
	// try to get it
	hTest = LocalAlloc(LMEM_MOVEABLE, (UINT)limit);
	if (!hTest) {
		// if couldn't get it, do binary search
		hiSize = limit;
		loSize = 0;

		while ((hiSize - loSize) > 1000) {
			currSize = (hiSize + loSize) / 2;
			hTest = LocalAlloc(LMEM_MOVEABLE, (UINT)currSize);
			if (hTest) {
				LocalFree(hTest);
				loSize = currSize;			
			} else {
				hiSize = currSize;
			}
		}
		limit = loSize;
	} else {
		LocalFree(hTest);
	}
#endif

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

int BaseMem(void)
{
unsigned char e1, e2;

	_disable();
	outp(0x70,0x16);
	e1 = inp(0x71);
	outp(0x70,0x15);
	e2 = inp(0x71);
	_enable();

	return (256 * e1) + e2;
}

int ExtendedMem(void)
{
unsigned char e1, e2;

	_disable();
	outp(0x70,0x18);
	e1 = inp(0x71);
	outp(0x70,0x17);
	e2 = inp(0x71);
	_enable();

	return (256 * e1) + e2;
}

