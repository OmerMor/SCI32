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


#include "VMDLIBS.HPP"
#include "VMD.HPP"


/* ----------------------------------------------------------------------*/

extern	BOOL	volatile InInterTrack;
extern	void	(*X_CloseTrack)();

/* ----------------------------------------------------------------------*/

LONG 
ViewDelayIMD(IMD * Imd)
{
	LONG ans;
	WORD B;

	if (X_CloseTrack)
	{
  		if (Imd->Type & VMD_SOUND)
    		B=2;
  		else
    		B=1;
	}
	else
  		B=1;

	if (Imd->Pos >= Imd->NbPict+Imd->NbBuffer+1)
  		return(VMD_END);
	ans=ViewIMD(Imd,Imd->Pos);
	if (Imd->Pos >= Imd->NbPict+B)
  		return(VMD_END);
	if (ans == VMD_WAITING)
  		Imd->TimeOut=0;
	else
	 	if (ans != VMD_END)
  			Imd->TimeOut++;
	if ((Imd->TimeOut > 5) && (Imd->Pos < Imd->NbPict)) {
  		ans |= VMD_TIMEOUT;
 		Imd->TimeOut = 0;
	}
	return(ans);
}



ULONG 
ViewIMDs(IMD *imd,WORD n)
{
	WORD 		j;
	ULONG 	A;
	ULONG 	B=0;
	BOOL 		OkPal=FALSE;
	WORD 		x1=-1;
	WORD 		y1;
	WORD 		x2;
	WORD 		y2;
	UWORD 	type=imd->Type;

	if ((imd->Pos == n) || (n == 0) || (imd->NbTrack == -1))
  		return(ViewIMD(imd,n));
	j=n;

	do
	{
  		BLOCINFO *N;

  		N=GetActiveTrack(imd,j,GRAPH_BLOCK);
  		if (N)
  		{
    		if (N->More.Pict.Mode & VMD_PAL)
				OkPal=TRUE;
    		if (!(N->More.Pict.Mode & VMD_DIFF))
				if (!(N->More.Pict.Mode & VMD_MINILIST))
  					break;
  		}
  		j--;
  		if (j == 0)
    		break;
	}
	while (TRUE);

	imd->Type |= VMD_NOPAL2;
	if ((j < imd->Pos) && (n > imd->Pos))
  		j=imd->Pos;
	if (j && (imd->Type & VMD_PAL) && !OkPal)
	{
  		BLOCINFO *N;
  		WORD j2;

  		j2=j;

  		do
  		{
    		N=GetActiveTrack(imd,j2,GRAPH_BLOCK);
    		if (N)
				if (N->More.Pict.Mode & VMD_PAL)
  					break;
    		j2--;
    		if (j2 == 0)
				break;
  		}
  		while (TRUE);

  		if (j2)
  		{
    		if (j!=j2)
    		{
				ViewIMD(imd,j2);
				B = VMD_PAL;
    		}
  		}
	}
	for (;j<=n;j++)
	{
  		A = ViewIMD(imd,j);
  		if (A & VMD_PAL)
    		B = VMD_PAL;
  		if (x1 == -1)
  		{
    		x1 = IMD_X1;
    		x2 = IMD_X2;
    		y1 = IMD_Y1;
    		y2 = IMD_Y2;
  		}
  		if (IMD_X1 != -1)
  		{
    		if (IMD_X1 < x1)
				x1 = IMD_X1;
    		if (IMD_Y1 < y1)
				y1 = IMD_Y1;
    		if (IMD_X2 > x2)
				x2 = IMD_X2;
    		if (IMD_Y2 > y2)
				y2 = IMD_Y2;
  		}
	}
	imd->Type=type;
	IMD_X1 = x1;
	IMD_X2 = x2;
	IMD_Y1 = y1;
	IMD_Y2 = y2;
	A |= B;
	if (A & VMD_PAL)
	{
  		WORD i;

  		for (i=0;i!=256;i++)
  		{
    		if (Video_R[i] != imd->TempPal[i*3])
				break;
    		if (Video_V[i] != imd->TempPal[i*3 + 1])
				break;
    		if (Video_B[i] != imd->TempPal[i*3 + 2])
				break;
  		}
  		if (i == 256)
    		A &= ~VMD_PAL;
  		else
  			if (!(imd->Type & VMD_NOPAL2))
    			AssignColorsPtr(0,255,0,imd->TempPal);
	}
	return(A);
}
