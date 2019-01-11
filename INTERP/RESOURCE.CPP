//	resource.cpp

#ifdef WIN32S
#include "sciwin.hpp"
#endif

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef DEBUG
#include <sys\stat.h>
#endif

#include "sol.hpp"

#include "audio.hpp"
#include "config.hpp"
#include "cursord.hpp"
#include "dcomp.hpp"
#include "dos.hpp"
#include "graphmgr.hpp"
#include "integ.hpp"
#include "kernel.hpp"
#include "memmgr.hpp"
#include "msg.hpp"
#include "newroom.hpp"
#include "pmachine.hpp"
#include "resource.hpp"
#include "savegame.hpp"
#ifdef WIN32S
#include "shiftpal.hpp"
#endif
#include	"sound.hpp"
#include "stampver.hpp"
#include "sync.hpp"
#include	"textid.hpp"

int				altVolFD;
MemID 			alternateMap;
int				altDirNum;
int				discNum = -1;
char				discPath[MaxPath+1];
int				preVolLength;
int				resVolFD;
int				resVolFDPre;
MemID 			resourceMap;
ResourceMgr*	resMgr;
MemID				resPatches;
MemID				resPatches36;

const	NOT_FOUND = 0;
const	RESOURCE_VOLUME = 4;
const	ALTERNATE_VOLUME = 2;
const	AUDIO = 1;
const	PATCH = 4;

#ifdef DEBUG
Bool	loadingDiskCursor = False;
Bool	unLoadingDiskCursor = False;
#endif

FakeTemplateID(FakeResTagID, ResTag, MemResourceList)
struct ResTagID : FakeResTagID
{
	ResTagID() {}
	ResTagID(MemID& id) : FakeResTagID(id) {}
	ResTag&	operator[](int s) { return (**this)[s]; }
};

struct ResourceList {
	ResourceList(SOL_ResType* obj, ReservedHandle handle = IllegalHandle,
			int chunkSize = 1) :
		obj(obj), chunkSize(chunkSize), curSize(0), maxSize(chunkSize),
		handle(handle)
	{
	}
	
	void
	Get()
	{
		if (handle)
			id.Get(sizeof(ResTag) * chunkSize, handle, INITMEMORY | MOVEABLE);
	}
	
	void
	MakeDebugSummaryStr(TextID& buf)
	{
		if (!obj->IsSupported() || !curSize)
			return;

		ResTag* resTags = *id;
		unsigned total = 0;
		for (int i = 0; i < curSize; i++)
			total += resTags[i].id.Size();
		buf.AddF("%s: %u ", obj->GetExtension() + 1, total);
	}
	
	void
	MakeDebugDetailStr(TextID& buf)
	{
		if (!obj->IsSupported() || !curSize)
			return;

		buf.AddF("%s: ", obj->GetExtension() + 1);

		for (int i = 0; i < curSize; i++) {
			MemAttrs attrs = id[i].id.Attrs();
			buf.AddF("%s%s%s%u ",
				!(attrs & DISCARDABLE) ? "*" : "",
				!(attrs & MOVEABLE) ? "^" : "",
				attrs & CRITICAL ? "!" : "",
				id[i].resNum
			);
		}
		
		buf.Add("\r\n");
	}

	void	Free()
	{
		delete obj;
		id.Free();
	}

	SOL_ResType*	obj;
	ResTagID			id;			// The id for the array for this type of resource
	int				curSize;
	int				chunkSize;
	int				maxSize;		// The size of the resource array for this type
	ReservedHandle	handle;

} static resources[] = {

	ResourceList(New ResView, ViewResListHandle, 10),
	ResourceList(New ResPic, PicResListHandle),
	ResourceList(New ResScript, ScriptResListHandle, 10),
	ResourceList(New SOL_ResType),				// OBJECT
	ResourceList(New ResSound, SoundResListHandle),
	ResourceList(New SOL_ResType),				// MEM (obsolete)
	ResourceList(New ResVocab, VocabResListHandle),
	ResourceList(New ResFont, FontResListHandle),
	ResourceList(New SOL_ResType),				// CURSOR
	ResourceList(New ResPatch, PatchResListHandle),
	ResourceList(New SOL_ResType),				// BITMAP
	ResourceList(New ResPal, PalResListHandle),
	ResourceList(New ResWave, WaveResListHandle),
	ResourceList(New ResAudio, AudioResListHandle),
	ResourceList(New ResSync, SyncResListHandle),
	ResourceList(New ResMsg, MsgResListHandle),
	ResourceList(New ResMap, MapResListHandle),
	ResourceList(New ResHeap, HeapResListHandle, 10),
   ResourceList(New ResChunk, ChunkResListHandle),
	ResourceList(New ResAudio36, Audio36ResListHandle),
	ResourceList(New ResSync36, Sync36ResListHandle),
	ResourceList(New ResMsgTranslation, MsgTranslationResListHandle),
	ResourceList(New ResRobot, RobotResListHandle)
};

