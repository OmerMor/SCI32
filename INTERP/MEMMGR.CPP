/****************************************************************************
			 32 bit flat memory model SCI interpreter

	Module:  memmgr.cpp

	Author:  Larry Scott

-----------------------------------------------------------------------------

  Memory management object.  This object manages a memory area and includes
  the capabilities:
	1) memory allocation
		a) moveable memory allocation is gotten at the low end of memory
		b) unmoveable memory allocation is gotten at the high end of memory
	2) memory releasing
	3) memory purging
	4) memory packing
	5) handle management
		a) handles are allocated dynamically
	6) memory can be made unmoveable after allocation
		but the memory will be moved high
	7) Realloc allows for changing the size of an allocation.
		If the size is larger a move of the memory may happen.
*****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "sol.hpp"

#include "config.hpp"
#ifdef DEBUG
#include "debug.hpp"
#endif
#include "kernel.hpp"
#include "memmgr.hpp"
#include "newroom.hpp"
#include "pmachine.hpp"
#include "savegame.hpp"

#define	MAXIMUM_SYSTEM_MEMORY			8192* 1024
#define	MINIMUM_SYSTEM_MEMORY			500 * 1024
#define	EXCLUDE_SYSTEM_MEMORY_INIT		40
#define	EXCLUDE_SYSTEM_MEMORY_INCR		4 * 1024
#define	EXCLUDE_CONVENTIONAL_MEMORY	200 * 1024

// Count of the initial number of descriptors
#define NUMDESCRIPTORS 2000

// Descriptors memory allocation attributes
#define HANDLESATTRIBUTES HANDLEMEMORY+MOVEABLE

#define MAXLOCKCOUNT 255				// the maximum lock allowed on a memory resource

class SaveDescriptor
{
	MemInfo        memInfo;    // Information about the block of memory and the handle
	size_t			size;       // Size in bytes of memory allocation
	char           initChar;   // If initialization character it's stored here
	uchar  			lockCount;	// The count of locks

public:
	SaveDescriptor& operator=(const Descriptor& theDesc) {
		memInfo = theDesc.memInfo;
		size = theDesc.size;
		initChar = 	theDesc.initChar;
		lockCount = 	theDesc.lockCount;
		return *this;
		}
	friend class Descriptor;
	friend class MemoryMgr;
};

//****************************************************************************
MemoryMgr::MemoryMgr()
{
	//	MemInfo depends on knowing the sizeof a MemType for its union to work
	//	Even this depends on a char being 8 bits
	//	The "volatile" stuff is an attempt to prevent the compiler from
	//	complaining that this test is always True
	volatile size_t i = sizeof(MemType);
	assert(i == 1);
	i = sizeof(Descriptor);
	assert(i == 32);
	i = SIGNATURESIZE & 3;
	assert(i == 0);
	totalMemory = 0;
}

void
MemoryMgr::Init(size_t maxSize)
{

#ifdef DEBUG
	moveToFront = True;
	strcpy(signature,"END OF MEMORY BLOCK");
	checkVolatile = 0;
#endif
	blackHoleAddr = NULL;

	int sysMem = GetSysMemAvail();
	if (sysMem < (totalMemory = configMgr->GetNum("minMemory", 0, sysMem)))
		exit(printf("Insufficient memory; %d bytes required\n",totalMemory));

 	if ((totalMemory = configMgr->GetNum("staticMemory",0))) {
		if (sysMem < totalMemory)
			exit(printf("Insufficient memory; %d bytes required\n",totalMemory));
	 	sysMem -= totalMemory;
	}

 	sysMem -= EXCLUDE_SYSTEM_MEMORY_INIT;
	if (!maxSize || maxSize > sysMem)
		maxSize = sysMem;
	if (maxSize > MAXIMUM_SYSTEM_MEMORY)
		maxSize = MAXIMUM_SYSTEM_MEMORY;

  	// Get system memory
	maxSize &= 0xfffffff0;
	while (!(memoryBase = GetSysMem(maxSize)))
		maxSize -= EXCLUDE_SYSTEM_MEMORY_INCR;
	totalMemory = maxSize;
	int t = totalMemory & 3;
	assert(t == 0);
	t = (int)memoryBase & 3;
	assert(t == 0);

	// Get conventional memory
	char* convBase = 0;
/*
	maxSize = GetConvMemAvail();
	if (maxSize > EXCLUDE_CONVENTIONAL_MEMORY) {
		maxSize -= EXCLUDE_CONVENTIONAL_MEMORY;
		convBase = GetConvMem(maxSize);	// (success tested later in this routine)
	}
*/

  	// Set up the Descriptors

	mruList = mruListEnd = freeList = freeListEnd = ordList = ordListEnd = 0;

	maxDescriptors = NUMDESCRIPTORS;
	needMoreDescs = False;
	size_t theDescriptorsSize = maxDescriptors * sizeof(Descriptor) + SIGNATURESIZE;
	size_t ii = theDescriptorsSize & 3;
	assert (ii == 0);

  	char* DescriptorsAddr = memoryBase;
	desc = new(DescriptorsAddr) Descriptor;
	for (int i = 1; i < maxDescriptors; ++i)
		new(DescriptorsAddr + i*sizeof(Descriptor)) Descriptor;

	descID = GetNewDescriptor(DescriptorTableHandle);
	desc[descID].size = theDescriptorsSize;
	desc[descID].data = DescriptorsAddr;
	desc[descID].memInfo.attributes = HANDLESATTRIBUTES;
	desc[descID].memInfo.type = MemDescriptors;
#ifdef DEBUG
	WriteSignature(descID);
#endif
	AddBefore(descID,mruList,&mruList,&mruListEnd,mruList,False);

	// Set up the freeList
	firstDesc	= RESERVEDHANDLES;
	SOL_Handle freeID = GetNewDescriptor();
	desc[freeID].size = totalMemory - theDescriptorsSize;
	desc[freeID].data = memoryBase + theDescriptorsSize;
	desc[freeID].memInfo.bits.freeMemory = True;
	desc[freeID].memInfo.bits.moveable = True;
	AddBefore(freeID,freeList,&freeList,&freeListEnd,mruList,False);
#ifdef DEBUG
//	memset(desc[freeID].data,FREE_MARK_CHAR,desc[freeID].size);
#endif

convBase = 0;
	if (convBase) {

		// adjust memory limits
		int gap = memoryBase - (convBase + maxSize); // unusable system memory
		totalMemory += maxSize + gap;
		memoryBase = convBase;

		// flag the region above conventional memory and below system memory
		// as unusable
		SOL_Handle id = GetNewDescriptor();
		desc[id].size = gap;
		desc[id].data = memoryBase + maxSize;
		desc[id].memInfo.bits.blackHole = True;
		AddBefore(id,mruList,&mruList,&mruListEnd,mruList,True);

		// add conventional memory allocated earlier to the free list
		id = GetNewDescriptor();
		desc[id].size = maxSize;
		desc[id].data = memoryBase;
		desc[id].memInfo.bits.freeMemory = True;
		AddBefore(id,freeList,&freeList,&freeListEnd,mruList,True);
	}
