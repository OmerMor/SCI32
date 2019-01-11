//*************************************************************************
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
//*************************************************************************

#include "sol.hpp"

#include "VMDLIBS.HPP"
#include "VMD.HPP"
#include <stdlib.h>
#include "msg.hpp"

//----------------------------------------------------------------------

#define 	BUFFAVANCE 		8
#define 	RBUFFAVANCE 	32
#define	TRACKBUFFERS	32
#define	BUFF_RAM			24

//----------------------------------------------------------------------

extern	WORD				NextTrackVolume;
extern	IMD*				NextTrackImd;
extern	BOOL				AllocReal;
extern	LONG				i_TrackSoundStart;
extern	IMD*				CurrentDataIMD;
extern	BOOL				RunOverSound;
extern	WORD				NextDataPict;
extern	LONG				GetSoundPos(BOOL);
extern	BOOL				InLooping(WORD,WORD);
extern	WORD				(*X_StartSound)(WORD,SOUND *,WORD,WORD);
extern	void				(*X_StopSound)(int,int);
extern	void				(*X_CloseTrack)();
extern	BOOL				(*X_TrackInProcess)(WORD);
extern	void				(*X_TrackLoop)();
extern	BOOL				InAnnex,InViewIMD,InAnnexClose;
extern	WORD				TrackFreq;
extern	WORD				TrackAutoPlay;
extern	void				SetIntTrack_R(void *);
extern	void				SetIntTrack(void *);
extern	BOOL				volatile InInteack(void *);
extern	BOOL				volatile InInterTrack;
extern	CACHE*			i_Cache;
extern	BOOL				IsDebugging;
extern	void				(* Export_Fct)();
extern  	WORD     		volatile Nbloop;
extern	int				DMAlastFreq;
extern	UWORD	volatile NextSizeAdd;
extern  	LONG  volatile TimerSooundBase;
extern  	LONG 	volatile TimerVariable;

//----------------------------------------------------------------------

			TRACKI			Tracks[4];
			WORD				i_Saturation;
			SOUND				SndTrack;

//----------------------------------------------------------------------

static	TRACKI	TracksMemo[4];
static  	WORD		CalculBack=-1;
static	BOOL		InLoop;
static	UWORD		AddBuffer[2048];
//static	UBYTE*	TempBuffer;
static	UBYTE far *TempBuffer;
static	BOOL		InMake;
static	WORD		LastPos,WaitPos=-1;
static	WORD		MaxNumberOfLoop=10;
static	WORD		OffsetWhere;
static	BOOL		OkFlash;
static	LONG		LastLoopTime;

//----------------------------------------------------------------------

			WORD		WherePos(void);
			void		SuspendTrack(BOOL m);
static	BOOL		BuildBuffer(WORD ch,WORD m);
static	BOOL		MakeTrack(WORD m);

//----------------------------------------------------------------------

void
InitTrack(WORD Sat)
{
//  UBYTE  *ptr;
  UBYTE  far *ptr;
  WORD i;

  i_Saturation=Sat;
  TrackFreq=22050;
//  SndTrack.Start=(char *) GetMem(65536L+16L);
  SndTrack.Start=(char far *) GXgetSoundMem(65536L+16L);
  SndTrack.Size=65536L;
  SndTrack.Bits=8;
//  ptr=(UBYTE *) SndTrack.Start;
  ptr=(UBYTE far *) SndTrack.Start;
//far  memset(ptr,128,32768U);
  _fmemset(ptr,128,32768U);
  ptr+=32768L;
//far  memset(ptr,128,32768U);
  _fmemset(ptr,128,32768U);
  SndTrack.Freq=-22050;
  for (i=0;i!=4;i++)
  {
    memset(&Tracks[i],0,sizeof(TRACKI));
//far    Tracks[i].Ram=(UBYTE *) GetMem(2048L*RBUFFAVANCE);
    Tracks[i].Ram=(UBYTE far *) GXgetSoundMem(2048L*RBUFFAVANCE);
  }
  X_StartSound=StartSoundT;
  X_StopSound=StopSoundT;
  X_TrackInProcess=TrackInProcess;
  X_TrackLoop=TrackLoop;
  X_CloseTrack=CloseTrack;
  IMDinProcess=TRUE;
  SoundTrack=UNKNOW;
  if (getenv("_FLASH_"))
    OkFlash=TRUE;
}

void 
CloseTrack()
{
  WORD i;

  if (!X_TrackLoop)
    return;
  SuspendTrack(TRUE);
  SoundTrack=FALSE;
  for (i=0;i!=4;i++)
//far    ReleaseMem(Tracks[i].Ram);
  GXreleaseSoundMem(Tracks[i].Ram);
//far  ReleaseMem(SndTrack.Start);
  GXreleaseSoundMem(SndTrack.Start);
  IMDinProcess=FALSE;
  X_TrackLoop=NULL;
  X_CloseTrack=NULL;
}

