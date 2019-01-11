//	memmgrw.hpp

#ifndef MEMMGRW_HPP
#define MEMMGRW_HPP

#ifndef MEMMGR_HPP
#include "memmgr.hpp"
#endif

#define	NOPAGELOCK	0
#define	PAGELOCK		1

#ifndef WIN32S
void far * AllocWinMem(int size, int pageLock);
void FreeWinMem(void far *lpMem, int pageLock);
#else
void * AllocWinMem(int size, int pageLock);
void FreeWinMem(void *lpMem, int pageLock);
#endif


struct MemoryMgrWin : MemoryMgr {
	MemoryMgrWin() : memPtrWin(0) {}
	~MemoryMgrWin();
	size_t	GetSysMemAvail();
	char *	GetSysMem(size_t);
	size_t	GetConvMemAvail();
	char*		GetConvMem(size_t);
	void		FreeConvMem(char *base);

	protected:
	char *	memPtrWin;
	unsigned totLockSize;
	short		linAddrHi;
	short		linAddrLo;
};

#endif
