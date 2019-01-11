//	heapleak.hpp		11/15/93
//		interface to object that checks heap leaks

#ifndef HEAPLEAK_HPP
#define HEAPLEAK_HPP

#ifndef DOS_HPP
#include "dos.hpp"
#endif

//	set this define to 1 to turn on heap checking, or 0 for normal
#define CHECKHEAPLEAKS	0

class HeapLeakChecker {
#if CHECKHEAPLEAKS
public:
	HeapLeakChecker() : nEntries(0) {}
	~HeapLeakChecker();

private:
	friend void* operator new(size_t size, char* file, int line);
	friend void* operator new(size_t size);
	friend void operator delete(void* p);
	
	enum { Size = 3000 };

	struct Allocation {
		void*	p;
		char	file[MaxFName + MaxExt + 1];
		int	line;
	} allocations[Size];
	
	void 	Add(char* file, int line, void* p);
	void 	Delete(void* p);
	
	int nEntries;

#endif
};
extern HeapLeakChecker * heapLeakChecker;

#endif