#ifdef NOT_SHIPPING_VERSION
	// Create a thrash array
	int size = ThrashListSize*sizeof(ThrashItem);
	SOL_Handle thrashArrayHandle = Alloc(THRASHATTRIBUTES,size,0,0,0,ThrashArrayHandle);
	thrashArray = (ThrashItem*) GetAddr(thrashArrayHandle);
#endif
}

//****************************************************************************
SOL_Handle
MemoryMgr::GetNewDescriptor(ReservedHandle requestHandle)
{
	int	i;

	if (requestHandle != IllegalHandle) {

		if (!desc[requestHandle].memInfo.bits.freeDescriptor)
			msgMgr->Fatal(SrcLoc,"Reserved handle %d unavailable",requestHandle);
		desc[requestHandle].memInfo.bits.freeDescriptor = False;
		desc[requestHandle].memInfo.bits.freeMemory = False;
		i = requestHandle;

	} else {

		// search for an available handle.
		// handle = 0 is not used.
		for (i=firstDesc; i<maxDescriptors; ++i)	{
			if(desc[i].memInfo.bits.freeDescriptor)
				break;
		}
		if (i == maxDescriptors)
			msgMgr->Fatal(SrcLoc,Msg_NoHandles,maxDescriptors);

		// we have found an available handle
		desc[i].memInfo.bits.freeDescriptor = False;
		desc[i].memInfo.bits.freeMemory = False;
		if (i == maxDescriptors - 3)
			needMoreDescs = True;
		firstDesc = i + 1;
	}

	desc[i].prevO = -1;
	desc[i].nextO = -1;

	return (SOL_Handle) i;
}

//****************************************************************************
void
MemoryMgr::GetMoreDescriptors(unsigned more)
{
		needMoreDescs = False;
		// Make another larger allocation for the handles (10% more)
   	// and copy the current handles to it.
		int newMaxDescriptors = maxDescriptors + more;
		size_t theDescriptorsSize = newMaxDescriptors*sizeof(Descriptor);
		Realloc(descID,theDescriptorsSize);
   	// Setup new handles
		for (int i = maxDescriptors;i < newMaxDescriptors; ++i)
			new(desc[descID].data + i*sizeof(Descriptor)) Descriptor;
		maxDescriptors = newMaxDescriptors;
}

//****************************************************************************
void
MemoryMgr::FreeDescriptor(SOL_Handle theID)
{
	MemInfo info;

	desc[theID].memInfo = info;
	if ((theID < firstDesc) && (theID >= RESERVEDHANDLES))
		firstDesc = theID;

	// remove node from ordered list

	if (desc[theID].nextO != -1) {
		if (desc[theID].nextO)
			desc[desc[theID].nextO].prevO = desc[theID].prevO;
		else
			ordListEnd = desc[theID].prevO;
		if (desc[theID].prevO)
			desc[desc[theID].prevO].nextO = desc[theID].nextO;
		else
			ordList = desc[theID].nextO;
	}
}

//****************************************************************************
void
MemoryMgr::AddBefore(SOL_Handle	newID,
							SOL_Handle	nextID,
							SOL_Handle*	theList,
							SOL_Handle*	theListEnd,
							SOL_Handle	refID,
							Bool			beforeRefID)
{
	SOL_Handle prevID;

	if (nextID)
		prevID = desc[nextID].prev;
	else
		prevID = 0;
	desc[newID].prev = prevID;
	desc[newID].next = nextID;
	if (prevID)
		desc[prevID].next = newID;
	else
		*theList = newID;
	if (nextID)
		desc[nextID].prev = newID;
	else
		*theListEnd = newID;

	// now add newID to ordered list

	if (!ordList) {
		ordList = newID;
		desc[newID].prevO = 0;
		desc[newID].nextO = 0;
		return;
	} else if (beforeRefID) {
		desc[newID].prevO = desc[refID].prevO;
		desc[newID].nextO = refID;
		desc[refID].prevO = newID;
		if (desc[newID].prevO)
			desc[desc[newID].prevO].nextO = newID;
		else
			ordList = newID;
	} else {
		desc[newID].prevO = refID;
		desc[newID].nextO = desc[refID].nextO;
		desc[refID].nextO = newID;
		if (desc[newID].nextO)
			desc[desc[newID].nextO].prevO = newID;
		else
			ordListEnd = newID;
	}
}

void
MemoryMgr::SwapOrderedLinks(SOL_Handle id1, SOL_Handle id2)
{
	if (id2 == desc[id1].nextO) {
	// id2 == n1 make it id1 == n2
	SOL_Handle tmp = id2;
	id2 = id1;
	id1 = tmp;
	}

	SOL_Handle p1 = desc[id1].prevO;
	SOL_Handle n1 = desc[id1].nextO;
	SOL_Handle p2 = desc[id2].prevO;
	SOL_Handle n2 = desc[id2].nextO;

	if (id1 != n2) {
		desc[id1].prevO = p2;
		desc[id1].nextO = n2;
		desc[id2].prevO = p1;
		desc[id2].nextO = n1;

		if (p1) {
			desc[p1].nextO = id2;
			if (p2)
				desc[p2].nextO = id1;
			else
				ordList = id1;
		}
		else {
			ordList = id2;
			desc[p2].nextO = id1;
		}

		if (n1) {
			desc[n1].prevO = id2;
			if (n2)
				desc[n2].prevO = id1;
			else
				ordListEnd = id1;
		}
		else {
			ordListEnd = id2;
			desc[n2].prevO = id1;
		}
	} 
	else {
		// id1 == n2
		desc[id1].prevO = p2;
		desc[id1].nextO = id2;
		desc[id2].prevO = id1;
		desc[id2].nextO = n1;

		if (p2)
			desc[p2].nextO = id1;
		else
			ordList = id1;

		if (n1)
			desc[n1].prevO = id2;
		else
			ordListEnd = id2;
	}
//// For speed, don't worry about assigning links to descriptor #0
//
//	if (desc[id2].data < desc[id1].data) {
//		SOL_Handle id = id1;
//		id1 = id2;
//		id2 = id;
//	}
//
//// At this point, id1 preceeds id2 in the order list
//
//	SOL_Handle p1 = desc[id1].prevO;
//	SOL_Handle n1 = desc[id1].nextO;
//	SOL_Handle p2 = desc[id2].prevO;
//	SOL_Handle n2 = desc[id2].nextO;
//
//	desc[p1].nextO = id2;
//	desc[n2].prevO = id1;
//	desc[id1].nextO = n2;
//	desc[id2].prevO = p1;
//
//	if (id1 != p2) {
//		desc[n1].prevO = id2;
//		desc[p2].nextO = id1;
//		desc[id1].prevO = p2;
//		desc[id2].nextO = n1;
//	} else {
//		desc[id1].prevO = id2;
//		desc[id2].nextO = id1;
//	}
//
//	if (!p1)
//		ordList = id2;
//	if (!n2)
//		ordListEnd = id1;
}

