//	memmgrd.cpp

#include <i86.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "sol.hpp"

#include "config.hpp"
#include "memmgrd.hpp"
#include "newroom.hpp"

short	convSel[MAX_CONV_ALLOCS];
short	convSeg[MAX_CONV_ALLOCS];
int	convAllocs = 0;

size_t
MemoryMgrDOS::GetSysMemAvail()
{
	_Packed struct memInfo {
		unsigned	largestBlockAvail;
		unsigned	other[11];
	} info;
	
	REGS regs;
	SREGS sregs;
	
	regs.x.eax = 0x500;
	memset(&sregs,0,sizeof sregs);
	sregs.es = FP_SEG(&info);
	regs.x.edi = FP_OFF(&info);
	int386x(0x31,&regs,&regs,&sregs);
	
	//	make WATCOM 9.5 not complain about unreferenced member
	*info.other = 1;

	return info.largestBlockAvail;
}

char *
MemoryMgrDOS::GetSysMem(size_t theSize)
{
	memPtrDOS = (char*) malloc(theSize);
	return memPtrDOS;
}

size_t
MemoryMgrDOS::GetConvMemAvail()
{
	REGS regs;
	
	int nHi = 0xffff;
	int nLo = 0;
	int n = nLo + (nHi - nLo)/2;
	memset(&regs,0,sizeof regs);
	do {
		regs.x.eax = 0x100;
		regs.x.ebx = n;
		int386(0x31,&regs,&regs);
		if (!regs.x.cflag) {
			regs.w.ax = 0x101;
			int386(0x31,&regs,&regs);
			if (nHi - nLo <= 4096)
				break;
			nLo = n;
		} else {
			nHi = n;
		}
		n = nLo + (nHi - nLo)/2;
	} while (n != nLo);
	return n * 16;
}

char *
MemoryMgrDOS::GetConvMem(size_t bytes)
{
	REGS regs;
	
	if (convAllocs == MAX_CONV_ALLOCS)
		return NULL;
	regs.w.ax = 0x100;
	regs.w.bx = (bytes + 15) / 16;
	int386(0x31,&regs,&regs);
	if (regs.x.cflag)
		return NULL;
	convSel[convAllocs] = regs.w.dx;
	convSeg[convAllocs] = regs.w.ax;
	convAllocs++;
	return (char *)(regs.w.ax * 16);
}

void
MemoryMgrDOS::FreeConvMem(char *base)
{
	REGS regs;
	int i, seg;
	
	seg = (int)base / 16;
	for (i = 0; i < convAllocs; i++)
		if (convSeg[i] == seg)
			break;
	if (i == convAllocs)
		return;
	regs.w.ax = 0x101;
	regs.w.dx = convSel[i];
	int386(0x31,&regs,&regs);
}

MemoryMgrDOS::~MemoryMgrDOS()
{
#ifdef DEBUG
	size_t highWaterMark = 0;
	_heapinfo heapInfo;
	heapInfo._pentry = 0;
	while (_heapwalk(&heapInfo) == _HEAPOK)
		//	ignore the allocation for the memory manager
		if (heapInfo._size < 1024L * 500)
			highWaterMark += heapInfo._size;

	char buf[100];
	sprintf(buf, "Heap high water mark: %u bytes\n", highWaterMark);
	msgMgr->AddFatalStr(buf);
	if (configMgr->Get(configMgr->RoomMinK)) {
		sprintf(buf,"Min bytes required:%d in Room# %d\n",gameMaxAloc,gameMaxRoom);
		msgMgr->AddFatalStr(buf);
	}
#endif

	for (int i = 0; i < convAllocs; i++) {
		REGS regs;
		regs.w.ax = 0x101;
		regs.w.dx = convSel[i];
		int386(0x31,&regs,&regs);
	}
	free(memPtrDOS);
}
