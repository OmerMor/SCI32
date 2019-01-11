//	resource.hpp

#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#ifndef MEMTYPE_HPP
#include "memtype.hpp"
#endif

#ifndef RESTYPE_HPP
#include "restype.hpp"
#endif

#ifndef MEMMGR_HPP
#include "memmgr.hpp"
#endif

class MemID;
class TextID;

#define AUDVOLNAME	"RESOURCE.AUD"
#define SFXVOLNAME	"RESOURCE.SFX"
const Bool lock = True;

_Packed struct PreLoadObj
{
	ushort resType;
	ushort resNum;
	uint	 offset;
	int	 volume;
};

class ResourceMgr : public MemClient {
public:
	ResourceMgr();
	virtual ~ResourceMgr();

	//	override MemClient's Free()
	void	Free(MemType, unsigned);
#ifdef DEBUG
	virtual void	ExtractResources();
	uint	GetChecksum(SOL_Handle);		// compute a checksum for the data
#endif

	virtual void	BuildPatchTables();
	virtual Bool	Check(MemType, ResNum);
	virtual void	CheckDiscNum(uint);
	virtual MemID	Find(MemType, ResNum);
	virtual MemID	Get(MemType, ResNum, Bool = False);
	virtual Bool	LoadResMaps(uint);
	virtual void	Lock(MemType, ResNum);
	virtual TextID	MakeDebugDisplayStr();
	virtual char*	MakeName(MemType, ResNum);
	virtual int		Open(MemType, ResNum, char*);
	virtual void	Release(MemType, ResNum);
	virtual void	SetDiscardable(MemType, ResNum);
	virtual void	SetNotDiscardable(MemType, ResNum);
	virtual void	Unlock(MemType, ResNum);

	int	  	FindOffset(MemType,ResNum,int&);

	MemID		DecompBuffer(int requestedSize = 0);
	MemID		decompWorkBuffer;
	void 		LoadChunk(ResTag& resTagH,int fLength);
	MemType	GetExtType(char*);


protected:
	// Find the index for a resource type and number
	// The index either matches or is the index of the first higher resource
	virtual int		AddIndex(MemType, ResNum);
	virtual Bool	Check(SOL_ResType* obj, ResNum num)	{ return obj->Check(num); }
	virtual void	DeleteIndex(MemType, ResNum);
	virtual int		GetIndex(MemType, ResNum);
	virtual void	InitResourceMgr();
	virtual void	Load(SOL_ResType* obj, ResTag& tag, Bool lock)
	 	{ obj->Load(tag,lock); }

	MemID		decompID;
	int		decompBufferSize;
	
};

_Packed struct ResPatchEntry {
	uchar		patchDir;
	MemType	resType;
	ResNum	resId;
};

_Packed struct ResPatch36Entry {
	uchar		patchDir;
	char		resName[13];
};

_Packed struct ResDirHdrEntry {
	MemType	resType;
	unsigned short resTypeOffset;
};

_Packed struct ResDirEntry {
	ResNum	resId;
	long	volOffset;
};

extern ResourceMgr*	resMgr;
extern int				resVolFD;
extern MemID			resourceMap;
extern int				altVolFD;
extern MemID			alternateMap;
extern MemID			resPatches;
extern MemID			resPatches36;
extern int				altDirNum;
extern int				discNum;

#endif  //RESOURCE_HPP