void
MemoryMgr::MoveOrderedLink(SOL_Handle id, Placement place, SOL_Handle refID)
{
	if (place != Before)
	{
	msgMgr->Fatal("ERROR");
	}

	SOL_Handle p = desc[id].prevO;
	SOL_Handle n = desc[id].nextO;

	desc[n].prevO = p;
	if (p)
		desc[p].nextO = n;
	else
		ordList = n;


	if (!refID) {
		desc[id].nextO = 0;
		desc[id].prevO = ordListEnd;
		desc[ordListEnd].nextO = id;
		ordListEnd = id;
	}
	else {
		desc[id].prevO = desc[refID].prevO;
		desc[id].nextO = refID;
		desc[desc[refID].prevO].nextO = id;
		desc[refID].prevO = id;
	}
//	if (place == Before && desc[id].nextO == refID ||
//		 place == After && desc[id].prevO == refID)
//		return;
//
//	SOL_Handle p = desc[id].prevO;
//	SOL_Handle n = desc[id].nextO;
//
//	desc[p].nextO = n;
//	desc[n].prevO = p;
//
//	if (!refID) {
//		desc[0].nextO = ordListEnd;
//		desc[0].prevO = ordListEnd;
//	}
//
//	if (place == Before) {
//		desc[id].prevO = p = desc[refID].prevO;
//		if (p)
//			desc[p].nextO = id;
//		else
//			ordList = id;
//		desc[id].nextO = refID;
//		desc[refID].prevO = id;
//	} else {
//		desc[id].nextO = n = desc[refID].nextO;
//		if (n)
//			desc[n].prevO = id;
//		else
//			ordListEnd = id;
//		desc[id].prevO = refID;
//		desc[refID].nextO = id;
//	}
}

//****************************************************************************
void
MemoryMgr::DeleteFromList(	SOL_Handle theID,
									SOL_Handle* theList,
									SOL_Handle* theListEnd)
{
	if(desc[theID].prev)
		desc[desc[theID].prev].next = desc[theID].next;
   else
		*theList = desc[theID].next;
	if(desc[theID].next)
		desc[desc[theID].next].prev = desc[theID].prev;
   else
		*theListEnd = desc[theID].prev;
}

//****************************************************************************
SOL_Handle
MemoryMgr::Alloc(	MemAttrs			theMemAttrs,
						size_t 			theSize,
						unsigned			theNumber,
						MemClient* 		theClient,
						char 				theChar,
						ReservedHandle	requestHandle)
{
#ifdef DEBUG
	if (checkVolatile)
		VolatileMethod("Alloc");
	if (configMgr->Get(configMgr->RoomMinK)) {
		int sum = theSize;
		for (int id = mruList; id; id = desc[id].next)
			if (!desc[id].memInfo.bits.moveable)
				sum += desc[id].size - SIGNATURESIZE;
		if (sum > roomMaxAloc)
			roomMaxAloc = sum;
	}
#endif

	SOL_Handle	theID;

	if (theSize > totalMemory)
		msgMgr->Fatal("Out of memory: %u bytes requested, type %08x, num %d",
			theSize, theMemAttrs);

#if 0
	if (!theSize)
		msgMgr->Fatal("Zero allocation request: type %08x, num %d",
			theMemAttrs, theNumber);
#endif

	// The following routine is here to prevent recursion.
	if (needMoreDescs)
		GetMoreDescriptors(maxDescriptors/10);

	theSize += SIGNATURESIZE;

	//  GetLow (moveable) or GetHigh (not moveable)
	if (theMemAttrs & MOVEABLE)
		theID = GetLow(theSize,requestHandle);
	else {
/*
		if (theMemAttrs & (DISCARDABLE | DISCARDLAST))
			msgMgr->Alert(
"Request for unmoveable yet discardable allocation %x/%d",
theMemAttrs&0xff,theNumber);
*/
		theID = GetHigh(theSize,requestHandle);
	}

	// Save the initilization character
	desc[theID].initChar = theChar;

	// Set the attributes
	desc[theID].memInfo.attributes = theMemAttrs & (~UNUSEDATTRS);
	desc[theID].memInfo.bits.alignBytes = (4 - (theSize & 3)) & 3;

	// Save the number
	desc[theID].resNumber = theNumber;

	// Install the manager for this memory
	desc[theID].client = theClient;

	// Initialize the memory if requested
	if (theMemAttrs & INITMEMORY) {
#ifdef DEBUG
		if (theChar == FREE_MARK_CHAR)
			msgMgr->Fatal(SrcLoc,Msg_MemInitError);
#endif
		memset(desc[theID].data,theChar,theSize-SIGNATURESIZE);
	}

	// Return the ID
	return (theID);
}

//****************************************************************************
SOL_Handle
MemoryMgr::GetLow(size_t theSize,ReservedHandle requestHandle)
{
	// using a best fit algorithm
	SOL_Handle 	bestFitID = 0;
	int			bestFit = 0x7FFFFFFF;
	int			alocSize = (theSize + 3) & 0xFFFFFFFC;

	for (	SOL_Handle freeID = freeList;
			freeID && desc[freeID].memInfo.bits.moveable;
			freeID = desc[freeID].next) {

		if (alocSize < desc[freeID].size) {
			int thisFit;
			if ((thisFit = desc[freeID].size - alocSize) < bestFit) {
				bestFit = thisFit;
				bestFitID = freeID;
			}
		} else if (alocSize == desc[freeID].size) {
			bestFit = 0;
			bestFitID = freeID;
			break;
		}
	}
	if (!bestFitID) {
		bestFitID = Purge(alocSize);
		if (!bestFitID)
			msgMgr->Fatal("Out of Low memory; %u bytes requested",theSize);
		bestFit = desc[bestFitID].size - alocSize;
	}

	// need a handle for the allocated memory
	SOL_Handle theID = GetNewDescriptor(requestHandle);
	desc[theID].size = alocSize;
	desc[theID].data = desc[bestFitID].data;
	AddBefore(theID,mruList,&mruList,&mruListEnd,bestFitID,True);

	if (bestFit == 0) {
   	// erradicate the consumed free block
   	DeleteFromList(bestFitID,&freeList,&freeListEnd);
   	FreeDescriptor(bestFitID);
   } else {
		// adjust the partially consumed free block
		desc[bestFitID].size -= alocSize;
		desc[bestFitID].data += alocSize;
   }

#ifdef DEBUG
  	WriteSignature(theID);
#endif

	return (theID);
}