WORD 
WherePos()
{
  LONG Where;
  if (!SoundInProcess)
    return(0);
  Where=GetSoundPos(TRUE);	// kk was ()
  i_TrackSoundStart=Where;
  OffsetWhere=Where&2047L;
  Where/=2048L;
  Where%=TRACKBUFFERS;
  if (Where<0 || Where>=TRACKBUFFERS)
    return(0);
  return(Where);
}

BOOL
TrackInProcess(WORD n)
{
  BOOL Ans;

  if (!SoundTrack)
    return(SoundInProcess);
  if (!SoundInProcess)
    return(FALSE);
  TrackLoop();
  if (n<-1 || n>3)
    return(FALSE);
  if (n==-1)
    n=0;
  if (Tracks[n].Used)
  {
    if (Tracks[n].GotoVolume==-1)
      Ans=UNKNOW;
    else
      Ans=TRUE;
  }
  else
  if (Tracks[n].Stop==-1)
    Ans=FALSE;
  else
    Ans=TRUE;
  return(Ans);
}

static BOOL 
BuildBuffer(WORD ch,WORD m)
{
//  UBYTE *ptr;
  UBYTE far *ptr;
//  UBYTE *ptrD;
  UBYTE far *ptrD;
  WORD i;
  LONG vpf,vs;
  WORD vf;
  WORD NbBytes=2048;
  LONG vp;
  BYTE _AL;
#pragma warning 555 9

  do
  {
    if (m!=Tracks[ch].WaitPos)
    {
//far      memcpy(TempBuffer,Tracks[ch].Ram+2048*(m%RBUFFAVANCE),2048);
      _fmemcpy(TempBuffer,Tracks[ch].Ram+2048*(m%RBUFFAVANCE),2048);
      if (Tracks[ch].Used)
      	if (Tracks[ch].Volume!=256)
      	{
				WORD v=Tracks[ch].Volume;
				for (i=0;i!=2048;i++)
	  				TempBuffer[i]=(((WORD)((UWORD)TempBuffer[i]-128)*v)>>8)+128;
      	}
      return(TRUE);
    }
    if (CalculBack!=-1 && CalculBack!=ch)
      return(FALSE);
    if (Tracks[ch].Imd)
    if (Tracks[ch].Imd->Pos<Tracks[ch].Imd->NbPict+Tracks[ch].Imd->NbBuffer)
      if ((LONG)Tracks[ch].NbBuff*2048L+2047L>=(LONG)(Tracks[ch].Imd->Pos+Tracks[ch].Imd->NbBuffer-2)*(LONG)Tracks[ch].Imd->Bytes*256L/(LONG)Tracks[ch].FreqF)
			return(FALSE);
    Tracks[ch].WaitPos=m+1;
    if (Tracks[ch].WaitPos>=TRACKBUFFERS)
      Tracks[ch].WaitPos-=TRACKBUFFERS;

    ptrD=TempBuffer;
    ptr=Tracks[ch].Start;
    vpf=Tracks[ch].PosF;
    vp=Tracks[ch].Pos;
    vf=Tracks[ch].FreqF;
    vs=Tracks[ch].Size;

    if (!Tracks[ch].Volume)
    {
//far      memset(ptrD,128,2048);
      _fmemset(ptrD,128,2048);
      break;
    }
    if (vp & 0x80000000L)
    {
      WORD eq=-vp;
//far      memset(ptrD,128,eq);
      _fmemset(ptrD,128,eq);
      NbBytes=2048-eq;
      ptrD+=eq;
      vp=vpf=0;
      Tracks[ch].Pos=Tracks[ch].PosF=0;
    }

    if (vf==256)
    {
      ptr+=vp;
      if (vp+NbBytes>vs)
      {
			WORD n;

			n=vs-vp;
//far		memcpy(ptrD,ptr,n);
			_fmemcpy(ptrD,ptr,n);
			if (Tracks[ch].Loop)
			{
	  			if ((ch==i_ChainChannel) && i_ChainSound)
	  			{
	      		Tracks[ch].Chain++;
//	      		Tracks[ch].Start=(UBYTE *) i_ChainSound->Start;
	      		Tracks[ch].Start=(UBYTE far *) i_ChainSound->Start;
	      		Tracks[ch].Size=i_ChainSound->Size;
	      		Tracks[ch].Freq=abs(i_ChainSound->Freq);
	      		Tracks[ch].FreqF=(LONG)(abs(Tracks[ch].Freq))*256L/TrackFreq;
	      		if (abs(Tracks[ch].FreqF-256)<6)
						Tracks[ch].FreqF=256;
	      		if (abs(Tracks[ch].FreqF-128)<6)
						Tracks[ch].FreqF=128;
	      		if (abs(Tracks[ch].FreqF-64)<5)
						Tracks[ch].FreqF=64;
	      		Tracks[ch].Pos=0;
	      		Tracks[ch].PosF=0;
	      		Tracks[ch].Loop=10000;
	      		i_ChainSound=NULL;
	  			}
	  			Tracks[ch].Loop--;
	  			if (!Tracks[ch].Loop)
	  			{
	    			Tracks[ch].Used=FALSE;
	    			Tracks[ch].Stop=m+2;
	    			if (Tracks[ch].Stop>=TRACKBUFFERS)
	      			Tracks[ch].Stop-=TRACKBUFFERS;
//far	  			memset(ptrD+n,128,NbBytes-n);
	  				_fmemset(ptrD+n,128,NbBytes-n);
	    			break;
	  			}
			}
//far		memcpy(ptrD+n,Tracks[ch].Start,NbBytes-n);
			_fmemcpy(ptrD+n,Tracks[ch].Start,NbBytes-n);
			Tracks[ch].Pos=NbBytes-n;
      }
      else
      {
//far		memcpy(ptrD,ptr,NbBytes);
			_fmemcpy(ptrD,ptr,NbBytes);
			Tracks[ch].Pos+=NbBytes;
      }
      break;
    }
    if (vs<65530L)
    {
//      UBYTE *pt;
      UBYTE far *pt;
      UWORD vs2,vpw;

      vpw=vp;
//      pt=(UBYTE *)ptr;
      pt=(UBYTE far *)ptr;
      vs2=vs;
      switch(vf)
      {
			case 128:
	  			for (i=0;i<NbBytes;i+=2)
	  			{
	    			_AL=pt[vpw++];
	    			ptrD[0]=ptrD[1]=_AL;
	    			ptrD+=2;
	    			if (vpw>=vs2)
	    			{
	      			if (Tracks[ch].Loop)
	      			{
	  						if ((ch==i_ChainChannel) && i_ChainSound)
							{
		    					Tracks[ch].Chain++;
//		    					Tracks[ch].Start=(UBYTE *) i_ChainSound->Start;
		    					Tracks[ch].Start=(UBYTE far *) i_ChainSound->Start;
		    					Tracks[ch].Size=i_ChainSound->Size;
		    					Tracks[ch].Freq=abs(i_ChainSound->Freq);
		    					Tracks[ch].FreqF=(LONG)(abs(Tracks[ch].Freq))*256L/TrackFreq;
		    					if (abs(Tracks[ch].FreqF-256)<6)
		      					Tracks[ch].FreqF=256;
		    					if (abs(Tracks[ch].FreqF-128)<6)
		      					Tracks[ch].FreqF=128;
		    					if (abs(Tracks[ch].FreqF-64)<5)
		      					Tracks[ch].FreqF=64;
		    					Tracks[ch].Pos=0;
		    					Tracks[ch].PosF=0;
		    					Tracks[ch].Loop=10000;
		    					i_ChainSound=NULL;
							}
							Tracks[ch].Loop--;
							if (!Tracks[ch].Loop)
							{
		  						Tracks[ch].Used=FALSE;
		  						Tracks[ch].Stop=m+2;
		  						if (Tracks[ch].Stop>=TRACKBUFFERS)
		    						Tracks[ch].Stop-=TRACKBUFFERS;
		  						if (NbBytes-i-1>0)
//far							   memset(ptrD,128,NbBytes-i-1);
		  							_fmemset(ptrD,128,NbBytes-i-1);
		  						break;
							}
	      			}
	      			vpw=0;
	    			}
	    			Tracks[ch].Pos=vpw;
	    			Tracks[ch].PosF=(LONG)vpw<<8;
	  			}
	  			break;
			case 64:
	  			for (i=0;i<NbBytes;i+=4)
	  			{
	    			_AL=pt[vpw++];
	    			ptrD[0]=ptrD[1]=ptrD[2]=ptrD[3]=_AL;
	    			ptrD+=4;
	    			if (vpw>=vs2)
	    			{
	      			if (Tracks[ch].Loop)
	      			{
	  						if ((ch==i_ChainChannel) && i_ChainSound)
							{
		    					Tracks[ch].Chain++;
//		    					Tracks[ch].Start=(UBYTE *) i_ChainSound->Start;
		    					Tracks[ch].Start=(UBYTE far *) i_ChainSound->Start;
		    					Tracks[ch].Size=i_ChainSound->Size;
		    					Tracks[ch].Freq=abs(i_ChainSound->Freq);
		    					Tracks[ch].FreqF=(LONG)(abs(Tracks[ch].Freq))*256L/TrackFreq;
		    					if (abs(Tracks[ch].FreqF-256)<6)
		      					Tracks[ch].FreqF=256;
		    					if (abs(Tracks[ch].FreqF-128)<6)
		      					Tracks[ch].FreqF=128;
		    					if (abs(Tracks[ch].FreqF-64)<5)
		      					Tracks[ch].FreqF=64;
		    					Tracks[ch].Pos=0;
		    					Tracks[ch].PosF=0;
		    					Tracks[ch].Loop=10000;
		    					i_ChainSound=NULL;
							}
							Tracks[ch].Loop--;
							if (!Tracks[ch].Loop)
							{
		  						Tracks[ch].Used=FALSE;
		  						Tracks[ch].Stop=m+2;
		  						if (Tracks[ch].Stop>=TRACKBUFFERS)
		    						Tracks[ch].Stop-=TRACKBUFFERS;
		  						if (NbBytes-i-1>0)
//far							   memset(ptrD,128,NbBytes-i-1);
		 							_fmemset(ptrD,128,NbBytes-i-1);
		  						break;
							}
	      			}
	      			vpw=0;
	    			}
	    			Tracks[ch].Pos=vpw;
	    			Tracks[ch].PosF=(LONG)vpw<<8;
	  			}
	  			break;
			default:
	  			for (i=0;i<NbBytes;i++)
	  			{
	    			ptrD[i]=pt[vp];
	    			vpf+=vf;
	    			vp=vpf>>8L;
            	if (vp>=vs2)
	    			{
	      			if (Tracks[ch].Loop)
	      			{
	  						if ((ch==i_ChainChannel) && i_ChainSound)
							{
		    					Tracks[ch].Chain++;
//		    					Tracks[ch].Start=(UBYTE *) i_ChainSound->Start;
		    					Tracks[ch].Start=(UBYTE far *) i_ChainSound->Start;
		    					Tracks[ch].Size=i_ChainSound->Size;
		    					Tracks[ch].Freq=abs(i_ChainSound->Freq);
		    					Tracks[ch].FreqF=(LONG)(abs(Tracks[ch].Freq))*256L/TrackFreq;
		    					if (abs(Tracks[ch].FreqF-256)<6)
		      					Tracks[ch].FreqF=256;
		    					if (abs(Tracks[ch].FreqF-128)<6)
		      					Tracks[ch].FreqF=128;
		    					if (abs(Tracks[ch].FreqF-64)<5)
		      					Tracks[ch].FreqF=64;
		    					Tracks[ch].Pos=0;
		    					Tracks[ch].PosF=0;
		    					Tracks[ch].Loop=10000;
		    					i_ChainSound=NULL;
							}
							Tracks[ch].Loop--;
							if (!Tracks[ch].Loop)
							{
		  						Tracks[ch].Used=FALSE;
		  						Tracks[ch].Stop=m+2;
		  						if (Tracks[ch].Stop>=TRACKBUFFERS)
		    						Tracks[ch].Stop-=TRACKBUFFERS;
		  						if (NbBytes-i-1>0)
//far								memset(ptrD+i,128,NbBytes-i-1);
									_fmemset(ptrD+i,128,NbBytes-i-1);
		  						break;
							}
	      			}
	      			vp=vpf=0;
	    			}
	  			}
	  			Tracks[ch].Pos=vp;
	  			Tracks[ch].PosF=vpf;
	  			break;
      }
      break;
    }
    else
    {
    	for (i=0;i<NbBytes;i++)
      {
			*ptrD++=ptr[vp];
			vpf+=vf;
			vp=vpf>>8L;
			if (vp>=vs)
			{
	  			if (Tracks[ch].Loop)
	  			{
	  				if ((ch==i_ChainChannel) && i_ChainSound)
	    			{
						Tracks[ch].Chain++;
//						Tracks[ch].Start=(UBYTE *) i_ChainSound->Start;
						Tracks[ch].Start=(UBYTE far *) i_ChainSound->Start;
						Tracks[ch].Size=i_ChainSound->Size;
						Tracks[ch].Freq=abs(i_ChainSound->Freq);
						Tracks[ch].FreqF=(LONG)(abs(Tracks[ch].Freq))*256L/TrackFreq;
						if (abs(Tracks[ch].FreqF-256)<6)
		  					Tracks[ch].FreqF=256;
						if (abs(Tracks[ch].FreqF-128)<6)
		  					Tracks[ch].FreqF=128;
						if (abs(Tracks[ch].FreqF-64)<5)
		  					Tracks[ch].FreqF=64;
						Tracks[ch].Pos=0;
						Tracks[ch].PosF=0;
						Tracks[ch].Loop=10000;
						i_ChainSound=NULL;
	    			}
	    			Tracks[ch].Loop--;
	    			if (!Tracks[ch].Loop)
	    			{
	      			Tracks[ch].Used=FALSE;
	      			Tracks[ch].Stop=m+2;
	      			if (Tracks[ch].Stop>=TRACKBUFFERS)
							Tracks[ch].Stop-=TRACKBUFFERS;
	      			if (NbBytes-i-1>0)
//far						memset(ptrD,128,NbBytes-i-1);
							_fmemset(ptrD,128,NbBytes-i-1);
	      			break;
	    			}
	  			}
	  		vp=vpf=0;
			}
      }
      Tracks[ch].Pos=vp;
      Tracks[ch].PosF=vpf;
    }
  }
  while (FALSE);
#pragma warning 555 0
//far  memcpy(Tracks[ch].Ram+2048*(m%RBUFFAVANCE),TempBuffer,2048);
  _fmemcpy(Tracks[ch].Ram+2048*(m%RBUFFAVANCE),TempBuffer,2048);
  if (Tracks[ch].Used)
  	if (Tracks[ch].Volume!=256)
  	{
    	WORD v=Tracks[ch].Volume;

    	for (i=0;i!=2048;i++)
      	TempBuffer[i]=(((WORD)((UWORD)TempBuffer[i]-128)*v)>>8)+128;
  	}
  return(TRUE);
}

