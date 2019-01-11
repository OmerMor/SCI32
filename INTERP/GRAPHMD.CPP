//	graphmd.cpp

#include <bios.h>
#include <conio.h>
#include <dos.h>
#include <string.h>
#include <stdlib.h>

#include "sol.hpp"

#include "graphmd.hpp"
#include "msg.hpp"
#include "si.hpp"
#include "video.hpp"
#include "movie.hpp"
#include "kernel.hpp"
#include "pmachine.hpp"

GraphicsMgrDOS::GraphicsMgrDOS(int xdim, int ydim, int vesaMode, Bool usePri) : GraphicsMgr(xdim, ydim),
	last(xdim, ydim), buffer(xdim, ydim, NULL), pmap(xdim, ydim, NULL)
{
	vmapID.Get(MemBitmap, xdim * ydim, VmapHandle);
	buffer.start = (uchar*) *vmapID;
   buffer.Clear(SOL_BLACK);

	if (usePri)
		CreatePriMap();
	vesa = vesaMode;

	if(vesaMode)
		vga.Set640x480();
	else
		vga.Set320x200();

	SetVMapWidth(xdim);

	modeX = False;
	cursor = New CursorDOS(buffer);
}

GraphicsMgrDOS::~GraphicsMgrDOS()
{
	DeletePriMap();
	vmapID.Free();
	delete cursor;
}

void
GraphicsMgrDOS::ResizeVmap(int xdim, int ydim)
{
	vmapID.Free();
	vmapID.Get(MemBitmap, xdim * ydim, VmapHandle);
	buffer = BufferStd(xdim, ydim, (uchar*) *vmapID);
}

void
GraphicsMgrDOS::CreatePriMap()
{
	if (!pmapID)  {
		pmapID.Get(MemBitmap, next.Xdim() * next.Ydim(), PmapHandle);
		pmap = BufferStd(next.Xdim(), next.Ydim(), (uchar*) *pmapID);
	}
}
void
GraphicsMgrDOS::AlterVMAP(SOLPalette* myPal,SOLPalette* newPal,int palIndex, char* palArray)
{
	int test	= palIndex;

	Rgb24 oldColor,newColor;
	uchar pixels[256];
	int i,j,index;
	long newVal, oldVal;

	for(i=0;i<256;i++) {
		if (palArray[i] == palIndex) {
			oldVal = 65535 * 4; // get a number larger than we will generate
			oldColor = (*myPal)[i];

			for(j=0;j<236;j++) {
				if (palArray[j] != palIndex) {
					newColor = (*myPal)[j];
					newVal   = (oldColor.red-newColor.red)*(oldColor.red-newColor.red) + 
								  (oldColor.blue-newColor.blue)*(oldColor.blue-newColor.blue) +
								  (oldColor.green-newColor.green)*(oldColor.green-newColor.green);

					if (newVal < oldVal)	{
						oldVal = newVal;
						index  = j;
					}
				}			
			}
			pixels[i] = index;		
		}
		if ((palIndex == 1) && (palArray[i] == 0)) {
			oldColor = (*myPal)[i];
			oldVal = 65535 * 4; // get a number larger than we generate

			for(j=0;j<236;j++) {
					newColor = (*newPal)[j];
					newVal   = (oldColor.red-newColor.red)*(oldColor.red-newColor.red) + 
								  (oldColor.blue-newColor.blue)*(oldColor.blue-newColor.blue) +
								  (oldColor.green-newColor.green)*(oldColor.green-newColor.green);
								  // sum of squares difference
					if (newVal < oldVal)	{
						oldVal = newVal;
						index  = j;
					}
		
			}
			pixels[i] = index;		
		}

	}

	for(i=0;i<next.Xdim()*next.Ydim();i++)	{

		if ((palArray[buffer.start[i]] == palIndex)&&(palIndex==-1)) 	 // found odd pixel
				buffer.start[i] = (uchar) pixels[buffer.start[i]];	

		if (((palArray[buffer.start[i]] == palIndex)&&(palIndex==1))||
			  ((palArray[buffer.start[i]] == 0)&&(palIndex==1)))	 	 // found odd pixel
				buffer.start[i] = (uchar) pixels[buffer.start[i]];	
	}

}

void
GraphicsMgrDOS::DeletePriMap()
{
	if ((int) pmapID)
		pmapID.Free();
	pmap.start = NULL;
}

void
GraphicsMgrDOS::ShowBits()
{
	int showListSize = next.ShowList().Length();
	if (showListSize)  {
		for (int i = 0;i < showListSize; i++)  {
			SOL_Rect rect = next.ShowList()[i];
			rect.A.x &= 0x7FFE;
			rect.B.x |= 1;
			GCursor().GonnaPaint(rect);
		}
		GCursor().PaintStarting();
		for (i = 0; i < showListSize; i++)  {
			SOL_Rect rect = next.ShowList()[i];
			rect.A.x &= 0x7FFE;
			rect.B.x |= 1;
			ShowRect(&rect, buffer.start);
		}
		GCursor().DonePainting();
		next.ShowList().Clear();
	}
}