const nResourceTypes = sizeof(resources) / sizeof(*resources);
const	int defaultDecompBufferSize = 10240;

//////////////////////////////////////////////////////////////////////////////

inline void
SCI16ToSCI32ResType(MemType& t)
{
	t = MemType(t & ~0x80);
}

/////////////////////////////////////////////////////////////////////////////

ResourceMgr::ResourceMgr()
{
	for (int i = 0; i < nResourceTypes; ++i)
		resources[i].Get();
	InitResourceMgr();
}

ResourceMgr::~ResourceMgr()
{
	for (int i = 0; i < nResourceTypes; ++i)
		resources[i].Free();

#ifdef DEBUG
	rmResList->Print();
	delete rmResList;
#endif
}

int
ResourceMgr::Open(MemType type, ResNum num, char* path)
{
	return resources[type].obj->Open(num, path);
}

/*
int
ResourceMgr::Open(MemType resType, ResNum resNum, char* path)
{
	//	SCI still uses the old numbers
	SCI16ToSCI32ResType(resType);

	SOL_ResType*	obj = resources[resType].obj;
	if (!obj->IsSupported())
		msgMgr->Fatal(SrcLoc, Msg_InvalidResType, (int) resType);
	return resources[resType].obj->Open(resNum,path);
}
*/

int
ResourceMgr::Close(int fd)
{
	if (fd != -1 && fd != resVolFD && fd != resVolFDPre && fd != altVolFD &&
			fd != sfxVolFD && fd != audVolFD)
		return ::Close(fd);
	return -1;
}

MemID
ResourceMgr::Get(MemType resType, ResNum resNum, Bool lock)
{
	// either return the MemID or load the resource and return the MemID

	//	SCI still uses the old numbers
	SCI16ToSCI32ResType(resType);

	MemID		id;
	long		attr = 0;
	
	SOL_ResType*	obj = resources[resType].obj;

	if (!obj->IsSupported())
		msgMgr->Fatal(SrcLoc, Msg_InvalidResType, (int) resType);

	// See if already in memory
	if (id = Find(resType, resNum)) {
		if (lock && (id.Attrs() & MOVEABLE))
			id.Lock();
		return id;
	}

#ifdef DEBUG
	// Going external for resource
	int cursorView;
	int cursorLoop;
	int cursorCel;
	if ((graphMgr != NULL)  						&&
		 configMgr->Get(configMgr->DiskCursor) && 
		 !loadingDiskCursor							&&
		 !unLoadingDiskCursor) {
		loadingDiskCursor = True;
		// Save old cursor information
		Bitmap& bitmap = graphMgr->GCursor().Get();
		cursorView = bitmap.View();
		cursorLoop = bitmap.Loop();
		cursorCel = bitmap.Cel();
		graphMgr->GCursor().Set(configMgr->Val(configMgr->DiskCursor),0,0);
		graphMgr->FrameOut();
		loadingDiskCursor = False;
	}
	if (configMgr->Get(configMgr->AudioMonitor) &&
		(activeSamples || graphMgr && graphMgr->RobotStatus() != ROBOT_UNUSED))
		if (!msgMgr->Alert("Loading %d%s",resNum,obj->GetExtension()))
			msgMgr->Fatal("User abort");
#endif

	ResTagID array = resources[resType].id;

	//	make a copy and work with that, in case the array moves
	ResTag resTag = array[AddIndex(resType, resNum)];
	Load(obj, resTag, lock);
	int newIndex = GetIndex(resType, resNum);
	array[newIndex] = resTag;
	id = array[newIndex].id;

#ifdef WIN32S
	if (WING) {
		// Add 10 to the palette index values for views and pics
		if (obj->GetType() == MemResView) 
		 	ShiftPaletteView((uchar*)*id);
		if (obj->GetType() == MemResPic)
			ShiftPalettePic((uchar*)*id);
	}
#endif

	if (obj->GetType() == MemResChunk) {
		SplitChunk(resTag);
		// Don't free the remaining chunk stub -- its presence can serve
		// as a flag to prevent re-loading a 'recent' chunk most of which
		// may still be memory-resident
		id.Realloc(1);
//	   id.Free();
	}

#ifdef DEBUG
	if ((graphMgr != NULL)  						&&
		 configMgr->Get(configMgr->DiskCursor)	&&
		 !loadingDiskCursor							&&
		 (cursorView != -1)) {
		unLoadingDiskCursor = True;
		graphMgr->GCursor().Set(cursorView,cursorLoop,cursorCel);
		graphMgr->FrameOut();
		unLoadingDiskCursor = False;
	}
	id.SetChecksum(id.Attrs() & CHECKSUMMABLE);
#endif
	return id;
}

