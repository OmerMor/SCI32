//	new.cpp		11/26/91
//		an error checking and reporting new

#include <stdio.h>
#include <string.h>

#include "sol.hpp"

#include "heapleak.hpp"
#include "msg.hpp"

#ifdef WINDOWS
#include "mbox.hpp"
#endif

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
operator new[](size_t size, char* file, int line)
{
	void* p = new char[size];
	if (!p)
#ifdef WINDOWS
		{
		MBox("Error allocating New","");
		file = file;
		line = line;
		exit(255);
		}
#else
		msgMgr->Fatal("Can't allocate %u bytes in %s(%d)", size, file, line);
#endif
#if CHECKHEAPLEAKS
	if (heapLeakChecker)
		heapLeakChecker->Add(file, line, p);
#endif
	return p;
}

void*
operator new(size_t size, char* file, int line)
{
	void* p = new char[size];
	if (!p)
#ifdef WINDOWS
		{
		MBox("Error allocating New","");
		file = file;
		line = line;
		exit(255);
		}
#else
		msgMgr->Fatal("Can't allocate %u bytes in %s(%d)", size, file, line);
#endif
#if CHECKHEAPLEAKS
	if (heapLeakChecker)
		heapLeakChecker->Add(file, line, p);
#endif
	return p;
}


#if CHECKHEAPLEAKS
void
operator delete(void* p)
{
	if (heapLeakChecker)
		heapLeakChecker->Delete(p);
	free(p);
}
#endif

