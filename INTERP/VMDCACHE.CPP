//**************************************************************************
//                                                                          
//   Copyright (c)1995,M.D.O.   Programmeur:Mathieu___  Cr‚ation:??/??/??   
//                                                                          
//   Portage :  [X]:PC  [_]:MAC  [_]:CDI  [_]:3DO  [_]:SATURN  [_]:PSX      
//              [_]:Sp‚cifique                                              
//                                                                          
//   Type de source :  [_]:Interne   [_]:Externe   [X]:Grand public         
//                                                                          
//   Programme rattach‚ : [X]:Librairies  [_]:Loader                        
//			  [_]:Autre : ______________                        
//                                                                          
//**************************************************************************

#include "sol.hpp"
#include "memmgr.hpp"
#include "msg.hpp"

#include "VMDLIBS.HPP"
#include "VMD.HPP"


//----------------------------------------------------------------------

extern	int	LoopPreLoad(int,int);
extern	WORD	i_NbPreload,i_CountPreload;
extern	BOOL	CachePresent,InAnnex;
extern	BOOL	volatile InInterTrack;
extern	void	(*X_CloseCache)(WORD);
extern	void	InitLooping(WORD);
extern	WORD	(*X_OpenCache)(char *,LONG);
extern	void	(*X_ReadCache)(WORD,WORD,void *,LONG);
extern	void	(*X_SeekCache)(WORD,LONG,WORD);
extern	void	(*X_InstallCache)(IMD *);
extern	void	(*X_EnableCache)(WORD,WORD);
extern	void	(*X_TrackLoop)(void);
extern	BOOL	SoundTrack;
extern	void	(*X_LoadCache)(void *,WORD);
extern	void	LoadCache(void *ptr,WORD nb);
extern	BOOL	InLooping(WORD,WORD);

//----------------------------------------------------------------------

	BOOL		i_FullCache;
	void		(*X_DuringLoadCache)(void);
	CACHE*	i_Cache;
	WORD		i_SizeMax,i_SizeTotal,i_LoopFreeMem;
	WORD		i_NbCache;

//----------------------------------------------------------------------

static	WORD	SizeUsed;
static	BOOL	SpMode;

//----------------------------------------------------------------------

static	BOOL	IsString(void *ptr);
static	void	Clip(char *str);
static	WORD	GetCache(void *ptr);
static	void	ReleaseCache(WORD n);
static	WORD	IT_OpenCache(char *Name,LONG mode);
static	void	IT_EnableCache(WORD h,WORD n);

/* ----------------------------------------------------------------------*/

static void 
Clip(char * str)
{
  char str2[20];
  WORD i;

  if (!str[0])
    return;
  for (i=strlen(str)-1;i && str[i]!='\\' && str[i]!=':';i--);
  if (i)
    strcpy(str2,str+i+1);
  else
    strcpy(str2,str);
  for (i=0;str2[i] && str2[i]!='.';i++);
  str2[i]=0;
  strcpy(str,str2);
}

static BOOL 
IsString(void *ptr)
{
  char a;
  char *pt;

  pt=(char *) ptr;
  a=*pt++;
  if (a<32)
    return(FALSE);
  if (!((a>='a' && a<='z') || (a>='A' && a<='Z') || (a>='0' && a<='9')))
    return(FALSE);
  a=*pt++;
  if (a<32)
    return(FALSE);
  if (!((a>='a' && a<='z') || (a>='A' && a<='Z') || (a>='0' && a<='9') || (a==':' || a=='\\' || a=='.')))
    return(FALSE);
  a=*pt++;
  if (!a)
    return(TRUE);
  if (a<32)
    return(FALSE);
  if (!((a>='a' && a<='z') || (a>='A' && a<='Z') || (a>='0' && a<='9') || (a=='\\' || a=='.')))
    return(FALSE);
  a=*pt;
  if (!a)
    return(TRUE);
  if (a<32)
    return(FALSE);
  if (!((a>='a' && a<='z') || (a>='A' && a<='Z') || (a>='0' && a<='9') || (a=='\\' || a=='.')))
    return(FALSE);
  return(TRUE);
}


