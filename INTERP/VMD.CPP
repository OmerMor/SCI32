//***************************************************************************
//                                                                          *
//   Copyright (c)1995,M.D.O.   Programmeur:Mathieu___  Cr‚ation:??/??/??   *
//                                                                          *
//   Portage :  [X]:PC  [_]:MAC  [_]:CDI  [_]:3DO  [_]:SATURN  [_]:PSX      *
//              [_]:Sp‚cifique                                              *
//                                                                          *
//   Type de source :  [_]:Interne   [_]:Externe   [X]:Grand public         *
//                                                                          *
//   Programme rattach‚ : [X]:Librairies  [_]:Loader                        *
//			  [_]:Autre : ______________                                       *
//                                                                          *
//***************************************************************************

#ifdef WINDOWS
#include	"windows.h"
#endif
#include "sol.hpp"
#include "msg.hpp"
#include "VMDLIBS.HPP"
#include "vmd.hpp"

// ----------------------------------------------------------------------

extern  COK_BITMAP*		BisRealScreen;
extern  void    			IMD_01(),IMD_03(),IMD_04();
extern  BOOL    			snd_DoDMAlist;
extern  WORD     			CurrentSoundID;
extern  char				WindowName[];
extern  int					RefSoundFrequence;
extern  void				(*X_TrackLoop)(void);
extern  BOOL				RunOverSound;
extern  WORD				(*X_StartSound)(WORD,SOUND *,WORD,WORD);
extern  void				(*X_StopSound)(int,int);
extern  void 				_fIT_ReadCache(WORD n,WORD h,void far *ptr,LONG s);
extern  LONG				TimerSoundBase;
extern  LONG    			CountSoundBase;
extern  ULONG   			Sound_Base[];
extern  UWORD   			Sound_Size[];
extern  LONG volatile	TimerVariable;
extern  CACHE*				i_Cache;

// ----------------------------------------------------------------------

IMD	*(*X_OpenVIG)(char *,COK_BITMAP *);
void	(*X_CloseVIG)(IMD *);
void	(*X_ViewVIG)(IMD *,WORD);
WORD	Track_X1=-1,Track_Y1,Track_X2,Track_Y2;
BOOL	Track_NewPal;
SOUND	*i_ChainSound;
LONG	i_TrackSoundStart;
WORD	i_ChainChannel=-1;
WORD	i_InsertBulle=-1,i_CountPreload;
char	i_InsertFileDAT[16];
BOOL	IMDinProcess;
BOOL	DataInstall;
BOOL	volatile InInterTrack;
BOOL	InAnnex,InViewIMD,InAnnexClose;
BOOL	IMD_get,IMD_check;
UBYTE	*IMD_ptr,*IMD_ptr2;
WORD	IMD_msg;
WORD	IMD_X1,IMD_Y1,IMD_X2,IMD_Y2;
void	(*X_CloseCache)(WORD);
WORD	(*X_OpenCache)(char *,LONG);
void	(*X_ReadCache)(WORD,WORD,void *,LONG);
void	(*X_EnableCache)(WORD,WORD);
void	(*X_SeekCache)(WORD,LONG,WORD);
BOOL	(*X_DataMerge)(WORD,WORD,BLOCINFO  *);
void	(*X_InstallCache)(IMD *);
void	(*X_LoadCache)(void *,WORD);
WORD	i_LoopingCache=256;
WORD	i_NbPreload;
WORD	TrackAutoPlay;
BOOL	FreeSector;
WORD	NextTrackLevel=2;
WORD	NextTrackVolume=256;
IMD	*NextTrackImd;
BOOL	LockTrack;
WORD	NextDataPict;
IMD	*CurrentDataIMD;
BOOL	CachePresent;
WORD	i_BaseFreq=22050;
WORD	i_SyncError;
BOOL	i_CheckSyncError,i_CheckCPU;
LONG	i_CPU;
BOOL	i_InsertPict;
WORD	i_BaseBytes=2205;
BOOL	i_ViewUnderRun;
LONG	i_LastSize;
LONG	i_FixedParams=0;
WORD	TrackFreq;
UBYTE	*i_TempResample;
WORD	i_NeedAssignC1,i_NeedAssignC2;

// ----------------------------------------------------------------------

static	char    IMD_path[40];
static	IMD	*CurrentSoundIMD;
static  BOOL    SkipNext=FALSE;
static	LONG	LastTimerVariable;
static	WORD	DeltaSynchro;
static	WORD	LastWhere;

// ----------------------------------------------------------------------

extern	LONG	GetSoundPos(BOOL);
extern  	void  AddGlobalList_(char *,BOOL);
extern  	void  DelGlobalList_(BOOL);
extern	void	DoIMD_VESA(IMD *);
extern  	BOOL  BuildDMAlist(SOUND *);
static	BOOL	WaitSync(IMD *Imd,WORD w);

// ----------------------------------------------------------------------

void 
IMDread(WORD n,WORD h,void *ptr,LONG s)
{
  if (CachePresent)
    X_ReadCache(n,h,ptr,s);
  else
    BlocProcess(GXread,h,(UBYTE *) ptr,s);
}

void 
_fIMDread(WORD n,WORD h,void far *ptr,LONG s)
{
  if (CachePresent)
    _fIT_ReadCache(n,h,ptr,s);
  else
    _fBlocProcess(_fGXread,h,(UBYTE far *) ptr,s);
}

