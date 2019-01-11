//      neww.cpp
//		Assign "new" requests from a memory cache.  Windows only.

#include <stdio.h>
#include <string.h>

#include "sciwin.hpp"
#include "sol.hpp"

#include "heapleak.hpp"
#include "memmgr.hpp"
#include "memmgrw.hpp"
#include "msg.hpp"
#include "neww.hpp"
#include "pmachine.hpp"
#include "mbox.hpp"


// A call to "new" causes an allocation to be to be made from one of
// the following sources:
//
// Local cache
// Some calls are made to "new" before the program even has control.  This
// is a small static data segment cache to handle calls until memory can
// be allocated.
//
//	Formatted memory caches
// There are three tables set up with different element sizes and number
// of elements.  These handle the vast majority of dynamic news and deletes.
//
// Reserve cache
// Most of the calls to "new" which occur before the pmachine starts tend
// to be static allocations.  That is, they are not deleted until the
// interpreter terminates.  They also tend to be of odd sizes.  Therefore,
// any call made before starting the pmachine for allocations which will
// not fit into the formatted caches are allocated here.  They are assigned
// contiguously since we don't need to worry about deleting them.
//
// Memory manager
// Finally, any requests made after the pmachine starts which will not fit
// one of the formatted caches, are allocated from the memory manager.  The
// intention is that this should be a rare occurance.
//
//
// Structure of "new" allocations.
// Each allocation is prepended by a 4-byte header.  The first two bytes
// identify the cache.  The second two bytes identify the element number
// within the cache if it is a formatted cache.  The purpose is to gain
// performance for deletes.  The address returned to the requestor points
// to the first byte past the header.  All allocations are on mod 4
// bounderies.

// Local cache for news occuring before we can allocate memory
static	void	*localNext;
static	void	*localEnd;
static	char	localCache[LOCALSIZE];

// Reserve cache for news occuring before pmachine starts
			void	*newCacheAddr;
int reserveSize;
static	void	*reserveNext;
static	void	*reserveEnd;

// Cache 1
int cacheSize1;
int entrySize1;
static	Bool	*inUse1;
static	void	**cache1Addr;

// Cache 2
int cacheSize2;
int entrySize2;
static	Bool	*inUse2;
static	void	**cache2Addr;

// Cache 3
int cacheSize3;
int entrySize3;
static	Bool	*inUse3;
static	void	**cache3Addr;

int	newState = NEW_FIRST_CALL;

// Statistics
static	int	localHigh = 0;
static	int	reserveHigh = 0;
static	int	cnt[5] = {0, 0, 0, 0, 0};
static	int	high[5] = {0, 0, 0, 0, 0};

HeapLeakChecker* heapLeakChecker;

#if CHECKHEAPLEAKS
void
HeapLeakChecker::Add(char* file, int line, void* p)
{
	assert(nEntries < Size);

	allocations[nEntries].p = p;
	allocations[nEntries].line = line;
	
	//	strip off directory, if any, and hope the name is not of the form
	//	'c:file'
	char* cp = strrchr(file, '\\');
	strcpy(allocations[nEntries].file, cp ? cp + 1 : file);
	nEntries++;
}

void
HeapLeakChecker::Delete(void* p)
{
	if (!p)
		return;

	for (int i = 0; i < nEntries; i++)
		if (p == allocations[i].p) {
			memcpy(&allocations[i], &allocations[i + 1],
				(nEntries - i - 1) * sizeof(*allocations));
			nEntries--;
			return;
		}
}

HeapLeakChecker::~HeapLeakChecker()
{
	if (nEntries) {
		FILE* fp = fopen("C:\\HEAPLEAK", "wt");
		puts("Heap leaks:");
		fputs("Heap leaks:\n", fp);
		for (int i = 0; i < nEntries; i++) {
			printf("%s(%d)  ", allocations[i].file, allocations[i].line);
			fprintf(fp, "%s(%d)\n", allocations[i].file, allocations[i].line);
		}
		fclose(fp);
	} else
		puts("No heap leaks");
}
#endif

