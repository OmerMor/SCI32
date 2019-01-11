#include "roomlist.hpp"
#include "memtype.hpp"
#include "config.hpp"
#include "kernel.hpp"
#include "resource.hpp"
#include "restype.hpp"
#include <stdio.h>
#include <string.h>
#ifdef  DEBUG

extern "C" {void	 qsort( void *__base, size_t __nmemb, size_t __size,
		    int (*__compar)( const void *, const void * ) ); };

RmResList* rmResList;

RmResList::RmResList(unsigned listSize)
{
   int len;
	rmResCount=0;
	rmChange = 0;
	room		= -1; // Initialize to impossible room number 

   strcpy(buf,configMgr->Get("tracking",0));
	len = strlen(buf);
   if (buf[len-1] != '/')
		strcat(buf,"/");	

	theList = new resourceInfo[listSize];
}

RmResList::~RmResList()
{
	delete [] theList;
}

void RmResList::InitList()
{

	for(int i=0;i<RmResListSize;i++) {
		theList[i].resourceType = NotFound;
		theList[i].resourceID   = 0;
	}

	rmResCount = 0;
}

void RmResList::Add(MemType resType, ResNum resourceID)
{
	if (room == -1)	//
		return;

	// Check for duplicates
	for(int i = 0;  i < rmResCount; i++) {
		if(theList[i].resourceType == resType && theList[i].resourceID == resourceID)
			return;
	}


	++rmChange;
	// add it
	theList[rmResCount].resourceType = resType;
	theList[rmResCount].resourceID = resourceID;
	++rmResCount;
}

int 
CompareRmList(const void * ptr1, const void * ptr2)
{
	resourceInfo * p1 = (resourceInfo *) ptr1;
	resourceInfo * p2 = (resourceInfo *) ptr2;

	if((*p1).resourceType < (*p2).resourceType)
		return -1;
	else if((*p1).resourceType > (*p2).resourceType)
		return 1;

	if((*p1).resourceID < (*p2).resourceID) 
		return -1;
	else if((*p1).resourceID > (*p2).resourceID)
		return 1;

	return 0;

}


void RmResList::Read()
{
	FILE *stream;
	char buffer[20];
	int  num;
	char buff[80];

	itoa(room,&buffer[0],10);
	num = strlen(buffer);
	buffer[num] =   '.';
	buffer[num+1] = 't';
	buffer[num+2] = 'r';
	buffer[num+3] = 'k';
	buffer[num+4] = 0;

   strcpy(buff,buf);
   strcat(buff,buffer);
  
	if ((stream = fopen(buff,"r+t")) == NULL)
		return;

	while (fscanf(stream,"\n %s %d",buff,&num) != EOF) {

		MemType type ; // make type some large unused number
		type = GetMemTypeFromStr(buff);


		if(type != -1) 
			rmResList->Add((MemType)type,(ResNum)num);
	}

   if (stream != NULL){
      fclose(stream);
   }
	rmChange = 0;
}

void RmResList::Print()
{
	FILE *stream;
	char buffer[8];
	char buff[80];
	int  num;

	if (room == -1)
		return;

	itoa(room,&buffer[0],10);
	num = strlen(buffer);
	buffer[num]   = '.';
	buffer[num+1] = 't';
	buffer[num+2] = 'r';
	buffer[num+3] = 'k';
	buffer[num+4] = 0;

   strcpy(buff,buf);
   strcat(buff,buffer);

   if ((stream = fopen(buff,"w+t")) == NULL)
		return;

	qsort(theList,rmResCount,sizeof(resourceInfo),CompareRmList);

	for(num=0;num<rmResCount;num++)
	{
		fprintf(stream,"\n %-12s %u",GetMemTypeStr((MemType)theList[num].resourceType),theList[num].resourceID);
	}

   if (stream != NULL){
      fclose(stream);
   }
}

void RmResList::NewRoom(int roomNum)
{

	if (rmChange)
		rmResList->Print();

	room = roomNum;
	rmResList->InitList();

	rmResList->Read();
}
#endif

void KResourceTrack(argList)
{
	int i = arg(1);  // This is a dummy to satisfy the compiler if !DEBUG
#ifdef  DEBUG
   if (strcmp("",configMgr->Get("tracking",0)))
		rmResList->NewRoom(arg(1));
#endif

}



