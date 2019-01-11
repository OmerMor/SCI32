//	memmgre.cpp

#ifdef DEBUG

#include <string.h>

#include "sol.hpp"

#include "debug.hpp"
#include "event.hpp"
#include "graph.hpp"
#include "graphmgr.hpp"
#include "integ.hpp"
#include	"kernel.hpp"
#include "memmgr.hpp"
#include "mouse.hpp"
#include "palmgr.hpp"
#include "window.hpp"


//***************************************************************************

unsigned
MemoryMgr::CheckVolatile(Bool set)
{
	// When this routine is called with True an error condition arises when
	// ever a volatile memory management method is invoked; False obviates
	// the last True. Checking ceases only when each True call has been
	// canceled by a corresponding False call.

	if (set)
		checkVolatile++;
	else if (checkVolatile)
		checkVolatile--;
	return checkVolatile;
}

void
MemoryMgr::VolatileMethod(char* where)
{
	if (!msgMgr->Alert("Volatile memory method invoked(%s)",where?where:""))
		EnterDebug();
}

//***************************************************************************

void
MemoryMgr::IntegrityChecking()
{
	static int oldCheck = 0;
	SOL_Handle id;

	if ((checkIntegrity & CHECK_CHECKSUMS) && !(oldCheck & CHECK_CHECKSUMS))
		for (id = mruList; id; id = desc[id].next)
			SetChecksum(id,(Bool)(desc[id].memInfo.attributes & CHECKSUMMABLE));
	if ((checkIntegrity & CHECK_FREE) && !(oldCheck & CHECK_FREE))
		for (id = freeList; id; id = desc[id].next)
			memset(desc[id].data,FREE_MARK_CHAR,desc[id].size);
	oldCheck = checkIntegrity;
}

Bool
MemoryMgr::CheckIntegrity(char* note)
{
	static Bool inCheck = False;

	if (inCheck)
		return True;
	inCheck = True;

	info = note;
	Bool ok = True;

	if (checkIntegrity & CHECK_SIGNATURES)
		ok &= CheckSignatures();
	if (checkIntegrity & CHECK_CHECKSUMS)
		ok &= CheckChecksums();
	if (checkIntegrity & CHECK_FREE)
		ok &= CheckFree();
	if (checkIntegrity & CHECK_LISTS)
		ok &= CheckLists();

	inCheck = False;
	return ok;
}

const Bool
MemoryMgr::ICError(char* error,SOL_Handle id)
{
	if (!msgMgr->Alert(
"%s %s Integrity Error: %s id=%u atr=$%08x num=%u size=%u adr=%u",info,error,
GetMemTypeStr(id),id,GetMemAttrs(id),GetResNumber(id),GetSize(id),GetAddr(id)));
		EnterDebug();
	return False;
}

char*	frees;
char*	useds;
char*	ords;
char	listError[100];

const Bool
MemoryMgr::ICListError(char* error,SOL_Handle id)
{
	delete[] useds;
	delete[] frees;
	delete[] ords;
	strcat(listError,error);
	return ICError(listError,id);
}

//***************************************************************************

void
MemoryMgr::WriteSignature(SOL_Handle id)
{
	strcpy(desc[id].data+desc[id].size-SIGNATURESIZE,signature);
}

void MemoryMgr::EraseSignature(SOL_Handle id)
{
	uint theCount =  Min(desc[id].size,SIGNATURESIZE);
	memset(desc[id].data+desc[id].size-theCount,FREE_MARK_CHAR,theCount);
}

Bool
MemoryMgr::CheckSignatures()
{
	for (SOL_Handle id=mruList; id; id=desc[id].next)	{
		if (strcmp(desc[id].data+desc[id].size-SIGNATURESIZE,signature))
			return ICError("Signature",id);
		if (id != DescriptorTableHandle) {
			if ((desc[id].memInfo.attributes & PERSIST) && id >= RESERVEDHANDLES)
				return ICError("PERSIST should be reserved",id);
			if ((!(desc[id].memInfo.attributes & PERSIST) && id < RESERVEDHANDLES))
				return ICError("Reserved not PERSIST",id);
		}
	}
	return True;
}

