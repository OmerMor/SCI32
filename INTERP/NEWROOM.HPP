#ifndef NEWROOM_HPP
#define NEWROOM_HPP

#include "sol.hpp"

#ifndef MEMTYPE_HPP
#include "memtype.hpp"
#endif

#ifndef RESTYPE_HPP
#include "restype.hpp"
#endif

extern Bool		tracking;
extern Bool		trackingFrameOut;

const unsigned RmResListSize = 500;

typedef _Packed struct {
	MemType	resourceType;
	ResNum	resourceID;
	int		resourceLen;
	Bool		handsOn;
} resourceInfo;

class RmResList 
{
	resourceInfo * theList;
	unsigned rmResCount;
	Bool 		rmChange;		// has the resource list for the room changed 
	int		room;	  			// which room
	char     buf[80];
public:
	RmResList(unsigned listSize = RmResListSize);
	~RmResList();
	void Add(MemType resourceType,ResNum resourceID,int length);
	void NewRoom(ResNum roomNum);
	void Print();
	void Read();
	void InitList();
private:
	void AddEntry(MemType resourceType,ResNum resourceID,int length);
};
extern RmResList* rmResList;

extern	int	roomMaxAloc;
extern	int	gameMaxAloc;
extern	int	gameMaxRoom;
extern	int	currentRoom;

#endif