void
GraphicsMgrDOS::PlayMovie(char* fileName, int tickCount, int xOff, int yOff)
{
	FrameOut();
	if(!vesa)
		SetVideoForMovies();
	//  do movie stuff
	RunMovie(fileName,tickCount,xOff,yOff);
}	

void
GraphicsMgrDOS::SetVideoForMovies()
{
	if (!vesa && !modeX)  {
		SetVideoMode(1);
		modeX = True;
	}
}

void
GraphicsMgrDOS::SetVideoForSpeed()
{
	if (!vesa && modeX)  {
		SetVideoMode(0);
		modeX = False;
		//////   Change this to frameout later
		ShowRect(&SOL_Rect(0, 0, Xdim() - 1, Ydim() - 1), buffer.start);
	}
}

#define GetTime(where) _bios_timeofday(_TIME_GETCLOCK,where)

void
GraphicsMgrDOS::TimingTest(const BufferStd& buffer)
{
	SOL_Rect rect1(0, 0, buffer.XDim() - 1, buffer.YDim() - 1);

	#define 	TIMERTRYS 100
	timingFlag = True;
	long timeStart;
	GetTime(&timeStart);
	for (int i = 0;i < TIMERTRYS;i++) {
		ShowRect(&rect1, buffer.start);
	}
	long timeEnd;
	GetTime(&timeEnd);
	long time1 = timeEnd - timeStart;
	long y1 = rect1.Area();


	rect1.B.x = (rect1.B.x - rect1.A.x) >> 1;
	GetTime(&timeStart);
	for (i = 0;i < TIMERTRYS;i++) {
		ShowRect(&rect1, buffer.start);
	}
	GetTime(&timeEnd);
	long time2 = timeEnd - timeStart;
	long y2 = rect1.Area();

	pixelsPerCall = y1 - (y2 - y1)*time1/(time2 - time1);
	// I see no reason pixelsPerCall should ever be less than zero.
	if (pixelsPerCall < 0)
		pixelsPerCall = 0;

}

void
GraphicsMgrDOS::Repaint(const SOL_Rect& rect)
{
	ShowRect(&rect, buffer.start);
}

void
KIsHiRes(kArgs)
{
	pm.acc = (graphMgr->Xdim() >= 640  &&  graphMgr->Ydim() >= 400);
}

_Packed struct BitMapInfoHeader
{
	ulong		biSize;
	long		biWidth;
	long		biHeight;
	ushort	biPlanes;
	ushort	biBitCount;
	ulong		biCompression;
	ulong		biSizeImage;
	long		biXPelsPerMeter;
	long		biYPelsPerMeter;
	ulong		biClrUsed;
	ulong		biClrImportant;
};

_Packed struct BitMapFileHeader
{
	short		bfType;
	ulong		bfSize;
	ushort	bfReserved1;
	ushort	bfReserved2;
	ulong		bfOffBits;
};

#ifdef DEBUG
char*
GraphicsMgrDOS::SnapShot(char* filename)
{
	BitMapInfoHeader header;
	header.biSize = sizeof(BitMapInfoHeader);
	header.biWidth = last.Xdim();
	header.biHeight = last.Ydim();
	header.biPlanes = 1;
	header.biBitCount = 8;
	header.biCompression = 0;
	header.biSizeImage = 0;
	header.biXPelsPerMeter = 0;
	header.biYPelsPerMeter = 0;
	header.biClrUsed = 0;

	BitMapFileHeader bmfh;
	bmfh.bfType = *((int *)"BM");
	bmfh.bfSize = sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader) +
						(((header.biClrUsed)?(header.biClrUsed):256) * 4) +
						(header.biWidth * header.biHeight);
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(BitMapFileHeader) + sizeof(BitMapInfoHeader) +
							(((header.biClrUsed)?(header.biClrUsed):256) * 4);
 	int fd = Creat(filename, 0);
	Write(fd, &bmfh, sizeof(BitMapFileHeader));
	Write(fd, &header, sizeof(BitMapInfoHeader));

	for (int i = 0; i < 256; i++) {
		uchar zero = 0;
		Rgb24 rgb = GPalette().Current()[i];
		Write(fd, &rgb.blue, 1);
		Write(fd, &rgb.green, 1);
		Write(fd, &rgb.red, 1);
		Write(fd, &zero, 1);
	}

	//  Calculate double word padding
	int yDim = last.Ydim();
	int extra = (header.biWidth/4 + ((header.biWidth%4)?1:0)) * 4 - header.biWidth;
	for (int y = yDim - 1; y >= 0; y--) {
		uchar* src = buffer.Addr(0, y);
		Write(fd, src, header.biWidth);
		if (extra)  //Write a padding to a DWORD as needed
			Write(fd, "    ", extra);
	}
	Close(fd);
	return NULL;
}
#endif
