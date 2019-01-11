/****************************************************************************/
/*                                                                          */
/*   Copyright (c)1995,M.D.O.   Programmeur:Mathieu___  Cr‚ation:??/??/??   */
/*                                                                          */
/*   Portage :  [X]:PC  [_]:MAC  [_]:CDI  [_]:3DO  [_]:SATURN  [_]:PSX      */
/*              [_]:Sp‚cifique                                              */
/*                                                                          */
/*   Type de source :  [_]:Interne   [_]:Externe   [X]:Grand public         */
/*                                                                          */
/*   Programme rattach‚ : [X]:Librairies  [_]:Loader                        */
/*			  [_]:Autre : ______________                        */
/*                                                                          */
/****************************************************************************/


//#ifdef	DOSX286
//#pragma	option -3
//#endif

#include "VMDLIBS.HPP"

//#ifndef	__WIN32__
//#pragma	inline
//#endif

#include "vmd.hpp"
#include "msg.hpp"

/* ----------------------------------------------------------------------*/

extern	int	NbS,i_NbCache;
//extern	far	BOOL	TimerVariable;
extern	BOOL	TimerVariable;
extern	BOOL	FreeSector;
extern	void	(*X_TrackLoop)(void);
extern	BOOL	InData;
extern	int	hBis;
extern	CACHE	*i_Cache;
extern	BOOL	volatile InInterTrack;

/* ----------------------------------------------------------------------*/

static	BOOL	AutoSeek=TRUE;

/* ----------------------------------------------------------------------*/

static	BOOL	GetSectorPreLoad(int h,UWORD s);
static	UBYTE	*GetSector(int h,UWORD s);
	int	CountLoopUse(int h);
	BOOL	InLooping(WORD n,WORD pict);

/* ----------------------------------------------------------------------*/



static UBYTE *GetSector(int h,UWORD s)
{
  UBYTE  *ptr;
  int j=-1;
  int n,n2;
  CACHE_LOOP *C;
  CACHE *CC;
  UWORD *ssv;
  int i;
  CC=&i_Cache[h];
  ptr=CC->Data;
  C=CC->Loop;
  ssv=C->Sector;
  n=C->NbSector;
  n2=C->UsedSector;
  if (n2!=n)
	 j=n2;
  for (i=0;i!=n2;i++)
  {
	 if (ssv[i]==s)
	 {
		ptr+=(LONG)i<<(LONG)SLOOP_BASE;
		C->LastSector=s;
		C->LastPtr=ptr;
		if (FreeSector==UNKNOW)
	ssv[i]=0;
		return((UBYTE *)ptr);
	 }
  }

  if (j==-1)
  {
    UWORD K=0xFFFF;

    for (i=0;i!=n;i++)
    {
      if (ssv[i]<K)
      {
	K=ssv[i];
	j=i;
      }
    }
  }
  else
    C->UsedSector++;

	if(j == -1)  {
		msgMgr->Mono("Sector Not Found in Loop\n");
		msgMgr->Fatal("Sector Not Found in Loop\n");
		exit(1);
	}

  if (s>C->NextSector)
    C->NextSector=s;
  C->Sector[j]=s;
  if (AutoSeek)
  {
    if (((LONG)s<<(LONG)SLOOP_BASE)!=C->Cpos)
    {
      C->Cpos=(LONG)s<<(LONG)SLOOP_BASE;
      GXlseek(CC->h,C->Cpos,SEEK_SET);
	 }
    AutoSeek=FALSE;
  }
  ptr+=(LONG)j<<(LONG)SLOOP_BASE;
  GXread(CC->h,ptr,LOOP_BASE);
  C->Cpos+=LOOP_BASE;
  C->LastSector=s;
  C->LastPtr=ptr;
  return((UBYTE *)ptr);
}