MemType
ResourceMgr::GetExtType(char* extension)
{
	int	i;

	for (i = 0; i < nResourceTypes; i++) {
		SOL_ResType* obj = resources[i].obj;
		if (!stricmp(obj->GetExtension(),extension))
			return obj->GetType();
	}
	return (MemType)0;
}

Bool
ResourceMgr::Check(MemType resType, ResNum resNum)
{
	// return TRUE if said resource is locateable

	//	SCI still uses the old numbers
	SCI16ToSCI32ResType(resType);

	SOL_ResType* obj = resources[resType].obj;

	if (!obj->IsSupported())
		return False;

	// See if already in memory
	if (Find(resType, resNum))
		return True;

	return Check(obj, resNum);
}

void
ResourceMgr::Lock(MemType resType, ResNum resNum)
{
	// See if the given resource is in memory

	MemID id = Find(resType, resNum);
	if (id)
		id.Lock();

}

void
ResourceMgr::SetNotDiscardable(MemType resType, ResNum resNum)
{
	// See if the given resource is in memory

	MemID id = Find(resType, resNum);
	if (id)
		id.SetNotDiscardable();

}

void
ResourceMgr::SetDiscardable(MemType resType, ResNum resNum)
{
	// See if the given resource is in memory

	MemID id = Find(resType, resNum);
	if (id)
		id.SetDiscardable();

}

TextID
ResourceMgr::MakeDebugDisplayStr()
{
	TextID	buf;
	int		i;
	
	for (i = 0; i < nResourceTypes; i++)
		resources[i].MakeDebugSummaryStr(buf);

	buf.Add("\n\n");
	
	for (i = 0; i < nResourceTypes; i++)
		resources[i].MakeDebugDetailStr(buf);

	return buf;
}

void
ResourceMgr::Unlock(MemType resType, ResNum resNum)
{
	// See if the given resource is in memory

	MemID id = Find(resType, resNum);
	if (id)
		id.Unlock();

}

void
ResourceMgr::Release(MemType resType, ResNum resNum)
{
	MemID id = Find(resType, resNum);
	if (id)
		id.Free();
}

MemID
ResourceMgr::Find(MemType resType, ResNum resNum)
{
	// See if the given resource is in memory

	SCI16ToSCI32ResType(resType);
	int i = GetIndex(resType, resNum);
	ResTag* array = *resources[resType].id;
	if (array[i].resNum == resNum)
		return array[i].id;
	else
		return 0;
}

char*
ResourceMgr::MakeName(MemType type, ResNum num)
{
	static char buf[MaxPath + 1];
	resources[type].obj->MakeName(buf, 0, num);
	return buf;
}

void
ResourceMgr::Free(MemType resType, unsigned resNum)
{
	//	(callback from memory manager)

	DeleteIndex(resType, resNum);
}

#ifdef DEBUG

#define RESMGR_IN_LINE_ASM

#ifdef RESMGR_IN_LINE_ASM

extern "C" uchar Checksum(int,int);
#pragma aux Checksum parm [EBX] [EDI] = \
 "dec edi", \
 "xor eax,eax", \
 "mov al,ds:[ebx]", \
 "add al,ds:[ebx+edi]", \
 "dec edi", \
 "jnz -6"

#endif

uint
ResourceMgr::GetChecksum(SOL_Handle handle)
{
	//	(callback from memory manager)

	if (!(checkIntegrity & CHECK_CHECKSUMS))
		return 0;

	MemID id = (MemID)handle;
	uchar* data = (uchar *)*(id);
	uchar checksum = *data;

#ifdef RESMGR_IN_LINE_ASM
	if (id.Size() > 1)
		// Asm code...
		checksum = Checksum((int)data,id.Size());
#else
	// C++ code...
	for (uint i = id.Size() - 1; i > 0; i--)
		checksum += data[i];
#endif
	return (uint)checksum;
}
#endif

int
ResourceMgr::GetIndex(MemType resType, ResNum resNum)
{
	// Find the index for a resource type and number
	// The index either matches or is the index of the first higher resource

	int		arraySize = resources[resType].curSize;
	ResTag*	array = *resources[resType].id;

	int i = 0;
	if (arraySize > 8) {
		// Binary search
		int k = arraySize;
		while (i != k) {
			int j = (i + k) >> 1;
			if (array[j].resNum == resNum)
				return j;
			if (array[j].resNum > resNum)
				k = j;
			else
				i = j + 1;
		}
		return i;

	} else {
		// Linear search
		for ( ; i < arraySize; ++i) {
			if (array[i].resNum == resNum)
				return i;
			if (array[i].resNum > resNum)
				break;
		}
		return i;
	}
}

