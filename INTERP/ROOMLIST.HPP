#ifndef ROOMRES_LIST
#define ROOMRES_LIST

#include "sol.hpp"


#ifndef MEMTYPE_HPP
#include "memtype.hpp"
#endif

#ifndef RESTYPE_HPP
#include "restype.hpp"
#endif


const unsigned RmResListSize = 500;

typedef _Packed struct {
	MemType resourceType;
	ResNum resourceID;
} resourceInfo;

class RmResList 
{
	resourceInfo * theList;
	unsigned rmResCount;
	int 		rmChange;		// has the resource list for the room changed 
	int		room;	  			// which room
	char     buf[80];
public:
	RmResList(unsigned listSize = RmResListSize);
	~RmResList();
	void Add(MemType resourceType,ResNum resourceID);
	void Print();
	void Read();
	void NewRoom(int roomNum);
	void InitList();
};
extern RmResList* rmResList;

#endif