static WORD 
GetCache(void * ptr)
{
  WORD n=-1,i;

  if (IsString(ptr))
  {
    char str[80];

    strcpy(str,(char *) ptr);
    Clip(str);
    for (i=0;i!=i_NbCache;i++)
    {
      if (!i_Cache[i].Size)
			continue;
      if (!stricmp(i_Cache[i].Name,str))
      {
			n=i;
			break;
      }
    }
  }
  else
  {
    IMD *Imd;

    Imd=(IMD *) ptr;
    n=Imd->h;
    if (n<0)
      n=-100-n;
    else
      n=-1;
  }
  return(n);
}

BOOL 
InCache(void * ptr)
{
  WORD n,i;

  n=GetCache(ptr);
  if (n==-1)
    return(FALSE);
  if (i_Cache[n].Looping)
    return(InLooping(n,-1));
  if (!i_Cache[n].Normal->Cached)
    return(FALSE);
  for (i=0;i!=i_Cache[n].NbPict;i++)
  {
    if (!i_Cache[n].Normal->Cached[i])
      return(FALSE);
  }
  return(TRUE);
}

static void 
ReleaseCache(WORD n)
{
  SizeUsed -= i_Cache[n].Size/1024;
  if (SizeUsed<0)
    SizeUsed=0;
  i_Cache[n].Size=0;
  ReleaseMem(i_Cache[n].Data);
  if (i_Cache[n].Looping)
  {
    ReleaseMem(i_Cache[n].Loop);
  }
  else
  {
    ReleaseMem(i_Cache[n].Normal->Cached);
    ReleaseMem(i_Cache[n].Normal);
  }
  memset(&i_Cache[n],0,sizeof(CACHE));
}

void 
FlushCache(void *ptr)		// Si NULL : Tout
			// ptr = Si Chaine de chars, Nom
{				// Si IMD *, l'imd.
  WORD n,i;

  if (!ptr)
  {
    for (i=0;i!=i_NbCache;i++)
    {
      if (!i_Cache[i].Size)
			continue;
      if (i_Cache[i].Used & CACHE_USING)
			i_Cache[i].Used|=CACHE_FLUSH;
      else
		ReleaseCache(i);
    }
  }
  else
  {
    n=GetCache(ptr);
    if (n==-1)
      return;
    if (i_Cache[n].Used & CACHE_USING)
      i_Cache[n].Used|=CACHE_FLUSH;
    else
      ReleaseCache(n);
  }
}

static void 
IT_CloseCache(WORD h)
{
  WORD i;
  UBYTE Use;

  if (h>0)
  {
    GXclose(h);
    return;
  }
  h=-100-h;
  Use=i_Cache[h].Used;
  i_Cache[h].Used=FALSE;
  if (Use & CACHE_VDUPLICATE)
  {
    i_Cache[h].Size=0;
    memset(&i_Cache[h],0,sizeof(CACHE));
    return;
  }
  for (i=0;i!=i_NbCache;i++)
  {
    if (i_Cache[i].Size)
    {
      if (i_Cache[i].Used & CACHE_VDUPLICATE)
      {
			if (i_Cache[i].Data==i_Cache[h].Data)
			{
	  			i_Cache[i].Used&=~CACHE_VDUPLICATE;
	  			i_Cache[h].Size=0;
	  			memset(&i_Cache[h],0,sizeof(CACHE));
	  			return;
			}
      }
    }
  }
  if (i_Cache[h].Looping)
  {
    GXclose(i_Cache[h].h);
    ReleaseCache(h);
    return;
  }
  if (!i_Cache[h].Writing)
  {
    if (Use & CACHE_FLUSH)
      ReleaseCache(h);
    return;
  }
  if (!(i_Cache[h].Imd->OpenInfo & VMD_AUTOPLAY))
  {
    for (i=0;i!=i_Cache[h].NbPict;i++)
    {
      if (!i_Cache[h].Normal->Cached[i] || (Use & CACHE_FLUSH))
      {
			GXclose(i_Cache[h].h);
			ReleaseCache(h);
			return;
      }
    }
  }
  GXclose(i_Cache[h].h);
}