int
ResourceMgr::AddIndex(MemType resType, ResNum resNum)
{
	// Add an entry to an array

	// Get the index of the first higher resource
	int i = GetIndex(resType, resNum);

	ResTagID array = resources[resType].id;
	int arraySize = resources[resType].curSize;
	int maxSize	= resources[resType].maxSize;

	if (arraySize == maxSize ) {
		// No room, increase the size of the array
		resources[resType].maxSize += resources[resType].chunkSize;
		array.Realloc(resources[resType].maxSize * sizeof(ResTag));
	}

	if (i < arraySize) {
		// Shift every resListObj down
		int size = (arraySize - i) * sizeof(ResTag);
		memmove((char*) &array[i+1], (char*) &array[i], size);
	}
	array[i].resNum = resNum;

	//Bump up the curSize property of the array
	resources[resType].curSize++;
	return i;
}

void
ResourceMgr::DeleteIndex(MemType resType, ResNum resNum)
{
	// Subtract an entry from a particular array

	// Check that it's in a resArray
	int i = GetIndex(resType, resNum);
	ResTag* array = *resources[resType].id;
	while (array[i].resNum != resNum) {
		if (resType == MemResAudio) {
			resType = MemResWAVE;
			i = GetIndex(resType, resNum);
			array = *resources[resType].id;
			if (array[i].resNum == resNum)
				break;
		}
		msgMgr->Fatal(SrcLoc, Msg_ResFreeErr, (int)resType, (int)resNum);
	}

	int arraySize = resources[resType].curSize;

	// Just move every entry below the index up one position
	int size = (arraySize - i - 1) * sizeof(ResTag);
	memmove((char *) &array[i], (char *) &array[i+1], size);
	// Zero the last element
	array[arraySize-1].resNum = 0;
	array[arraySize-1].id = 0;

	//decrement the curSize property of the array
	resources[resType].curSize--;
}

#ifdef DEBUG
void
ResourceMgr::ExtractResources()
{
	char* str = configMgr->Str(configMgr->ExtractResource);
	char* ext = strchr(str,'.');
	MemType resType = resMgr->GetExtType(ext);
	ResNum resNum = atoi(str);
	if (!str || !ext ||
			(resNum == 0 && str[0] != '0' && str[0] != '*') ||
			(resType == 0 && stricmp(ext,".V56")))
		msgMgr->Fatal("Invalid filename (%s)",str);
	Bool all = str[0] == '*';
	Bool display = True;
	if (all) {
		display = msgMgr->Alert("Press ENTER to display progess or ESC to extract without display...");
		resNum = 0;
	}
	do {
		if (all && !resMgr->Check(resType, resNum))
			continue;
		char	tstr[10];
		sprintf(tstr,"%u%s",resNum,ext);
		MemID resID = resMgr->Get(resType, resNum);
		if (!resID)
			msgMgr->Fatal("Can't allocate memory or read %s",tstr);
		int fd=open(tstr,O_BINARY|O_RDWR|O_CREAT|O_EXCL,S_IWRITE);
		if (fd == -1)
			msgMgr->Fatal("%s already exists or no write-rights to current directory",tstr);
		if (resType != MemResAudio && resType != MemResWAVE) {
			short type = resType;
			if (Write(fd,&type,2) != 2)
				msgMgr->Fatal("Can't write to %s",tstr);
		}
		int len = Write(fd,*resID,resID.Size()-1);
		if (len != resID.Size()-1)
			msgMgr->Fatal("Can't write to %s",tstr);
		Close(fd);
		resID.Free();
		msgMgr->Mono("%s extracted\n",tstr);
		if (display && !msgMgr->Alert("%s extracted",tstr)) {
			if (all && !msgMgr->Alert("Press ENTER to continue without displaying or ESC to quit...\n"))
				break;
			display = False;
		}
		if (!all)
			break;
	} while(++resNum);
}
#endif

void
ResourceMgr::InitResourceMgr()
{
#ifdef DEBUG
	rmResList = New RmResList();
#endif

	strcpy(discPath, configMgr->Get("resCDisc",0));
	if (strlen(discPath)) {
		char* cp = &discPath[strlen(discPath)-1];
		if (*cp != ':' && *cp != '\\')
			strcat(discPath,"\\");
/*
		int attempts = 0;
		do {
			CheckDiscNum(1);
			if (discNum == 1)
				return;
			if (++attempts == 3) {
				puts("First disc not found in the CD-ROM drive.");
				exit(1);
			}
			puts("Insert the first disc in the CD-ROM drive; then press ENTER");
			getch();
		} while(1);
*/
		CheckDiscNum(0);
		return;
	}
	BuildPatchTables();
	LoadResMaps(0);
}

