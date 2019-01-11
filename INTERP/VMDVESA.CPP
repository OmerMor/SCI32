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


#include "vmdlibs.hpp"
#include "vmd.hpp"
#include "msg.hpp"
#include "mbox.hpp"
#include "video.hpp"
#include "config.hpp"

// ----------------------------------------------------------------------

extern	void	UnPackIMD(void);
extern	BOOL	WinDouble;
extern	WORD	WidthOfRealScreen,CurrPict;

// ----------------------------------------------------------------------

static	WORD	NmemcpyH(UBYTE  *ptrD,UBYTE  *ptrS,WORD n);

// ----------------------------------------------------------------------

static WORD 
NmemcpyH(UBYTE * ptrD,UBYTE * ptrS,WORD n)
{
  UWORD*	ptrd;
  WORD 	nb;
  UBYTE 	a;
  UBYTE*	ptrS0 = ptrS;

  ptrS++;
  if (n & 1)
    *ptrD++ = *ptrS++;
  ptrd=(UWORD *)ptrD;
  nb=0;

  {
    n>>=1;
    do
    {
      a = *ptrS++;
      if (a & 128)
      {
			a &= 127;
			memcpy(ptrd,ptrS,a<<1);
			ptrd += a;
			nb += a;
			ptrS += a<<1;
      }
      else
      {
			UBYTE 	v0,v1;
			WORD 		i;
			UBYTE*	ptrdw;

			v0 = *ptrS++;
			v1 = *ptrS++;

			ptrdw = (UBYTE *)ptrd;
			for (i=0;i<a;i++)
			{
	  			*ptrdw++ = v0;
	  			*ptrdw++ = v1;
			}
			ptrd = (UWORD *)ptrdw;
			nb += a;
      }
    }
    while (nb<n);

    return(ptrS-ptrS0);
  }
}

#ifdef WINDOWS
static WORD 
NmemcpyHDBL(UBYTE * ptrD,UBYTE * ptrS,WORD n,WORD sx)
{
  UBYTE*	ptrd;
  WORD 	nb;
  UBYTE 	a;
  UBYTE*	ptrS0=ptrS;
  UBYTE*	ptrD2;

  ptrD2 = ptrD;	// point to next line
  ptrD2 += sx;		// "

  ptrS++;
  if (n & 1) {
    *ptrD++ = *ptrS;
	 *ptrD2++ = *ptrS++;
  }

  ptrd = ptrD;
  nb = 0;

  {
    n>>=1;
    do
    {
      a = *ptrS++;
      if (a & 128)
      {
			a &= 127;
//			memcpyDBL(ptrd,ptrS,a<<1,sx);
			if (vmdBlackLines)
				MemcpyBlack(ptrd,ptrS,a<<1);
			else
				MemcpyDBL(ptrd,ptrS,a<<1,sx);
			ptrd += a<<2; 
			ptrD2 += a<<2;
			nb += a;
			ptrS += a<<1;
      }
      else
      {
			UBYTE v0,v1;
			WORD 	i;

			v0 = *ptrS++;
			v1 = *ptrS++;

			for (i=0;i<a;i++)
			{
	  			*ptrd++ = v0;
	  			*ptrd++ = v0;
	  			*ptrD2++ = v0;
	  			*ptrD2++ = v0;
	  			*ptrd++ = v1;
	  			*ptrd++ = v1;
	  			*ptrD2++ = v1;
	  			*ptrD2++ = v1;
			}
			nb+=a;
      }
    }
    while (nb<n);

    return(ptrS-ptrS0);
  }
}
#endif