//****************************************************************************
SOL_Handle
MemoryMgr::GetHigh(size_t theSize,ReservedHandle requestHandle)
{
	// using a first fit algorithm

	int alocSize = (theSize + 3) & 0xFFFFFFFC;

	if (!freeList && !Purge(alocSize))
		msgMgr->Fatal("Out of High memory; %u bytes requested",theSize);

	SOL_Handle freeID;

	Bool found = False;
	for (	freeID = freeListEnd; freeID; freeID = desc[freeID].prev) {

		if (alocSize <= desc[freeID].size) {
			if (!desc[freeID].memInfo.bits.moveable ||
				 !desc[freeID].nextO ||
				 !desc[desc[freeID].nextO].memInfo.bits.moveable
				)
			found = True;
			break;
		}
		if (desc[freeID].memInfo.bits.moveable)
			break;
	}

  	if (!found) {
		if (!Purge(alocSize))
			msgMgr->Fatal("Out of High memory; %u bytes requested",theSize);
		Pack();
		freeID = freeList;
	}

	// need a handle for the allocated memory
	SOL_Handle theID = GetNewDescriptor(requestHandle);
	desc[theID].size = alocSize;
	desc[theID].data = desc[freeID].data + desc[freeID].size - alocSize;
	AddBefore(theID,mruList,&mruList,&mruListEnd,freeID,False);

	if (alocSize == desc[freeID].size) {
   	// erradicate the consumed free block
   	DeleteFromList(freeID,&freeList,&freeListEnd);
   	FreeDescriptor(freeID);
	} else {
		// adjust the partially consumed free block
		desc[freeID].size -= alocSize;
	}

#ifdef DEBUG
  	WriteSignature(theID);
#endif

	desc[theID].lockCount = 1;
	return(theID);
}

//****************************************************************************
void
MemoryMgr::Free(SOL_Handle theID)
{
#ifdef NOT_SHIPPING_VERSION

	// See if thrashing
	if (!thrashReporting) {
		// Prevent recursion
		thrashReporting = True;

		if (ThrashAdd(theID) >= thrashDetectCount ) {
			// Thrashing give a thrash report
			doThrashReport = True;
		}
		else {
			thrashReporting = False;
		}
	}
#endif
	MemInfo  	info;

	// Signatures may not be valid at this time 
	// due to an internal call from the memory manager.
	CheckValid(theID);

	// message back to the data's manager to release theID from it's list
	if (desc[theID].client)
		desc[theID].client->Free(desc[theID].memInfo.type,desc[theID].resNumber);

	// If theID is a reserved handle, switch handles so that
	// the reserved handle won't get into a free list
	if (theID <	RESERVEDHANDLES) {
		SOL_Handle tmpDescriptor = GetNewDescriptor();
		desc[tmpDescriptor] = desc[theID];
		if (desc[tmpDescriptor].prev)
			desc[desc[tmpDescriptor].prev].next = tmpDescriptor;
		else
			mruList = tmpDescriptor;
		if (desc[tmpDescriptor].next)
			desc[desc[tmpDescriptor].next].prev = tmpDescriptor;
		else
			mruListEnd = tmpDescriptor;
		if (desc[tmpDescriptor].prevO)
			desc[desc[tmpDescriptor].prevO].nextO = tmpDescriptor;
		else
			ordList = tmpDescriptor;
		if (desc[tmpDescriptor].nextO)
			desc[desc[tmpDescriptor].nextO].prevO = tmpDescriptor;
		else
			ordListEnd = tmpDescriptor;
		// Free theID
		desc[theID].nextO = desc[theID].prevO = -1;
		FreeDescriptor(theID);
		theID = tmpDescriptor;
	}

	DeleteFromList(theID,&mruList,&mruListEnd);

#ifdef DEBUG
  	EraseSignature(theID);
#endif

	desc[theID].memInfo = info;
	desc[theID].memInfo.bits.freeDescriptor = False;
	// Add to a freeList
	if (!freeList)	{
   	// the only entry in the list
   	freeList = theID;
		desc[theID].prev = 0;
		desc[theID].next = 0;
   } else {
// HOOK
//		for (SOL_Handle id = desc[theID].nextO; id; id = desc[id].nextO) {
//			if (desc[id].memInfo.bits.freeMemory)
//				break;
//		}
		for (SOL_Handle id = freeList;
			  (id && desc[id].data < desc[theID].data);
			  id = desc[id].next) {	}

		if (!id) {
			// no higher free blocks
			desc[freeListEnd].next = theID;
			desc[theID].next = 0;
			desc[theID].prev = freeListEnd;
			freeListEnd = theID;
		} else if (id == freeList)	{
			// no lower free blocks
			desc[freeList].prev = theID;
			desc[theID].prev = 0;
			desc[theID].next = freeList;
			freeList = theID;
		} else {
			// somewhat prior to a free block
			SOL_Handle prevID = desc[id].prev;
			desc[id].prev = theID;
			desc[theID].next = id;
			desc[prevID].next = theID;
			desc[theID].prev = prevID;
		}
	}

#ifdef DEBUG
	if (checkIntegrity & CHECK_FREE)
	  	// fill the freed memory
		memset(desc[theID].data,FREE_MARK_CHAR,desc[theID].size);
#endif

	SOL_Handle prevID = desc[theID].prevO;
	SOL_Handle nextID = desc[theID].nextO;

	if (prevID)
		desc[theID].memInfo.bits.moveable = desc[prevID].memInfo.bits.moveable;
   else
		desc[theID].memInfo.bits.moveable = 1;

	// coalesce with preceeding free block if adjacent
	if (prevID && desc[prevID].memInfo.bits.freeMemory) {
		desc[theID].size += desc[prevID].size;
		desc[theID].data = desc[prevID].data;
		desc[theID].prev = desc[prevID].prev;
		if (desc[prevID].prev)
			desc[desc[prevID].prev].next = theID;
     	else
        	freeList = theID;
     	FreeDescriptor(prevID);
	}

	// coalesce with ensuing free block if adjacent
	if (nextID && desc[nextID].memInfo.bits.freeMemory) {
		desc[theID].size += desc[nextID].size;
		desc[theID].next = desc[nextID].next;
		if (desc[nextID].next)
			desc[desc[nextID].next].prev = theID;
     	else
        	freeListEnd = theID;
     	FreeDescriptor(nextID);
   }
}