#ifdef	__TRACE__
static	WORD	hOut;
void 
Printf(char *format,...)
{
  char str[256];

  if (InInterTrack)
    return;
  if (!hOut)
    hOut=_creat("X:\\BAL\\MATHIEU\\T.T",0);
  vsprintf(str,format,...);
  _write(hOut,str,strlen(str));
}
#endif

IMD*
OpenIMD(char *str,COK_BITMAP *bmp,LONG pal)
{
  WORD 			i,h;
  char 			nom2[64];
  IMD*			Imd;
  LONG 			fp;
//  UBYTE *DataB;
  UBYTE far*	DataB;
  int 			l;

  if (X_OpenVIG)
  {
    l=strlen(str);
    if (l>4)
    {
      if (str[l-4]=='.')
      if (str[l-3]=='V')
      if (str[l-2]=='I')
      if (str[l-1]=='G')
	return(X_OpenVIG(str,bmp));
    }
  }
  DeltaSynchro=0;
  if (!IMD_get)
  {
    char *ptr;

    ptr=getenv("_IMD_");
    if (!ptr)
      IMD_path[0]=0;
    else
      strcpy(IMD_path,ptr);
    IMD_get=TRUE;
  }
  strcpy(nom2,IMD_path);
  strcat(nom2,str);
  for (i=0;nom2[i]!='.' && nom2[i];i++);
  if (!nom2[i])
    strcat(nom2,".VMD");


  if (CachePresent && (pal & VMD_CACHE))
  {
    BOOL f=IMD_check;

    if (IMD_path[0])
      IMD_check=TRUE;
    h=X_OpenCache(nom2,pal);
    IMD_check=f;
  }
  else
    h=GXopen(nom2,O_RDONLY);

  if (h==-1)
  {
    strcpy(nom2,str);
    for (i=0;nom2[i]!='.' && nom2[i];i++);
    if (!nom2[i])
      strcat(nom2,".VMD");
    if (CachePresent && (pal & VMD_CACHE))
      h=X_OpenCache(nom2,pal);
    else
      h=GXopen(nom2,O_RDONLY);
  }

  if (h==-1)
  {
    if (!IMD_check)
	 {
  		msgMgr->Alert(SrcLoc,"VMD error %d -> IMD unknown : %s",-51,str);
		VMD_Shutdown();
	 }
    else
      return(NULL);
  }
//       +--------------------------------------------------------+
//       +   Allocation m‚moire du VMD                            +
//       +--------------------------------------------------------+
  Imd=(IMD *) GetMem(sizeof(IMD)+RAZ_MEM);
  for (i=strlen(nom2)-1;nom2[i]!='\\' && nom2[i]!=':' && i;i--);
  if (!i)
    strcpy(Imd->Name,nom2);
  else
    strcpy(Imd->Name,nom2+i+1);
//       +--------------------------------------------------------+
//       +   Mise … jour de Vmd->OpenInfo                         +
//       +--------------------------------------------------------+
  Imd->OpenInfo=pal & (VMD_PRELOAD | VMD_FORCESOUND | VMD_NOTRACK | VMD_AUTOPLAY | VMD_DATA);
  if ((pal & VMD_LOOPCACHE)==VMD_LOOPCACHE)
    Imd->OpenInfo|=VMD_CACHE;
  if (Imd->OpenInfo & VMD_DATA)
    CurrentDataIMD=Imd;
  if ((pal & 3)==3)
    Imd->OpenInfo|=1;
  if (pal & VMD_AUTOPLAY)
    TrackAutoPlay++;
//       +------------------------------------------------------------+
//       +   Lecture de la longueur de structure VMD, puis chargement +
//       +------------------------------------------------------------+
  if (CachePresent)
  {
    X_ReadCache(-1,h,&i,2);
    X_ReadCache(-1,h,Imd,i);
  }
  else
  {
    GXread(h,&i,2);
    GXread(h,Imd,i);
  }
  Imd->h=h;
  Imd->S.Track=9;
  Imd->S.Level=2;
  Imd->RealFreq=Imd->Freq;
//       +--------------------------------------------------------+
//       +   Allocation des tableaux TracksInfo de VMD            +
//       +--------------------------------------------------------+
  Imd->PictInfo=(PICTINFO *) GetMem(sizeof(PICTINFO)*Imd->NbPict);
  Imd->BlocInfo=(BLOCINFO *) GetMem(sizeof(BLOCINFO)*(LONG)Imd->NbPict*(LONG)Imd->NbTrack);
//       +--------------------------------------------------------+
//       +   Lecture des tableaux TracksInfo                      +
//       +--------------------------------------------------------+
  if (CachePresent)
  {
    X_SeekCache(h,Imd->InfoOfs,SEEK_SET);
    X_ReadCache(-1,h,Imd->PictInfo,sizeof(PICTINFO)*Imd->NbPict);
    X_ReadCache(-1,h,Imd->BlocInfo,(LONG)sizeof(BLOCINFO)*(LONG)Imd->NbPict*(LONG)Imd->NbTrack);
  }
  else
  {
    GXlseek(h,Imd->InfoOfs,SEEK_SET);
    GXread(h,Imd->PictInfo,sizeof(PICTINFO)*Imd->NbPict);
    BlocProcess(GXread,h,(UBYTE *)Imd->BlocInfo,sizeof(BLOCINFO)*(LONG)Imd->NbPict*(LONG)Imd->NbTrack);
  }
  if (CachePresent)
    X_InstallCache(Imd);
  if (Imd->X<0 || Imd->Y<0)
    SetIMD_XY(Imd,0,0);
  Imd->Bmp=bmp;
  if (Imd->Type & VMD_PAL)
    Imd->TempPal=(UBYTE *) GetMem(768);
  else
    Imd->TempPal=(UBYTE *) Imd->Pal;
  if (pal & VMD_NOPAL)
    Imd->Type|=VMD_NOPAL2;
  if (pal&1)
  {
//       +--------------------------------------------------------+
//       +   Changement de palette                                +
//       +--------------------------------------------------------+
    AssignColorsPtr(0,255,0,Imd->Pal);
  }
  fp=0;
  if ((pal & 0x8000) && !(Imd->Type & VMD_SOUND))
    Imd->Freq=((ULONG)pal>>16L) & 0xFF;
  if (Imd->Type & VMD_SOUND)
    pal&=~VMD_FORCESOUND;
  if (pal & VMD_FORCESOUND)
  {
    Imd->Type|=VMD_SOUND;
    Imd->Freq=i_BaseFreq;
    Imd->RealFreq=i_BaseFreq;
    Imd->Bytes=i_BaseBytes;
    Imd->NbBuffer=16;
    Imd->SoundInfo=VMD_SOUND_SOUNDFIRST|VMD_SOUND_DECAL1;
  }
  if (pal & VMD_NOWAIT)
    Imd->SoundInfo|=VMD_NOWAIT;
  if (!(pal & VMD_NOSOUND))
  if (Imd->Type & VMD_SOUND)
  {
//       +--------------------------------------------------------+
//       +   Initialisation du Son                                +
//       +--------------------------------------------------------+
    size_t SizeT;

    fp|=i_FixedParams & (VMD_8BITS+VMD_11K);
//       +--------------------------------------------------------+
//       +   Taille de m‚moire allou‚e pour le son                +
//       +--------------------------------------------------------+
    SizeT=(size_t)Imd->Bytes*(LONG)Imd->NbBuffer;

    Imd->S.oBytes=Imd->Bytes;
    if (Imd->SoundInfo & VMD_SOUND16BITS)
      fp|=VMD_8BITS;

    if (fp & VMD_11K)
    {
      SizeT/=2;
      Imd->Freq/=2;
      Imd->Bytes/=2;
      Imd->RealFreq/=2;
    }
    if ((fp & VMD_8BITS) && (Imd->SoundInfo & VMD_SOUND16BITS))
    {
      SizeT/=2;
      Imd->Bytes/=2;
    }
    else
      fp&=~VMD_8BITS;
    if (pal & VMD_SOUNDFIRST)
      Imd->SoundInfo|=VMD_SOUND_SOUNDFIRST;
    else
    if (pal & VMD_PICTFIRST)
      Imd->SoundInfo&=~VMD_SOUND_SOUNDFIRST;

//       +--------------------------------------------------------+
//       +   Allocation du bloc pour les donn‚es sonores          +
//       +--------------------------------------------------------+
// HOOK - May have to be done differently - TM
//    DataB=(UBYTE *) GetMem(SizeT);

//	hAudBuf = GlobalAlloc(GMEM_MOVEABLE, SizeT);
//	DataB = (UBYTE *)GlobalLock(hAudBuf);

//	DataB = (UBYTE *)GXgetSoundMem(SizeT);
	DataB = (UBYTE far *)GXgetSoundMem(SizeT);


//    Imd->S.Sound.Start=(char *) DataB;
    Imd->S.Sound.Start=(char far *) DataB;
    Imd->S.Release=DataB;	/* Pointeur … Releaser au CloseIMD */

    if (DataB)
    {
//       +--------------------------------------------------------+
//       +   Initialisation des buffers sonores … Silence (0x80)  +
//       +--------------------------------------------------------+
      for (i=0;i!=Imd->NbBuffer;i++)
      {
//	Imd->S.Buffer[i]=(UBYTE *)((UBYTE  *)((UBYTE  *)(DataB)+(LONG)i*(LONG)Imd->Bytes));
	Imd->S.Buffer[i]=(UBYTE far *)((UBYTE far *)((UBYTE far *)(DataB)+(LONG)i*(LONG)Imd->Bytes));
	Imd->S.Zero[i]=TRUE;
      }
//far      memset(Imd->S.Buffer[0],128,SizeT);
      _fmemset(Imd->S.Buffer[0],128,SizeT);
    }
    Imd->S.Sound.Size=SizeT;
    Imd->S.Sound.Freq=-Imd->Freq;
    Imd->S.Sound.Bits=8;		// Champ … laisser existant (Bits dans SOUND *)
    Imd->S.TotalBuffers=0;
    Imd->S.CurrentBuffer=0;
    if (!(pal & VMD_SKIPPLAY))
    {
      Imd->OpenInfo|=VMD_SOUND;
      if (!SoundTrack || (pal & VMD_NOTRACK))
      {
	CurrentSoundIMD=Imd;
	IMDinProcess=TRUE;
      }
    }
  }
  Imd->FixedParams=fp;

//       +--------------------------------------------------------+
//       +   On alloue les IMD_ptr                                +
//       +--------------------------------------------------------+

  if (pal & VMD_ALLOCPTR)
  {
    Imd->OpenInfo|=0x8000;
    AllocIMD_ptr(Imd);
  }

//       +--------------------------------------------------------+
//       +   On se positionne pour le ViewIMD                     +
//       +--------------------------------------------------------+
  if (CachePresent)
    X_SeekCache(h,Imd->Start,SEEK_SET);
  else
    GXlseek(h,Imd->Start,SEEK_SET);
  return(Imd);
}




