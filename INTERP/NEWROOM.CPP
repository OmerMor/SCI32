#include <stdio.h>
#include <string.h>

#include "newroom.hpp"
#include "config.hpp"
#include "globals.hpp"
#include "kernel.hpp"
#include "memtype.hpp"
#include "object.hpp"
#include "pmachine.hpp"
#include "resource.hpp"
#include "restype.hpp"

int	currentRoom = 0;

Bool	tracking		= False;
Bool	handsOn		= False;
Bool	trackingFrameOut = False;
Bool	roomFirstFrameOut = False;
int	gameMaxRoom	= 0;
int	gameMaxAloc	= 0;
int	roomMaxAloc	= 0;
int	numRooms		= 0;
int	maxRooms		= 100;

struct MaxAloc {
	ResNum	room;
	int		aloc;
};
MaxAloc	*maxAloc;
MemID		maxAlocID;

RmResList* rmResList;

#ifdef  DEBUG

RmResList::RmResList(unsigned listSize)
{
	char	c;
	rmResCount = 0;
	rmChange = False;
	room		= -1; // Initialize to impossible room number 

	if (*configMgr->Get("tracking", 0))  {
	   strcpy(buf,configMgr->Get("tracking",0));
		c = buf[strlen(buf)-1];
  		if (c != '/' && c != '\\' && c != ':')
			strcat(buf,"\\");	
		tracking = True;
	} else
		tracking = False;

	theList = New resourceInfo[listSize];
	if (configMgr->Get(configMgr->RoomMinK))
		maxAlocID.Get(MemResourceList,maxRooms*sizeof(MaxAloc));
}

RmResList::~RmResList()
{
	if (configMgr->Get(configMgr->RoomMinK)) {
		maxAloc = (MaxAloc*)*maxAlocID;
		for (int i = 0; i < numRooms; i++)
			msgMgr->Dump("Room#%-5d: %d\n",maxAloc[i].room,maxAloc[i].aloc);
		maxAlocID.Free();
	}
	delete [] theList;
}

void
RmResList::Read()
{
	FILE *stream;
	int  len, num;
	char buff[81];
	char typeStr[13];
	char handsStr[40];

	sprintf(buff,"%s%d.TRK",buf,room);
  
	if ((stream = fopen(buff,"r+t")) == NULL)
		return;

	Bool saveHandsOn = handsOn;
	while (fgets(buff,80,stream) != NULL) {
		if (buff[0] == ';') {
			if (!strncmp(buff,"; First Frame Out",17))
				trackingFrameOut = True;
			continue;
		}
		sscanf(buff,"%s %d %d %s",typeStr,&num,&len,handsStr);
		if (handsStr[0] != 'H')
			continue;
		MemType type = GetMemTypeFromStr(typeStr);
		if(type != -1) {
			handsOn = !stricmp(handsStr,"HANDS_ON");
			rmResList->AddEntry((MemType)type,(ResNum)num,len);
		}
	}
	handsOn = saveHandsOn;
	fclose(stream);
	rmChange = False;
	return;
}

void
RmResList::Add(MemType resType, ResNum resourceID, int length)
{
	if (room == -1)
		return;
	if (!handsOn && room != 0) {
//BEW CHANGED		ObjectID user = pm.Global(g_user);
		ObjectID user = pmGlobal(g_user);
		if (user.IsObject() && user.GetProperty(s_input))
			handsOn = True;
	}
	rmResList->AddEntry(resType, resourceID, length);
}

void
RmResList::AddEntry(MemType resType, ResNum resourceID, int length)
{
	if (room == -1)	//
		return;

	// Check for duplicates
	for(int i = 0;  i < rmResCount; i++) {
		if(theList[i].resourceType == resType &&
				theList[i].resourceID == resourceID)
			return;
	}

	// add it
	theList[rmResCount].resourceType = resType;
	theList[rmResCount].resourceID = resourceID;
	theList[rmResCount].resourceLen = length;
	theList[rmResCount].handsOn = handsOn;
	if (!roomFirstFrameOut && trackingFrameOut) {
		theList[rmResCount].resourceLen |= 0x80000000;
		roomFirstFrameOut = True;
	}
	++rmResCount;
	rmChange = True;
}

void
RmResList::Print()
{
	FILE *stream;
	char buff[80];
	int  num;

	if (room == -1)
		return;

	sprintf(buff,"%s%d.TRK",buf,room);

   if ((stream = fopen(buff,"w+t")) == NULL)
		return;

	for(num = 0; num < rmResCount; num++) {
		if (theList[num].resourceLen & 0x80000000) {
			fprintf(stream,"; First Frame Out\n");
			theList[num].resourceLen &= 0x7fffffff;
		}
		fprintf(stream,"%-12s %-5u %7u HANDS_%s\n",
			GetMemTypeStr((MemType)theList[num].resourceType),
			theList[num].resourceID, theList[num].resourceLen,
			theList[num].handsOn?"ON":"OFF");
	}
	fclose(stream);
}

#endif

void
RmResList::NewRoom(ResNum roomNum)
{
#ifdef  DEBUG
   if (!tracking) {
#endif
		if (resMgr->Check(MemResChunk, roomNum))
			resMgr->Get(MemResChunk, roomNum);
#ifdef  DEBUG
	} else {
		if (rmChange)
			rmResList->Print();
		room = roomNum;
		rmResCount = 0;
		roomFirstFrameOut = False;
		trackingFrameOut = False;
		rmResList->Read();
	}
	handsOn = False;
	if (configMgr->Get(configMgr->RoomMinK) && roomNum != 0) {
		maxAloc = (MaxAloc*)*maxAlocID;
		int i;
		for (i = 0; i < numRooms; i++)
			if (maxAloc[i].room == roomNum)
				break;
		if (i == numRooms) {
			if (i == maxRooms) {
				maxAlocID.Realloc((maxRooms += 10) * sizeof(MaxAloc));
				maxAloc = (MaxAloc*)*maxAlocID;
			}
			numRooms++;
			maxAloc[i].room = roomNum;
			maxAloc[i].aloc = roomMaxAloc;
		} else if (roomMaxAloc < maxAloc[i].aloc)
			maxAloc[i].aloc = roomMaxAloc;
		if (roomMaxAloc > gameMaxAloc) {
			gameMaxAloc = roomMaxAloc;
			gameMaxRoom = currentRoom;
		}
		roomMaxAloc = 0;
	}
#endif
	currentRoom = (int)roomNum;
}

///////////////////////////////////////////////////////////////////////////////

void
KNewRoom(argList)
{
	rmResList->NewRoom((ResNum)arg(1));
}