static WORD 
IT_OpenCache(char * Name,LONG mode)
{
  WORD 	i,n;
  size_t S;
  LONG	Sreal,T;
  WORD 	OAC=AutoMemCheck;
  char 	str[80];

  strcpy(str,Name);
  Clip(str);

  if (!SpMode)
  for (i=0;i!=i_NbCache;i++)
  {
    if (!i_Cache[i].Size)
      continue;
    if (!stricmp(i_Cache[i].Name,str))
    {
      if (i_Cache[i].Used & CACHE_USING)
      {
			WORD old=i;

			for (i=0;i!=i_NbCache;i++)
	  			if (!i_Cache[i].Size)
	    			break;
			if (i==i_NbCache)
	  			return(-1);
			memcpy(&i_Cache[i],&i_Cache[old],sizeof(CACHE));
			i_Cache[i].Used|=CACHE_VDUPLICATE;
      }
      else
			i_Cache[i].Used=CACHE_USING;
      i_Cache[i].Writing=FALSE;
      i_Cache[i].Offset=0;
      i_Cache[i].Time=ReadTimer();
      if (i_Cache[i].Looping)
      {
			if (!(i_Cache[i].Used & CACHE_VDUPLICATE))
			{
	  			i_Cache[i].h=GXopen(Name,O_RDONLY);
	  			i_Cache[i].Loop->Cpos=0L;
	  			if (i_Cache[i].h==-1)
				{
  					msgMgr->Alert(SrcLoc,"VMD error %d -> Open Cache %s",301,Name);
					VMD_Shutdown();
				}
			}
      }
      else
      {
			i_Cache[i].Normal->Cpos=0L;
			if (i_Cache[i].Normal->Cached)
			{
	  			WORD j;

	  			for (j=0;j!=i_Cache[i].NbPict;j++)
	    			if (!i_Cache[i].Normal->Cached[j])
	    			{
	      			i_Cache[i].Writing=UNKNOW;
	      			i_Cache[i].h=GXopen(Name,O_RDONLY);
	      			if (i_Cache[i].h==-1)
	      			{
							i_Cache[i].Used=FALSE;
							return(-1);
	      			}
	      			break;
	    			}
			}
      }
      return(-100-i);
    }
  }

  if (!SpMode)
  {
    WORD h;

    h=GXopen(Name,O_RDONLY);
    if (h==-1)
    {
      if (!IMD_check)
		{
  			msgMgr->Alert(SrcLoc,"VMD error %d -> IMD  unknown : %s",-51,Name);
			VMD_Shutdown();
		}
      else
		{
			return(-1);
		}
    }
    GXclose(h);
  }

  Sreal = GXfilesize(Name);
  S = Sreal;
  if (!i_LoopingCache)
  {
    mode&=~VMD_LOOPCACHE;
    mode|=VMD_CACHE;
  }
  if ((mode & VMD_LOOPCACHE)==VMD_LOOPCACHE)
  {
    if (S > i_LoopingCache*1024L)
    	S = i_LoopingCache*1024;
    if ((memMgr->FreeMemory()/1024 - 200 - S/1024) < i_LoopFreeMem)
      return(GXopen(Name,O_RDONLY));
  }
  else
  {
    if (!SpMode)
    {
      if (i_SizeMax)
			if (S/1024L>i_SizeMax)
	  			return(GXopen(Name,O_RDONLY));
      if (i_SizeTotal)
			if (SizeUsed+S/1024L>i_SizeTotal)
	  			return(GXopen(Name,O_RDONLY));
    }
  }

  n=-1;
  for (i=0;i!=i_NbCache;i++)
  {
    if (!i_Cache[i].Size)
    {
      n=i;
      break;
    }
  }
  if (n==-1)
  {
    T=ReadTimer();
    n=0;
    for (i=0;i!=i_NbCache;i++)
    {
      if (i_Cache[i].Used & CACHE_USING)
			continue;
      if (i_Cache[i].Time<T && !(i_Cache[i].Used & CACHE_LOCKED))
      {
			T=i_Cache[i].Time;
			n=i;
      }
    }
  }
  if (i_Cache[n].Size)
    ReleaseCache(n);
  strcpy(i_Cache[n].Name,str);
  i_Cache[n].Size=S;
  if (Sreal<LOOP_BASE)
    Sreal=LOOP_BASE;
  i_Cache[n].Writing=TRUE;
  if (S<LOOP_BASE)
    S=LOOP_BASE;
  {
    AutoMemCheck|=CHK_6;
    i_Cache[n].Data=(UBYTE *) GetMem(S);
    AutoMemCheck=OAC;
  }
  if (!i_Cache[n].Data)
  {
    i_Cache[n].Size=0;
    return(GXopen(Name,O_RDONLY));
  }

  SizeUsed+=S/1024;
  i_Cache[n].h=GXopen(Name,O_RDONLY);
  if (i_Cache[i].h==-1)
  {
  	 msgMgr->Alert(SrcLoc,"VMD error %d -> Open Cache %s",302,Name);
	 VMD_Shutdown();
  }	
  i_Cache[n].Offset=0;
  i_Cache[n].Time=ReadTimer();
  i_Cache[n].Used=CACHE_USING;
  if ((mode & VMD_LOOPCACHE)==VMD_LOOPCACHE)
  {
    S/=LOOP_BASE;
    i_Cache[n].Looping=i_LoopingCache;
    i_Cache[n].Loop=(CACHE_LOOP *) GetMem(2L*S+sizeof(CACHE_LOOP)-2);
    i_Cache[n].Loop->Cpos=0L;
    i_Cache[n].Loop->NbSector=S;
    i_Cache[n].Loop->UsedSector=0;
    i_Cache[n].Loop->LastSector=-1;
    i_Cache[n].Loop->NextSector=0;
    i_Cache[n].Loop->RealSize=Sreal>>SLOOP_BASE;
    memset(i_Cache[n].Loop->Sector,-1,(UWORD)(2*S));
  }
  else
  {
    i_Cache[n].Looping=0;
    i_Cache[n].Normal=(CACHE_NORMAL *) GetMem(sizeof(CACHE_NORMAL));
    i_Cache[n].Normal->Cached=NULL;
    i_Cache[n].Normal->Cpos=0L;
  }
  return(-100-n);
}