void*
operator new(size_t size, char* file, int line)
{

	switch (newState) {

		case NEW_FIRST_CALL:
			// First call to new.  Initialize local cache.
			InitLocalCache();
			newState = NEW_PRE_MEM_CACHE;

		case NEW_PRE_MEM_CACHE:
			// Memory cache not yet available.  Use local cache.
			return AllocInLocal(size);

		case NEW_MEM_CACHE_AVAIL:
			// Memory cache now available.  Format it.
			InitMemoryCache();
			newState = NEW_USE_MEM_CACHE;

		case NEW_USE_MEM_CACHE:
			// Cache is formatted.  Use cache tables.
			return AllocInTables(size,file,line);

		default:
			MBox("Invalid newState","");
			return (void *)0;
	}
}

void
operator delete(void* p)
{
int	cacheNum, entryNum;


	if (!p)
		return;

	cacheNum = (int)*((ushort *)p - 2);
	entryNum = (int)*((ushort *)p - 1);

	switch (cacheNum) {

		case NEW_CACHE_ONE:
			inUse1[entryNum] = False;
			if (SHOWNEWS)
				cnt[1]--;
			return;

		case NEW_CACHE_TWO:
			inUse2[entryNum] = False;
			if (SHOWNEWS)
				cnt[2]--;
			return;

		case NEW_CACHE_THREE:
			inUse3[entryNum] = False;
			if (SHOWNEWS)
				cnt[3]--;
			return;

		case NEW_CACHE_LOCAL:
			// nothing to do
			return;

		case NEW_CACHE_RESERVE:
			// nothing to do
			return;

		case NEW_CACHE_MEMMGR:
			p = (char *)p - HEADERSIZE;
//			free(p);
		  	memMgr->Free(memMgr->GetHandle(p));
			if (SHOWNEWS)
				cnt[4]--;
			return;

		default:
#ifdef DEBUG
			sprintf(mbuff,"unknown cache ID cache:%d entry:%d addr:%p",cacheNum,entryNum,p);
			MBox(mbuff,"");
#endif
			// probably a "new" instead of a "New"
			free(p);
	}
}

void
InitLocalCache(void)
// Set up pointers for local data segment cache
{
	localNext = localCache + HEADERSIZE;
	localEnd = (void *)(localCache + LOCALSIZE);
}

void*
AllocInLocal(size_t size)
// Allocate contiguously from local cache
{
void	*p;

		size = ((size + 3) >> 2) << 2;
		p = localNext;
		localNext = (char *)localNext + size + HEADERSIZE;
		if (localNext > localEnd) {
			MBox("local overflow","");
			exit(255);
		}
		localHigh += size + HEADERSIZE;
		*((ushort *)p - 2) = NEW_CACHE_LOCAL;
		*((ushort *)p - 1) = 0;
		return p;
}

void
InitMemoryCache(void)
// Format the allocated memory into three caches of fixed-length blocks
// and one reserve for contiguous assignments.  We pre-set the headers and
// establish a table of pointers so it will be fast.
{
ushort	i;
char	*cache1;
char	*cache2;
char	*cache3;

	inUse1 = (Bool *)newCacheAddr;
	cache1Addr = (void **)((char *)inUse1 + (cacheSize1 * sizeof(Bool)));

	inUse2 = (Bool *)((char *)cache1Addr  + (cacheSize1 * sizeof(void *)));
	cache2Addr = (void **)((char *)inUse2 + (cacheSize2 * sizeof(Bool)));

	inUse3 = (Bool *)((char *)cache2Addr  + (cacheSize2 * sizeof(void *)));
	cache3Addr = (void **)((char *)inUse3 + (cacheSize3 * sizeof(Bool)));

	reserveNext = (char *)((char *)cache3Addr  + (cacheSize3 * sizeof(void *))) + HEADERSIZE;
	reserveEnd = (void *)((char *)reserveNext + reserveSize - HEADERSIZE);

	cache1 = (char *)reserveEnd;
	cache2 = cache1 + cacheSize1 * (entrySize1 + HEADERSIZE);
	cache3 = cache2 + cacheSize2 * (entrySize2 + HEADERSIZE);

	for (i = 0; i < cacheSize1; i++) {
		inUse1[i] = False;
		cache1Addr[i] = (void *)((char *)cache1 + (i * (entrySize1 + HEADERSIZE)));
		*((ushort *)cache1Addr[i] - 2) = NEW_CACHE_ONE;
		*((ushort *)cache1Addr[i] - 1) = i;
	}

	for (i = 0; i < cacheSize2; i++) {
		inUse2[i] = False;
		cache2Addr[i] = (void *)((char *)cache2 + (i * (entrySize2 + HEADERSIZE)));
		*((ushort *)cache2Addr[i] - 2) = NEW_CACHE_TWO;
		*((ushort *)cache2Addr[i] - 1) = i;
	}

	for (i = 0; i < cacheSize3; i++) {
		inUse3[i] = False;
		cache3Addr[i] = (void *)((char *)cache3 + (i * (entrySize3 + HEADERSIZE)));
		*((ushort *)cache3Addr[i] - 2) = NEW_CACHE_THREE;
		*((ushort *)cache3Addr[i] - 1) = i;
	}
}

