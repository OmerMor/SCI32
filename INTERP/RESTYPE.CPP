//	restype.cpp

#include <stdio.h>
#include <string.h>

#include "sol.hpp"

#include "audio.hpp"
#include "config.hpp"
#include "dos.hpp"
#include "msg.hpp"
#include "resource.hpp"
#include "dcomp.hpp"
#include "newroom.hpp"
#include "memtype.hpp"

int	SOL_ResType::fd = -1;
int	SOL_ResType::resLength;
int	SOL_ResType::compressedLength;
int	SOL_ResType::resOffset;

_Packed struct VolEntryHeader {
	uchar 	type;
	ResNum	resId;
	int		compressedLength;
	int		expandedLength;
	ushort	compressUsed;
} static volEntryHeader;

SOL_ResType::SOL_ResType(char *extension, char* label) :
	label(label)
{
	strcpy(ext, extension);
}

Bool
SOL_ResType::Check(ResNum resNum)
{
	int	found;

	Close(found = Open(resNum));
	if (found != -1)
		return True;
	
//	if ((GetType() == MemResAudio || GetType() == MemResWAVE) &&
//			FindAudEntry(resNum) != -1L)
//		return True;

	return False;
}

void
SOL_ResType::Load(ResTag& tag, Bool lock)
{
	if ((fd = Open(tag.resNum)) == -1)
		msgMgr->Fatal(SrcLoc, Msg_ResLoadErr, tag.resNum, GetExtension());

	MemAttrs attrs = lock ? GetAttrs() & ~MOVEABLE : GetAttrs();
	tag.id.Get(GetType(), resLength + 1, attrs, tag.resNum, resMgr);

	int result = 0;
	if (fd == sfxVolFD) {
		result = ::ReadMemID(fd, tag.id, resLength);
	} else if (fd != altVolFD && fd != resVolFD && fd != resVolFDPre) {
		result = tag.id.Read(fd);
		Close(fd);

	} else if (volEntryHeader.compressUsed == 0x20) {
		if(volEntryHeader.resId != tag.resNum)
			msgMgr->Fatal(SrcLoc,Msg_ResourceData);
		tag.id.SetNotDiscardable();
		Decompress(tag);
		tag.id.SetDiscardable();
	} else {
		if(volEntryHeader.resId != tag.resNum)
			msgMgr->Fatal(SrcLoc,Msg_ResourceData);

		result = ::ReadMemID(fd, tag.id, resLength);
		if (result != resLength)
			msgMgr->Fatal(SrcLoc, Msg_ResLoadErr, tag.resNum, GetExtension());
		if (tag.id[0] == 0xff) {
			tag.id.Realloc(resLength);
			return;
		}
	}

	if (result == -1)
		msgMgr->Fatal(SrcLoc, Msg_ResLoadErr, tag.resNum, GetExtension());

	tag.id.Realloc(resLength);
}

void
SOL_ResType::Decompress(ResTag& tag) const
{
	MemID srcID = resMgr->DecompBuffer(compressedLength);
	if (::ReadMemID(fd, srcID, compressedLength) != compressedLength)
		msgMgr->Fatal(SrcLoc, Msg_ResLoadErr, tag.resNum, GetExtension());
	char* dest = (char*) *tag.id;
	char* workBuffer = (char*) *resMgr->decompWorkBuffer;
	ulong destLength = resLength + 1;
	ulong sourceLength = compressedLength;

	Initcompress(workBuffer);

	char* src = (char*)*srcID;
	int result = ::Decompress(&src, &dest, &sourceLength,&destLength,workBuffer);

	if (sourceLength != 0 || result != 1)
		msgMgr->Fatal(Msg_CompressSource);

	sourceLength = 0;
 	result = ::Decompress(&src, &dest, &sourceLength, &destLength, workBuffer);

	if (destLength > 1)
		msgMgr->Fatal(Msg_CompressDestination);
	resMgr->DecompBuffer(0);
}