//****************************************************************************
size_t 
MemoryMgr::FreeMemory()
{
	// Returns the maximum amount of memory available as one allocation

	if (totalMemory == 0) {
		// memory manager has not been inited yet...
		// return available system memory
		totalMemory = GetSysMemAvail();
		if (totalMemory > MAXIMUM_SYSTEM_MEMORY)
			totalMemory = MAXIMUM_SYSTEM_MEMORY;
		return totalMemory;
	}

	size_t sum = 0;

	for (SOL_Handle freeID = freeList; freeID; freeID = desc[freeID].next) {
		if (!desc[freeID].memInfo.bits.moveable)
      	break;
		sum += desc[freeID].size;
	}
	return sum > SIGNATURESIZE ? sum - SIGNATURESIZE : 0;
}

//****************************************************************************
// Returns the total memory available to the memory manager
void
MemoryMgr::TotalMemory(size_t* conv, size_t* ext)
{
	if (blackHoleAddr) {
		*conv = ((blackHoleAddr - memoryBase)/1024U);
		*ext = (((memoryBase + totalMemory) - (blackHoleAddr+desc[blackHoleID].size))/1024U);
	}
	else {
		*conv = 0;
		*ext = (totalMemory/1024U);
	}
}

//****************************************************************************
SOL_Handle
MemoryMgr::Purge(size_t theSize)
{
	// If possible, Purge() frees and/or packs memory so that there is at
	// least one free hunk of at least "theSize" bytes (whose handle will
	// be returned; else returns 0)

#ifdef DEBUG
	if (checkVolatile)
		VolatileMethod("Purge");
#endif

	size_t	 	sum = 0;
	SOL_Handle	id;

	// If we only need to pack don't purge!
	for (id = freeList; id; id = desc[id].next) {
		if ((!desc[id].memInfo.bits.moveable) ||
				(sum += desc[id].size) >= theSize)
      	break;
	}

	if (sum >= theSize) {
//		if (id && theSize <= desc[id].size)
//			return id;
   	Pack(theSize);
   	// The very first free hunk is now at least as large as theSize
		return freeList;
	}

	// Have to purge something(s)
  	// find LRU item
	if (!(id = mruListEnd))
		return 0;
	SOL_Handle purgeList = 0;
	do {
     	if ((desc[id].memInfo.bits.discardable) &&
		    (desc[id].memInfo.bits.moveable)) {

			desc[id].tempLink = purgeList;
			purgeList = id;
           sum += desc[id].size;
     	}
	} while (sum < theSize && (id = desc[id].prev));

	if (!purgeList)
		return 0;

  	if (desc[purgeList].size >= theSize) {
     	// the last item checked will free enough memory
     	Free(purgeList);
		return purgeList;
  	}
	do {
		Free(purgeList);
	} while (purgeList = desc[purgeList].tempLink);
	if (sum < theSize) {
		Pack();
		return 0;
	}
	Pack(theSize);
	return freeList;
}

//****************************************************************************
size_t
MemoryMgr::Pack(size_t theSize)
{
#ifdef DEBUG
	if (checkVolatile)
		VolatileMethod("Pack");
#endif

	SOL_Handle begID, endID, freeID = freeList;

	if (!freeID || !desc[freeID].memInfo.bits.moveable)
		return 0;

	// Get rid of possible fragmentation
	while (begID = desc[freeID].nextO) {

		char* descAddr = 0;
		Bool critical = False;
		size_t sum = 0;
		for (endID = begID; endID; endID = desc[endID].nextO) {
			if (desc[endID].memInfo.bits.freeMemory ||
				!desc[endID].memInfo.bits.moveable)
				break;
			critical |= desc[endID].memInfo.bits.critical;
			if (endID == descID)
				// The descriptors table will be moved -- save the new location
				descAddr = desc[descID].data - desc[freeID].size;
			sum += desc[endID].size;
      }
		if (!sum)
			break;

		if (critical)
			_disable();

		memmove(desc[freeID].data,desc[begID].data,sum);
     	// fixup addresses
		if (descAddr) {
			desc = (Descriptor*) descAddr;
		}
		for ( ; begID != endID; begID = desc[begID].nextO)
			desc[begID].data -= desc[freeID].size;
		desc[freeID].data += sum;
		MoveOrderedLink(freeID,Before,endID);

		if (critical)
			_enable();

#ifdef DEBUG
		// initialize free memory to special character
		if (checkIntegrity & CHECK_FREE)
			memset(desc[freeID].data,FREE_MARK_CHAR,desc[freeID].size);
#endif

		if ((endID = desc[freeID].next) &&
			 (desc[freeID].data + desc[freeID].size) == desc[endID].data) {
			// coalesce the now adjacent free blocks
			desc[freeID].size += sum = desc[endID].size;
			DeleteFromList(endID,&freeList,&freeListEnd);
			FreeDescriptor(endID);
			if (desc[freeID].size >= theSize)
				break;
		}
	}

	return desc[freeList].size;
}

//****************************************************************************
void*
MemoryMgr::Lock(SOL_Handle theID)
{
#ifdef DEBUG
	if (checkVolatile)
		VolatileMethod("Lock");
#endif

	CheckValid(theID);

	if (desc[theID].memInfo.bits.moveable) {
		// Move this memory high
	Move(theID);
	}
	else {
		if (desc[theID].lockCount < MAXLOCKCOUNT)
			++desc[theID].lockCount;
	}
	return GetAddr(theID);
}

//****************************************************************************
void*
MemoryMgr::Unlock(SOL_Handle theID)
{
#ifdef DEBUG
	if (checkVolatile)
		VolatileMethod("Unlock");
#endif

	CheckValid(theID);

	if (desc[theID].lockCount) --desc[theID].lockCount;
	if (
		!desc[theID].lockCount
		&& 
		!desc[theID].memInfo.bits.moveable
		) {
		// Move this memory low
		Move(theID);
	}
	return GetAddr(theID);
}