void
SetTrackVolume(WORD ch,WORD vol,WORD mode)
{
  WORD n,i;

  if (mode)
  {
    Tracks[ch].GotoVolume=vol;
    Tracks[ch].BaseVolume=Tracks[ch].Volume;
    if (mode<100)
      mode=100;
    Tracks[ch].DecayTime=0;
    Tracks[ch].DecayTimeBase=mode/100;
  }
  else
  {
    Tracks[ch].Volume=vol;
    Tracks[ch].GotoVolume=vol;
  }
  InAnnex=TRUE;
  n=WherePos();
  WaitPos=n;
  CalculBack=99;
  for (i=0;i!=BUFFAVANCE;i++)
  {
    if (!MakeTrack(n))
      break;
    if ((Tracks[ch].Used) && (Tracks[ch].GotoVolume!=Tracks[ch].Volume))
    {
		Tracks[ch].DecayTime++;

		if (Tracks[ch].DecayTime>=Tracks[ch].DecayTimeBase)
	  		Tracks[ch].Volume=Tracks[ch].GotoVolume;
		else
		{
	  		LONG delta;
	  		LONG base;

	  		base=delta=Tracks[ch].DecayTimeBase;
	  		base*=base;
	  		delta-=Tracks[ch].DecayTime;
	  		delta*=delta;

	  		Tracks[ch].Volume=Tracks[ch].GotoVolume+
	   		(LONG)(Tracks[ch].BaseVolume-Tracks[ch].GotoVolume)*delta/base;
		}

		if (Tracks[ch].Volume==-1)
		{
	  	Tracks[ch].Used=FALSE;
	  	Tracks[ch].Stop=n+2;
	  	if (Tracks[ch].Stop>=TRACKBUFFERS)
	   	Tracks[ch].Stop-=TRACKBUFFERS;
		}
    }
    n++;
    if (n>=TRACKBUFFERS)
      n=0;
  }
  LastPos=n;
  WaitPos=LastPos-BUFFAVANCE;
  if (WaitPos<0)
    WaitPos+=TRACKBUFFERS;
  CalculBack=-1;
  InAnnex=FALSE;
  TrackLoop();
}

