//	vga.cpp

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sol.hpp"
#include "msg.hpp"

#include "rect.hpp"
#include "Vga.hpp"
#include "video.hpp"





/*
 *
 *  Constructor
 *  ===========
 *
 */

Vga::Vga()
{
	union REGS reg;

	lenx = 80;
	leny = 25;

	reg.h.ah = 0x0F;
	int386(0x10, &reg, &reg);
	startupMode = reg.w.ax & 0xFF;
}



/*
 *
 *  Destructor
 *  ==========
 *  Restore previous palette
 *
 */

Vga::~Vga()
{
	// Set mode co80
	union REGS reg;

	reg.w.ax = 3;
	reg.w.ax = startupMode;
	int386(0x10, &reg, &reg);
}



/*
 *
 *
 *  Set320x200 Method
 *  =================
 *  Sets 320x200x256 graphics mode
 *
 */

void Vga::Set320x200()
{
	union REGS reg;

	reg.w.ax = 0x0013;
	int386(0x10, &reg, &reg);
	SetVideoMode(1);	// clear all video memory
	SetVideoMode(0);	// back to Normal Mode 13
	lenx = 320;
	leny = 200;
}



/*
 *
 *
 *  Set640x480 Method
 *  =================
 *	 Sets the 640x480x16 graphics mode, clears screen
 *
 */

void Vga::Set640x480()
{

	SetupVesa();

	lenx = 640;
	leny = 480;
}



/*
 *
 *
 *  SetText Method
 *  ==============
 *  Sets the screen to 80x25 text, clears the screen
 *
 */

void Vga::SetText()
{
	union REGS reg;

	reg.w.ax = 0x0002;
	int386(0x10, &reg, &reg);
}




#if 0
void
ShowRect(SOL_Rect* rect, uchar* vmap)
{
	int xlen = rect->Xlen();
	int ylen = rect->Ylen();
	int offset = (rect->A.y * 320) + rect->A.x;
	uchar* video = (uchar*) 0xA0000;

	for (int y = 0; y < ylen; y++)  {
		memcpy(video + offset, vmap + offset, xlen);
		offset += 320;
	}
}
#endif