void
ResourceMgr::CheckDiscNum(uint num)
{
	char	discName[MaxPath+1];
	FindBuffer	fileinfo;

	if (strlen(discPath) == 0 || num > 99)
		return;

	char drive[10];
	_splitpath(discPath,drive,0,0,0);
	if (strlen(drive)) {
#ifndef WIN32S
		find_t buffer;
		strcat(drive,"\\*.*");
		int result =_dos_findfirst(drive,_A_VOLID,&buffer);
#else
		GetVolumeInformation(drive,NULL,0,NULL,NULL,0,NULL,0);
#endif
	}

	if (num == 0) {
		sprintf(discName,"%sRESSCI.0??",discPath);
		if (!FirstFile(discName,0,&fileinfo))
			return;
		char* cp = strrchr(fileinfo.name,'.');
		num = atoi(++cp);
	} else {
		sprintf(discName,"%sRESSCI.0%02d",discPath,num);
		if (!FirstFile(discName,0,&fileinfo))
			return;
	}
	if (discNum != num) {
		BuildPatchTables();
		if (LoadResMaps(num))
			discNum = num;
	}
}

Bool
ResourceMgr::LoadResMaps(uint volNum)
{
	char mapName[MaxPath+1], volName[MaxPath+1], str[4];

	if (volNum > 99)
		return False;
	sprintf(str,"%03d",volNum);
	// Load the resource-map and Open the resource-volume if present
	strcpy(mapName, configMgr->Get("resMap",0));
	if (strlen(mapName)) {
		char* cp = &mapName[strlen(mapName)-1];
		if (*cp != ':' && *cp != '\\')
			strcat(mapName,"\\");
	}
	strcpy(volName,mapName);
	strcat(mapName,"RESMAP.");
	strcat(mapName,str);

	resourceMap.Free();
	if (!resourceMap.Load(MemResMap, mapName, PERSIST | MOVEABLE, ResMapHandle))
// some temporary code for backwards compatibility (CFS)...
{
 mapName[strlen(mapName)-10] = '\0';
 strcat(mapName,"RESOURCE.MAP");
 if (!resourceMap.Load(MemResMap, mapName, PERSIST | MOVEABLE, ResMapHandle))
// ...end of temporary code
		return False;
// some temporary code for backwards compatibility (CFS)...
}
// ...end of temporary code

	strcat(volName,"RESSCI.");
	strcat(volName,str);
	if ((resVolFD = ::Open(volName, O_RDONLY | O_BINARY)) == -1) {
// some temporary code for backwards compatibility (CFS)...
 volName[strlen(volName)-10] = '\0';
 strcat(volName,"RESOURCE.000");
 if ((resVolFD = ::Open(volName, O_RDONLY | O_BINARY)) == -1) {
// ...end of temporary code
		resourceMap.Free();
		return False;
	}
// some temporary code for backwards compatibility (CFS)...
}
// ...end of temporary code

	// Open the 'preferred' resource-volume if specified
	preVolLength = 0;
	strcpy(volName, configMgr->Get("resMap",1));
	if (strlen(volName)) {
		char* cp = &volName[strlen(volName)-1];
		if (*cp != ':' && *cp != '\\')
			strcat(volName,"\\");
		sprintf(str,"RESSCI.%03d",volNum);
		strcat(volName,str);
		resVolFDPre = ::Open(volName, O_RDONLY | O_BINARY);
// some temporary code for backwards compatibility (CFS)...
		if (resVolFDPre == -1) {
			volName[strlen(volName)-10] = '\0';
			strcat(volName,"RESOURCE.000");
			resVolFDPre = ::Open(volName, O_RDONLY | O_BINARY);
		}
// ...end of temporary code
		if (resVolFDPre != -1)
			preVolLength = LSeek(resVolFDPre,0,SEEK_END);
	}

	// Find trackable version stamp in resource map file
	ResDirHdrEntry* header = (ResDirHdrEntry *) *resourceMap;
	while ((uchar)header->resType != 0xff)
		++header;
	++header;
	mapStamp = *(int *)header;

	// Load the alternate-resource-map and Open the alternate-resource-volume
	// (per optional patchDir specification)
	alternateMap.Free();
	sprintf(mapName,"RESFLMAP.%s",str);
	sprintf(volName,"RESFLSCI.%s",str);
	for (int dirNum = 0; dirNum <= configMgr->GetNTokens("patchDir"); dirNum++) {
		char buf[MaxPath + 1];
		if (dirNum < configMgr->GetNTokens("patchDir"))
			sprintf(buf, "%s\\%s", configMgr->Get("patchDir",dirNum), mapName);
		else
			strcpy(buf,mapName);
		if (!alternateMap.Load(MemResMap, buf, PERSIST | MOVEABLE, AltResMapHandle))
// some temporary code for backwards compatibility (CFS)...
{
 if (dirNum < configMgr->GetNTokens("patchDir"))
  sprintf(buf, "%s\\ALTRES.MAP", configMgr->Get("patchDir",dirNum));
 else
  strcpy(buf,"ALTRES.MAP");
 if (!alternateMap.Load(MemResMap, buf, PERSIST | MOVEABLE, AltResMapHandle))
// ...end of temporary code
			continue;
// some temporary code for backwards compatibility (CFS)...
}
// ...end of temporary code
		if (dirNum < configMgr->GetNTokens("patchDir"))
			sprintf(buf, "%s\\%s", configMgr->Get("patchDir",dirNum), volName);
		else
			strcpy(buf,volName);
		if ((altVolFD = ::Open(buf, O_RDONLY | O_BINARY)) == -1)
// some temporary code for backwards compatibility (CFS)...
{
 if (dirNum < configMgr->GetNTokens("patchDir"))
  sprintf(buf, "%s\\ALTRES.000", configMgr->Get("patchDir",dirNum));
 else
  strcpy(buf,"ALTRES.000");
 if ((altVolFD = ::Open(buf, O_RDONLY | O_BINARY)) == -1)
// ...end of temporary code
			alternateMap.Free();
// some temporary code for backwards compatibility (CFS)...
 else
  altDirNum = dirNum;
}
// ...end of temporary code
		else
			altDirNum = dirNum;
		break;
	}
	InitAudioVols();
	return True;
}

