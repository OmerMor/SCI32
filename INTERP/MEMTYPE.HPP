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
	MemResLast = MemResRobot,

	//	other memory types
	MemDriver,							// 0x17
	MemResourceList,					// 0x18
	MemPatchTable,						// 0x19
	MemText,								// 0x1a
	MemObject,							// 0x1b
	MemArray,							// 0x1c
	MemMovieBuffer,					// 0x1d
	MemSample,							// 0x1e
	MemList,								// 0x1f
	MemListNode,						// 0x20
	MemListKNode,						// 0x21
	MemDictionary,						// 0x22
	MemClassTbl,						// 0x23
	MemDispatchTbl,					// 0x24
	MemScriptEntry,					// 0x25
	MemVariables,						// 0x26
	MemScript,							// 0x27
	MemViewHeader,						// 0x28
	MemMsgStack,						// 0x29
	MemMovie,							// 0x2a
	MemCode,								// 0x2b
	MemPolygonList,					// 0x2c
	MemPointList,						// 0x2d
	MemSound,							// 0x2e
	MemSync,								// 0x2f
	MemPMStack,							// 0x30
	MemEditStruct,						// 0x31
	MemBitmap,							// 0x32
	MemSpecialCode,					// 0x33
	MemDescriptors,					// 0x34
	MemDecompBuffer,					// 0x35
	MemAudioBuffer,					// 0x36
	MemSaveGameDumpBuffer,			// 0x37
	MemCodeFixups,						// 0x38
	MemWindow,  						// 0x39
   MemWindowEntry,               // 0x3a
   MemFontMgr,                   // 0x3b
   MemEdit,                   	// 0x3c
	
	//	special types					
	MemResNone,							// 0x3d
	MemFree,								// 0x3e
	NotFound = 0xff					
};

MemType			GetSCITypeFromStr(const char* str);
const char*		GetSCIMemTypeStr(MemType type);


const char* GetMemTypeStr(MemType);
MemType GetMemTypeFromStr(const char* str);

#endif
