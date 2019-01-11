#if !defined(NEWW_HPP)
#define NEWW_HPP

void	InitLocalCache(void);
void*	AllocInLocal(size_t size);
void	InitMemoryCache(void);
void*	AllocInTables(size_t size,char* file,int line);
void	DisplayStats(int cacheNum);

// define header size and size of local cache
#define HEADERSIZE	4
#define LOCALSIZE		600

// define cache IDs
#define NEW_CACHE_ONE		1
#define NEW_CACHE_TWO		2
#define NEW_CACHE_THREE		3
#define NEW_CACHE_LOCAL		6
#define NEW_CACHE_RESERVE	7
#define NEW_CACHE_MEMMGR	9

extern	int reserveSize;
extern	int cacheSize1;
extern	int entrySize1;
extern	int cacheSize2;
extern	int entrySize2;
extern	int cacheSize3;
extern	int entrySize3;

extern	void	*newCacheAddr;
extern	int	newState;

enum	newStateVals {
	NEW_FIRST_CALL,
	NEW_PRE_MEM_CACHE,
	NEW_MEM_CACHE_AVAIL,
	NEW_USE_MEM_CACHE
};


#endif