//****************************************************************************
void
MemoryMgr::Move(SOL_Handle theID)
{
	MemAttrs attributes;
	if	(desc[theID].memInfo.bits.moveable)
		attributes = desc[theID].memInfo.attributes & (~MOVEABLE);
	else
		attributes = (desc[theID].memInfo.attributes | MOVEABLE);
	SOL_Handle theNewID = Alloc
					(
					attributes,
					GetSize(theID),
					desc[theID].resNumber,
					desc[theID].client,
					desc[theID].initChar
					);

#ifdef DEBUG
	desc[theNewID].checksum = desc[theID].checksum;
#endif
//?	desc[theNewID].initChar = desc[theID].initChar;

	// Move the data
	memcpy(desc[theNewID].data,desc[theID].data,GetSize(theID));

   // Lets switch handles now
	Descriptor saveDescriptor = desc[theNewID];
	desc[theNewID] = desc[theID];
	desc[theID] = saveDescriptor;

	SOL_Handle link;

	link = desc[theNewID].prev;
	desc[theNewID].prev = desc[theID].prev;
	desc[theID].prev = link;
	link = desc[theNewID].next;
	desc[theNewID].next = desc[theID].next;
	desc[theID].next = link;
	link = desc[theNewID].prevO;
	desc[theNewID].prevO = desc[theID].prevO;
	desc[theID].prevO = link;
	link = desc[theNewID].nextO;
	desc[theNewID].nextO = desc[theID].nextO;
	desc[theID].nextO = link;

	SwapOrderedLinks(theID,theNewID);

   // free the old memory allocation
	desc[theNewID].client = NULL;
   Free(theNewID);
}

//****************************************************************************
void
MemoryMgr::Realloc(SOL_Handle theID,size_t theNewSize)
{
#ifdef DEBUG
	if (checkVolatile)
		VolatileMethod("Realloc");
#endif

	SOL_Handle	theNewID = desc[theID].nextO;

	CheckValid(theID);

	size_t memorySize = ((theNewSize + 3) & 0xFFFFFFFC) + SIGNATURESIZE;

	if (desc[theID].size < memorySize) {

		// Expanding...

		size_t moreBytes = memorySize - desc[theID].size;
		if (	theNewID && 
				desc[theNewID].memInfo.bits.freeMemory &&
				desc[theNewID].size >= moreBytes
			) {
			// the very next block is free and large enough...

			if (desc[theNewID].size > moreBytes) {
				// ...but *more* than large enough
				desc[theNewID].data += moreBytes;
				desc[theNewID].size -= moreBytes;
#ifdef DEBUG
				EraseSignature(theID);
#endif

				// Initialize if neccessary
				if (desc[theID].memInfo.bits.initMemory) {
					memset(desc[theID].data+desc[theID].size-SIGNATURESIZE,
							desc[theID].initChar,
							moreBytes);
				}

				desc[theID].size += moreBytes;

#ifdef DEBUG
				WriteSignature(theID);
#endif
			} else {
				// ... but *exactly* large enough
#ifdef DEBUG
				EraseSignature(theID);
#endif

				// Initialize if neccessary
				if (desc[theID].memInfo.bits.initMemory) {
					memset(desc[theID].data+desc[theID].size-SIGNATURESIZE,
							desc[theID].initChar,
							moreBytes);
				}

				desc[theID].size += moreBytes;
#ifdef DEBUG
				WriteSignature(theID);
#endif
   			DeleteFromList(theNewID,&freeList,&freeListEnd);
   			FreeDescriptor(theNewID);
			}

		} else {

   		// Allocate a new area of memory large enough
  			theNewID = 
     			Alloc
        			(
					desc[theID].memInfo.attributes,
         		memorySize,
					desc[theID].resNumber,
					desc[theID].client
        			);
			memcpy(desc[theNewID].data,desc[theID].data,desc[theID].size - SIGNATURESIZE);

			// If initialize attribute then set the extra memory to the init character
			if (desc[theID].memInfo.bits.initMemory) {
     			memset
        			(
					desc[theNewID].data + desc[theID].size - SIGNATURESIZE,
					desc[theID].initChar,
					memorySize - desc[theID].size
        			);
      	}
			if (theID == descID)	{
				desc = (Descriptor*) desc[theNewID].data;
				desc[theNewID].memInfo.attributes |= HANDLESATTRIBUTES;
				desc[theNewID].memInfo.type = MemDescriptors;
			}
			// Lets switch the data that each handle is looking at!
			char* data = desc[theNewID].data;
			desc[theNewID].data = desc[theID].data;
			desc[theID].data = data;

			size_t size = desc[theNewID].size;
			desc[theNewID].size = desc[theID].size;
			desc[theID].size = size;

			SwapOrderedLinks(theID,theNewID);

   		// free the old memory allocation
			desc[theNewID].client = 0;
  			Free(theNewID);
		}

   } else if (desc[theID].size > memorySize) {

		// Contracting...

		theNewID = GetNewDescriptor();
		desc[theNewID].size = desc[theID].size - memorySize;
		desc[theNewID].data = desc[theID].data + memorySize;
		desc[theNewID].memInfo.attributes = desc[theID].memInfo.attributes;
		desc[theID].size = memorySize;

#ifdef DEBUG
		WriteSignature(theID);
#endif

     	// Free the unwanted piece
		desc[theNewID].client = 0;
		// Link to other desc
		AddBefore(theNewID,mruList,&mruList,&mruListEnd,theID,False);
     	Free(theNewID);

	} // (else no change in size)
	desc[theID].memInfo.bits.alignBytes = (4 - (theNewSize & 3)) & 3;
}

//*************************************************************************
SOL_Handle 
MemoryMgr::Split(SOL_Handle   theID,
                 int          offset,
                 MemAttrs     theMemAttrs,
			        uint         theNumber,
					  MemClient*   theClient)
{
   // Split a memory allocation into two parts 
   // creating another handle for the bottom part
#ifdef DEBUG
   // Get a new space for the bottom part
	SOL_Handle returnID = Alloc(theMemAttrs,
					                desc[theID].size - offset - SIGNATURESIZE,
					                theNumber,
					                theClient);
   // move the data
	memcpy(desc[returnID].data,
          desc[theID].data + offset,
          desc[theID].size - offset  - SIGNATURESIZE);
   // Now reallocate the orginal to the new size
   Realloc(theID,offset);

	if (desc[returnID].memInfo.bits.checksummable && desc[returnID].client) {
		// message back to the data's manager to do the checksum
		desc[returnID].checksum = desc[returnID].client->GetChecksum(returnID);
	}
	if (desc[theID].memInfo.bits.checksummable && desc[theID].client) {
		// message back to the data's manager to do the checksum
		desc[theID].checksum = desc[theID].client->GetChecksum(theID);
	}
#else
   // Get another handle
	SOL_Handle returnID = GetNewDescriptor();
	// Install the manager for this memory
	desc[returnID].client = theClient;
	// Save the number
	desc[returnID].resNumber = theNumber;
	// Set the attributes
	desc[returnID].memInfo.attributes = theMemAttrs & (~UNUSEDATTRS);
   // split the allocation
   desc[returnID].data = desc[theID].data + offset;
   desc[returnID].size = desc[theID].size - offset;
   desc[theID].size = offset;

   // link in the new handle
	AddBefore(returnID,mruList,&mruList,&mruListEnd,theID,False);

   if (
      (desc[returnID].memInfo.bits.moveable && !desc[theID].memInfo.bits.moveable)
      ||
      (!desc[returnID].memInfo.bits.moveable && desc[theID].memInfo.bits.moveable)
      ) {

      if (desc[theID].memInfo.bits.moveable) {
         // Need to allocate unmoveable memory
		   desc[returnID].memInfo.attributes |= MOVEABLE;
         Move(returnID);
      }
      else {
         // Need to allocate moveable memory
		   desc[returnID].memInfo.attributes &= ~MOVEABLE;
         Move(returnID);
      }
   }
#endif
   return returnID;
}