void
ResourceMgr::BuildPatchTables()
{
	char			filename[MaxPath+1];
	FindBuffer	fileinfo;
	int			alloced, alloced36;
	const int	ALOC = 100, ALOC36 = 10;

	// get the patch tables...
	resPatches.Free();
	resPatches.Get(MemPatchTable, alloced = ALOC * sizeof(ResPatchEntry),
		PatchTableHandle, MOVEABLE);
	resPatches36.Free();
	resPatches36.Get(MemPatchTable, alloced36 = ALOC36 * sizeof(ResPatch36Entry),
		Patch36TableHandle, MOVEABLE);

	// ... and point to the start of them
	ResPatchEntry* entry = (ResPatchEntry *) *resPatches; 
	ResPatch36Entry* entry36 = (ResPatch36Entry *) *resPatches36; 

	// locate all (if any) stand-alone resources...
	for (int dirNum = 0, npatches = 0, npatches36 = 0;
		  dirNum < configMgr->GetNTokens("patchDir"); dirNum++) {
		// ... resource-type by resource-type...
		for (MemType type = MemResFirst; type <= MemResLast; type = MemType(type + 1)) {
			if (!resources[type].obj->GetExtension()[0])
				continue;
			resources[type].obj->
				MakeWildName(filename, configMgr->Get("patchDir",dirNum));
			if (FirstFile(filename,0,&fileinfo)) {
				// ... resource-id by resource-id...
				do {
					// only consider this file if its basename is numeric...
					int	id;
					if ((id = atoi(fileinfo.name)) || fileinfo.name[0] == '0') {
						// found one!
						if (!(npatches % ALOC) && npatches >= ALOC) {
							// increase the patch table size
							resPatches.Realloc(alloced += ALOC * sizeof(ResPatchEntry));
							entry = (ResPatchEntry *) *resPatches + npatches; 
						}
						// skip if duplicate...
						for (ResPatchEntry* t = (ResPatchEntry *) *resPatches;
								t != entry; t++)
							if (t->resType == type && t->resId == id)
								// it's a dupe!
								break;
						if (t == entry) {
							// wasn't found so it's a new resource!
							entry->patchDir = dirNum;
							entry->resType = type;
							entry->resId = id;
							entry++;
							npatches++;
						}
					}
				} while (NextFile(&fileinfo));
			}
		}
		strcpy(filename,configMgr->Get("patchDir",dirNum));
		strcat(filename,"\\????????.???");
		if (FirstFile(filename,0,&fileinfo)) {
			do {
				if (strlen(fileinfo.name) != 12)
					continue;
				strupr(fileinfo.name);
				if (fileinfo.name[0] != 'A' && fileinfo.name[0] != 'S')
					continue;
				if (!(npatches36 % ALOC36) && npatches36 >= ALOC36) {
					// increase the patch36 table size
					resPatches36.Realloc(alloced36 += ALOC36 * sizeof(ResPatch36Entry));
					entry36 = (ResPatch36Entry *) *resPatches36 + npatches36; 
				}
				// skip if duplicate...
				for (ResPatch36Entry* t = (ResPatch36Entry *) *resPatches36;
						t != entry36; t++)
					if (!strcmp(t->resName,fileinfo.name))
						// it's a dup!
						break;
				if (t == entry36) {
					// wasn't found so it's a new resource!
					entry36->patchDir = dirNum;
					strcpy(entry36->resName,fileinfo.name);
					entry36++;
					npatches36++;
				}
			} while (NextFile(&fileinfo));
		}
	}

	if (!npatches)
		resPatches.Free();
	else {
		// flag the end of the patch table...
		entry->resType = (MemType) -1;
		// ...and trim it down to size
		resPatches.Realloc((npatches + 1) * sizeof(ResPatchEntry));
	}

	if (!npatches36)
		resPatches36.Free();
	else {
		// flag the end of the patch table...
		entry36->resName[0] = '\0';
		// ...and trim it down to size
		resPatches36.Realloc((npatches36 + 1) * sizeof(ResPatch36Entry));
	}
}