static BOOL GetSectorPreLoad(int h,UWORD s)
{
  UBYTE  *ptr;
  int j=-1;
  int n;
  CACHE *CC;
  CACHE_LOOP *C;
  UWORD *ssv;

  CC=&i_Cache[h];
  C=CC->Loop;
  ssv=C->Sector;
  n=C->NbSector;
  if (n!=C->UsedSector)
    j=C->UsedSector;
  if (j==-1)
  {
//#if	defined(_MAC_)+defined(__WIN32__)
    UWORD K=C->LastSector;
    int i;

    for (i=0;i!=n;i++)
    {
      if (ssv[i]<K)
      {
	j=i;
	break;
      }
    }
  }
  else
    C->UsedSector++;

	if(j == -1)
		return 0;

  if (s>C->NextSector)
    C->NextSector=s;
  C->Sector[j]=s;
  if (AutoSeek)
  {
    if (((LONG)s<<(LONG)SLOOP_BASE)!=C->Cpos)
	 {
      C->Cpos=(LONG)s<<(LONG)SLOOP_BASE;
      GXlseek(CC->h,C->Cpos,SEEK_SET);
    }
    AutoSeek=FALSE;
  }
  ptr=CC->Data;
  ptr+=(LONG)j<<(LONG)SLOOP_BASE;
  GXread(CC->h,ptr,LOOP_BASE);
  C->Cpos+=LOOP_BASE;
  return(TRUE);
}


int LoopPreLoad(int n,int nb)
{
  int i;

  AutoSeek=TRUE;
  for (i=0;i!=nb;i++)
  {
    if (i_Cache[n].Loop->NextSector>=i_Cache[n].Loop->RealSize)
      break;

	if(i_Cache[n].Loop->NextSector == -1)
		break;

    if (!GetSectorPreLoad(n,i_Cache[n].Loop->NextSector+1))
      break;
  }
  return(i);
}


BOOL InLooping(WORD n,WORD pict)
{
  LONG L;
  IMD *Imd;

  Imd=i_Cache[n].Imd;
  if (pict==Imd->NbPict-1 || pict==-1)
    L=Imd->InfoOfs;
  else
    L=Imd->PictInfo[pict+1].Offset;
  L>>=(LONG)SLOOP_BASE;
  if (i_Cache[n].Loop->NextSector<L)
    return(FALSE);
  else
    return(TRUE);
}


void LoopRead(WORD h,UBYTE * ptr,LONG s)
{
  LONG l;
  int nbs;
  int ofs;
  UBYTE  *p;

  l=i_Cache[h].Offset;
  nbs=l>>(LONG)SLOOP_BASE;
  AutoSeek=TRUE;
  if (nbs==i_Cache[h].Loop->LastSector)
  {
    if (nbs>i_Cache[h].Loop->NextSector)
      i_Cache[h].Loop->NextSector=nbs;
    p=(UBYTE *) i_Cache[h].Loop->LastPtr;
    nbs++;
  }
  else
    p=GetSector(h,nbs++);
  ofs=l&(LOOP_BASE-1);
  if (s<=LOOP_BASE-ofs)
  {
	 memcpy(ptr,p+ofs,s);
    return;
  }
  else
    memcpy(ptr,p+ofs,LOOP_BASE-ofs);
  ofs=LOOP_BASE-ofs;
  ptr+=ofs;
  s-=ofs;
  if (FreeSector)
    FreeSector=UNKNOW;
  while (s>=(LONG)LOOP_BASE)
  {
    memcpy(ptr,GetSector(h,nbs++),LOOP_BASE);
    ptr+=LOOP_BASE;
    s-=LOOP_BASE;
  }
  FreeSector=FALSE;
  if (s)
    memcpy(ptr,GetSector(h,nbs++),s);
}