//*************************************************************************
Descriptor& 
Descriptor::operator=(const SaveDescriptor& theDesc)
{
	memInfo = theDesc.memInfo;
	size = theDesc.size;
	initChar = theDesc.initChar;
	lockCount = theDesc.lockCount;
	return *this;
}

//*************************************************************************
void
MemoryMgr::SaveRestore(SaveGame* saveGame,Bool save)
{
	SOL_Handle index;

	if (save) {
		// Save the handle table size
		saveGame->SaveRestore(&maxDescriptors,sizeof(unsigned),save);

		// Save the amount of data being saved
		size_t saveSize = 0;
		for (index = 1;index < maxDescriptors;index++) {

			//	check that locked handles are either marked as either ok to save
			//	or ok not to save
			if (
				(!desc[index].memInfo.bits.freeDescriptor)  	&&
				(!desc[index].memInfo.bits.discardable) 		&&
				(!desc[index].memInfo.bits.discardlast) 		&&
				(!desc[index].memInfo.bits.persist)	  			&&
				(!desc[index].memInfo.bits.transitory)  		&&
				(!desc[index].memInfo.bits.freeMemory)  		&&
				(!desc[index].memInfo.bits.blackHole)  		&&
				(index != descID)
				) {

				if (!desc[index].memInfo.bits.moveable)
					if (desc[index].memInfo.bits.lockedNoSave)
						continue;
					else if (!desc[index].memInfo.bits.lockedSave)
						continue;

				if (!desc[index].client)
					saveSize +=	desc[index].size - SIGNATURESIZE;
			}
		}
		saveGame->SaveRestore(&saveSize,sizeof(size_t),save);

		
		// Save the memory data
		for (index = 1;index < maxDescriptors;index++) {

			//	check that locked handles are either marked as either ok to save
			//	or ok not to save
			if (
				(!desc[index].memInfo.bits.freeDescriptor)  	&&
				(!desc[index].memInfo.bits.discardable) 		&&
				(!desc[index].memInfo.bits.discardlast) 		&&
				(!desc[index].memInfo.bits.persist)	  			&&
				(!desc[index].memInfo.bits.transitory)  		&&
				(!desc[index].memInfo.bits.freeMemory)  		&&
				(!desc[index].memInfo.bits.blackHole)  		&&
				(index != descID)
				) {

				if (!desc[index].memInfo.bits.moveable)
					if (desc[index].memInfo.bits.lockedNoSave)
						continue;
					else if (!desc[index].memInfo.bits.lockedSave) {
						if (!msgMgr->Alert(SrcLoc,Msg_SavingLockedHandle,index,
							 GetMemTypeStr(index)))
							exit(2);
						continue;
					}

				if (desc[index].client) {
					if (!msgMgr->Alert(SrcLoc,Msg_SavingHandleWithClient,index,
						 GetMemTypeStr(index)))
						exit(2);
				}
				else {
					// index of the descriptor
					saveGame->SaveRestore(&index,sizeof(SOL_Handle),save);
					// descriptor information
					SaveDescriptor saveDescriptor;
					saveDescriptor = desc[index];
					saveDescriptor.size -= SIGNATURESIZE;
					saveGame->SaveRestore(&saveDescriptor,sizeof(SaveDescriptor),save);
					// memory data
					saveGame->SaveRestore(desc[index].data,
												 desc[index].size - SIGNATURESIZE,
												 save);
				}
			}
		}
		// The last entry is descriptor 0
		index = 0;
		saveGame->SaveRestore(&index,sizeof(SOL_Handle),save);
	}
	else {
		// Get rid of all purge on restore items and
		// all !purgeable items that are not persist.
		for (index = 1;index < maxDescriptors;index++) {
			if (desc[index].memInfo.bits.transitory)
				Free(index);
		}
		// Now restore saved memory data
		// make sure there are enough handles
		unsigned descNeeded;
		saveGame->SaveRestore(&descNeeded,sizeof(unsigned),save);
		if (descNeeded > maxDescriptors)
			GetMoreDescriptors(descNeeded - maxDescriptors);

		// Purge enough memory to load the saved data
		size_t saveSize;
		saveGame->SaveRestore(&saveSize,sizeof(size_t),save);
		saveSize += descNeeded*SIGNATURESIZE;
		Purge(saveSize);

		saveGame->SaveRestore(&index,sizeof(SOL_Handle),save);
		while(index) {
			if (!desc[index].memInfo.bits.freeDescriptor	&&
				 !desc[index].memInfo.bits.freeMemory) {
				//	make sure what we're writing over is not persistent or locked
				if	(desc[index].memInfo.bits.persist)
					msgMgr->Fatal(SrcLoc,Msg_InvalidRestoreData,
						GetMemTypeStr(index),"persistent",index,desc[index].size);
				//	KLUDGE:  temporarily allow sounds to get dumped even if locked
				else if (!desc[index].memInfo.bits.moveable &&
							(!IsValid(index) || GetMemType(index) != MemResSound))
					msgMgr->Fatal(SrcLoc,Msg_InvalidRestoreData,
						GetMemTypeStr(index),"!moveable",index,desc[index].size);
			}
			// Read in the data
			SaveDescriptor savedDescriptor;
			saveGame->SaveRestore(&savedDescriptor,sizeof(SaveDescriptor),save);
			savedDescriptor.size += SIGNATURESIZE;
			Descriptor restoreDescriptor;
			restoreDescriptor = savedDescriptor;

			SOL_Handle tempID;

			// Since Free() may reuse the desc[index],
			// get a new descriptor and save the one we need.

			// Note the desc[index] will be
			// temporarily unhooked from any list
			if (!desc[index].memInfo.bits.freeDescriptor) {
				tempID = GetNewDescriptor();
				desc[tempID] = desc[index];
				if (desc[index].prev)
					desc[desc[index].prev].next = tempID;
				else {
					if (desc[index].memInfo.bits.freeMemory)
						freeList = tempID;
					else
						mruList = tempID;
				}
				if (desc[index].next)
					desc[desc[index].next].prev = tempID;
				else {
					if (desc[index].memInfo.bits.freeMemory)
						freeListEnd = tempID;
					else
						mruListEnd = tempID;
				}

				if (desc[index].prevO)
					desc[desc[index].prevO].nextO = tempID;
				else
					ordList = tempID;
				if (desc[index].nextO)
					desc[desc[index].nextO].prevO = tempID;
				else
					ordListEnd = tempID;

				if (!desc[index].memInfo.bits.freeMemory)
					Free(tempID);
			}
			else {
				desc[index].memInfo.bits.freeDescriptor = False;
			}
			tempID = Alloc(restoreDescriptor.memInfo.attributes,
								restoreDescriptor.size - SIGNATURESIZE,
								0,
								0,
								restoreDescriptor.initChar);

			// Lets switch handles now
			Descriptor restoredDescriptor = desc[tempID];
			desc[tempID] = desc[index];
			desc[index] = restoredDescriptor;
			desc[index].lockCount = restoreDescriptor.lockCount;

			if (desc[index].prev)
				desc[desc[index].prev].next = index;
			else
				mruList = index;
			if (desc[index].next)
				desc[desc[index].next].prev = index;
			else
				mruListEnd = index;

			if (desc[index].prevO)
				desc[desc[index].prevO].nextO = index;
			else
				ordList = index;
			if (desc[index].nextO)
				desc[desc[index].nextO].prevO = index;
			else
				ordListEnd = index;

			// temp handle no longer needed
			desc[tempID].nextO = desc[tempID].prevO = -1;
			FreeDescriptor(tempID);

			saveGame->SaveRestore(desc[index].data,
										 desc[index].size - SIGNATURESIZE,
										 save);
			saveGame->SaveRestore(&index,sizeof(SOL_Handle),save);
		}
		firstDesc = RESERVEDHANDLES;
	}
}

