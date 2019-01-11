//	memmgrd.hpp

#ifndef MEMMGRD_HPP
#define MEMMGRD_HPP

#ifndef MEMMGR_HPP
#include "memmgr.hpp"
#endif

#define	MAX_CONV_ALLOCS	12

struct MemoryMgrDOS : MemoryMgr {
	MemoryMgrDOS() : memPtrDOS(0) {}

	size_t	GetSysMemAvail();
	char *	GetSysMem(size_t);
	size_t	GetConvMemAvail();
	char*		GetConvMem(size_t);
	void		FreeConvMem(char *base);

	~MemoryMgrDOS();

	protected:
	char *	memPtrDOS;
};

#endif
