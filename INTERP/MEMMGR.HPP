/*****************************************************************************
			 32 bit flat memory model SCI interpreter

	Module:  memmgr.hpp

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

#ifndef MEMMGR_HPP
#define MEMMGR_HPP

#ifndef MEMTYPE_HPP
#include "memtype.hpp"
#endif

#ifndef MSG_HPP
#include "msg.hpp"
#endif

#ifndef SAVEABLE_HPP
#include "saveable.hpp"
#endif

class TextID;

typedef UInt32		MemAttrs;
typedef SCIWord	SOL_Handle;

//****************************************************************************
// theMemInfo defaults are:          theMemInfo user attributes are: 
//
//   !critical                         critical
//   moveable                          moveable
//   discardable                       discardable
//   freeMemory                        
//
//   !markedMemory                     markedMemory
//   !swapable                         swapable
//   !handleMemory                     
//   !checksummable                    checksummable
//
//   !initMemory                       initMemory
//   freeDescriptor                        
//   !convMemory                       convMemory
//   !persist									persist
//   !transitory								transitory
//
//   !lockedsave								locked and saveable
//   !lockednosave							not locked and saveable
//   !discardable								discard-as-last-resort
//
//   !alignBytes                       zero-byte pad
//
//   !blackhole								unusable memory


// The following defines affect some of the equates in assembly.asm
// Allocation attributes
#define LOCKED               0    // (used to override MemID's default MOVEABLE)
#define CRITICAL      0x00000100L // Interrrupts must be disabled when the data moved
#define MOVEABLE		 0x00000200L // This memory can be moved
#define DISCARDABLE   0x00000400L // Can be purged when memory needed
#define FREEMEMORY    0x00000800L // The memory for this handle is unallocated
//--------------------------------------------------------------------------------
#define MARKEDMEMORY  0x00001000L // This memory will be marked on the mono monitor
#define SWAPMEMORY    0x00002000L // This memory may be swapped to disk if unlocked
#define HANDLEMEMORY  0x00004000L // This memory is the Descriptors memory
#define CHECKSUMMABLE 0x00008000L // This memory data will not change
//--------------------------------------------------------------------------------
#define INITMEMORY    0x00010000L // This memory was initialized to some byte value
#define FREEHANDLE    0x00020000L // This handle is available
#define CONVMEMORY    0x00040000L // This memory will be allocated in conventional memory
#define PERSIST		 0x00080000L // This memory will persist across game saving
#define TRANSITORY	 0x00100000L // 

//	There are three states a "locked" allocation can be in:  !MOVEABLE,
//	LOCKEDSAVE, LOCKEDNOSAVE.  There should be no !MOVEABLE allocations before
//	saving or restoring.  However, there can be LOCKEDSAVE and LOCKEDNOSAVE
//	allocations, as explained in each comment:
#define LOCKEDSAVE	 0x00200000L // Save this even though locked (e.g. sound nodes)
#define LOCKEDNOSAVE	 0x00400000L // Don't save, but don't complain because locked (e.g. sound resources)
#define DISCARDLAST   0x00800000L // Can be purged as a last resort
//--------------------------------------------------------------------------------
#define ALIGNBYTES    0x03000000L // Number of bytes padded on allocation
//--------------------------------------------------------------------------------
#define BLACKHOLE		 0x80000000L // This memory has not been allocated to the memory manager
#define UNUSEDATTRS	 0x7C000000L // mask of unassigned memory attributes
//--------------------------------------------------------------------------------

#ifdef DEBUG
	// fill free memory with bad opcodes (such as underscore)
	#define	FREE_MARK_CHAR		0x5f
	#define	FREE_MARK_DWORD	0x5f5f5f5f

	#define THRASHATTRIBUTES LOCKEDNOSAVE+PERSIST+INITMEMORY
#endif

// Initial handle attributes
#define MEMATTRIBUTES FREEMEMORY+DISCARDABLE+FREEHANDLE

// The reserved handles enum
// The valid reserved descriptors are:
// 1 through (RESERVEDHANDLES - 1)

// RESERVEDHANDLES is the count of reserved 
// handles (must be requested by number)
enum ReservedHandle {
	IllegalHandle,
	DescriptorTableHandle,
	BlackHoleHandle,
	blackHoleID = BlackHoleHandle,
	ViewResListHandle,
	PicResListHandle,
	ScriptResListHandle,
	SoundResListHandle,
	VocabResListHandle,
	FontResListHandle,
	PatchResListHandle,
	PalResListHandle,
	WaveResListHandle,
	AudioResListHandle,
	SyncResListHandle,
	MsgResListHandle,
	MapResListHandle,
	HeapResListHandle,
	ChunkResListHandle,
	Audio36ResListHandle,
	Sync36ResListHandle,
	MsgTranslationResListHandle,
	RobotResListHandle,
	VMDResListHandle,
   ResMapHandle,
	AltResMapHandle,
   PatchTableHandle,
	AudioDriverHandle,
	SoundDriverHandle,
	VmapHandle,
	PmapHandle,
	PMStackHandle,
	RetIPHandle,
	TossRetIPHandle,
	DecompBufHandle,
	AudioConvBufHandle,
	AudioFillBufHandle,
	DecompWorkHandle,
	SaveGameDirHandle,
	AudioXferBufHandle,
   Patch36TableHandle,
	ThrashArrayHandle,
	AudioPriorityHandle,
	RESERVEDHANDLES
};

enum Placement {
	Before,
	After
};

#ifdef DEBUG
// size of signature written at the end of every allocation 
// NOTE: This should be a multiple of 4
#define SIGNATURESIZE  (5*4)
#else
#define SIGNATURESIZE  0
#endif

struct MemClient
{
	//	the user of memory

	//	callback to let us know something's	been freed by the memory manager
	virtual void	Free(MemType,unsigned) = 0;
#ifdef DEBUG
	virtual uint	GetChecksum(SOL_Handle) = 0;	// compute checksum
#endif
};

class MemInfo
	{
	union
   	{
		MemAttrs attributes;		// Memory and handle information
		MemType	type;   			// This is the memory type
		struct
			{
			int fill0:8;        // This must be the same size as a MemType
			int critical:1;     // Must have interrupts disabled when moved
			int moveable:1;     // Can be moved - if not moveable memory allocated high
			int discardable:1;  // Can be released if needed
			int freeMemory:1;   // Is not allocated
			int markedMemory:1; // Marked to show on mono monitor output
			int swapMemory:1;   // This memory is swapable to disk if unlocked
			int handleMemory:1; // This is the Descriptors memory allocation
			int checksummable:1;// Should not change in value
			int initMemory:1;   // The memory is initialized to some given character
			int freeDescriptor:1;   // This handle is not being used
			int convMemory:1;   // Is loaded in conventional memory
			int persist:1;		  // This memory allocation will persist across game  saving
			int transitory:1;	  // Don't save on save, purge on restore
			int lockedSave:1;	  // Save, though locked
			int lockedNoSave:1; // Don't save and don't complain about being locked
			int discardlast:1;  // Can be released as a last resort
			int alignBytes:2;   // 0,1,2 or 3 byte pad to force a multiple of 4 bytes
			int fill:5;         // Available for other flags
			int blackHole:1;    // Can't be used by the memory manager
			} bits ;
		};

public:
MemInfo()
		{
		attributes = MEMATTRIBUTES;
		type = MemFree;
   	}
	friend class MemoryMgr;
	};

class SaveDescriptor;

class Descriptor
{
	MemInfo        memInfo;    // Information about the block of memory and
										// the handle
										// client processing of purged resources)
	char*				data;       // Address of the chunk
	size_t			size;       // Size in bytes of the chunk
										// (includes the signature block if written)
	MemClient*  	client;     // The manager of the data this handle references
	SOL_Handle     next;       // Next ID in free or mru link
	SOL_Handle     prev;       // Previous ID in free or mru link
	SOL_Handle     nextO;		// Next ID in physically-ordered link
	SOL_Handle     prevO;		// Previous ID in physically-ordered link
	SOL_Handle		tempLink;	// Used to line up candidates for purge
	ushort			resNumber;  // The resource number 	(allows for faster
#ifdef DEBUG
	ushort			checksum;	// Checksum if checksummable
#else
	ushort			unused;		// eat 4 bytes
#endif
	uchar  			lockCount;	// The count of locks
	char           initChar;   // If initialization character it's stored here
// Note: for performance purposes, Descriptor should be a power-of-two bytes
// (not to mention a multiple of 4 bytes)
// in size (a line in MEMMGR.CPP asserts this).

public:

	Descriptor() : size(0),next(0),prev(0),lockCount(0),client(0)
	{
	}

	void* operator new(size_t,void* where)
	{
		return where;
	}

	void operator delete(void*)
	{
	}

	Descriptor& operator=(const SaveDescriptor& theDesc);
	friend class MemoryMgr;
	friend class SaveDescriptor;
	friend class MemID;
	friend void* operator new(size_t,SOL_Handle);
};

#ifdef DEBUG
	#define ThrashListSize 20
	class ThrashItem
	{
		uchar		count;			// The count of times freed
		MemType	type;   			// The memory type
		ushort	resNumber;  	// The resource number
	public:
		friend class MemoryMgr;
	};
#endif

class MemoryMgr : public Saveable
{
public:

	// Constructor
	MemoryMgr();
	
	//	Initializor
	void Init(size_t = 0);

	virtual ~MemoryMgr() {}
	
	// Gets memory.
	SOL_Handle Alloc(MemAttrs theMemAttrs,
						  size_t theSize,
						  uint theNumber = 0,
						  MemClient* theClient = 0,
						  char theChar = 0,
						  ReservedHandle requestHandle = IllegalHandle);

	// Resizes a memory allocation
	void Realloc(SOL_Handle theID,size_t theNewSize);

   // Split a memory allocation into two parts 
   // creating another handle for the bottom part
	SOL_Handle Split(SOL_Handle   theID,
                    int          offset,
                    MemAttrs     theMemAttrs,
						  uint         theNumber = 0,
						  MemClient*   theClient = 0);

	// Frees memory
	void Free(SOL_Handle theID);

	// Returns the maximum amount of memory available as one allocation
	size_t FreeMemory();

	// Returns the total memory available to the memory manager
	void TotalMemory(size_t* conventional, size_t* extended);

	// Purge discardable memory
	SOL_Handle Purge(size_t theSize);

	// Defragment memory
	size_t Pack(size_t theSize = 0xffffffff);

	//Set locked
	void* Lock(SOL_Handle theID);

	//Set unlocked
	void* Unlock(SOL_Handle theID);

	// Set critical or not
	void Critical(SOL_Handle theID,Bool critical = True)
	{
		desc[theID].memInfo.bits.critical = critical;
	}

	// Gets memory address.
	void* GetAddr(SOL_Handle theID)
	{
#ifdef DEBUG
		CheckValid(theID);
#endif
		return desc[theID].data;
	}
	
	// Marks the memory as discardable
	void SetDiscardable(SOL_Handle theID)
	{
#ifdef DEBUG
		CheckValid(theID);
#endif
		desc[theID].memInfo.attributes &= (~TRANSITORY);
		desc[theID].memInfo.attributes |= DISCARDABLE;
	}
	
	// Marks the memory as not discardable
	void SetNotDiscardable(SOL_Handle theID)
	{
#ifdef DEBUG
		CheckValid(theID);
#endif
		desc[theID].memInfo.attributes &= (~DISCARDABLE);
		desc[theID].memInfo.attributes |= TRANSITORY;
	}
	
	// Gets memory allocation size.
	size_t GetSize(SOL_Handle theID)
	{
#ifdef DEBUG
		CheckValid(theID);
#endif
		return (desc[theID].size - SIGNATURESIZE - 
			desc[theID].memInfo.bits.alignBytes); 
	}

	// Returns the memory type
	MemType GetMemType(SOL_Handle h)
	{
#ifdef DEBUG
		CheckValid(h);
#endif
		return desc[h].memInfo.type;
	}

	// Returns the memory attributes
	MemAttrs GetMemAttrs(SOL_Handle theID)
	{
		return (desc[theID].memInfo.attributes);
	}

	// Returns the resource number
	uint GetResNumber(SOL_Handle theID)
	{
		return (desc[theID].resNumber);
	}

#ifdef DEBUG
	void	ThrashReport();
#endif


	friend class MemID;
	friend void* operator new(size_t,SOL_Handle);

	//	Checks whether handle is valid, and aborts if it isn't
	void CheckValid(SOL_Handle theID)
	{
		if (!IsValid(theID))
			msgMgr->Fatal(SrcLoc,Msg_InvalidHandle,theID);
	}

	//	Returns whether handle is valid
	Bool IsValid(SOL_Handle theID)
	{
		return(
				(theID)															&&
				(theID < maxDescriptors)									&&
				(desc[theID].memInfo.bits.freeDescriptor == False)	&&
				(desc[theID].memInfo.bits.freeMemory == False)
				);
	}

	// Returns the name of the memory type
	const char* GetMemTypeStr(SOL_Handle theID);

	//	returns a string containing a summary of memory use by type
	TextID MakeMemTypeDspStr();

	//	returns a string containing a list of each MemID
	TextID MakeMemIDListStr(int start, int* end, int count);

	//	Performs the IO for a save or restore
	void SaveRestore(SaveGame*,Bool save);

	//	Dump savegame information to disk for debugging
	void Dump(SaveGame*, Bool);

	virtual size_t	GetConvMemAvail() = 0;
	virtual char*	GetConvMem(size_t bytes) = 0;
	virtual void	FreeConvMem(char *base) = 0;

	// returns total number of allocations of a certain type
	int	TotalType(MemType);

#ifdef DEBUG

	// if selected, warns when a call is made to a volatile memmgr routine
	unsigned CheckVolatile(Bool set);
	
	// turns memory checking on or off
	void IntegrityChecking();

	// checks the whole ball of wax
	Bool CheckIntegrity(char* note);

	// report an Integrity Error
	const Bool ICError(char* error, SOL_Handle id);

	// report an Integrity Error and delete temporary list arrays
	const Bool ICListError(char* error, SOL_Handle id);

	// checks the signature in all memory blocks
	Bool CheckSignatures();

	// checks that all free memory is initialized to zeros.
	Bool CheckFree();

	//	checks the free list and mru list for consistency
	Bool CheckLists();

	// checks the checksums
	Bool CheckChecksums();

	// checks a checksum
	Bool CheckChecksum(SOL_Handle theID);

	// Set checksummable bit and get checksum
	void SetChecksum(SOL_Handle theID,Bool set = True);

	// Set checksummable bit and get checksum
	void SetChecksum(void* data,Bool set = True);

	// returns a bitmap showing the memory allocation display
	SOL_Handle MemoryBitMap();

	// Shows all bitmaps in memory
	void MemoryMgr::ShowBitmaps();

	// Shows all text in memory
	void MemoryMgr::ShowText(int from,int to);

	// Marks a memory allocation in the memory map
	void MarkMemory(SOL_Handle theID)
	{
		CheckValid(theID);
		desc[theID].memInfo.attributes |= MARKEDMEMORY;
	}
	
	// UnMarks a memory allocation in the memory map
	void UnMarkMemory(SOL_Handle theID)
	{
		CheckValid(theID);
		desc[theID].memInfo.attributes &= (~MARKEDMEMORY);
	}
	
#endif

	SOL_Handle MemoryMgr::GetHandle(void* adr);

protected:
	unsigned			maxDescriptors;// Maximum number of handles
	Descriptor*		desc;				// The handle array
	SOL_Handle		firstDesc;		// An available descriptor
	Bool				needMoreDescs;	// True when running out of Descriptors
	SOL_Handle     descID;			// The handle to the handles
	SOL_Handle     freeList;      // ID of first free hunk in free list
	SOL_Handle     freeListEnd;   // ID of last free hunk in free list
	SOL_Handle     mruList;       // ID of first allocated hunk in mru list
	SOL_Handle     mruListEnd;		// ID of last allocated hunk in mru list
	SOL_Handle     ordList;       // ID of first hunk in ordered list
	SOL_Handle     ordListEnd;		// ID of last hunk in ordered list
	size_t			totalMemory;   // Amount of memory controlled
											//	by the memory manager
	char*       	memoryBase;    // Pointer to allocated memory for
											// the memory manager
	char*				blackHoleAddr;	// Address of the black hole
#ifdef DEBUG
	ThrashItem* 	thrashArray;	// Thrash detection array
	Bool				moveToFront;  	// Used in CheckChecksums
	unsigned			checkVolatile;	// error volatile method invoked
	char*				info;				// integrity check caller
	char        	signature		// Signature written at the end of each aloc
							[SIGNATURESIZE];
	int	ThrashAdd(SOL_Handle theID);
	void	ThrashReset();
#endif

	// get an available handle
	SOL_Handle GetNewDescriptor(ReservedHandle requestHandle = IllegalHandle);

	// get more desc
	void	GetMoreDescriptors(unsigned more);

	// make available a handle
	void FreeDescriptor(SOL_Handle theID);

	// Adds handle to list in front of given handle
	void AddBefore(SOL_Handle	newID,
						SOL_Handle	nextID,
						SOL_Handle*	theList,
						SOL_Handle*	theListEnd,
						SOL_Handle	refID,
						Bool			beforeRefID);

	// Swaps ordered links
	void SwapOrderedLinks(SOL_Handle id1, SOL_Handle id2);

	// Adjust ordered links
	void MoveOrderedLink(SOL_Handle id, Placement place, SOL_Handle refID);

	// deletes a handle from an ordered list. Works for freeList and MRUList
	void DeleteFromList(	SOL_Handle theID,
								SOL_Handle* theList,
								SOL_Handle* theListEnd);

	// Gets low memory.
	SOL_Handle GetLow(size_t theSize,ReservedHandle requestHandle = IllegalHandle);

	// Gets high memory.
	SOL_Handle GetHigh(size_t theSize,ReservedHandle requestHandle = IllegalHandle);

	virtual size_t	GetSysMemAvail() = 0;
	virtual char *	GetSysMem(size_t bytes) = 0;

	// Moves an allocation from high to low OR from low to high memory
	void Move(SOL_Handle theID);

#ifdef DEBUG
	// Bumps the 'volatile method envoked' counter
	void VolatileMethod(char* where = 0);
	
	// write signature in memory block
	void WriteSignature(SOL_Handle theID);

	// Erase signature in memory block
	void EraseSignature(SOL_Handle theID);

	// Used for creating a bitmap showing the memory allocation display
	void	FillMemBM(char* target,char* start,size_t size,uchar color,int bump,int special = False);
	void	FillIndex(char* target,int startIndex,uchar color,int special);
#endif

};

//BEW CHANGED extern MemoryMgr* memMgr;
extern "C" MemoryMgr* memMgr;

#ifdef DEBUG
	extern Bool 		doThrashReport;
	extern Bool 		thrashReporting;
	extern int  		thrashDetectCount;
#endif

#endif  //MEMMGR_HPP
