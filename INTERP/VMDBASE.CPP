//*********************************************************************
//                                                                          
//   Copyright (c)1995,M.D.O.   Programmeur:Mathieu___  Création:21/02/95   
//                                                                          
//   Portage :  [X]:PC  [_]:MAC  [_]:CDI  [_]:3DO  [_]:SATURN  [_]:PSX      
//              [_]:Spécifique                                              
//                                                                          
//   Type de source :  [X]:Interne   [_]:Externe   [_]:Grand public         
//                                                                          
//   Programme rattaché : [X]:Librairies  [_]:Loader                        
//                        [_]:Autre : ______________                        
//                                                                          
//*********************************************************************

#include "sol.hpp"

#include "VMDMDO.HPP"
#include "VMD.HPP"
#ifdef WINDOWS
#include "sciwin.hpp"
#endif
#include "msg.hpp"
#include "graphmgr.hpp"
#include "resource.hpp"
#include "palette.hpp"
#include "palmgri.hpp"
#include "rect.hpp"

// #include <IO.H>
#include <ERRNO.H>
#include <SYS\STAT.H>
#include <STDIO.H>

int blockPaletteUpdate;

//*********************************************************************
//                                                                     
//  Here is the functions you need to implement before running...      
//  (The examples are implemented here for Window 32s)                 
//                                                                     
//*********************************************************************


LONG 
GXlseek(WORD handle,LONG offs,WORD mode)
{
	return LSeek(handle, offs, mode);
}


UWORD 
GXread(WORD handle,void * ptr,UWORD size)	
{
	static int result = 0;	// debug only
	// Read in a file
	if (X_TrackLoop)
		X_TrackLoop();
	TrackLoop();
	result = Read(handle, ptr, size);
	return (UWORD) result; 
}


UWORD 
_fGXread(WORD handle,void far * ptr,UWORD size)	
{
#ifndef WINDOWS
unsigned	count;
#else
#ifndef WIN32S
LPVOID	lpMem16;
#endif
#endif

	// Read in a file
	if (X_TrackLoop)
		X_TrackLoop();
	TrackLoop();

#ifdef WINDOWS
#ifndef WIN32S
	lpMem16 = (LPVOID)MK_FP16(ptr);
	return (UWORD) (_lread(handle,lpMem16,size));
#else
	return (UWORD) (_lread(handle,ptr,size));
#endif
#else
//	return (UWORD) (read(handle, ptr, size));
	_dos_read(handle, ptr, size, &count);
	return (UWORD)count;
#endif
}

#pragma	argsused
WORD 
GXopen(char *file,WORD)
{
	// par2 = mode Open a file
	int handle;

	if (X_TrackLoop)
		X_TrackLoop();

   handle = resMgr->Open(MemResVMD,atoi(file));
	LSeek(handle,0,SEEK_SET);

	if (X_TrackLoop)
		X_TrackLoop();

	return (WORD) (handle);
}

void 
GXclose(WORD h)
{
	// Close a file
	resMgr->Close(h);
	if (X_TrackLoop)
		X_TrackLoop();
}

#pragma	argsused
void 
AssignColors(WORD n1,WORD n2)
{
	// Assign a set of colors
	int i;

	// Assignation of colors n1 to n2.
	// Colors informations are in Video_R[], Video_V[i], Video_B[i];

	if(blockPaletteUpdate)
		return;

#if defined(WINDOWS) && defined(BITBLT)
	if(n1 < 10)
		n1 = 10;
	if(n2 > 245)
		n2 = 245;
#endif
	if(n1 < vmdPaletteStart)
		n1 = vmdPaletteStart;
	if(n2 > vmdPaletteEnd)
		n2 = vmdPaletteEnd;

	SOLPalette newPal;
	for (i=0;i < n1;i++) {
		Rgb24F newColor(0,0,0,0);
		#if defined(WINDOWS) && defined(BITBLT)
			newPal[i - 10] = newColor;
		#else
			newPal[i] = newColor;
		#endif
	}

	for (i=n2+1;i < 256;i++) {
		Rgb24F newColor(0,0,0,0);
		#if defined(WINDOWS) && defined(BITBLT)
			newPal[i - 10] = newColor;
		#else
			newPal[i] = newColor;
		#endif
	}

	for (i=n1;i<=n2;i++)
	{
		// If requested up the RBG values for the vmd movie
		int red = (Video_R[i] << 2);
		int green = (Video_V[i] << 2);
		int blue = (Video_B[i] << 2);

		if (vmdRGB != 100) {
			if ((i >= vmdRGBStart) && (i <= vmdRGBEnd)) {
				red = red*vmdRGB/100;
				green = green*vmdRGB/100;
				blue = blue*vmdRGB/100;
				if (red > 255) 
					red = 255;
				if (green > 255) 
					green = 255;
				if (blue > 255) 
					blue = 255;
			}
		}

		Rgb24F newColor(red,green,blue,1);
		#if defined(WINDOWS) && defined(BITBLT)
			newPal[i - 10] = newColor;
		#else
			newPal[i] = newColor;
		#endif
	}
	graphMgr->GPalette().Submit(newPal);
	graphMgr->GPalette().UpdateForFrame();
	graphMgr->GPalette().UpdateHardware();

	// graphMgr->GPalette().Submit(newPal);
	// graphMgr->FrameOut(TRUE);

}

LONG 
GXfilesize(char * file)
{
	// Return size of a file
	struct stat statbuf;
	int err;

	err=stat(file,&statbuf);
	if (err!=-1)
		return(statbuf.st_size);

	msgMgr->Alert(SrcLoc,"VMD error %d -> File %s not found!",-1,file);
	VMD_Shutdown();

	return(0L);
}
