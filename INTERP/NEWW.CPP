//	new.cpp		11/26/91
//		an error checking and reporting new

#include <stdio.h>
#include <string.h>

#include "sol.hpp"

#include "heapleak.hpp"
#include "memmgr.hpp"
#include "msg.hpp"
#include "pmachine.hpp"

#ifdef WINDOWS
#include "mbox.hpp"
#endif

#define CACHESIZE1 200
#define ENTRYSIZE1 32
int	dummy1;
char	cache1[CACHESIZE1 * ENTRYSIZE1];
Bool	inUse1[CACHESIZE1];
void	*cache1Start;
void	*cache1End;

#define CACHESIZE2 350
#define ENTRYSIZE2 512
int	dummy2;
char	cache2[CACHESIZE2 * ENTRYSIZE2];
Bool	inUse2[CACHESIZE2];
void	*cache2Start;
void	*cache2End;

#define CACHESIZE3 100
#define ENTRYSIZE3 1140
int	dummy3;
char	cache3[CACHESIZE3 * ENTRYSIZE3];
Bool	inUse3[CACHESIZE3];
void	*cache3Start;
void	*cache3End;

#define RESERVESIZE 35000
int	dummy4;
char	reserve[RESERVESIZE];
void	*reserveStart;
void	*reserveNext;
void	*reserveEnd;
Bool	firstTime = True;
Bool	firstInit = True;
int	preTot = 0;

int	totCount = 0;
int	newCount = 0;
int	cnt1 = 0;
int	cnt2 = 0;
int	cnt3 = 0;
int	cnt4 = 0;

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
void	*p;

	if (firstTime) {
		firstTime = False;
		reserveStart = (void *)reserve;
		reserveNext = (void *)reserve;
		reserveEnd = (void *)((char *)reserveStart + RESERVESIZE);

		cache1Start = (void *)((((int)cache1) >> 2) << 2);
		cache1End = (void *)((char *)cache1Start + (CACHESIZE1 * ENTRYSIZE1));
		for (int i = 0; i < CACHESIZE1; i++)
			inUse1[i] = False;

		cache2Start = (void *)((((int)cache2) >> 2) << 2);
		cache2End = (void *)((char *)cache2Start + (CACHESIZE2 * ENTRYSIZE2));
		for (i = 0; i < CACHESIZE2; i++)
			inUse2[i] = False;

		cache3Start = (void *)((((int)cache3) >> 2) << 2);
		cache3End = (void *)((char *)cache3Start + (CACHESIZE3 * ENTRYSIZE3));
		for (i = 0; i < CACHESIZE3; i++)
			inUse3[i] = False;
	}

	if (!pm.game) {
		preTot += size;
		p = reserveNext;
		reserveNext = (char *)reserveNext + size;
		if (reserveNext > reserveEnd) {
			MBox("reserve overflow","");
			exit(255);
		}
		return p;
	}

#ifdef JUNK
	if (firstInit) {
		firstInit = False;
		sprintf(mbuff,"init:%d",preTot);
		MBox(mbuff,"");
	}

	totCount++;
	if (totCount > 1000) {
		totCount = 0;
		sprintf(mbuff,"new:%d 1:%d 2:%d 3:%d 4:%d", newCount, cnt1, cnt2, cnt3, cnt4);
		newCount = 0;
		cnt1 = 0;
		cnt2 = 0;
		cnt3 = 0;
		cnt4 = 0;
		SciDisplay(mbuff);
	}

	if (size <= ENTRYSIZE1)
		cnt1++;
	else if (size <= ENTRYSIZE2)
		cnt2++;
	else if (size <= ENTRYSIZE3)
		cnt3++;
	else
		cnt4++;
#endif

	if (size <= ENTRYSIZE1) {
		for (int i = 0; i < CACHESIZE1; i++) {
			if (!inUse1[i]) {
				inUse1[i] = True;
				p = (void *)((char *)cache1Start + (i * ENTRYSIZE1));
				return p;
			}
		}
//		SciDisplay("cache1 overflow");
	}

	if (size <= ENTRYSIZE2) {
		for (int i = 0; i < CACHESIZE2; i++) {
			if (!inUse2[i]) {
				inUse2[i] = True;
				p = (void *)((char *)cache2Start + (i * ENTRYSIZE2));
				return p;
			}
		}
//		SciDisplay("cache2 overflow");
	}

	if (size <= ENTRYSIZE3) {
		for (int i = 0; i < CACHESIZE3; i++) {
			if (!inUse3[i]) {
				inUse3[i] = True;
				p = (void *)((char *)cache3Start + (i * ENTRYSIZE3));
				return p;
			}
		}
//		SciDisplay("cache3 overflow");
	}

//	newCount++;

	p = malloc(size);
	if (!p)
		msgMgr->Fatal("Can't allocate %u bytes in %s(%d)", size, file, line);
#if CHECKHEAPLEAKS
	if (heapLeakChecker)
		heapLeakChecker->Add(file, line, p);
#endif
	return p;
}

//#if CHECKHEAPLEAKS
void
operator delete(void* p)
{
				
	if ((p < cache1End) && (p >= cache1Start)) {
		int i = ((int)p - (int)cache1Start) / ENTRYSIZE1;
		inUse1[i] = False;
		return;
	}

	if ((p < cache2End) && (p >= cache2Start)) {
		int i = ((int)p - (int)cache2Start) / ENTRYSIZE2;
		inUse2[i] = False;
		return;
	}

	if ((p < cache3End) && (p >= cache3Start)) {
		int i = ((int)p - (int)cache3Start) / ENTRYSIZE3;
		inUse3[i] = False;
		return;
	}

	if ((p < reserveNext) && (p >= reserveStart)) {
		return;
	}

//	if (heapLeakChecker)
//		heapLeakChecker->Delete(p);
	if (p) {
		free(p);
	}
}
//#endif
