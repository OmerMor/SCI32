// vocab.cpp

#include "sol.hpp"

#include "resource.hpp"
#include	"vocab.hpp"

int
GetVocabStr(int moduleNum, int entryNum, char* dest)
{
	// Get a handle to the appropriate resource.
	MemID dh = resMgr->Get(MemResVocab, moduleNum);
	char* dp = (char*) *dh;

	// Value check the requested entryNum (first int of resource contains
	// the maximum directory entry).
	if (entryNum > *(SCIWord*) dp)
		return 0;

	// Get a pointer to the data by looking up its offset in the directory.
	int ofs = ((SCIWord*) dp)[entryNum + 1];
	dp += ofs;

	// Get the length of the data.
	int length = *(SCIWord*) dp;
	dp += 2;

	// Copy the data to its destination.
	for (int i = length; i > 0 ; --i)
		*dest++ = *dp++;
	*dest = 0;

	// Return the length of the data.
	return length;
}