static BOOL
MakeTrack(WORD m)
{
  WORD ch,NbT=1,n=0,ch2,i;
//  UBYTE *ptrS;
//  UBYTE *ptrD2;
//  UBYTE *ptrD;
  UBYTE far *ptrS;
  UBYTE far *ptrD2;
  UBYTE far *ptrD;
  BOOL done;

  InMake=TRUE;
  for (i=3;i>=0;i--)
    if (Tracks[i].Used)
    {
      ch2=i;
      n++;
    }
//far  ptrD=(UBYTE *) SndTrack.Start;
  ptrD=(UBYTE far *) SndTrack.Start;
  ptrD+=2048L*m;
//  TempBuffer=(UBYTE *)ptrD;
  TempBuffer=(UBYTE far *)ptrD;
//  ptrD2=(UBYTE *)ptrD;
  ptrD2=(UBYTE far *)ptrD;
  if (!n)
  {
//far    memset(ptrD,128,2048);
    _fmemset(ptrD,128,2048);
    InMake=FALSE;
    return(TRUE);
  }
  if (n!=1)
    memcpy(TracksMemo,Tracks,4*sizeof(TRACKI));
  done=BuildBuffer(ch2,m);
  ptrS=TempBuffer;
  if (n==1 || !done)
  {
    if (i_Saturation)
    {
      NbT=Tracks[ch2].Level*i_Saturation;
      switch(NbT)
      {
			case 8:
	  			for (i=0;i<2048;i++)
	  			{
	    			UWORD a;

	    			a=(((WORD)((UWORD)ptrS[i]-128))>>1)+128;
	    			if ((WORD)a<0)
	      			a=0;
	    			else
	    			if ((UWORD)a>255)
	      			a=255;
	    			ptrS[i]=a;
	  			}
	  			break;
			case 16:
	  			break;
			case 24:
	  			for (i=0;i<2048;i++)
	  			{
	    			UWORD a;

	    			a=(((WORD)((UWORD)ptrS[i]-128)*3)>>1)+128;
	    			if ((WORD)a<0)
	      			a=0;
	    			else
	    			if ((UWORD)a>255)
	      			a=255;
	    			ptrS[i]=a;
	  			}
	  			break;
			default:
	  			for (i=0;i<2048;i++)
	  			{
	    			UWORD a;

	    			a=(((WORD)((UWORD)ptrS[i]-128)*NbT)>>4)+128;
	    			if ((WORD)a<0)
	      			a=0;
	    			else
	    			if ((UWORD)a>255)
	      			a=255;
	    			ptrS[i]=a;
	  			}
	  			break;
      }
    }
    InMake=FALSE;
    return(done);
  }
  switch(Tracks[ch2].Level)
  {
    case 1:
      for (i=0;i<2048;i++)
			AddBuffer[i]=ptrS[i];
      break;
    case 2:
      NbT++;
      for (i=0;i<2048;i++)
			AddBuffer[i]=ptrS[i]<<1;
      break;
    case 3:
      NbT+=2;
      for (i=0;i<2048;i++)
			AddBuffer[i]=ptrS[i]+ptrS[i]+ptrS[i];
      break;
    case 8:
      NbT+=7;
      for (i=0;i<2048;i++)
			AddBuffer[i]=(WORD)(ptrS[i])<<3;
      break;
    default:
    	{
      	WORD n;

      	n=Tracks[ch2].Level;
      	NbT+=n-1;
      	for (i=0;i<2048;i++)
				AddBuffer[i]=(WORD)(ptrS[i])*n;
      	break;
    	}
  }
  n--;
  for (ch=ch2+1;n!=0;ch++)
  {
    if (Tracks[ch].Used)
    {
      if (!BuildBuffer(ch,m))
      {
			memcpy(Tracks,TracksMemo,4*sizeof(TRACKI));
			InMake=FALSE;
			return(FALSE);
      }
      ptrS=TempBuffer;
      switch(Tracks[ch].Level)
      {
			case 1:
	  			NbT++;
	  			for (i=0;i<2048;i++)
	    			AddBuffer[i]+=ptrS[i];
	  			break;
			case 2:
	  			NbT+=2;
	  			for (i=0;i<2048;i++)
	    			AddBuffer[i]+=ptrS[i]<<1;
	  			break;
			case 3:
	  			NbT+=3;
	  			for (i=0;i<2048;i++)
	    			AddBuffer[i]+=ptrS[i]+ptrS[i]+ptrS[i];
	  			break;
			case 8:
	  			NbT+=8;
	  			for (i=0;i<2048;i++)
	    			AddBuffer[i]+=(WORD)(ptrS[i])<<3;
	  			break;
			default:
				{
	  				WORD n;

	  				n=Tracks[ch].Level;
	  				NbT+=n;
	  				for (i=0;i<2048;i++)
	    				AddBuffer[i]+=(WORD)(ptrS[i])*n;
	  				break;
				}
      }
      n--;
    }
  }
  if (!i_Saturation)
  {
    switch(NbT)
    {
      case 2:
			for (i=0;i<2048;i++)
	  			ptrD2[i]=AddBuffer[i]>>1;
			break;
      case 4:
			for (i=0;i<2048;i++)
	  			ptrD2[i]=AddBuffer[i]>>2;
			break;
      case 8:
			for (i=0;i<2048;i++)
	  			ptrD2[i]=AddBuffer[i]>>3;
			break;
      default:
			for (i=0;i<2048;i++)
	  			ptrD2[i]=AddBuffer[i]/NbT;
			break;
    }
  }
  else
  {
    WORD dv=NbT<<7;

    ptrD2=TempBuffer;
    switch(i_Saturation)
    {
      case 4:
			for (i=0;i<2048;i++)
			{
	  			UWORD a;

	  			a=((WORD)(AddBuffer[i]-dv)>>2)+128;
	  			if ((WORD)a<0)
	    			a=0;
	  			else
	  				if (a>255)
	    				a=255;
	  			ptrD2[i]=a;
			}
			break;
      case 8:
      	{
				for (i=0;i<2048;i++)
				{
	  				UWORD a;

	  				a=((WORD)(AddBuffer[i]-dv)>>1)+128;
	  				if ((WORD)a<0)
	    				a=0;
	  				else
	  					if ((UWORD)a>255)
	    					a=255;
	  				ptrD2[i]=a;
				}
				break;
      	}
      case 16:
			for (i=0;i<2048;i++)
			{
	  			UWORD a;

	  			a=AddBuffer[i];
	  			if ((WORD)a<0)
	    			a=0;
	  			else
	  				if ((UWORD)a>255)
	    				a=255;
	  			ptrD2[i]=a;
			}
			break;
      default:
			for (i=0;i<2048;i++)
			{
	  			UWORD a;

	  			a=(((WORD)(AddBuffer[i]-dv)*i_Saturation)>>4)+128;
	  			if ((WORD)a<0)
	    			a=0;
	  			else
	  				if ((UWORD)a>255)
	    				a=255;
	  			ptrD2[i]=a;
			}
			break;
    }
  }
  InMake=FALSE;
  return(TRUE);
}