extern	void	LoopRead(WORD,UBYTE *,LONG);

static void IT_ReadCache(WORD n,WORD h,void *ptr,LONG s)
{
  UBYTE  *p;

  if (h>0)
  {
#ifdef	SND
    GXsound(5000);
#endif
    BlocProcess(GXread,h,(UBYTE *) ptr,s);
#ifdef	SND
    GXnosound();
#endif
    return;
  }
  h=-100-h;
  if (i_Cache[h].Looping)
    LoopRead(h,(UBYTE *) ptr,s);
  else
  {
    p=i_Cache[h].Data;
    p+=i_Cache[h].Offset;
    if (i_Cache[h].Writing)
    {
      if (n!=-1)
      {
			if (i_Cache[h].Normal->Cached[n])
			{
	  			if (ptr)
	  	 			memcpy(ptr,p,(size_t) s);
	  			if (!InInterTrack && i_Cache[h].Normal->Cpos!=i_Cache[h].Offset)
	  			{
	    			GXlseek(i_Cache[h].h,i_Cache[h].Offset,SEEK_SET);
	    			i_Cache[h].Normal->Cpos=i_Cache[h].Offset;
	  			}
	  			i_Cache[h].Offset+=s;
	  			return;
			}
      }

#ifdef	SND
      GXsound(5000);
#endif
      BlocProcess(GXread,i_Cache[h].h,(UBYTE *)p,s);
      i_Cache[h].Normal->Cpos+=s;
#ifdef	SND
      GXnosound();
#endif
    }
    if (ptr)
	  	 memcpy(ptr,p,(size_t) s);
  }
  i_Cache[h].Offset+=s;
}

extern	void	_fLoopRead(WORD,UBYTE far *,LONG);

void _fIT_ReadCache(WORD n,WORD h,void far *ptr,LONG s)
{
  UBYTE  *p;

  if (h>0)
  {
#ifdef	SND
    GXsound(5000);
#endif
    _fBlocProcess(_fGXread,h,(UBYTE far *) ptr,s);
#ifdef	SND
    GXnosound();
#endif
    return;
  }
  h=-100-h;
  if (i_Cache[h].Looping)
    _fLoopRead(h,(UBYTE far *) ptr,s);
  else
  {
    p=i_Cache[h].Data;
    p+=i_Cache[h].Offset;
    if (i_Cache[h].Writing)
    {
      if (n!=-1)
      {
			if (i_Cache[h].Normal->Cached[n])
			{
	  			if (ptr)
//	  	 			memcpy(ptr,p,(size_t) s);
	  	 			_fmemcpy(ptr,p,(size_t) s);
	  			if (!InInterTrack && i_Cache[h].Normal->Cpos!=i_Cache[h].Offset)
	  			{
	    			GXlseek(i_Cache[h].h,i_Cache[h].Offset,SEEK_SET);
	    			i_Cache[h].Normal->Cpos=i_Cache[h].Offset;
	  			}
	  			i_Cache[h].Offset+=s;
	  			return;
			}
      }

#ifdef	SND
      GXsound(5000);
#endif
      BlocProcess(GXread,i_Cache[h].h,(UBYTE *)p,s);
      i_Cache[h].Normal->Cpos+=s;
#ifdef	SND
      GXnosound();
#endif
    }
    if (ptr)
//	  	 memcpy(ptr,p,(size_t) s);
	  	 _fmemcpy(ptr,p,(size_t) s);
  }
  i_Cache[h].Offset+=s;
}