void
CloseIMD(IMD * Imd)
{
  if (Imd->NbTrack==-1 && X_CloseVIG)
  {
    X_CloseVIG(Imd);
    return;
  }
  InAnnexClose=TRUE;
  if (Imd->OpenInfo & VMD_AUTOPLAY)
    TrackAutoPlay--;
  if (Imd->OpenInfo & 0x8000)
    AllocIMD_ptr(NULL);
  if (DataInstall && (Imd->OpenInfo & VMD_DATA))
  {
    X_DataMerge(0,0,NULL);
    CurrentDataIMD=NULL;
  }
  if (CurrentSoundIMD==Imd || (SoundTrack && (Imd->OpenInfo & VMD_SOUND)))
  {
    if (SoundTrack && !(Imd->OpenInfo & VMD_NOTRACK))
    {
      X_StopSound(Imd->S.Track,0);
      IMDinProcess--;
      if (IMDinProcess==FALSE)
	IMDinProcess=TRUE;
    }
    else
    {
      GXstopSound(0);
      CurrentSoundIMD=NULL;
      IMDinProcess=FALSE;
    }
  }
  if (Imd->Type & VMD_SOUND)
    if (Imd->S.Release)
// HOOK - May have to be done differently -TM
//      ReleaseMem(Imd->S.Release);

//	GlobalUnlock(hAudBuf);
//	GlobalFree(hAudBuf);

	GXreleaseSoundMem(Imd->S.Release);

  if (CachePresent)
    X_CloseCache(Imd->h);
  else
    GXclose(Imd->h);
  if (Imd->TempPal && Imd->TempPal!=(UBYTE *) Imd->Pal)
    ReleaseMem(Imd->TempPal);
  ReleaseMem(Imd->PictInfo);
  ReleaseMem(Imd->BlocInfo);
  ReleaseMem(Imd);
  LastTimerVariable=-1;
  InAnnexClose=FALSE;
}

