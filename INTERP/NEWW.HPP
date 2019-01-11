#if !defined(NEWW_HPP)
#define NEWW_HPP

void	InitLocalCache(void);
void*	AllocInLocal(size_t size);
void	InitMemoryCache(void);
void*	AllocInTables(size_t size,char* file,int line);
void	DisplayStats(int cacheNum);

// define entry size and number of entries for each cache
#define HEADERSIZE	4
#define LOCALSIZE		600
#define RESERVESIZE	35000
#define CACHESIZE1	200
#define ENTRYSIZE1	32
#define CACHESIZE2	300
#define ENTRYSIZE2	512
#define CACHESIZE3	30
//#define ENTRYSIZE3 1024
#define ENTRYSIZE3	1200

#define NEWCACHESIZE RESERVESIZE + \
						CACHESIZE1 * (ENTRYSIZE1 + HEADERSIZE) + \
						CACHESIZE2 * (ENTRYSIZE2 + HEADERSIZE) + \
						CACHESIZE3 * (ENTRYSIZE3 + HEADERSIZE)

// define cache IDs
#define NEW_CACHE_ONE		1
#define NEW_CACHE_TWO		2
#define NEW_CACHE_THREE		3
#define NEW_CACHE_LOCAL		6
#define NEW_CACHE_RESERVE	7
#define NEW_CACHE_MEMMGR	9

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