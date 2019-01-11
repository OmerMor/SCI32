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


#include "VMDLIBS.HPP"
#include "VMD.HPP"


/* ----------------------------------------------------------------------*/

extern	BOOL	InAnnex;

/* ----------------------------------------------------------------------*/

	size_t	IMD_ptr_size;
	size_t	IMD_ptr2_size;
	int		IMD_NbAlloc;

/* ----------------------------------------------------------------------*/


BOOL CheckIMD_ptr(LONG s1,LONG s2)
{
  if (s1<=IMD_ptr_size && s2<=IMD_ptr2_size)
    return(TRUE);
  else
    return(FALSE);
}


BOOL AllocIMD_ptr(IMD *Imd)
{
  BOOL	ans=FALSE;
  size_t	s1,s2;

  if (!Imd)
  {
    if (!IMD_NbAlloc)
      return(FALSE);
    IMD_NbAlloc--;
    if (IMD_NbAlloc)
      return(FALSE);
    if (IMD_ptr)
      ReleaseMem(IMD_ptr);
    if (IMD_ptr2)
      ReleaseMem(IMD_ptr2);
    IMD_ptr=IMD_ptr2=NULL;
    IMD_ptr_size=IMD_ptr2_size=0L;
    return(TRUE);
  }
  s1=Imd->MaxPtr;
  s2=Imd->MaxPtr2;
  if (s1)
    s1+=512L;
  if (s2)
    s2+=512L;
  IMD_NbAlloc++;
  if (s1>IMD_ptr_size)
  {
    InAnnex=TRUE;
    if (IMD_ptr)
      ReleaseMem(IMD_ptr);
    IMD_ptr=(UBYTE *) GetMem(s1);
    IMD_ptr_size=s1;
    InAnnex=FALSE;
    ans=TRUE;
  }
  if (s2>IMD_ptr2_size)
  {
    InAnnex=TRUE;
    if (IMD_ptr2)
      ReleaseMem(IMD_ptr2);
    IMD_ptr2=(UBYTE *) GetMem(s2);
    IMD_ptr2_size=s2;
    InAnnex=FALSE;
    ans=TRUE;
  }
  return(ans);
}