//***************************************************************************

#define MEMMGR_IN_LINE_ASM

#ifdef MEMMGR_IN_LINE_ASM

extern "C" int Scasb(int,int,char);
#pragma aux Scasb parm [EDI] [ECX] [AL] = \
 "repe scasb", \
 "mov eax,ecx", \

extern "C" int Scasd(int,int,int);
#pragma aux Scasd parm [EDI] [ECX] [EAX] = \
 "repe scasd", \
 "mov eax,ecx", \

#endif


Bool
MemoryMgr::CheckFree()
{
	for (SOL_Handle id=freeList; id; id=desc[id].next) {
#ifdef MEMMGR_IN_LINE_ASM
		int ofs = (int)desc[id].data;
		int len = desc[id].size;
		int quo = len % sizeof(int);
		// Asm code...
		if (Scasb(ofs + len - quo, quo, FREE_MARK_CHAR) ||
		    Scasd(ofs, len / sizeof(int), FREE_MARK_DWORD)) {
#else
		// C++ code...
		for (int n = 0; n < desc[id].size; n++)
			if (desc[id].data[n] != FREE_MARK_CHAR) {
#endif
				ICError("Free mem",id);
				memset(desc[id].data,FREE_MARK_CHAR,desc[id].size);
				return False;
		}
	}
	return True;
}

Bool
MemoryMgr::CheckChecksums()
{
	// CheckChecksum(id) may move id to the front of the mruList
	moveToFront = False;
//	for (SOL_Handle id=mruList; id; id=desc[id].next) {
for (SOL_Handle id=desc[mruList].next; id; id=desc[id].next) {
		if (!CheckChecksum(id)) {
			moveToFront = True;
			return False;
		}
	}
	moveToFront = True;
	return True;
}

Bool
MemoryMgr::CheckChecksum(SOL_Handle id)
{
	if (desc[id].memInfo.bits.checksummable && desc[id].client) {
		if (desc[id].checksum != desc[id].client->GetChecksum(id)) {
			ICError("Checksum",id);
			SetChecksum(id,True);
			return False;
		}
	}
	return True;
}

void
MemoryMgr::SetChecksum(void *d,Bool set)
{
	if (!(checkIntegrity & CHECK_CHECKSUMS))
		return;
	for (SOL_Handle id=mruList; id; id=desc[id].next)
		if (d >= desc[id].data &&
		     d < desc[id].data + desc[id].size) {
			SetChecksum(id, set);
			break;
		}
}

void
MemoryMgr::SetChecksum(SOL_Handle id,Bool set)
{
	desc[id].memInfo.bits.checksummable = set? True : False;
	if (!(checkIntegrity & CHECK_CHECKSUMS))
		return;
	if (desc[id].memInfo.bits.checksummable && desc[id].client) {
		// message back to the data's manager to do the checksum
		desc[id].checksum = desc[id].client->GetChecksum(id);
	}
}

//***************************************************************************

Bool
MemoryMgr::CheckLists()
{
	//	make sure each list is disjoint and acyclic and that every descriptor
	//	is on one of the list or is a free descriptor

//	const Size = 5000;
	SOL_Handle id;

	frees = New char[maxDescriptors];
	useds = New char[maxDescriptors];
	ords  = New char[maxDescriptors];

	memset(frees,False,maxDescriptors);
	memset(useds,False,maxDescriptors);
	memset(ords,False,maxDescriptors);

	//	scan the free list
	strcpy(listError,"free-list: ");
	if (freeList && desc[freeList].prev)
		return ICListError("first ptr",freeList);
	if (freeListEnd && desc[freeListEnd].next)
		return ICListError("last ptr",freeListEnd);
	for (id = freeList; id; id = desc[id].next) {
		if (!id || id >= maxDescriptors || desc[id].memInfo.bits.freeDescriptor)
			return ICListError("invalid",id);
		if (!desc[id].memInfo.bits.freeMemory)
			return ICListError("marked not free",id);
		if (desc[id].next && desc[desc[id].next].prev != id)
			return ICListError("next link",id);
		if (frees[id])
			return ICListError("cyclic",id);
		frees[id] = True;
	}

	//	scan the used list
	strcpy(listError,"mru-list: ");
	if (mruList && desc[mruList].prev)
		return ICListError("first ptr",mruList);
	if (mruListEnd && desc[mruListEnd].next)
		return ICListError("last ptr",mruListEnd);
	for (id = mruList; id; id = desc[id].next) {
		if (!id || id >= maxDescriptors || desc[id].memInfo.bits.freeDescriptor)
			return ICListError("invalid",id);
		if (desc[id].memInfo.bits.freeMemory)
			return ICListError("marked free",id);
		if (desc[id].next && desc[desc[id].next].prev != id)
			return ICListError("next link",id);
		if (useds[id])
			return ICListError("cyclid",id);
		useds[id] = True;
		if (frees[id])
			return ICListError("free and in-use",id);
	}

	//	scan the ordered list
	Bool highArea = False;
	strcpy(listError,"ordered-list: ");
	if (ordList && desc[ordList].prevO)
		return ICListError("first ptr",ordList);
	if (ordListEnd && desc[ordListEnd].nextO)
		return ICListError("last ptr",ordListEnd);
	for (id = ordList; id; id = desc[id].nextO) {
		if (!id || id >= maxDescriptors || desc[id].memInfo.bits.freeDescriptor)
			return ICListError("invalid",id);
		if (desc[id].nextO) {
			if (desc[desc[id].nextO].prevO != id)
				return ICListError("next link",id);
			if (desc[id].data + desc[id].size > desc[desc[id].nextO].data)
				return ICListError("gap",id);
			if (desc[id].data + desc[id].size < desc[desc[id].nextO].data)
				return ICListError("overlap",id);
			if (desc[id].memInfo.bits.freeMemory &&
				 desc[desc[id].nextO].memInfo.bits.freeMemory)
				return ICListError("adjacent free blocks",id);
		}
		if (!desc[id].memInfo.bits.moveable) {
			if (!highArea) {
				if (desc[id].memInfo.bits.freeMemory)
					return ICListError("first high block marked free",id);
				highArea = True;
			}
		} else if (highArea)
			return ICListError("high block marked moveable or vice-versa",id);
		if (ords[id])
			return ICListError("cyclic",id);
		ords[id] = True;
		if (!frees[id] && !useds[id])
			return ICListError("not free or in-use",id);
	}

	//	make sure all handles are accounted for
	strcpy(listError,"descriptor-table: ");
	for (id = 1; id < maxDescriptors; id++) {
		if (!desc[id].memInfo.bits.freeDescriptor) {
			if (!useds[id] && !frees[id])
				return ICListError("neither free nor in-use",id);
			if (!ords[id])
				return ICListError("not ordered",id);
		} else {
			if (useds[id])
				return ICListError("unallocated in use",id);
			if (frees[id])
				return ICListError("unallocated in free",id);
			if (ords[id])
				return ICListError("unallocated in order",id);
			; // this empty statement needed to prevent compile-time warning
			  // (erroneous diagnostic is 'Unreachable code' for the lines below) 
		}
	}

	delete[] useds;
	delete[] frees;
	delete[] ords;
	return True;
}

//***************************************************************************

const		rows = 80;
const 	cols = 100;
const 	indexXPixels = 3;
const 	indexYPixels = 2;
const 	arrayPixels = rows*cols;
size_t	BMSize;
size_t	memPerIndex;

char* lowHighMemory = 0;
char* highLowMemory = 0;

SOL_Handle
MemoryMgr::MemoryBitMap()
{
	// returns a bitmap showing the memory allocation display

	// calculate the basic colors
	uchar red = graphMgr->GPalette().Current().Match(Rgb24(255,0,0));
	uchar green = graphMgr->GPalette().Current().Match(Rgb24(0,255,0));
	uchar blue = graphMgr->GPalette().Current().Match(Rgb24(0,0,255));

	BMSize = (rows+1)*cols*indexXPixels*indexYPixels + CELHEADERSIZE;
	// Allocate the memory for the bitmap
	MemID BM = Alloc(MOVEABLE+MemResBitmap,BMSize,0,0);

	// build the header for the BM
	BuildHeader(BM,cols*indexXPixels,(rows+1)*indexYPixels,SOL_SKIP);

	char* BMAddr = (char*) GetAddr(BM);
	// Fill the bitmap with SOL_BLACK
	memset(BMAddr + CELHEADERSIZE,SOL_BLACK,BMSize - CELHEADERSIZE);
	memPerIndex = (totalMemory + arrayPixels - 1)/arrayPixels;

	// Go through the ordered list to find the lowHighMemory
	SOL_Handle nextID = ordList;
	while (nextID) {
		if (!desc[nextID].memInfo.bits.freeDescriptor	&&
			 !desc[nextID].memInfo.bits.freeMemory			&&
			 !desc[nextID].memInfo.bits.moveable)
				break;
		nextID = desc[nextID].nextO;
	}
	if (nextID) 
		lowHighMemory = desc[nextID].data;
	else
		lowHighMemory = memoryBase + totalMemory;


	// Go through the free list
	SOL_Handle freeID = freeList;
	while (freeID) {
		FillMemBM(BMAddr + CELHEADERSIZE,
					 desc[freeID].data,
					 desc[freeID].size,
					 SOL_WHITE,
					 cols);
		freeID = desc[freeID].next;
	}

	// Go through the mruList
	nextID = mruList;
	while (nextID) {
		uchar color;
 		if (desc[nextID].memInfo.bits.discardable)
			color = green;
		else
			color = red;
		FillMemBM(BMAddr + CELHEADERSIZE,
					 desc[nextID].data,
					 desc[nextID].size,
					 color,
					 cols);
		nextID = desc[nextID].next;
	}

	// Put in the lowHighMemory marker
	FillMemBM(BMAddr + CELHEADERSIZE,
				 lowHighMemory,
				 cols*memPerIndex,
				 blue,
				 0);

	// Show any marked memory
	nextID = mruList;
	while (nextID) {
 		if (desc[nextID].memInfo.bits.markedMemory) {
			int special;
 			if (!desc[nextID].memInfo.bits.discardable)
				special = 1;
			else
				special = 2;
			FillMemBM(BMAddr + CELHEADERSIZE,
					 	desc[nextID].data,
					 	desc[nextID].size,
					 	0,
					 	cols,
					 	special);
		}
		nextID = desc[nextID].next;
	}

	// done
	return BM;
}

void
MemoryMgr::FillMemBM(char* target,char* start,size_t size,uchar color,int bump,int special)
{
	// calculate the start and end indices
	int startIndex = (start - memoryBase)/memPerIndex;
	int endIndex = (start + size + memPerIndex - 2 - memoryBase)/memPerIndex;
	if (start >= lowHighMemory) {
		// This memory is in the locked area
		startIndex += bump;
		endIndex += bump;
	}
	// Now fill the memory with the given color
	for (;startIndex <= endIndex;++startIndex) {
		FillIndex(target,startIndex,color,special);
	}
}

void
MemoryMgr::FillIndex(char* target,int startIndex,uchar color,int special)
{
	// Color rules:
	// If the color is SOL_BLACK then replace with the new color 
	// If the color is not SOL_BLACK then replace a mix with the new color 

	// Get the current color for this index
	char* startPtr = target + 
						  (startIndex/cols)*cols*indexXPixels*indexYPixels +
						  (startIndex%cols)*indexXPixels;
	uchar oldColor = *startPtr;

	// Determine the color for this index
	if ((oldColor != SOL_BLACK) && (!special)) {
		Rgb24 rgb1 = (Rgb24) graphMgr->GPalette().Current()[color];
		Rgb24 rgb2 = (Rgb24) graphMgr->GPalette().Current()[oldColor];
		rgb1.red = (rgb1.red + rgb2.red)/2;
		rgb1.green = (rgb1.green + rgb2.green)/2;
		rgb1.blue = (rgb1.blue + rgb2.blue)/2;
		color = graphMgr->GPalette().Current().Match(rgb1);
	}

	// Fill the index with the color
	int i;
	switch(special) {
		case 0:
			for (i = 0;i < indexYPixels;++i) {
				// This must agree with indexXPixels
				startPtr[0] = color;
				startPtr[1] = color;
				startPtr[2] = color;
				startPtr += cols*indexXPixels;
			}
			break;
		case 1:
			// Not discardable
			startPtr[0] = SOL_BLACK;
			startPtr[1] = SOL_WHITE;
			startPtr[2] = SOL_BLACK;
			startPtr += cols*indexXPixels;
			startPtr[0] = SOL_WHITE;
			startPtr[1] = SOL_BLACK;
			startPtr[2] = SOL_WHITE;
			break;
		case 2:
			// Discardable
			startPtr[0] = SOL_WHITE;
			startPtr[1] = SOL_BLACK;
			startPtr[2] = SOL_WHITE;
			startPtr += cols*indexXPixels;
			startPtr[0] = SOL_BLACK;
			startPtr[1] = SOL_WHITE;
			startPtr[2] = SOL_BLACK;
			break;
	}
}

void
KMarkMemory(argList)
{
	// Input
	//      arg1-----the memory handle
	//      arg2-----True to mark, False to unmark

	if (arg(2))
		memMgr->MarkMemory((SOL_Handle) arg(1));
	else
		memMgr->UnMarkMemory((SOL_Handle) arg(1));
}

void
MemoryMgr::ShowBitmaps()
{
	Plane* plane = NULL;
	ScreenItem* si = NULL;

	int width = SCIRESX - 50 * XSCALE;
	int height = SCIRESY - 30 * YSCALE;

	for (int i=RESERVEDHANDLES;i<maxDescriptors; ++i)	{
		if(!desc[i].memInfo.bits.freeDescriptor && !desc[i].memInfo.bits.freeMemory) {
			if (desc[i].memInfo.type == MemBitmap)	{
				if (si == NULL) {
					// HOOK
					// Why do non bitmaps have a MemBitmap type!
					CelHeader*  celPtr = (CelHeader*) desc[i].data;
					if (celPtr->colorOffset != CELHEADERSIZE)
						continue;

					// Create a plane to show the bitmaps
					plane = New Plane(SOL_Rect(0,0,width,height));
					graphMgr->AddPlane(plane);

					// A screen item for the plane to hold the bitmap
					Bitmap bitmap((MemID) i);
					SOL_Point pt(0,0);
					ScaleInfo scaleInfo;
					si = New ScreenItem(plane->Id(),bitmap,pt,scaleInfo);

					graphMgr->AddScreenItem(si);
				}
				else {
					// HOOK
					// Why do non bitmaps have a MemBitmap type!
					CelHeader*  celPtr = (CelHeader*) desc[i].data;
					if (celPtr->colorOffset != CELHEADERSIZE)
						continue;

					si->bitmap.SetHandle((MemID) i);
					graphMgr->UpdateScreenItem(si);

				}
				graphMgr->FrameOut();
				EventWindow* w = NULL;
				if (WaitOnUser(w))
					break;
			}
		}
	}
	if (plane != NULL) {
		graphMgr->DeletePlane(plane);
		graphMgr->FrameOut();
	}
}

void
MemoryMgr::ShowText(int from,int to)
{
	TextID	buf;
	int width = SCIRESX - 50 * XSCALE;
	Bool escape = False;
	from = Max(from,RESERVEDHANDLES);
	to = Min(to,maxDescriptors);

	for (int maxUsedHandle=maxDescriptors;RESERVEDHANDLES < maxUsedHandle;--maxUsedHandle)	{
		if (desc[maxUsedHandle].memInfo.type == MemText)
			break;
	}

	for (int i=from;i<=to;++i)	{
		if(!desc[i].memInfo.bits.freeDescriptor && !desc[i].memInfo.bits.freeMemory) {
			if (desc[i].memInfo.type == MemText)	{
				buf.Free();
				buf.AddF("TEXT DUMP HANDLE %d of max text handle %d",i,maxUsedHandle);
				DebugWindow(desc[i].data,*buf,width,SOL_Point(0,0),True,&escape);
				if (escape) break;
			}
		}
	}
}

#endif

