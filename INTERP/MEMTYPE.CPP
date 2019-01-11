
//	memtype.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sol.hpp"

#include "memmgr.hpp"
#include "memtype.hpp"
#include "textid.hpp"
#include "window.hpp"

class MemTypeDesc;

struct MemTypeStat {
	void
	Clear()
	{
		nTot				= 0;
		memTot			= 0;
		nLocked			= 0;
		memLocked		= 0;
		nPurgeable		= 0;
		memPurgeable	= 0;
		nPersistent		= 0;
		memPersistent	= 0;
		nTransitory		= 0;
		memTransitory	= 0;
	}
	
	char*
	MakeStr()
	{
		static char buf[100];
		
		*buf = 0;

		sprintf(buf, "%u(%u)", memTot, nTot);
		
		Bool allOneType =
			!!nLocked + !!nPurgeable + !!nPersistent + !!nTransitory == 1;
		
		if (memLocked == memTot)
			strcat(buf, " locked");
		if (memPurgeable == memTot)
			strcat(buf, " purgeable");
		if (memPersistent == memTot)
			strcat(buf, " persistent");
		if (memTransitory == memTot)
			strcat(buf, " transitory");

		if (memLocked && memLocked != memTot)
			sprintf(buf + strlen(buf), "  %u(%u) locked", memLocked, nLocked);
		
		if (memPurgeable && memPurgeable != memTot)
			sprintf(buf + strlen(buf), "  %u(%u) purgeable", memPurgeable,
				nPurgeable);
		
		if (memPersistent && memPersistent != memTot)
			sprintf(buf + strlen(buf), "  %u(%u) persistent", memPersistent,
				nPersistent);

		if (memTransitory && memTransitory != memTot)
			sprintf(buf + strlen(buf), "  %u(%u) transitory", memTransitory,
				nTransitory);
		
		return buf;
	}

	void
	Update(size_t size, Bool locked, Bool purgeable, Bool persistent,
			 Bool transitory)
	{
		nTot++;
		memTot += size;
		
		if (locked) {
			nLocked++;
			memLocked += size;
		}

		if (purgeable) {
			nPurgeable++;
			memPurgeable += size;
		}

		if (persistent) {
			nPersistent++;
			memPersistent += size;
		}

		if (transitory) {
			nTransitory++;
			memTransitory += size;
		}
	}

	int	nTot;
	uint	memTot;
	
	int	nLocked;
	uint	memLocked;
	
	int	nPersistent;
	uint	memPersistent;
	
	int	nPurgeable;
	uint	memPurgeable;
	
	int	nTransitory;
	uint	memTransitory;
};
	
struct MemTypes {
	void				AddStat(MemType type, size_t size, Bool locked,
							Bool purgeable, Bool persistent, Bool transitory);
	void				ClearStats();
	const char*		GetMemTypeStr(MemType type);
	const char*		GetSCIMemTypeStr(MemType type);
	MemTypeDesc*	FindMemTypeDesc(MemType type);
	TextID			MakeStatsStr();
	MemType			GetTypeFromStr(const char* str);
	MemType			GetSCITypeFromStr(const char* str);
private:
	static int	Compare(void const* a, void const* b);

	MemTypeStat	unknownStat;
	MemTypeStat	totStat;

	static MemTypeDesc descs[];

} static memTypes;

class MemTypeDesc {

	MemType		type;
	char*			desc;
	char*       SCIdesc;
	MemTypeStat	stat;
	
