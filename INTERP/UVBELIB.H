/****************************************************************************
*
*                         The Universal VESA VBE
*
*                   Copyright (C) 1994 SciTech Software.
*                           All rights reserved.
*
* Filename:     $RCSfile$
* Version:      $Revision$
*
* Language:     ANSI C
* Environment:  IBM PC (MS DOS) real and protected mode.
*
* Description:  Header file for the UVBELib(tm) functions.
*
* $Id$
*
****************************************************************************/

#ifndef __UVBELIB_H
#define __UVBELIB_H

#ifndef	__DEBUG_H
#include "debug.h"
#endif

/*-------------------------- Function Prototypes --------------------------*/

#ifdef  __cplusplus
extern "C" {            			/* Use "C" linkage when in C++ mode */
#endif

/* Routines to install the UVBELib VBE 2.0 device support routines */

bool    _cdecl UV_install(char *driverDir,bool ignoreVBE);
void	_cdecl UV_exit(void);
void    _cdecl UV_getDriverPath(char *argv0,char *buf);

#ifdef  __cplusplus
}                       			/* End of "C" linkage for C++   	*/
#endif

#endif  /* __UVBELIB_H */