MemID 
ResourceMgr::DecompBuffer(int requestedSize)
{
	
	if (!decompID) {
		if (!requestedSize)
			return 0;
		decompID.Get(
			MemDecompBuffer,
			decompBufferSize = defaultDecompBufferSize,
			DecompBufHandle,
			MOVEABLE);
		decompWorkBuffer.Get(
			MemDecompBuffer,
			SCRATCHRAM_SIZE,
			DecompWorkHandle,
			MOVEABLE);
	}

	if(requestedSize > decompBufferSize)
		decompID.Realloc(decompBufferSize = requestedSize);
	else if (!requestedSize)
		decompID.Realloc(decompBufferSize = defaultDecompBufferSize);
	return decompID;
}

int
ResourceMgr::FindOffset(MemType resType, ResNum resNum, int& location)
{
	//	SCI still uses the old numbers
	SCI16ToSCI32ResType(resType);

	SOL_ResType*	obj = resources[resType].obj;

	int dirNum = obj->FindPatchEntry(resNum);
	int resOffset = obj->FindDirEntry(resNum, alternateMap);

	if (dirNum != -1 && (resOffset == -1 || altDirNum >= dirNum)) {
		location   = PATCH;
		return(0);
	}
 
	if(resOffset != -1) {
		location   = ALTERNATE_VOLUME;
		return(resOffset);
	}

	// check resource volume...

	if (obj->GetType() == MemResAudio || obj->GetType() == MemResWAVE) {
		location  = AUDIO;
	   return(FindAudEntry(resNum));

	} else if ((resOffset = obj->FindDirEntry(resNum, resourceMap)) != -1) {
		location = RESOURCE_VOLUME;
		return(resOffset);
	}

	// If it's another type of file, we're not interested in preloading
	// it anyway.  This is not necessarily an error though

	location = NOT_FOUND;
	return(0);

}

int
Compare(const void* ptr1, const void* ptr2)
{
	PreLoadObj *p1 = (PreLoadObj *)ptr1;
	PreLoadObj *p2 = (PreLoadObj *)ptr2;

	if ((*p1).volume < (*p2).volume)
		return(-1);
	if ((*p1).volume > (*p2).volume)
		return(1);

   if ((*p1).offset < (*p2).offset)
		return(-1);

   if ((*p1).offset > (*p2).offset)
		return(1);

	return(0);
}

void
ResourceMgr::SplitChunk(ResTag& resTagH)
{
	int num,j;
	int newIndex;
	SOL_ResType*   obj;
	ResTagID array;
	ResTag resTag;
	MemAttrs attrs;
	MemID id;
 
	typedef _Packed struct {
	MemType resourceType;
	ResNum  resourceID;
	int	  fileOffset;
	int	  fileLength;
	}	chkHeaderInfo;

	chkHeaderInfo  header;
   chkHeaderInfo  *headers;

	memcpy(&header,*resTagH.id,sizeof(chkHeaderInfo));
	num    =	(header.fileOffset)/sizeof(chkHeaderInfo);
	headers = (chkHeaderInfo*)malloc((num)*sizeof(chkHeaderInfo));
	memcpy(&headers[0],*resTagH.id,num*sizeof(chkHeaderInfo));

	resTagH.id.SetNotDiscardable();

	for(j=num-1; j>=0; j--) {

		// If fileLength is zero skip this resource
		if (!headers[j].fileLength)
			continue;

		// If its already in memory, discard it and continue loading
	   if (id = Find(headers[j].resourceType, headers[j].resourceID)) {
		  	memMgr->Realloc(resTagH.id,resTagH.id.Size()-headers[j].fileLength);
			continue;
		}
	
    	obj = resources[headers[j].resourceType].obj;		

		if (obj->CheckPatches(headers[j].resourceID)) {
		  	memMgr->Realloc(resTagH.id,resTagH.id.Size()-headers[j].fileLength);
			continue;
		}

		array = resources[headers[j].resourceType].id;

	   // make a copy and work with that, in case the array moves
	   resTag = array[AddIndex(headers[j].resourceType,headers[j].resourceID)];

		attrs =  obj->GetAttrs()  ;
		resTag.id = memMgr->Split(resTagH.id,headers[j].fileOffset,
			attrs|headers[j].resourceType,headers[j].resourceID,resMgr);
		
    	newIndex = GetIndex(headers[j].resourceType,headers[j].resourceID);
   	array[newIndex] = resTag;
	}

	resTagH.id.SetDiscardable();
	free(headers);
}