WORD 
StartSoundT(WORD ch,SOUND * Snd,WORD nbr,WORD freq)
{
  WORD n,i;
  BOOL f=FALSE;
  BOOL Old=InAnnex;

  if (!SoundTrack)
  {
    NextTrackLevel=2;
    NextTrackImd=NULL;
    return(-1);
  }
  TrackLoop();
  InAnnex=TRUE;
  if (ch<0)
  {
    f=TRUE;
    ch=-ch;
  }
  if (ch==999)
  {
	 for (ch=1;ch!=4;ch++)
      if (!Tracks[ch].Used)
			break;
    if (ch==4)
    {
  		msgMgr->Alert(SrcLoc,"VMD error %d -> Too much SoundTracks",1);
      NextTrackLevel=2;
      NextTrackImd=NULL;
      InAnnex=Old;
      return(-1);
    }
  }
  SuspendTrack(FALSE);
  Tracks[ch].Used=TRUE;
  Tracks[ch].Level=NextTrackLevel;
  Tracks[ch].Volume=NextTrackVolume;
  Tracks[ch].GotoVolume=NextTrackVolume;
  Tracks[ch].Stop=0;
  Tracks[ch].Chain=0;
  Tracks[ch].DecayTime=0;
  Tracks[ch].DecayTimeBase=0;
//  Tracks[ch].Start=(UBYTE *) Snd->Start;
  Tracks[ch].Start=(UBYTE far *) Snd->Start;
  Tracks[ch].Size=Snd->Size;
  Tracks[ch].Loop=nbr;
  Tracks[ch].Imd=NextTrackImd;
  NextTrackLevel=2;
  NextTrackImd=NULL;
  NextTrackVolume=256;
  if (freq)
    Tracks[ch].Freq=freq;
  else
    Tracks[ch].Freq=abs(Snd->Freq);
  Tracks[ch].FreqF=(LONG)(abs(Tracks[ch].Freq))*256L/TrackFreq;
  if (abs(Tracks[ch].FreqF-256)<6 || f)
    Tracks[ch].FreqF=256;
  if (abs(Tracks[ch].FreqF-128)<6)
    Tracks[ch].FreqF=128;
  if (abs(Tracks[ch].FreqF-64)<5)
    Tracks[ch].FreqF=64;
  Tracks[ch].Pos=0;
  Tracks[ch].PosF=0;
  n=WherePos();
  WaitPos=n;
  Tracks[ch].WaitPos=n;
  Tracks[ch].NbBuff=0;
  CalculBack=ch;
  Tracks[ch].Pos=-OffsetWhere;

  for (i=0;i!=BUFFAVANCE;i++)
  {
    if (!MakeTrack(n))
      break;
    Tracks[ch].NbBuff++;
    n++;
    if (n>=TRACKBUFFERS)
      n=0;
  }
  CalculBack=-1;
  LastPos=n;
  WaitPos=LastPos-BUFFAVANCE;
  if (WaitPos<0)
    WaitPos+=TRACKBUFFERS;
  InAnnex=Old;
  TrackLoop();
  return(ch);
}

