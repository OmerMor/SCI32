//	memtype.hpp

#ifndef MEMTYPE_HPP
#define MEMTYPE_HPP

enum MemType {
	//	resource memory types
	MemResFirst,
	MemResView = MemResFirst,		// 0x00
	MemResPic,							// 0x01
	MemResHunk,							// 0x02
	MemResAnimation,					// 0x03
	MemResSound,						// 0x04
	MemUNUSED2,							// 0x05
	MemResVocab,						// 0x06
	MemResFont,							// 0x07
	MemResCursor,						// 0x08
	MemResPatch,						// 0x09
	MemResBitmap,						// 0x0a
	MemResPalette,						// 0x0b
	MemResWAVE,							// 0x0c
	MemResAudio,						// 0x0d
	MemResSync,							// 0x0e
	MemResMsg,							// 0x0f
	MemResMap,							// 0x10
	MemResHeap,							// 0x11
   MemResChunk,						// 0x12
	MemResAudio36,						// 0x13
	MemResSync36,						// 0x14
	MemResMsgTranslation,			// 0x15
	MemResRobot,						// 0x16
	MemResVMD,							// 0x17
	MemResLast = MemResVMD,

	//	other memory types
	MemDriver = 0x30,					// 0x30
	MemResourceList,					// 0x31
	MemPatchTable,						// 0x32
	MemText,								// 0x33
	MemObject,							// 0x34
	MemArray,							// 0x35
	MemMovieBuffer,					// 0x36
	MemSample,							// 0x37
	MemList,								// 0x38
	MemListNode,						// 0x39
	MemListKNode,						// 0x3a
	MemDictionary,						// 0x3b
	MemClassTbl,						// 0x3c
	MemDispatchTbl,					// 0x3d
	MemScriptEntry,					// 0x3e
	MemVariables,						// 0x3f
	MemScript,							// 0x40
	MemViewHeader,						// 0x41
	MemMsgStack,						// 0x42
	MemMovie,							// 0x43
	MemCode,								// 0x44
	MemPolygonList,					// 0x45
	MemPointList,						// 0x46
	MemSound,							// 0x47
	MemSync,								// 0x48
	MemPMStack,							// 0x49
	MemEditStruct,						// 0x4a
	MemBitmap,							// 0x4b
	MemSpecialCode,					// 0x4c
	MemDescriptors,					// 0x4d
	MemDecompBuffer,					// 0x4e
	MemAudioBuffer,					// 0x4f
	MemSaveGameDumpBuffer,			// 0x50
	MemCodeFixups,						// 0x51
	MemWindow,  						// 0x52
   MemWindowEntry,               // 0x53
   MemFontMgr,                   // 0x54
   MemEdit,                   	// 0x55
	
	//	special types					
	MemResNone = 70,					// 0x70
	MemFree,								// 0x71
	NotFound = 0xff					// 0xff
};

MemType			GetSCITypeFromStr(const char* str);
const char*		GetSCIMemTypeStr(MemType type);


const char* GetMemTypeStr(MemType);
MemType GetMemTypeFromStr(const char* str);

#endif