	friend class MemTypes;

} MemTypes::descs[] = {
	{ MemResView,			"View_Res","RES_VIEW" },
	{ MemResPic,			"Pic_Res","RES_PIC" },
	{ MemResHunk,			"Hunk_Res","RES_SCRIPT" },
	{ MemResSound,			"Sound_Res","RES_SOUND" },
	{ MemResVocab,			"Vocab_Res","RES_VOCAB" },
	{ MemResFont,			"Font_Res","RES_FONT" },
	{ MemResCursor,		"Cursor_Res","RES_CURSOR" },
	{ MemResPatch,			"Patch_Res","RES_PATCH" },
	{ MemResBitmap,		"Bitmap_Res","RES_BITMAP" },
	{ MemResPalette,		"Palette_Res","RES_PALETTE" },
	{ MemResWAVE,			"WAVE_Res","RES_WAVE" },
	{ MemResAudio,			"Audio_Res","RES_AUDIO" },
	{ MemResSync,			"Sync_Res","RES_SYNC" },
	{ MemResMsg,			"Msg_Res","RES_MESSAGE" },
	{ MemResMap,			"Map_Res","RES_MAP" },
	{ MemResHeap,			"Heap_Res","RES_HEAP" },
   { MemResChunk,       "Chunk_Res"},		
	{ MemResAudio36,		"Audio36_Res","RES_AUDIO36" },
	{ MemResSync36,		"Sync36_Res","RES_SYNC36" },
	{ MemResMsgTranslation,"Msg_Translation_Res" },
	{ MemResAnimation,	"Animation_Res", "RES_ANIMATION" },

	{ MemDriver,			"Driver" },
	{ MemResourceList,	"Resource_List" },
	{ MemPatchTable,		"Patch_Table" },
	{ MemText,				"Text" },
	{ MemObject,			"Object" },
	{ MemArray,				"Array" },
	{ MemMovieBuffer,		"Movie_Buffer" },
	{ MemSample,			"Sample" },
	{ MemList,				"List" },
	{ MemListNode,			"List_Node" },
	{ MemListKNode,		"Kernel_List_Node" },
	{ MemDictionary,		"Dictionary" },
	{ MemClassTbl,			"Class_Table" },
	{ MemDispatchTbl,		"Dispatch_Table" },
	{ MemScriptEntry,		"Script_Entry" },
	{ MemVariables,		"Variables" },
	{ MemScript,			"Script" },
	{ MemViewHeader,		"View_Header" },
	{ MemMsgStack,			"Msg_Stack" },
	{ MemMovie,				"Movie" },
	{ MemCode,				"Code" },
	{ MemPolygonList,		"Polygon_List" },
	{ MemPointList,		"Point_List" },
	{ MemSound,				"Sound" },
	{ MemSync,				"Sync" },
	{ MemPMStack,			"PMachine_Stack" },
	{ MemEditStruct,		"Edit_Struct" },
	{ MemBitmap,			"Bitmap" },
	{ MemSpecialCode,		"Special_Code" },
	{ MemDescriptors,		"Descriptor_Table" },
	{ MemDecompBuffer,	"Decompression_Buffer" },
	{ MemAudioBuffer,		"Audio_Buffer" },
	{ MemSaveGameDumpBuffer,"SaveGame_Dump_Buffer" },
	{ MemCodeFixups,		"Code_Fixup_Table" },
	{ MemWindow,			"Window" },
   { MemWindowEntry,		"Window_Entry" },
   { MemFontMgr,			"Font_Mgr" },
   { MemEdit,				"Edit" },

	{ MemResNone,			"No_Resource" },
	{ MemFree,				"Free" }
};

#define NDescs sizeof(MemTypes::descs) / sizeof(*MemTypes::descs)

MemType
GetMemTypeFromStr(const char* str)
{
	return memTypes.GetTypeFromStr(str);
}

MemType
GetSCITypeFromStr(const char* str)
{
	return memTypes.GetSCITypeFromStr(str);
}

const char*
GetMemTypeStr(MemType t)
{
	return memTypes.GetMemTypeStr(t);
}

const char*
GetSCIMemTypeStr(MemType t)
{
	return memTypes.GetSCIMemTypeStr(t);
}

void
MemTypes::AddStat(MemType type, size_t size, Bool locked, Bool purgeable,
						Bool persistent, Bool transitory)
{
	MemTypeDesc* desc = FindMemTypeDesc(type);
	MemTypeStat* stat = desc ? &desc->stat : &unknownStat;
	
	totStat.Update(size, locked, purgeable, persistent, transitory);
	stat->Update(size, locked, purgeable, persistent, transitory);
}

void
MemTypes::ClearStats()
{
	for (int i = 0; i < NDescs; i++)
		descs[i].stat.Clear();
	totStat.Clear();
	unknownStat.Clear();
}

MemTypeDesc*
MemTypes::FindMemTypeDesc(MemType type)
{
	for (int i = 0; i < NDescs; i++)
		if (descs[i].type == type)
			return &descs[i];
	return 0;
}