int
SOL_ResType::Open(ResNum num, char* path)
{
	char		nameBuf[MaxPath + 1];
	MemType	type;
	int		hdrLength;

	// check the language directory for MAP resources
	if (langPath[0] && GetFileType() == MemResMap && num != SFXMODNUM) {
		MakeName(nameBuf, langPath, num);
		if ((fd = ::Open(nameBuf, O_BINARY | O_RDONLY)) != -1) {
			if (path)
				strcpy(path, nameBuf);
			Read(&type, 1);
			ConfirmType((MemType)(type & 0xff));
			resOffset = SeekToData();
			resLength = GetFileSize() - resOffset;
#ifdef DEBUG	
		   if (tracking)
				rmResList->Add(GetType(), num, resLength);
#endif
			return fd;
		}
	}

	// check patch table and alternate-resource volume
	int dirNum = FindPatchEntry(num);
	int dirNum36 = FindPatch36Entry(path);
	resOffset = FindDirEntry(num, alternateMap);

	// if patched resource found, use it unless it is also found in the
   // alternate-resource volume at an earlier patchDir-specified dir
	if (dirNum != -1 && (resOffset == -1 || altDirNum >= dirNum)) {
		MakeName(nameBuf, configMgr->Get("patchDir",dirNum), num);
		if ((fd = ::Open(nameBuf, O_BINARY | O_RDONLY)) != -1) {
			if (path)
				strcpy(path, nameBuf);
			Read(&type, 1);
			ConfirmType((MemType)(type & 0xff));
			resOffset = SeekToData();
			resLength = GetFileSize() - resOffset;
#ifdef DEBUG	
		   if (tracking)
				rmResList->Add(GetType(), num, resLength);
#endif
			return fd;
		}
	}

	// if found in patch36 table use it
	if (dirNum36 != -1) {
		strcpy(nameBuf, configMgr->Get("patchDir",dirNum36));
		strcat(nameBuf, "\\");
		strcat(nameBuf, path);
		if ((fd = ::Open(nameBuf, O_BINARY | O_RDONLY)) != -1) {
			strcpy(path, nameBuf);
			Read(&type, 1);
			ConfirmType((MemType)(type & 0xff));
			resOffset = SeekToData();
			resLength = GetFileSize() - resOffset;
#ifdef DEBUG	
		   if (tracking)
				rmResList->Add(GetType(), num, resLength);
#endif
			return fd;
		}
	}

	// if found in alternate-resource volume use it
	if (resOffset != -1) {
		// requested resource is alternate volume-resident
		LSeek(fd = altVolFD,resOffset,SEEK_SET);
		Read(&volEntryHeader, sizeof volEntryHeader);
		ConfirmType((MemType)volEntryHeader.type);
		resOffset += sizeof(volEntryHeader);
		resLength = volEntryHeader.expandedLength;
		compressedLength = volEntryHeader.compressedLength;
#ifdef DEBUG	
	   if (tracking)
			rmResList->Add(GetType(), num, resLength);
#endif
		return altVolFD;
	}

	// check resource-config file path(s)...
	for (int i = 0; i < configMgr->GetNTokens(GetType()); i++) {
		if (path && *path)
			MakeName(nameBuf, configMgr->Get(GetType(), i), path);
		else
			MakeName(nameBuf, configMgr->Get(GetType(), i), num);
		if ((fd = ::Open(nameBuf, O_BINARY | O_RDONLY)) != -1) {
			if (path)
				strcpy(path, nameBuf);
			if (GetType() != MemResAudio && GetType() != MemResAudio36 && GetType() != MemResWAVE) {
				Read(&type, 1);
				ConfirmType((MemType)(type & 0xff));
				resOffset = SeekToData();
			} else
				resOffset = 0;
			resLength = GetFileSize() - resOffset;
#ifdef DEBUG	
		   if (tracking)
				rmResList->Add(GetType(), num, resLength);
#endif
			return fd;
		}
	}

	// check resource volume...

	if (GetType() == MemResAudio || GetType() == MemResWAVE) {
		if ((resOffset = FindAudEntry(num)) != -1) {
			LSeek(fd = sfxVolFD,resOffset,SEEK_SET);
			Read(&type, 1);
			if((MemType)(type & ~0x80) == MemResAudio) {
				// SOL Format
				Read(nameBuf, 8);
				hdrLength = int(nameBuf[0]) + 2;
			} else {
				// WAVE Format
				Read(nameBuf, 3);
				hdrLength = 8;
			}
			Read((char*)&resLength, 4);
			resLength += hdrLength;
			LSeek(fd,resOffset,SEEK_SET);
#ifdef DEBUG	
		   if (tracking)
				rmResList->Add(GetType(), num, resLength);
#endif
			return sfxVolFD;
		}
	} else if ((resOffset = FindDirEntry(num, resourceMap)) != -1) {
		// requested resource is volume-resident
		if (resOffset + sizeof(volEntryHeader) < preVolLength) {
			LSeek(fd = resVolFDPre,resOffset,SEEK_SET);
			Read(&volEntryHeader, sizeof volEntryHeader);
			ConfirmType((MemType)volEntryHeader.type);
			resOffset += sizeof(volEntryHeader);
			resLength = volEntryHeader.expandedLength;
			compressedLength = volEntryHeader.compressedLength;
			if (resOffset + resLength > preVolLength)
				LSeek(fd = resVolFD,resOffset,SEEK_SET);
		} else {
			LSeek(fd = resVolFD,resOffset,SEEK_SET);
			Read(&volEntryHeader, sizeof volEntryHeader);
			ConfirmType((MemType)volEntryHeader.type);
			resOffset += sizeof(volEntryHeader);
			resLength = volEntryHeader.expandedLength;
			compressedLength = volEntryHeader.compressedLength;
		}

#ifdef DEBUG	
	   if (tracking)
			rmResList->Add(GetType(), num, resLength);
#endif
		return fd;
	}

	return -1;
}