void _fLoopRead(WORD h,UBYTE far * ptr,LONG s)
{
  LONG l;
  int nbs;
  int ofs;
  UBYTE  *p;

  l=i_Cache[h].Offset;
  nbs=l>>(LONG)SLOOP_BASE;
  AutoSeek=TRUE;
  if (nbs==i_Cache[h].Loop->LastSector)
  {
    if (nbs>i_Cache[h].Loop->NextSector)
      i_Cache[h].Loop->NextSector=nbs;
    p=(UBYTE *) i_Cache[h].Loop->LastPtr;
    nbs++;
  }
  else
    p=GetSector(h,nbs++);
  ofs=l&(LOOP_BASE-1);
  if (s<=LOOP_BASE-ofs)
  {
//	 memcpy(ptr,p+ofs,s);
	 _fmemcpy(ptr,p+ofs,s);
    return;
  }
  else
//    memcpy(ptr,p+ofs,LOOP_BASE-ofs);
    _fmemcpy(ptr,p+ofs,LOOP_BASE-ofs);
  ofs=LOOP_BASE-ofs;
  ptr+=ofs;
  s-=ofs;
  if (FreeSector)
    FreeSector=UNKNOW;
  while (s>=(LONG)LOOP_BASE)
  {
//    memcpy(ptr,GetSector(h,nbs++),LOOP_BASE);
    _fmemcpy(ptr,GetSector(h,nbs++),LOOP_BASE);
    ptr+=LOOP_BASE;
    s-=LOOP_BASE;
  }
  FreeSector=FALSE;
  if (s)
//    memcpy(ptr,GetSector(h,nbs++),s);
    _fmemcpy(ptr,GetSector(h,nbs++),s);
}


int CountLoopUse(int h)
{
  int n;
  UWORD K;
  CACHE *C;
  CACHE_LOOP *CC;
  UWORD *ssv;
  int Free=0,i;

  if (h<0)
    return(-1);
  C=&i_Cache[h];
  CC=C->Loop;
  ssv=CC->Sector;
  n=CC->NbSector;
  K=CC->LastSector;

  if (CC->NextSector>=CC->RealSize)
    return(-1);
  for (i=0;i!=n;i++)
  {
    if (ssv[i]<K || ssv[i]==65535)
      Free++;
  }
  return(Free);
}

void InitLooping(WORD n)
{
  if (i_Cache[n].Used & CACHE_VDUPLICATE)
    return;
  i_Cache[n].Loop->NextSector=0;
  i_Cache[n].Loop->UsedSector=0;
  i_Cache[n].Loop->LastSector=-1;
  AutoSeek=TRUE;
  GetSector(n,0);
}


void LoadInLooping(IMD * Imd,LONG time)
{
  int h,pict;
  LONG L;

  h=Imd->h;
  if (h>0)
    return;
  h=-100-h;
  if (!i_Cache[h].Looping)
    return;
  do
  {
    if (CountLoopUse(h)<20)
      return;
    L=time;
    L*=Imd->Freq;
    L/=Imd->Bytes;
    L/=1000L;
    pict=Imd->Pos+L;
    if (pict>=Imd->NbPict)
      pict=Imd->NbPict-1;
    if (InLooping(h,pict))
      return;
    ViewDelayIMD(Imd);
    if (X_TrackLoop)
      X_TrackLoop();
  }
  while (TRUE);
}



LONG TimeStayLooping(IMD * Imd)
{
  int h,pict;
  LONG L;

  h=Imd->h;
  if (h>0)
    return(0L);
  h=-100-h;
  if (!i_Cache[h].Looping)
    return(0L);

  if (i_Cache[h].Loop->NextSector>=i_Cache[h].Loop->RealSize)
    return(400000L);

  for (pict=Imd->Pos;pict<Imd->NbPict;pict++)
  {
    if (pict==Imd->NbPict-1)
      L=Imd->InfoOfs;
    else
      L=Imd->PictInfo[pict+1].Offset;
    L>>=(LONG)SLOOP_BASE;
    if (i_Cache[h].Loop->NextSector<L)
      break;
  }
  L-=Imd->Pos;
  L*=Imd->Bytes;
  L*=1000L;
  L/=Imd->Freq;
  return(L);
}