static BOOL
WaitSync(IMD *Imd,WORD w)
{
  if (IMDinProcess>=UNKNOW)
  {
    LONG T;
    WORD WaitSnd,dv;
    LONG T2;

    WaitSnd=Imd->S.TotalBuffers+1-Imd->NbBuffer;	// Buffer à attendre
    if (Imd->SoundInfo & VMD_SOUND_DECAL1)
      WaitSnd++;
    if (SoundTrack)
      T=(LONG)WaitSnd*(LONG)Imd->Bytes*(LONG)TrackFreq/(LONG)Imd->Freq;
    else
      T=(LONG)WaitSnd*(LONG)Imd->Bytes;
    if (w)
      if (GetSoundPos(TRUE)<T)
	return(TRUE);
    do
    {
      T2=GetSoundPos(TRUE);
    }
    while (T2<T);
    T=T2-T;
    T/=(LONG)Imd->Bytes;
    dv=T-WaitSnd;
    if (dv>Imd->NbBuffer/4)
    {
      if (Imd->SoundInfo & VMD_SOUND_SOUNDFIRST)
	SkipNext=TRUE;
      else
      {
	Imd->S.CurrentBuffer+=dv+1;
	Imd->S.TotalBuffers+=dv+1;
	Imd->S.CurrentBuffer%=(Imd->NbBuffer-1);
      }
    }
  }
  return(FALSE);
}