int
SOL_ResType::Close(int fd) const
{
	if (fd != -1 && fd != resVolFD && fd != resVolFDPre && fd != altVolFD &&
			fd != sfxVolFD && fd != audVolFD)
		return ::Close(fd);
	return -1;
}

void
SOL_ResType::MakeName(char *buf, char* fileSpec, ResNum num) const
{
	char	rootName[MaxFName + 1];

	sprintf(rootName, "%u", num);
	MakeName(buf, fileSpec, rootName);
}

void
SOL_ResType::MakeName(char* buf, char* fileSpec, char* rootName) const
{
	char	drive[MaxDrive + 1];
	char	dir[MaxDir + 1];
	char	ext[MaxExt + 1];
	char	fullName[MaxPath + 1];
	
	//	if there are no wildcard characters in fileSpec, assume it's a dir and
	// slap on a backslash so _splitpath knows it, unless it's just a drive
	//	or the last char is already a backslash
	if (!strchr(fileSpec, '*') && fileSpec[strlen(fileSpec) - 1] != ':' &&
	    fileSpec[strlen(fileSpec) - 1] != '\\') {
		strcpy(fullName, fileSpec);
		strcat(fullName, "\\");
		fileSpec = fullName;
	}
	
	//	get the drive, directory and extension
	_splitpath(fileSpec, drive, dir, 0, ext);
	
	//	and add the name and extension
	_makepath(buf, drive, dir, rootName, *ext ? ext : GetExtension());
}

void
SOL_ResType::MakeWildName(char *buf, char* fileSpec) const
{
	MakeName(buf, fileSpec, "*");
}

long
SOL_ResType::GetFileSize() const
{
	if (fd != -1)
		return FileLength(fd);
	else
		msgMgr->Fatal(SrcLoc, Msg_InvalidResFileHandle, label);
	return 0;
}