void 
DoIMD_VESA(IMD * Imd)
{
  UBYTE*	ptr;
  UBYTE*	ptr2;
  WORD 	Y;
  UBYTE 	a,code;
  WORD 	SizeX;

  ptr2 = IMD_ptr;
  code = *ptr2++;
  if (code & 0x80)
  {
		DecompressData(ptr2,IMD_ptr2);
		ptr2 = IMD_ptr2;
		code &= 127;
  }
  switch(code)
  {

	 // *******************************************
	 // CASE 1
	 // *******************************************

    case 1:
      if (!(Imd->Bmp->Video & 128))
      {
      }
      else
		{
#ifdef WINDOWS
      	if (vmdDouble)
      	{
				UBYTE*	ptr0;
				UBYTE*	ptr2b;
				WORD   	sx=Imd->Bmp->SizeX;
				UBYTE*	ptr;
				sx *= 2;
				ptr2b = (UBYTE *)ptr2;
				ptr0 = Imd->Bmp->Start;
				ptr0 += ((LONG)sx*2*((LONG)Imd->Bmp->SizeY - (LONG)(Imd->Y) - 1)+((LONG)Imd->X)*2L);
				for (Y=0;Y<Imd->SizeY;Y++)
				{
	  				ptr=ptr0;
	  				SizeX=0;
	  				do
	  				{
	    				a=*ptr2b++;
	    				if (a & 0x80)
	    				{
	      				a &= 127;
	      				a++;
//	      				memcpyDBL(ptr,ptr2b,a,sx);
							if (vmdBlackLines)
								MemcpyBlack(ptr,ptr2b,a);
							else
								MemcpyDBL(ptr,ptr2b,a,sx);
	      				ptr2b += a;
	      				ptr += a*2;
	      				SizeX += a;
	    				}
	    				else
	    				{
	      				a++;
	      				ptr += a*2;
	      				SizeX += a;
	    				}
	  				}
	  				while (SizeX < Imd->SizeX);

					if (SizeX > Imd->SizeX)
				  		msgMgr->Fatal("case1 dbl overrun");
	  				ptr0 -= sx*2;
				}
      	}
      	else
#endif
      	{
				UBYTE*	ptr0;
				ptr0 = Imd->Bmp->Start;
#ifdef WINDOWS
				ptr0 += (LONG)Imd->Bmp->SizeX*((LONG)Imd->Bmp->SizeY - (LONG)Imd->Y - 1) + (LONG)Imd->X;
#else
				ptr0 += (LONG)Imd->Bmp->SizeX*(LONG)(Imd->Y)+(LONG)Imd->X;
#endif
				for (Y=0;Y<Imd->SizeY;Y++)
				{
	  				ptr = ptr0;
	  				SizeX = 0;
	  				do
	  				{
	    				a = *ptr2++;
	    				if (a & 0x80)
	    				{
	      				a &= 127;
	      				a++;
	      				memcpy(ptr,ptr2,a);
	      				ptr2 += a;
	      				ptr += a;
	      				SizeX += a;
	    				}
	    				else
	    				{
	      				a++;
	      				ptr += a;
	      				SizeX += a;
	    				}
	  				}
	  				while (SizeX < Imd->SizeX);
					if (SizeX > Imd->SizeX)
				  		msgMgr->Fatal("case1 overrun");
#ifdef WINDOWS
	  				ptr0 -= Imd->Bmp->SizeX;
#else
	  				ptr0 += Imd->Bmp->SizeX;
#endif
				}
      	}
		}
      break;

	 // *******************************************
	 // CASE 2
	 // *******************************************

    case 2:
	 	/* Not Diff */
      if (Imd->Bmp->Video & 128)
      {
#ifdef WINDOWS
			if (vmdDouble)
			{
	  			UBYTE*	ptr;
	  			WORD 		sx = Imd->Bmp->SizeX;
				sx *= 2;
	  			ptr = Imd->Bmp->Start;
	  			ptr += (LONG)sx*2*((LONG)Imd->Bmp->SizeY - (LONG)(Imd->Y) - 1)+((LONG)Imd->X)*2L;
	  			for (Y=0;Y<Imd->SizeY;Y++)
	  			{
//	    			memcpyDBL((UBYTE *)ptr,(UBYTE *)ptr2,Imd->SizeX,sx);
					if (vmdBlackLines)
						MemcpyBlack(ptr,ptr2,Imd->SizeX);
					else
						MemcpyDBL(ptr,ptr2,Imd->SizeX,sx);
	    			ptr -= sx<<1;
	    			ptr2 += Imd->SizeX;
	  			}
			}
			else
#endif
			{
	  			ptr = Imd->Bmp->Start;
#ifdef WINDOWS
				ptr += (LONG)Imd->Bmp->SizeX*((LONG)Imd->Bmp->SizeY - (LONG)Imd->Y - 1) + (LONG)Imd->X;
#else
	  			ptr += (LONG)Imd->Bmp->SizeX*(LONG)Imd->Y+(LONG)Imd->X;
#endif
	  			for (Y=0;Y<Imd->SizeY;Y++)
	  			{
	    			memcpy(ptr,ptr2,Imd->SizeX);
	    			ptr2 += Imd->SizeX;
#ifdef WINDOWS
	    			ptr -= Imd->Bmp->SizeX;
#else
	    			ptr += Imd->Bmp->SizeX;
#endif
	  			}
			}
      }
      break;

	 // *******************************************
	 // CASE 3
	 // *******************************************

    case 3:
      if (!(Imd->Bmp->Video & 128))
      {
      }
      else
      {
#ifdef WINDOWS
			if(vmdDouble) 
			{

				UBYTE*	ptr0;
				WORD   	sx = Imd->Bmp->SizeX;
				sx *= 2;
				ptr0 = Imd->Bmp->Start;
				ptr0 += ((LONG)sx*2*((LONG)Imd->Bmp->SizeY - (LONG)(Imd->Y) - 1)+(LONG)Imd->X)*2L;

				for (Y=0;Y!=Imd->SizeY;Y++)
				{
	  				ptr = ptr0;
	  				SizeX = 0;
	  				do
	  				{
	    				a = *ptr2++;
	    				if (a & 0x80)
	    				{
	      				WORD b;

	      				a &= 127;
	      				a++;
	      				if (*ptr2 == 0xFF) {
								b=NmemcpyHDBL(ptr,ptr2,a,sx);
							}
	      				else
	      				{
								b=a;
//								 memcpyDBL(ptr,ptr2,a,sx);
								if (vmdBlackLines)
									MemcpyBlack(ptr,ptr2,a);
								else
									MemcpyDBL(ptr,ptr2,a,sx);
	      				}
	      				ptr2 += b;
	      				ptr += a*2;
	      				SizeX += a;
	    				}
	    				else
	    				{
	      				a++;
	      				ptr += a*2;
	      				SizeX += a;
	    				}
	  				}
	  				while (SizeX < Imd->SizeX);

	  				ptr0 -= sx*2;
				}
			}
			else 
#endif
			{
				UBYTE*	ptr0;
				ptr0 = Imd->Bmp->Start;
#ifdef WINDOWS
				ptr0 += (LONG)Imd->Bmp->SizeX*((LONG)Imd->Bmp->SizeY - (LONG)(Imd->Y) - 1)+(LONG)Imd->X;
#else
				ptr0 += (LONG)Imd->Bmp->SizeX*(LONG)(Imd->Y)+(LONG)Imd->X;
#endif
				for (Y=0;Y!=Imd->SizeY;Y++)
				{
	  				ptr = ptr0;
	  				SizeX = 0;
	  				do
	  				{
	    				a = *ptr2++;
	    				if (a & 0x80)
	    				{
	      				WORD b;

	      				a &= 127;
	      				a++;
	      				if (*ptr2 == 0xFF)
								b=NmemcpyH(ptr,ptr2,a);
	      				else
	      				{
								b=a;
								memcpy(ptr,ptr2,a);
	      				}
	      				ptr2 += b;
	      				ptr += a;
	      				SizeX += a;
	    				}
	    				else
	    				{
	      				a++;
	      				ptr += a;
	      				SizeX += a;
	    				}
	  				}
	  				while (SizeX < Imd->SizeX);

#ifdef WINDOWS
	    			ptr0 -= Imd->Bmp->SizeX;
#else
	    			ptr0 += Imd->Bmp->SizeX;
#endif
				}
			}
      }
      break;
  }
}