ULONG 
ViewIMD(IMD *Imd,WORD n)
{
  PICTINFO *PictInfo;
  BLOCINFO  *BlocInfo;
  WORD h,bl;
  ULONG Ans=0;
  BOOL LaunchSound=FALSE,Waiting=FALSE,OkTime=FALSE,NeedAssignPal=FALSE;
  if (Imd->NbTrack==-1 && X_ViewVIG)
  {
    X_ViewVIG(Imd,n);
    return(2);
  }
  IMD_msg=0;
  if (!Imd)
    return(VMD_ERROR);
  if (!IMD_ptr && Imd->Bmp)
    if (WindowName[0]!='>')
	 {
  		msgMgr->Alert(SrcLoc,"VMD error %d -> IMD_ptr NULL",1);
		VMD_Shutdown();
	 }
#ifdef __TRACE__
  PRINTF("View frame : %d, Timer = %ld, SoundPos = %ld, CurrentBuffer = %d, TotalBuffers = %d\n",
  	n,
  	ReadTimer(), 
  	IMDSoundPos(Imd),
  	(WORD) Imd->S.CurrentBuffer,
  	Imd->S.TotalBuffers);
#endif
  InViewIMD=TRUE;
  h=Imd->h;
  if (n==-1)
    n=Imd->NbPict;
  if (n==1 && !Imd->Pos)
  {
    Imd->SoundInfo|=VMD_DECAL1;
    n=0;
  }
  Ans|=VMD_NULL;
  if (n)
  if (!(Imd->SoundInfo & VMD_DECAL1) || n!=1)
  {
    WORD n2=n;

    if (Imd->SoundInfo & VMD_DECAL1)
      n2--;
    if (SoundTrack && !(Imd->OpenInfo & VMD_NOTRACK))
    {
      LONG d;
      LONG Tb;

      if (Imd->SoundInfo & VMD_NOWAIT)
      {
	{
	  LONG Delta;

	  if ((Imd->Type & VMD_SOUND) && RefSoundFrequence && Imd->S.Sound.Start)
	  {
	    Tb=GetSoundPos(FALSE)-(ULONG)Imd->StartCount;
	    if (SoundTrack && Imd->Freq<20000)
	      Tb=Tb*100L/(LONG)(RefSoundFrequence/10);
	    else
	      Tb=Tb*100L/(LONG)(Imd->Freq/10);
	    Delta=ReadTimer()-Imd->StartTime-Tb;
	    if (Delta<0L)
	      Delta=0;
	    if (Delta>150L)
	      Imd->ToAddTime=TRUE;
	    else
	    if (Delta<60)
	      Imd->ToAddTime=FALSE;
	    if (Imd->ToAddTime)
	      Imd->StartTime++;
	    Tb=ReadTimer()-Imd->StartTime;
	  }
	  else
	    Tb=ReadTimer()-Imd->StartTime;
	}
	if (Imd->Type & VMD_SOUND)
	{
	  if (n2>500)
	    d=(LONG)n2*100L*(LONG)Imd->Bytes/(LONG)(Imd->Freq/10);
	  else
	    d=(LONG)n2*1000L*(LONG)Imd->Bytes/(LONG)Imd->Freq;
	  if (Tb<d)
	    Waiting=TRUE;
	}
	else
	{
	  d=(LONG)n2*1000L/(LONG)Imd->Freq;
	  if (Tb<d)
	    Waiting=TRUE;
	}
      }
    }
    else
    {
      if (Imd->Type & VMD_SOUND)
      {
	if (Imd->OpenInfo & VMD_SOUND)
	if (IMDinProcess)
	  if (WaitSync(Imd,Imd->SoundInfo & VMD_NOWAIT))
	    Waiting=TRUE;
      }
      else
      {
	LONG d;

	if (Imd->SoundInfo & VMD_NOWAIT)
	{
	  if (i_InsertPict==TRUE)
	  {
	    DeltaSynchro+=i_InsertPict;
	    i_InsertPict=UNKNOW;
	  }

	if(!Imd->Freq)	// kk
		Imd->Freq = 10;

	  d=Imd->StartTime+(LONG)(n2+DeltaSynchro)*1000L/(LONG)Imd->Freq;
	  if (ReadTimer()<d)
	    Waiting=TRUE;
	}
      }
    }
#ifdef  __TRACE__
    if (Waiting)
      printf("Wait\n");
    else
      printf("Play\n");
#endif
    if (Waiting)
    {
      if (CachePresent)
	if (Imd->OpenInfo & VMD_CACHE)
	  if (Imd->OpenInfo & VMD_PRELOAD)
	    X_LoadCache(Imd,1);
      InViewIMD=FALSE;
      return(VMD_WAITING);
    }
  }

  if (i_InsertPict==UNKNOW)
  {
    i_InsertPict=FALSE;
    return(0);
  }
  if (n>=Imd->NbPict)
  {
    Imd->Pos=n+1;
    if (Imd->S.Sound.Start)
    {
      if (!Imd->S.Zero[Imd->S.CurrentBuffer])
      {
//far	memset(Imd->S.Buffer[Imd->S.CurrentBuffer],128,Imd->Bytes);
	_fmemset(Imd->S.Buffer[Imd->S.CurrentBuffer],128,Imd->Bytes);
	Imd->S.Zero[Imd->S.CurrentBuffer]=TRUE;
      }
      Imd->S.LastPtr=Imd->S.Buffer[Imd->S.CurrentBuffer];
      Imd->S.LastSize=Imd->Bytes;
      Imd->S.CurrentBuffer++;
      Imd->S.TotalBuffers++;
      Imd->S.CurrentBuffer%=Imd->NbBuffer;
    }

    InViewIMD=FALSE;
    return(VMD_NULL);
  }
  PictInfo=&Imd->PictInfo[n];
  BlocInfo=&Imd->BlocInfo[n*Imd->NbTrack];
  IMD_msg=PictInfo->Message;
  if (IMD_msg)
    Ans|=VMD_MSG;

  if (n!=Imd->Pos)
  {
    Ans|=VMD_MOVE;
    Imd->Pos=n;
    if (CachePresent)
      X_SeekCache(h,PictInfo->Offset,SEEK_SET);
    else
      GXlseek(h,PictInfo->Offset,SEEK_SET);
  }

  IMD_X1=-1;

  if (Imd->Bmp && (n==0))
  	{
		// Palette
    	if (Imd->TempPal)
      	memcpy(Imd->TempPal,Imd->Pal,768);
    	if (!(Imd->Type & VMD_NOPAL2) && (Imd->Type & VMD_PAL))
      	AssignColorsPtr(0,255,0,Imd->Pal);
  	}
  if (!n)
    Ans|=VMD_PAL;
  for (bl=0;bl!=Imd->NbTrack && BlocInfo->Type;bl++,BlocInfo++)
  {
    BOOL Proceed=FALSE;

    switch(BlocInfo->Type)
    {


//		  +-----------------------+
//		  +	Datas Bulles        +
//		  +-----------------------+
      case BULLE_BLOCK:
	i_InsertBulle=BlocInfo->More.Bulle.Bulle;
	strncpy(i_InsertFileDAT,BlocInfo->More.Bulle.Name_DAT,8);
	i_InsertFileDAT[8]=0;
	Ans|=VMD_BULLE;
	Proceed=TRUE;
	break;


//		  +-----------------------+
//		  +	Datas Graphiques    +
//		  +-----------------------+
      case GRAPH_BLOCK:
      {
	WORD X,Y,SizeX,SizeY;
	UBYTE *OldPtr=IMD_ptr;
	LONG Size;
	WORD IX1,IY1,IX2,IY2;
	LONG T;

	if (SkipNext) {
	  break;
}

	if (!Imd->Bmp)
	  break;
	Proceed=TRUE;
	Ans&=~VMD_NULL;
	X=Imd->X;
	Y=Imd->Y;
	SizeX=Imd->SizeX;
	SizeY=Imd->SizeY;
	Ans|=BlocInfo->More.Pict.Mode;
	IX1=Imd->X=BlocInfo->More.Pict.X1;
	IY1=Imd->Y=BlocInfo->More.Pict.Y1;
	IX2=Imd->SizeX=BlocInfo->More.Pict.X2-Imd->X+1;
	IY2=Imd->SizeY=BlocInfo->More.Pict.Y2-Imd->Y+1;
	IX2+=IX1-1;
	IY2+=IY1-1;
	if (IMD_X1==-1)
	{
	  IMD_X1=IX1;
	  IMD_X2=IX2;
	  IMD_Y1=IY1;
	  IMD_Y2=IY2;
	}
	else
	{
	  if (IX1<IMD_X1)
	    IMD_X1=IX1;
	  if (IX2>IMD_X2)
	    IMD_X2=IX2;
	  if (IY1<IMD_Y1)
	    IMD_Y1=IY1;
	  if (IY2>IMD_Y2)
	    IMD_Y2=IY2;
	}

	IMDread(n,h,IMD_ptr,Size=BlocInfo->Size);

	i_LastSize=Size;
	if (i_CheckCPU)
	  T=ReadTimer();
	if (BlocInfo->More.Pict.Mode & VMD_PAL)
	{
	  WORD c1,c2;
	  BOOL decal=FALSE;

	  c1=IMD_ptr[0];
	  c2=IMD_ptr[1];
	  IMD_ptr+=2;
	  if (Imd->TempPal)
	  {
	    WORD i;

	    if ((Imd->OpenInfo & 1) && c1==0 && c2==255)
	    {
	      decal=TRUE;
	      for (i=0;i!=256;i++)
	      {
		if ((Imd->TempPal[i * 3]!=IMD_ptr[i*3]) ||
		    (Imd->TempPal[i * 3 + 1] !=IMD_ptr[i*3+1]) ||
		    (Imd->TempPal[i * 3 + 2] !=IMD_ptr[i*3+2]))
		  break;
	      }
	      c1=i;
	      if (c1!=256)
	      {
		for (i=255;i>c1;i--)
		{
		  if ((Imd->TempPal[i * 3] !=IMD_ptr[i*3]) ||
		      (Imd->TempPal[i * 3 + 1] !=IMD_ptr[i*3+1]) ||
		      (Imd->TempPal[i * 3 + 2] !=IMD_ptr[i*3+2]))
		    break;
		}
		c2=i;
	      }
	      else
		c2=256;
	      if (c1!=c2)
		memcpy(Imd->TempPal +c1,IMD_ptr+c1*3,(c2-c1+1)*3);
	    }
	    else
	      memcpy(Imd->TempPal+c1,IMD_ptr,(c2-c1+1)*3);
	  }
	  i_NeedAssignC1=c1;
	  i_NeedAssignC2=c2;
	  if (!(Imd->Type & VMD_NOPAL2))
	  {
	    WORD m;
	    UBYTE *ptr=IMD_ptr;

	    for (m=c1;m<=c2;m++)
	      Video_R[m]=Video_V[m]=Video_B[m]=0;
	    AssignColors(c1,c2);
	    NeedAssignPal=TRUE;
	    if (decal)
	      ptr+=c1*3;
	    for (m=c1;m<=c2;m++)
	    {
	      Video_R[m]=*ptr++;
	      Video_V[m]=*ptr++;
	      Video_B[m]=*ptr++;
	    }
	  }
	  if (decal)
	  {
	    IMD_ptr+=768;
	    Size-=768+2;
	  }
	  else
	  {
	    IMD_ptr+=(c2-c1+1)*3;
	    Size-=(c2-c1+1)*3+2;
	  }
	  Ans|=VMD_PAL;
	}
	if (Size)
	{
	  if (Imd->Y+Imd->SizeY>Imd->Bmp->SizeY)
	    Imd->SizeY=Imd->Bmp->SizeY-Imd->Y;
	  if (Imd->SizeY>0)
	  {
	  	DoIMD_VESA(Imd);
	  }
	}
	if (i_CheckCPU)
	  i_CPU+=ReadTimer()-T;
	Imd->X=X;
	Imd->Y=Y;
	Imd->SizeX=SizeX;
	Imd->SizeY=SizeY;
	IMD_ptr=OldPtr;
	break;
      }


//		  +-----------------------+
//		  +	Datas Track         +
//		  +-----------------------+
      case DATA_BLOCK:
	if (!DataInstall)
	  break;
	if (!(Imd->OpenInfo & VMD_DATA))
	  break;
	Proceed=X_DataMerge(n,h,BlocInfo);
	if (Proceed)
	{
	  WORD i;
	  BLOCINFO  *BlocInfo;
	  WORD bl;

	  NextDataPict=32000;
	  for (i=n+1;i<Imd->NbPict;i++)
	  {
	    BlocInfo=&Imd->BlocInfo[i*Imd->NbTrack];
	    for (bl=0;bl!=Imd->NbTrack && BlocInfo->Type;bl++,BlocInfo++)
	    {
	      if (BlocInfo->Type==DATA_BLOCK)
	      {
		NextDataPict=i;
		break;
	      }
	    }
	    if (NextDataPict!=32000)
	      break;
	  }
	}
	break;

//		  +-----------------------+
//		  +	Datas Sonores		  +
//		  +-----------------------+
      case SND_BLOCK:
      {
	if (!Imd->S.Sound.Start)
	  break;
	Proceed=TRUE;
	switch(BlocInfo->More.Sound.Mode)
	{
	  case SILENT_SOUND:
	    if (!Imd->S.Zero[Imd->S.CurrentBuffer])
	    {
//far	      memset(Imd->S.Buffer[Imd->S.CurrentBuffer],128,Imd->Bytes);
	      _fmemset(Imd->S.Buffer[Imd->S.CurrentBuffer],128,Imd->Bytes);
	      Imd->S.Zero[Imd->S.CurrentBuffer]=TRUE;
	    }
	    Imd->S.LastPtr=Imd->S.Buffer[Imd->S.CurrentBuffer];
	    Imd->S.LastSize=Imd->Bytes;
	    break;
	  case CURRENT_SOUND:
	    Imd->S.Zero[Imd->S.CurrentBuffer]=FALSE;
	    if (Imd->FixedParams & (VMD_11K+VMD_8BITS))
	    {
//	      UBYTE *p,*p2;
	      UBYTE far *p;
	      UBYTE *p2;
	      WORD j;

	      IMDread(n,h,i_TempResample,Imd->S.oBytes);
	      p=Imd->S.Buffer[Imd->S.CurrentBuffer];
	      p2=i_TempResample;
	      if ((Imd->FixedParams & VMD_11K) &&
		  (Imd->FixedParams & VMD_8BITS))
	      {
		for (j=0;j!=Imd->Bytes;j++,p2+=4)
		  *p++=*p2;
	      }
	      else
	      {
		for (j=0;j!=Imd->Bytes;j++,p2+=2)
		  *p++=*p2;
	      }
	    }
	    else
//far	      IMDread(n,h,Imd->S.Buffer[Imd->S.CurrentBuffer],Imd->Bytes);
	      _fIMDread(n,h,Imd->S.Buffer[Imd->S.CurrentBuffer],Imd->Bytes);
	    Imd->S.LastPtr=Imd->S.Buffer[Imd->S.CurrentBuffer];
	    Imd->S.LastSize=Imd->Bytes;
	    break;
	  case START_SOUND:
	  {
	    ULONG Pack;
	    WORD i,d;

	    if (Imd->OpenInfo & VMD_SOUND)
	    {
	      if (SoundTrack && !(Imd->OpenInfo & VMD_NOTRACK))
	      {
		if (Imd->S.Track!=9)
		{
		  X_StopSound(Imd->S.Track,0);
		  IMDinProcess--;
		  if (IMDinProcess==FALSE)
		    IMDinProcess=TRUE;
		}
	      }
	      else
	      {
		GXstopSound(0);
		IMDinProcess=TRUE;
	      }
	      LaunchSound=TRUE;
	    }
	    SkipNext=FALSE;
	    IMDread(n,h,&Pack,4);
	    if (Imd->SoundInfo & VMD_SOUND_DECAL1)
	      d=1;
	    else
	      d=0;
	    if (Imd->SoundInfo & VMD_DECAL1)
	    {
//far	      IMDread(n,h,Imd->S.Buffer[0],Imd->Bytes);
	      _fIMDread(n,h,Imd->S.Buffer[0],Imd->Bytes);
	      d++;
	    }
	    for (i=0;i!=Imd->NbBuffer-d;i++)
	    {
	      Imd->S.Zero[i]=FALSE;
	      if (Pack & 1)
//far		memset(Imd->S.Buffer[i],128,Imd->Bytes);
		_fmemset(Imd->S.Buffer[i],128,Imd->Bytes);
	      else
	      {
		if (Imd->FixedParams & (VMD_11K+VMD_8BITS))
		{
//		  UBYTE *p,*p2;
		  UBYTE far *p;
		  UBYTE *p2;
		  WORD j;

		  IMDread(n,h,i_TempResample,Imd->S.oBytes);
		  p=Imd->S.Buffer[i];
		  p2=i_TempResample;
		  if ((Imd->FixedParams & VMD_11K) &&
		      (Imd->FixedParams & VMD_8BITS))
		  {
		    for (j=0;j!=Imd->Bytes;j++,p2+=4)
		      *p++=*p2;
		  }
		  else
		  {
		    for (j=0;j!=Imd->Bytes;j++,p2+=2)
		      *p++=*p2;
		  }
		}
		else
//far		  IMDread(n,h,Imd->S.Buffer[i],Imd->Bytes);
		  _fIMDread(n,h,Imd->S.Buffer[i],Imd->Bytes);
	      }
	      Pack>>=1;
	    }
	    Imd->S.LastPtr=Imd->S.Buffer[0];
	    Imd->S.LastSize=Imd->Bytes*(Imd->NbBuffer-d);
	    Imd->S.CurrentBuffer=Imd->NbBuffer-1-d;
	    LastWhere=-1;
	    Imd->S.TotalBuffers=Imd->S.CurrentBuffer;
	    break;
	  }
	}
	break;
      }
    }
    if (!Proceed)
    {
      if (BlocInfo->Size)
      {
	if (CachePresent)
	  X_SeekCache(h,BlocInfo->Size,SEEK_CUR);
	else
	  GXlseek(h,BlocInfo->Size,SEEK_CUR);
      }
    }
  }
  if (CachePresent)
    X_EnableCache(Imd->h,n);
  if (Ans & VMD_NULL)
    Ans|=VMD_DIFF|VMD_MINILIST;
  Imd->Pos++;
  if (Imd->Type & VMD_SOUND)
  {
    Imd->S.CurrentBuffer++;
    Imd->S.TotalBuffers++;
    Imd->S.CurrentBuffer%=Imd->NbBuffer;
  }
  if (Imd->OpenInfo & VMD_FORCESOUND)
    if (!n)
    {
      LaunchSound=TRUE;
      Imd->S.CurrentBuffer=Imd->NbBuffer-1;
      if (Imd->SoundInfo & VMD_SOUND_DECAL1)
	Imd->S.CurrentBuffer--;
      LastWhere=-1;
      Imd->S.TotalBuffers=Imd->S.CurrentBuffer;
    }
  if (Imd->OpenInfo & VMD_SOUND)
  {
    if (LaunchSound)
    {
      if (SoundTrack && !(Imd->OpenInfo & VMD_NOTRACK))
      {
	NextTrackImd=Imd;
	NextTrackLevel=Imd->S.Level;
	if (!n)
	{
	  Imd->StartTime=ReadTimer();
	  OkTime=TRUE;
	}
	Imd->S.Track=X_StartSound(999,&Imd->S.Sound,0,0);
	Imd->StartCount=i_TrackSoundStart;
	Imd->ToAddTime=FALSE;
	IMDinProcess++;
      }
      else
      {
//	GXstartSound(&Imd->S.Sound,0,0);
	GXstartSound(&Imd->S.Sound,0,Imd->Freq);
	if (SoundInProcess)
	  IMDinProcess=UNKNOW;
      }
    }
    if (n==Imd->NbPict-1 && (!SoundTrack || (Imd->OpenInfo & VMD_NOTRACK)))
      WaitSync(Imd,FALSE);
    if (SkipNext)
    {
      SkipNext=FALSE;
      Ans|=VMD_TIMEOUT;
    }
  }
  if (NeedAssignPal)
    AssignColors(i_NeedAssignC1,i_NeedAssignC2);
  if (!n && !OkTime)
  {
    Imd->StartTime=ReadTimer();
    Imd->StartCount=GetSoundPos(FALSE);
  }
  InViewIMD=FALSE;
  if (n==0 && (Imd->SoundInfo & VMD_DECAL1))
  {
    ViewIMD(Imd,1);
    IMD_X1=Imd->X;
    IMD_Y1=Imd->Y;
    IMD_X2=Imd->SizeX+IMD_X1-1;
    IMD_Y2=Imd->SizeY+IMD_Y1-1;
  }
#ifdef WINDOWS
  if (vmdDouble && IMD_X1!=-1)
  {
    IMD_X1*=2;
    IMD_Y1*=2;
    IMD_X2*=2;
    IMD_Y2*=2;
    IMD_X2++;
    IMD_Y2++;
  }
#endif
  return(Ans);
}