struct SummaryEntry {
	int		count;
	size_t	size;
	MemType	type;
};

static int
compare(const void* a, const void* b)
{
	return ((SummaryEntry*) b)->size - ((SummaryEntry*) a)->size;
}

void
MemoryMgr::Dump(SaveGame* saveGame, Bool)
{
	SummaryEntry summarys[MemFree];

	memset(summarys, 0, sizeof summarys);
	for (int ji = 0; ji < MemFree; ji++)
		summarys[ji].type = MemType(ji);

	summarys[MemDescriptors].count = 1;

	SOL_Handle index;

	saveGame->Dump("MemoryMgr\n");
	unsigned nDescs;
	saveGame->SaveRestore(&nDescs, sizeof nDescs, False);
	saveGame->Dump("%u descriptors\n", nDescs);

	size_t saveSize;
	saveGame->SaveRestore(saveSize, False);

	saveGame->SaveRestore(index, False);
	while (index) {
		summarys[MemUNUSED2].count++;
		summarys[MemUNUSED2].size += sizeof index;

		saveGame->Dump("%u: ", index);

		SaveDescriptor d;
		saveGame->SaveRestore(&d, sizeof d, False);
		saveGame->Dump("%6u, %s\n", d.size, ::GetMemTypeStr(d.memInfo.type));

		summarys[d.memInfo.type].count++;
		summarys[d.memInfo.type].size += d.size;

		summarys[MemDescriptors].size += sizeof d;

		MemID id;
		id.Get(MemSaveGameDumpBuffer, d.size);
		saveGame->SaveRestore(*id, d.size, False);

		id.Free();

		saveGame->SaveRestore(index, False);
	}

	qsort(summarys, sizeof summarys / sizeof *summarys, sizeof *summarys,
		compare);

	for (int i = 0; i < MemFree; i++)
		if (summarys[i].count)
			saveGame->Dump("%7d %4d %s\n", summarys[i].size,
				summarys[i].count,
				summarys[i].type == MemUNUSED2 ? "Descriptor Table Index" :
						::GetMemTypeStr(summarys[i].type));
}

void
KPurge(argList)
{
	// Input
	//      arg1-----the size to purge (in K)
	// Output
	// 	  acc------True if able to purge the requested amount
	// 	  acc------False if unable to purge the requested amount
	pm.acc = (Acc)memMgr->Purge(arg(1) * 1024U);
}

enum {
	MIFreeK,
	MIFreeLow,
	MIFreeHigh,
	MITotalType,
	MIGetType
};

void
KMemoryInfo(argList)
{
	switch (arg(1)) {
		case MIFreeK:
			pm.acc = Acc(memMgr->FreeMemory() / 1024);
			break;

		case MIFreeLow:
			pm.acc = SCIUWord(memMgr->FreeMemory());
			break;

		case MIFreeHigh:
			pm.acc = memMgr->FreeMemory() >> 16;
			break;

		case MITotalType:
			pm.acc = memMgr->TotalType((MemType) arg(2));
			break;

		case MIGetType:
			if (((MemID) arg(2)).IsValid())
				pm.acc = ((MemID) arg(2)).GetMemType();
			else
				pm.acc = -1;
			break;

		default:
			msgMgr->Fatal(SrcLoc, Msg_InvalidMemInfoCall, arg(1));
	}
}

SOL_Handle MemoryMgr::GetHandle(void* adr)
{
	SOL_Handle id;
	for (id = mruList; id; id = desc[id].next)
		if (desc[id].data == adr)
			return id;
	return 0;
}

#ifdef NOT_SHIPPING_VERSION

int
MemoryMgr::ThrashAdd(SOL_Handle theID)
{
	MemType	theType = desc[theID].memInfo.type;
	ushort	theResNumber = desc[theID].resNumber;

// HOOK
// This shows that 0.heap 0.hunk are thrashing early in the game!!	
//	if (theType > MemResLast)
// This shows that some one is using resNumber 0 and type 0 illegally
//	if ((theType != MemResView) && (theType != MemResPic))
	if (
		((theType != MemResView) && (theType != MemResPic))  ||
		(theResNumber == 0))
		return 0;


	for (int i=0;i<ThrashListSize;++i) {
		if ((thrashArray[i].count != 0)						&& 
			 (theResNumber == thrashArray[i].resNumber)	&&
			 (theType == thrashArray[i].type))

			return ++thrashArray[i].count;
	}
	// A new entry
	// Shift all entries
	for (i=ThrashListSize-1;i>0;--i) {
		thrashArray[i] = thrashArray[i-1];
	}
	thrashArray[0].count = 1;
	thrashArray[0].type = theType;
	thrashArray[0].resNumber = theResNumber;
	return 0;
}

void
MemoryMgr::ThrashReset()
{
	for (int i=0;i<ThrashListSize;++i) {
		thrashArray[i].count = 0;
	}
}

#endif

// The memory manager
MemoryMgr*	memMgr;

#ifdef NOT_SHIPPING_VERSION
Bool 			doThrashReport = False;
Bool 			thrashReporting = True;
int  			thrashDetectCount = 20;
#endif