MemType
MemTypes::GetTypeFromStr(const char* str)
{
	for(int i=0; i<NDescs; i++)
		if (!strcmp(str,descs[i].desc))
			return descs[i].type;

	return (MemType) -1;
}

MemType
MemTypes::GetSCITypeFromStr(const char* str)
{
	for(int i=0; i<NDescs; i++)
		if (!strcmp(str,descs[i].SCIdesc))
			return descs[i].type;

	return (MemType) -1;
}

const char*
MemTypes::GetMemTypeStr(MemType type)
{
	MemTypeDesc* desc = FindMemTypeDesc(type);
	if (!desc) {
		static char buf[40];
		sprintf(buf, "Unknown MemType: %u", type);
		return buf;
	}

	return desc->desc;
}

const char*
MemTypes::GetSCIMemTypeStr(MemType type)
{
	MemTypeDesc* desc = FindMemTypeDesc(type);
	if (!desc) {
		static char buf[40];
		sprintf(buf, "Unknown MemType: %u", type);
		return buf;
	}

	return desc->SCIdesc;
}

int
MemTypes::Compare(void const* a, void const* b)
{
	//	sort in order of decreasing size

	MemTypeDesc* da = (MemTypeDesc*) a;
	MemTypeDesc* db = (MemTypeDesc*) b;
	
	return db->stat.memTot - da->stat.memTot;
}

TextID
MemTypes::MakeStatsStr()
{
	TextID buf;
	qsort(descs, NDescs, sizeof *descs, Compare);
	for (int i = 0; i < NDescs; i++)
		if (descs[i].stat.nTot)
			buf.AddF("%s:  %s\n", descs[i].desc, descs[i].stat.MakeStr());
	if (unknownStat.nTot)
		buf.AddF("%s:  %s\n", "Unknown type", unknownStat.MakeStr());
	return buf;
}

////////////////////////////////////////////////////////////////////////////

const char*
MemoryMgr::GetMemTypeStr(SOL_Handle h)
{
#ifdef DEBUG
	CheckValid(h);
#endif
	return memTypes.GetMemTypeStr(GetMemType(h));
}

TextID
MemoryMgr::MakeMemTypeDspStr()
{
	memTypes.ClearStats();

	for (int i = 1; i < maxDescriptors; i++) {
		if (!IsValid(i))
			continue;
		memTypes.AddStat(
			desc[i].memInfo.type,
			desc[i].size,
			!desc[i].memInfo.bits.moveable,
			desc[i].memInfo.bits.discardable,
			desc[i].memInfo.bits.persist,
			desc[i].memInfo.bits.transitory);
	}
	
	return memTypes.MakeStatsStr();
}

int
MemoryMgr::TotalType(MemType type)
{
	//	return total number of allocations of a certain type, or -1 if type
	//	is invalid

	if (type >= MemFree)
		return -1;

	for (int i = 1, nTypes = 0; i < maxDescriptors; i++)
		if (IsValid(i) && desc[i].memInfo.type == type)
			nTypes++;

	return nTypes;
}

TextID
MemoryMgr::MakeMemIDListStr(int start, int* end, int count)
{
	TextID buf;
	*end = start + count;
	int max = 0;

	for (int i = start; i < maxDescriptors && i < *end; i++) {
		if (!IsValid(i))
			continue;
			
		max = i;
		buf.AddF("%08x  %u: %s  %u\n", GetMemAttrs(i), i, GetMemTypeStr(i),
			GetSize(i));
	}
	
	*end = max;

	return buf;
}

#ifdef NOT_SHIPPING_VERSION

void
MemoryMgr::ThrashReport()
{
	// Thrashing detection report 

	TextID	buf;

	doThrashReport = False;

	for (int i=0;i<ThrashListSize;++i) {
		if (thrashArray[i].count == 0)
			break;
		//	resource
		buf.AddF("count = %d  resource = %8u.%s\n",
					thrashArray[i].count,
					thrashArray[i].resNumber,
					memTypes.GetMemTypeStr(thrashArray[i].type));
	}

	const width = 100 * XSCALE;
	EventWindow	* debugWindow =
		DebugWindow(*buf,"Thrashing Report",width,SOL_Point((SCIRESX - 1 - width)/2,50*YSCALE),True);

	buf.Free();

	delete debugWindow;
}

#endif