///////////////////////////////////////////////////////////////////////////////

void
KPreloadResource(argList)
{
	if ((argCount/2) > 100)	{
		msgMgr->Alert("PreLoad has exceeded 100\n");
		return;
	}

	PreLoadObj loadArray[100];

   int iterations = argCount/2;
	int location;

	for (int count=0;count<iterations;count++)	{
		loadArray[count].resType = arg(2*count+1);
		loadArray[count].resNum  = arg(2*count+2);
	   loadArray[count].offset  = resMgr->FindOffset((MemType)loadArray[count].resType,(ResNum)loadArray[count].resNum,location);
		loadArray[count].volume  = location;
	}

	qsort(loadArray,iterations,sizeof(PreLoadObj),Compare);
	
	for (count=0;count<iterations;count++)	{
		pm.acc = (Acc) resMgr->Get((MemType)loadArray[count].resType,(ResNum)loadArray[count].resNum);
	}
}

void
KLoadChunk(argList)
{

      pm.acc = (Acc) resMgr->Get((MemType) MemResChunk, (ResNum) arg(1));
}

void
KLoad(argList)
{
	for (int count = 2; count <= argCount; ++count)
		pm.acc = (Acc) resMgr->Get((MemType) arg(1), (ResNum) arg(count));
}

void
KUnload(argList)
{
	resMgr->Release((MemType) arg(1), (ResNum) arg(2));
}

void
KFlushResources(kArgs)
{
#if 0
	// If we are tracking resource usage, set the room number of the room
	// we are about to enter, then flush all unlocked resources.

	newRoomNum = arg(1);

	if (trackResUse)
		while (!PurgeLast())
			;
#endif
}

void
KLock(argList)
{
	// Lock or UnLock a resource
	// pass resource type, resource number and True or False
	//	to LOCK or UNLOCK

	MemType	resType	= (MemType) arg(1);
	ResNum	resId		= arg(2);
	Bool		yes		= arg(3);

	//	SCI still uses the old numbers
	SCI16ToSCI32ResType(resType);

	if (resType == MemResSound)
		resType = GetSoundResType(resId);

	if	(yes) {
		resMgr->Get(resType, resId);
		resMgr->SetNotDiscardable(resType, resId);
	} else {
		resMgr->SetDiscardable(resType, resId);
	}
}

void
KResCheck(argList)
{
	pm.acc = True;
	int	count, fd;
	char	pathName[MaxPath + 1];
	MemType	type;

	type = (MemType)arg(1);
	SCI16ToSCI32ResType(type);
	switch (type) {
		case MemResSound:
			for (count = 2; count <= argCount; ++count) {
				type = GetSoundResType((ResNum) arg(count));
				pm.acc = (Acc) resMgr->Check(type,(ResNum) arg(count));
				if (!pm.acc)
					return;
			}
			break;
		case MemResSync36:
			MakeName36(MemResSync, pathName, (ResNum)arg(2),
				(uchar)arg(3), (uchar)arg(4), (uchar)arg(5), (uchar)arg(6));
			if ((fd = resMgr->Open(MemResSync36, 0, pathName)) != -1)
				Close(fd);
			else
				pm.acc = (Acc) (FindSync36Entry((ResNum)arg(2), (uchar)arg(3),
					(uchar)arg(4), (uchar)arg(5), (uchar)arg(6), &count) != -1);
			break;
		case MemResAudio36:
			MakeName36(MemResAudio, pathName, (ResNum)arg(2),
				(uchar)arg(3), (uchar)arg(4), (uchar)arg(5), (uchar)arg(6));
			if ((fd = resMgr->Open(MemResAudio36, 0, pathName)) != -1)
				Close(fd);
			else
				pm.acc = (Acc) (FindAud36Entry((ResNum)arg(2), (uchar)arg(3),
					(uchar)arg(4), (uchar)arg(5), (uchar)arg(6)) != -1);
			break;
		default:
			for (count = 2; count <= argCount; ++count) {
				pm.acc = (Acc) resMgr->Check(type, (ResNum) arg(count));
				if (!pm.acc)
					return;
			}
			break;
	}
}

void
KCD(argList)
{
	enum {
		CheckCD,
		GetSaveCD
	};

	switch (arg(1)) {
		case CheckCD:
			if (argCount >= 2)
				resMgr->CheckDiscNum((uint)arg(2));
			else
				resMgr->CheckDiscNum(0);

			pm.acc = (Acc) discNum;
			break;

		case GetSaveCD:
			pm.acc = saveGame->GetCDisc();
			break;
	}
}