static void 
IT_InstallCache(IMD * Imd)
{
  WORD n;

  if (Imd->h>0)
    return;
  n=-100-Imd->h;
  i_Cache[n].Imd=Imd;
  i_Cache[n].NbPict=Imd->NbPict;
  Imd->OpenInfo|=VMD_CACHE;
  if (i_Cache[n].Looping)
    InitLooping(n);
  else
  if (i_Cache[n].Writing==TRUE)
  {
    i_Cache[n].Normal->Cached=(BOOL *) GetMem(Imd->NbPict);
    memset(i_Cache[n].Normal->Cached,0,Imd->NbPict);
  }
}


static void 
IT_SeekCache(WORD h,LONG Ofs,WORD mode)
{
  if (h>0)
  {
    GXlseek(h,Ofs,mode);
    return;
  }
  h=-100-h;
  if (mode==SEEK_SET)
    i_Cache[h].Offset=Ofs;
  else
    i_Cache[h].Offset+=Ofs;
  if (i_Cache[h].Writing && !i_Cache[h].Looping && !InInterTrack)
  if (i_Cache[h].Offset!=i_Cache[h].Normal->Cpos)
  {
    GXlseek(i_Cache[h].h,i_Cache[h].Offset,SEEK_SET);
    i_Cache[h].Normal->Cpos=i_Cache[h].Offset;
  }
}


static void 
IT_EnableCache(WORD h,WORD n)
{
  if (h>0)
    return;
  h=-100-h;
  if (!i_Cache[h].Looping)
    i_Cache[h].Normal->Cached[n]=TRUE;
}

void 
InitCache(WORD nb,WORD sizemax,WORD size,WORD sizeloop)
{
  // nb : Nombre de fichiers
  // nb				-- Sizemax : Taille du plus grand imd
  // sizemax     -- Size:Taille maximale allouŽe au cache
  // size        -- si 0, pas de maximum
  // sizeloop    -- sizeloop:MMfreeMem minimum pour cache tournant
  i_Cache=(CACHE *) GetMem(sizeof(CACHE)*nb+RAZ_MEM);
  i_NbCache=nb;
  i_SizeMax=sizemax;
  i_SizeTotal=size;
  i_LoopFreeMem=sizeloop;
  X_CloseCache=IT_CloseCache;
  X_OpenCache=IT_OpenCache;
  X_ReadCache=IT_ReadCache;
  X_SeekCache=IT_SeekCache;
  X_EnableCache=IT_EnableCache;
  X_InstallCache=IT_InstallCache;
  X_LoadCache=LoadCache;
  CachePresent=TRUE;
  SizeUsed=0;
}


void CloseCache()
{
  WORD i;

  for (i=0;i!=i_NbCache;i++)
  {
    if (i_Cache[i].Size)
    {
      ReleaseMem(i_Cache[i].Data);
      if (i_Cache[i].Looping)
			ReleaseMem(i_Cache[i].Loop);
      else
      {
			ReleaseMem(i_Cache[i].Normal->Cached);
			ReleaseMem(i_Cache[i].Normal);
      }
    }
  }
  if (i_NbCache)
  	ReleaseMem(i_Cache);
  X_CloseCache=NULL;
  X_OpenCache=NULL;
  X_ReadCache=NULL;
  X_SeekCache=NULL;
  X_InstallCache=NULL;
  X_EnableCache=NULL;
  X_LoadCache=NULL;
  CachePresent=FALSE;
}


