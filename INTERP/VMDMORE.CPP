#include "VMDLIBS.HPP"
#include "sol.hpp"
#include "MEMMGR.HPP"
#include "time.hpp"

	WORD    		AutoMemCheck;
	char			WindowName[32]="VMD Library";
	int			DMAlastFreq=22050;
	BOOL			SoundTrack;
	WORD			(*X_StartSound)(WORD,SOUND *,WORD,WORD);
	BOOL			(*X_TrackInProcess)(WORD);
	void			(*X_TrackLoop)(void);
	void			(*X_StopSound)(int,int);
	void			(*X_CloseTrack)(void);
	BYTE    		Video_R[256];
	BYTE    		Video_V[256];
	BYTE    		Video_B[256];
	static LONG	TimerBase;
	SOL_Handle	VMDHandlesID = 0;
	int			VMDHandlesCount = 0;


void *
GetMem(size_t size)
{
	const int VMDHandlesChunk = 20;

	int attributes = MemResVMD + MARKEDMEMORY + LOCKED;
	if (size & RAZ_MEM)
		attributes += INITMEMORY;
	SOL_Handle handle = memMgr->Alloc(attributes,size & ~RAZ_MEM,0,0,0);
	void* ptr = memMgr->GetAddr(handle);

	SOL_Handle* 	VMDHandlesPtr;
	if (!VMDHandlesCount)
	{
		// Get some memory to store handles
		VMDHandlesID = memMgr->Alloc(MemResVMD+MOVEABLE+INITMEMORY+MARKEDMEMORY,
								  	VMDHandlesChunk*sizeof(SOL_Handle),0,0,0);
		VMDHandlesCount = VMDHandlesChunk;
		VMDHandlesPtr = (SOL_Handle*) memMgr->GetAddr(VMDHandlesID);
	}
	else
	{
		VMDHandlesPtr = (SOL_Handle*) memMgr->GetAddr(VMDHandlesID);
	}	
	for (int i=0;i<VMDHandlesCount;++i)
	{
		// Find an empty slot
		if (VMDHandlesPtr[i] == 0)
			break;
	}
	if (i == VMDHandlesCount)
	{
		// Need to increase the handles save area
		VMDHandlesCount += VMDHandlesChunk;
		memMgr->Realloc(VMDHandlesID,VMDHandlesCount*sizeof(SOL_Handle));
		VMDHandlesPtr = (SOL_Handle*) memMgr->GetAddr(VMDHandlesID);
	}
	VMDHandlesPtr[i] = handle;

	if (!TimerBase)
//		TimerBase=timeGetTime();
		TimerBase=(LONG)timeMgr->GetMilliSec();
	return(ptr);
}

void
ReleaseMem(void * memoryPtr)
{
#ifdef DEBUG
	if (memoryPtr == NULL)
		msgMgr->Fatal(SrcLoc,"VMD releasing null pointer");
	if (!VMDHandlesCount)
		msgMgr->Fatal(SrcLoc,"VMD releasing non allocated memory");
#endif

	SOL_Handle* VMDHandlesPtr = (SOL_Handle*) memMgr->GetAddr(VMDHandlesID);

	for (int i=0;i<VMDHandlesCount;++i)
		// Find handle for the memory pointer
		if (VMDHandlesPtr[i] && (memMgr->GetAddr(VMDHandlesPtr[i]) == memoryPtr))
			break;
#ifdef DEBUG
	if (i == VMDHandlesCount)
		msgMgr->Fatal(SrcLoc,"VMD releasing non allocated memory");
#endif

	memMgr->Free(VMDHandlesPtr[i]);
	VMDHandlesPtr[i] = 0;
}

void
ReleaseVMDMem()
{
	if (!VMDHandlesCount)
		return;

	SOL_Handle* VMDHandlesPtr = (SOL_Handle*) memMgr->GetAddr(VMDHandlesID);
	for (int i=0;i<VMDHandlesCount;++i)
	{
		// Find handle for the memory pointer
		if ((int) VMDHandlesPtr[i] != 0)
			memMgr->Free(VMDHandlesPtr[i]);
	}

	memMgr->Free(VMDHandlesID);
	VMDHandlesCount = 0;
	VMDHandlesID = 0;
}

void
VMD_Shutdown()
{
  static BOOL NoMoreErr=FALSE;

  if (NoMoreErr)
    return;

  NoMoreErr=TRUE;
  GXstopSound(0);
  if (X_CloseTrack)
  {
    X_CloseTrack();
    X_CloseTrack=NULL;
  }
  ReleaseVMDMem();
  exit(0);
}

LONG ReadTimer()
{
//  return(timeGetTime()-TimerBase);
	 return((LONG)timeMgr->GetMilliSec());
}

typedef UWORD (*FCT)(int,UBYTE  *,unsigned int);

LONG BlocProcess(void *fct2,WORD val,UBYTE *ptrA2,LONG Size)
{
  FCT fct = (FCT) fct2;
  LONG Sout=0L;
  UBYTE  *ptrA=ptrA2;

  while (Size>=65536L)
  {
    Sout+=fct(val,ptrA,32768U);
    ptrA+=32768L;
    Sout+=fct(val,ptrA,32768U);
    ptrA+=32768L;
    Size-=65536L;
  }
  if (Size>=65000L)
  {
    Sout+=fct(val,ptrA,65000U);
    ptrA+=65000L;
    Size-=65000L;
  }
  if (Size)
    Sout+=fct(val,ptrA,(UWORD)Size);
  return(Sout);
}

typedef UWORD (*_fFCT)(int,UBYTE far *,unsigned int);

LONG _fBlocProcess(void *fct2,WORD val,UBYTE far *ptrA2,LONG Size)
{
  _fFCT fct = (_fFCT) fct2;
  LONG Sout=0L;
  UBYTE far *ptrA=ptrA2;

  while (Size>=65536L)
  {
    Sout+=fct(val,ptrA,32768U);
    ptrA+=32768L;
    Sout+=fct(val,ptrA,32768U);
    ptrA+=32768L;
    Size-=65536L;
  }
  if (Size>=65000L)
  {
    Sout+=fct(val,ptrA,65000U);
    ptrA+=65000L;
    Size-=65000L;
  }
  if (Size)
    Sout+=fct(val,ptrA,(UWORD)Size);
  return(Sout);
}



void AssignColorsPtr(WORD c1,WORD c2,WORD cd,void * ptr)
{
  UBYTE *pal;
  int i;

  pal=(UBYTE *) ptr;
  for (i=c1;i<=c2;i++)
  {
    Video_R[i]=pal[(i-c1+cd) * 3];
    Video_V[i]=pal[(i-c1+cd) * 3 + 1];
    Video_B[i]=pal[(i-c1+cd) * 3 + 2];
  }
  AssignColors(c1,c2);
}