void
SOL_ResType::ConfirmType(MemType type) const
{
	if(fd == -1)
		msgMgr->Fatal(SrcLoc, Msg_InvalidResFileHandle, label);

	if (GetFileType() == MemResAudio)
		return;

// Delete the next line when robot resources are converted to the proper type
	if (GetFileType() == MemResRobot) return;

	//	allow 0x80-based or 0-based numbers
	if((MemType)(type & ~0x80) != GetFileType())
		msgMgr->Fatal(Msg_WrongResType, label);
}

int
SOL_ResType::Read(void* dest, int size) const
{
	return ::Read(fd, dest, size);
}

int
SOL_ResType::SeekToData() const
{
	char	len;

	LSeek(fd,1,SEEK_SET);
	Read(&len, 1);
	return LSeek(fd,len,SEEK_CUR);
}

int
SOL_ResType::FindDirEntry(ResNum resId, MemID resMap) const
{
	if (!resMap)
		return -1;

	// find the resource type in the resource map header
	ResDirHdrEntry* header = (ResDirHdrEntry*) *resMap;
	MemType theType = GetType();
	while ((uchar)(header->resType & ~0x80) != theType) {
		if (header->resType == 255)
			return -1; 
		++header;
	}
	int firstOffset = header->resTypeOffset;
	int lastOffset = (header+1)->resTypeOffset - sizeof(ResDirEntry);

	// utilize a binary search to locate the resource id
	while (firstOffset <= lastOffset) {
		int midOffset = (lastOffset - firstOffset) / 2
			/ sizeof(ResDirEntry) * sizeof(ResDirEntry) + firstOffset;
		ResDirEntry* entry = (ResDirEntry *) ((char *) *resMap + midOffset); 
		if (entry->resId == resId) {
			// build the found volume offset
			return entry->volOffset;
		}
		if (entry->resId < resId)
			firstOffset = midOffset + sizeof(ResDirEntry);
		else
			lastOffset = midOffset - sizeof(ResDirEntry);
	}

	return -1;
}

int
SOL_ResType::FindPatchEntry(ResNum resId) const
{
	ResPatchEntry*	entry;

	if (!resPatches)
		return -1;

	for (entry = (ResPatchEntry*) *resPatches; (uchar) entry->resType != 0xff; ++entry)
		if (entry->resType == GetType() && entry->resId == resId)
			return (int) entry->patchDir;
	return -1;
}

int
SOL_ResType::FindPatch36Entry(char* name) const
{
	ResPatch36Entry*	entry36;

	if (!resPatches36 || !name || strlen(name) != 12)
		return -1;

	for (entry36 = (ResPatch36Entry*) *resPatches36; entry36->resName[0]; ++entry36)
		if (!stricmp(entry36->resName,name))
			return (int) entry36->patchDir;
	return -1;
}

Bool
SOL_ResType::CheckPatches(ResNum num) const
{
	// check patch table and alternate-resource volume
	return FindPatchEntry(num) != -1 || FindDirEntry(num, alternateMap) != -1;
}

/////////////////////////////////////////////////////////////////////////////

int
ResView::SeekToData() const
{
	char	len;

	LSeek(fd,3,SEEK_SET);
	Read(&len, 1);
	return LSeek(fd,len + 22,SEEK_CUR);
}

int
ResPic::SeekToData() const
{
	char	len;

	LSeek(fd,3,SEEK_SET);
	Read(&len, 1);
	return LSeek(fd,0,SEEK_CUR);
}

int
ResPal::SeekToData() const
{
	char	len;

	LSeek(fd,3,SEEK_SET);
	Read(&len, 1);
	return LSeek(fd,len,SEEK_CUR);
}

int
ResWave::SeekToData() const
{
	return 0;
}

int
ResAudio::SeekToData() const
{
	return 0;
}

int
ResAudio36::SeekToData() const
{
	return 0;
}