void 
LoadCache(void * ptr,WORD nb)
{
  // Idem dessus
  // nb=Nombre d'images ˆ charger. Si 0 tout
  WORD n,i;
  IMD *Imd;

  if (nb==-1)
    nb=0;
  n=GetCache(ptr);
  if (n==-1)
  {
    IMD *Imd;

    if (!IsString(ptr))
      return;
    Imd=OpenIMD((char *) ptr,NULL,VMD_CACHE+VMD_NOSOUND);
    if (!Imd)
	 {
  		msgMgr->Alert(SrcLoc,"VMD error %d -> LoadCache : %s unknown",-1,ptr);
		VMD_Shutdown();
	 }
    if (Imd->h>-100)
    {
      CloseIMD(Imd);
      return;
    }
    for (i=0;i!=Imd->NbPict;i++)
    {
      BLOCINFO  *BlocInfo;
      WORD bl;

      BlocInfo=&Imd->BlocInfo[i*Imd->NbTrack];

      for (bl=0;bl!=Imd->NbTrack && BlocInfo->Type;bl++,BlocInfo++)
      {
			switch(BlocInfo->Type)
			{
	  		case GRAPH_BLOCK:
	  		case SND_BLOCK:
	  		case DATA_BLOCK:
	    		IT_ReadCache(i,Imd->h,NULL,BlocInfo->Size);
	    		break;
	  		default:
	    		IT_SeekCache(Imd->h,BlocInfo->Size,SEEK_CUR);
	    		break;
	  		case BULLE_BLOCK:
	    		break;
			}
      }
      IT_EnableCache(Imd->h,i);
    }
    n=-100-Imd->h;
    i_Cache[n].Used|=CACHE_LOCKED;
    CloseIMD(Imd);
    return;
  }
  if (i_Cache[n].Looping)
  {
    if (i_Cache[n].Used & CACHE_VDUPLICATE)
      return;
    InAnnex=TRUE;
    if (nb==1)
    {
      WORD nn=4096/LOOP_BASE;

		nn<<=1;
      i_NbPreload+=LoopPreLoad(n,nn)*(LOOP_BASE/1024);
    }
    else
    {
      WORD nb=0;

      do
      {
			if (SoundTrack)
	  			X_TrackLoop();
			if (X_DuringLoadCache)
          	X_DuringLoadCache();
			nb+=8*4;
			if (nb>=i_CountPreload && i_CountPreload)
	  			break;
      }
      while (LoopPreLoad(n,8*4/(LOOP_BASE/1024))==8*4/(LOOP_BASE/1024));
      i_Cache[n].Loop->LastSector=-1;
    }
    InAnnex=FALSE;
    if (SoundTrack)
      X_TrackLoop();
    return;
  }
  if (!(i_Cache[n].Used & CACHE_USING))
    return;
  Imd=i_Cache[n].Imd;
  for (i=0;i!=Imd->NbPict;i++)
  {
    if (!i_Cache[n].Normal->Cached[i])
    {
      PICTINFO *PictInfo;
      BLOCINFO  *BlocInfo;
      WORD bl;

      if (SoundTrack)
			X_TrackLoop();
      InAnnex=TRUE;
      PictInfo=&Imd->PictInfo[i];
      BlocInfo=&Imd->BlocInfo[i*Imd->NbTrack];
      IT_SeekCache(Imd->h,PictInfo->Offset,SEEK_SET);

      for (bl=0;bl!=Imd->NbTrack && BlocInfo->Type;bl++,BlocInfo++)
      {
			switch(BlocInfo->Type)
			{
	  		case BULLE_BLOCK:
	    		break;
	  		case GRAPH_BLOCK:
	  		case SND_BLOCK:
	  		case DATA_BLOCK:
	    		IT_ReadCache(i,Imd->h,NULL,BlocInfo->Size);
	    		break;
	  		default:
	    		IT_SeekCache(Imd->h,BlocInfo->Size,SEEK_CUR);
	    		break;
			}
      }
      IT_EnableCache(Imd->h,i);
      if (Imd->Pos<Imd->NbPict)
			IT_SeekCache(Imd->h,Imd->PictInfo[Imd->Pos].Offset,SEEK_SET);
      else
			IT_SeekCache(Imd->h,Imd->PictInfo[Imd->NbPict-1].Offset,SEEK_SET);
      InAnnex=FALSE;
      nb--;
      if (!nb)
			break;
    }
  }
  if (SoundTrack)
    X_TrackLoop();
}

WORD GetNextPictCached(void* ptr)
{
  WORD i,n;

  n=GetCache(ptr);
  if (n==-1)
    return(-1);
  if (!(i_Cache[n].Used & CACHE_USING))
    return(-1);
  for (i=0;i!=i_Cache[n].NbPict;i++)
    if (!i_Cache[n].Normal->Cached[i])
      return(i);
  return(-1);
}