#pragma	argsused
void 
StopSoundT(int ch,int val)
{
  WORD n,i;
  BOOL Old=InAnnex;

  if (!SoundTrack)
    return;
  if (ch<0 || ch>3)
    return;
  TrackLoop();
  InAnnex=TRUE;
  if (ch==i_ChainChannel && !val)
    i_ChainSound=NULL;
  if (val)
  {
    InAnnex=Old;
    SetTrackVolume(ch,-1,val);
    return;
  }
  else
  {
    Tracks[ch].Used=FALSE;
    Tracks[ch].Stop=-1;
  }
  n=WherePos();
  WaitPos=n;
  CalculBack=99;
  for (i=0;i!=BUFFAVANCE;i++)
  {
    if (!MakeTrack(n))
      break;
    n++;
    if (n>=TRACKBUFFERS)
      n=0;
  }
  LastPos=n;
  WaitPos=LastPos-BUFFAVANCE;
  if (WaitPos<0)
    WaitPos+=TRACKBUFFERS;
  CalculBack=-1;
  InAnnex=Old;
  TrackLoop();
}

void 
TrackLoop()
{
  WORD n,l,i,j,l2;

  if (!SoundTrack)
    return;
  if (SoundTrack==UNKNOW)
    return;
  if (InLoop)
    return;
  if (!SoundInProcess)
  {
    SoundTrack=UNKNOW;
    return;
  }
  if (!InInterTrack)
  {
    LONG T;

    T=ReadTimer()-LastLoopTime;
    if (T<=2)
      return;
    LastLoopTime=ReadTimer();
  }
#pragma warning 555 9

  do
  {
    if (!InInterTrack)
      InLoop=TRUE;
    if (!InViewIMD)
    if (!InAnnex)
    if (TrackAutoPlay)
    {
      for (i=0;i!=4;i++)
      {
			if ((Tracks[i].Used) && Tracks[i].Imd)
			{
	    		WORD h;
	    		IMD *Imd;
	    		LONG ans;

	    		Imd=Tracks[i].Imd;
	    		h=Imd->h;
	    		if (!InInterTrack && h>0)
	      		h=-100;
	    		if (h<=-100 && (Imd->OpenInfo & VMD_AUTOPLAY))
	    		{
	      		WORD NbLoop;

	      		h=-100-h;
	      		NbLoop=MaxNumberOfLoop;
	      		do
	      		{
						ans=VMD_END;
						if (Imd->Pos<Imd->NbPict+Imd->NbBuffer)
						{
		  					BOOL ViewNext=FALSE;

		  					if (Imd->Pos>=Imd->NbPict || !InInterTrack)
		  					{
		    					if (Imd->Pos)
		      					ViewNext=TRUE;
		  					}
		  					else
							{
		  						if (i_Cache[h].Looping)
		  						{
		    						if (InLooping(h,Imd->Pos))
		      						ViewNext=TRUE;
		  						}
		  						else
								{
		  							if (i_Cache[h].Normal->Cached)
		  							{
		    							if (i_Cache[h].Normal->Cached[Imd->Pos] && Imd->Pos)
		      							ViewNext=TRUE;
		  							}
								}
		  						if (Imd->Pos<Imd->NbPict)
		  						if (Imd==CurrentDataIMD && Imd->Pos>=NextDataPict)
		    						ViewNext=FALSE;
		  						if (ViewNext)
		  						{
		    						UWORD A;

		    						A=Imd->OpenInfo;
		    						Imd->OpenInfo&=~VMD_PRELOAD;
		    						ans=ViewDelayIMD(Imd);
		    						Imd->OpenInfo=A;
		    						NbLoop--;
		    						if (ans!=VMD_WAITING && ans!=VMD_END)
		    						{
		      						if (ans & VMD_PAL)
											Track_NewPal=TRUE;
		      						if (IMD_X1!=-1)
		      						{
											if (Track_X1==-1)
											{
			  									Track_X1=IMD_X1;
			  									Track_X2=IMD_X2;
			  									Track_Y1=IMD_Y1;
			  									Track_Y2=IMD_Y2;
											}
											else
											{
												if (IMD_X1!=-1)
												{
			  										if (IMD_X1<Track_X1)
			   										Track_X1=IMD_X1;
			  										if (IMD_Y1<Track_Y1)
			   										Track_Y1=IMD_Y1;
			  										if (IMD_X2>Track_X2)
			   										Track_X2=IMD_X2;
			  										if (IMD_Y2>Track_Y2)
			   										Track_Y2=IMD_Y2;
												}
											}
		      						}
		    						}
		  						}
		  					}
						}
	      		}
	      		while (ans!=VMD_WAITING && ans!=VMD_END && NbLoop);
	    		}
			}
      }
    }
    n=WherePos();
    if (n==WaitPos)
      break;
    l=n-WaitPos;
    if (l<0)
      l+=TRACKBUFFERS;
    l2=LastPos-WaitPos;
    if (l2<0)
      l2+=TRACKBUFFERS;
    if (l2<BUFF_RAM)
      l2=1;
    else
      l2=0;
    for (i=0;i<l+l2;i++)
    {
      if (!MakeTrack(LastPos))
			break;
      if (i!=l)
      {
			WaitPos++;
			if (WaitPos>=TRACKBUFFERS)
	  			WaitPos=0;
      }
      LastPos++;
      if (LastPos>=TRACKBUFFERS)
		LastPos=0;
      for (j=0;j!=4;j++)
      {
			Tracks[j].NbBuff++;
			if (Tracks[j].Used)
			{
	  			if (Tracks[j].GotoVolume!=Tracks[j].Volume)
	  			{
	    			Tracks[j].DecayTime++;

	    			if (Tracks[j].DecayTime>=Tracks[j].DecayTimeBase)
	      			Tracks[j].Volume=Tracks[j].GotoVolume;
	    			else
	    			{
	      			LONG delta;
	      			LONG base;

	      			base=delta=Tracks[j].DecayTimeBase;
	      			base*=base;
	      			delta-=Tracks[j].DecayTime;
	      			delta*=delta;

	      			Tracks[j].Volume=Tracks[j].GotoVolume+
							(LONG)(Tracks[j].BaseVolume-Tracks[j].GotoVolume)*delta/base;
	    			}
	    			if (Tracks[j].Volume==-1)
	    			{
	      			Tracks[j].Used=FALSE;
	      			Tracks[j].Stop=LastPos+2;
	      			if (Tracks[j].Stop>=TRACKBUFFERS)
							Tracks[j].Stop-=TRACKBUFFERS;
	    			}
	  			}
			}
			else
				if (Tracks[j].Stop==WaitPos)
	  				Tracks[j].Stop=-1;
      }
    }
  }
  while (FALSE);
#pragma warning 555 0
  InLoop=FALSE;
}

void
SuspendTrack(BOOL s)
{
  WORD i;

  if (!SoundTrack)
    return;
  if (SoundInProcess)
  if (s+1==SoundTrack)
    return;
  if (s)
  {
    SoundTrack=FALSE;
    GXstopSound(0);
    SoundTrack=UNKNOW;
    for (i=0;i!=4;i++)
    {
      Tracks[i].Used=FALSE;
      Tracks[i].Stop=-1;
    }
  }
  else
  {
    SoundTrack=FALSE;
    if (SoundInProcess)
      GXstopSound(0);
//far    memset(SndTrack.Start,128,4096);
    _fmemset(SndTrack.Start,128,4096);
    GXstartSound(&SndTrack,0,0);
    if (!SoundInProcess)
      GXstartSound(&SndTrack,0,0);
    TrackFreq=DMAlastFreq;
    SoundTrack=TRUE;
  }
}