void*
AllocInTables(size_t size,char* file,int line)
{
void	*p;

	// Can we use table 1?
	if (size <= entrySize1) {
		for (int i = 0; i < cacheSize1; i++) {
			if (!inUse1[i]) {
				inUse1[i] = True;
				p = (void *)cache1Addr[i];
				if (SHOWNEWS)
					DisplayStats(1);
				return p;
			}
		}
#ifdef DEBUG
		sprintf(mbuff,"cache1 overflow:%d",cacheSize1);
		SciDisplay(mbuff);
#endif
	}

	// Can we use table 2?
	if (size <= entrySize2) {
		for (int i = 0; i < cacheSize2; i++) {
			if (!inUse2[i]) {
				inUse2[i] = True;
				p = (void *)cache2Addr[i];
				if (SHOWNEWS)
					DisplayStats(2);
				return p;
			}
		}
#ifdef DEBUG
		sprintf(mbuff,"cache2 overflow:%d",cacheSize2);
		SciDisplay(mbuff);
#endif
	}

	// Can we use table 3?
	if (size <= entrySize3) {
		for (int i = 0; i < cacheSize3; i++) {
			if (!inUse3[i]) {
				inUse3[i] = True;
				p = (void *)cache3Addr[i];
				if (SHOWNEWS)
					DisplayStats(3);
				return p;
			}
		}
#ifdef DEBUG
		sprintf(mbuff,"cache3 overflow:%d",cacheSize3);
		SciDisplay(mbuff);
#endif
	}

	// If the pmachine is not started, use the reserve cache
	if (!pm.game) {
		int sizeMod4 = ((size + 3) >> 2) << 2;
		if (SHOWNEWS)
			reserveHigh += sizeMod4 + HEADERSIZE;
	  	p = reserveNext;
	  	reserveNext = (char *)reserveNext + sizeMod4 + HEADERSIZE;
	  	if (reserveNext < reserveEnd) {
	  		*((ushort *)p - 2) = NEW_CACHE_RESERVE;
	  		*((ushort *)p - 1) = 0;
	  		return p;
	  	}
	  	reserveNext = p;
#ifdef DEBUG
		sprintf(mbuff,"reserve overflow:%d",reserveSize);
		SciDisplay(mbuff);
#endif
	}

	// All else failed.  Get it from the memory manager.
//	p = new char[size + HEADERSIZE];
	p = memMgr->GetAddr(memMgr->Alloc(LOCKEDNOSAVE, size + HEADERSIZE, 0, 0, 0));
	if (!p)
		msgMgr->Fatal("Can't allocate %u bytes in %s(%d)", size, file, line);

#if CHECKHEAPLEAKS
	if (heapLeakChecker)
		heapLeakChecker->Add(file, line, p);
#endif

	p = (char *)p + HEADERSIZE;
	*((ushort *)p - 2) = NEW_CACHE_MEMMGR;
	*((ushort *)p - 1) = 0;
	if (SHOWNEWS)
		DisplayStats(4);
	return p;
}

void
DisplayStats(int cacheNum)
// set the "showNews" option in resource.win to display cache
// high-water marks.
{
	cnt[cacheNum]++;
	if (cnt[cacheNum] > high[cacheNum]) {
		high[cacheNum] = cnt[cacheNum];
		sprintf(mbuff,"high loc:%d 1:%d 2:%d 3:%d res:%d mgr:%d",
						localHigh,high[1],high[2],high[3],reserveHigh,high[4]);
		SciDisplay(mbuff);
	}
}