BLOCINFO*
GetActiveTrack(IMD *Imd,WORD n,WORD mode)
{
  BLOCINFO  *B;
  WORD i=n*Imd->NbTrack,j=0;

  if (n>=Imd->NbPict)
    return(NULL);
  if (Imd->NbTrack==-1)
    return(NULL);
  B=&Imd->BlocInfo[i];
  while (j<Imd->NbTrack && B->Type!=mode && B->Type)
  {
    B++;
    j++;
  }
  if (j==Imd->NbTrack)
    return(NULL);
  if (!B->Type)
    return(NULL);
  return((BLOCINFO *)B);
}

void 
SetIMD_XY(IMD *Imd,WORD X,WORD Y)
{
  WORD i;

  if (Imd->NbTrack==-1)
  {
    Imd->X=X;
    Imd->Y=Y;
    return;
  }
  for (i=0;i!=Imd->NbPict*Imd->NbTrack;i++)
  {
    if (Imd->BlocInfo[i].Type==GRAPH_BLOCK)
    {
      Imd->BlocInfo[i].More.Pict.X1-=Imd->X;
      Imd->BlocInfo[i].More.Pict.Y1-=Imd->Y;
      Imd->BlocInfo[i].More.Pict.X2-=Imd->X;
      Imd->BlocInfo[i].More.Pict.Y2-=Imd->Y;
      Imd->BlocInfo[i].More.Pict.X1+=X;
      Imd->BlocInfo[i].More.Pict.Y1+=Y;
      Imd->BlocInfo[i].More.Pict.X2+=X;
      Imd->BlocInfo[i].More.Pict.Y2+=Y;
    }
  }
  Imd->X=X;
  Imd->Y=Y;
}
