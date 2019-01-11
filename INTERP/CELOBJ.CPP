//	celobj.cpp

#include <string.h>

#include "sol.hpp"

#include "celobj.hpp"
#include "graphmgr.hpp"
#include "msg.hpp"
#include "palmgr.hpp"
#include "resource.hpp"
#include "si.hpp"
#include "buffer.hpp"
#include "remap.hpp"

struct LinkPoint {
	Int16 x;
	Int16 y;
	uchar positionType;
	char  priority;
};

long	CelObj::timeStamp = 1;
int	CelObj::scaleX1[maxScaledLine];
int	CelObj::scaleY1[maxScaledLine];
int*	CelObj::scaleX;
int*	CelObj::scaleY;
uchar	CelObj::line[maxScaledLine];
CelObj::CelObjArray CelObj::array;

enum {SKIP_PRESENT = 1, REMAP_PRESENT = 2};

//  I have deliberately omitted remapState, since I don't think
//  it will make any difference to the compare

CelObj::CelObj(const CelObj& src)
{
	width = src.width;
	height = src.height;
	xorg = src.xorg;
	yorg = src.yorg;
	resx = src.resx;
	resy = src.resy;
	skip = src.skip;
	hasSkip = src.hasSkip;
	mirrored = src.mirrored;
	remapState = src.remapState;
	celOffset =	src.celOffset;
	paletteOffset = src.paletteOffset;
	compressType = src.compressType;
	bitmap = src.bitmap;
}


Bool
CelObj::operator==(const CelObj& src) const
{
	return (bitmap == src.bitmap  &&
		 celOffset == src.celOffset  &&
		 paletteOffset == src.paletteOffset  &&
		 width == src.width  &&
		 height == src.height  &&
		 xorg == src.xorg  &&
		 yorg == src.yorg  &&
		 skip == src.skip  &&
		 mirrored == src.mirrored  &&
		 compressType == src.compressType);
}


int
CelObj::SearchCache(Bitmap bitmap, int* availIndex) const
{
	*availIndex = -1;
	long loTime = timeStamp + 1;
	long loIndex = 0;// initialized to prevent compiler warning

	for (int i = 0; i < maxCOV; i++)  {
		if (!array[i].cel)  {
			if (*availIndex == -1)
				*availIndex = i;
		}
		else if (array[i].cel->bitmap == bitmap)  {
			array[i].timeStamp = ++timeStamp;
			return i;
		}
		else  {
			if (array[i].timeStamp < loTime)  {
				loTime = array[i].timeStamp;
				loIndex = i;
			}
		}
	}
	if (*availIndex == -1)
		*availIndex = loIndex;

	return -1;
}


void
CelObj::PutCopyInCache(int loIndex)
{
	if (loIndex == -1)
		msgMgr->Fatal(SrcLoc, Msg_BadCacheIndex, loIndex);

	delete array[loIndex].cel;
	array[loIndex].cel = Duplicate();
	array[loIndex].timeStamp = ++timeStamp;
}


void
CelObj::SubmitPalette()
{
	if (paletteOffset)  {
		HunkPalette hp;
		hp.Init(ResPointer() + paletteOffset);
		graphMgr->GPalette().Submit(hp); // if picture pal then full remap is done
	}
}



void
CelObj::Draw(const Buffer& dest, const ScreenItem& si, const SOL_Rect& drawRect)
{
	//
	//  If cel has no remap data, do it easy
	//

	Buffer& pmap = graphMgr->PriMap();
	if (remapState == NoRemapInCel)
	{
		//
		//  No remap, no priority map
		//

		if (!si.ScaleX().IsOne() || !si.ScaleY().IsOne())
		{
			if (compressType)
				ScaleDrawNoMD(dest, si.ScaleX(), si.ScaleY(), drawRect, SOL_Point(si.X1(),si.Y1()));
			else
				ScaleDrawUncompNoMD(dest, si.ScaleX(), si.ScaleY(), drawRect, SOL_Point(si.X1(),si.Y1()));
		}
		else
		{
			if (compressType)
			{
				if (horzFlip)
					DrawHzFlipNoMD(dest, drawRect, SOL_Point(si.X1(), si.Y1()));
				else
					DrawNoFlipNoMD(dest, drawRect, SOL_Point(si.X1(), si.Y1()));
			}
			else
			{
				if (!hasSkip)
				{
					if (horzFlip)
						DrawUncompHzFlipNoMDNoSkip(dest, drawRect, SOL_Point(si.X1(), si.Y1()));
					else
						DrawUncompNoFlipNoMDNoSkip(dest, drawRect, SOL_Point(si.X1(), si.Y1()));
				}
				else
				{
					if (horzFlip)
						DrawUncompHzFlipNoMD(dest, drawRect, SOL_Point(si.X1(), si.Y1()));
					else
						DrawUncompNoFlipNoMD(dest, drawRect, SOL_Point(si.X1(), si.Y1()));
				}
			}
		}
		return;
	}
		

	//
	//  If we are remapping, use this set of draw routines
	//

	if (Remap::remapCount  &&  remapState == RemapInCel)
	{
		if (!si.ScaleX().IsOne() || !si.ScaleY().IsOne())
		{
			if (compressType)
				ScaleDrawMap(dest, si.ScaleX(), si.ScaleY(), drawRect, SOL_Point(si.X1(),si.Y1()));
			else
				ScaleDrawUncompMap(dest, si.ScaleX(), si.ScaleY(), drawRect, SOL_Point(si.X1(),si.Y1()));
		}
		else
		{
			if (compressType)
			{
				if (horzFlip)
					DrawHzFlipMap(dest, drawRect, SOL_Point(si.X1(), si.Y1()));
				else
					DrawNoFlipMap(dest, drawRect, SOL_Point(si.X1(), si.Y1()));
			}
			else
			{
				if (horzFlip)
					DrawUncompHzFlipMap(dest, drawRect, SOL_Point(si.X1(), si.Y1()));
				else
					DrawUncompNoFlipMap(dest, drawRect, SOL_Point(si.X1(), si.Y1()));
			}
		}
	}
	else
	{
		if (!si.ScaleX().IsOne() || !si.ScaleY().IsOne())
		{
			if (compressType)
				ScaleDraw(dest, si.ScaleX(), si.ScaleY(), drawRect, SOL_Point(si.X1(),si.Y1()));
			else
				ScaleDrawUncomp(dest, si.ScaleX(), si.ScaleY(), drawRect, SOL_Point(si.X1(),si.Y1()));
		}
		else
		{
			if (compressType)
			{
				if (horzFlip)
					DrawHzFlip(dest, drawRect, SOL_Point(si.X1(), si.Y1()));
				else
					DrawNoFlip(dest, drawRect, SOL_Point(si.X1(), si.Y1()));
			}
			else
			{
				if (horzFlip)
					DrawUncompHzFlip(dest, drawRect, SOL_Point(si.X1(), si.Y1()));
				else
					DrawUncompNoFlip(dest, drawRect, SOL_Point(si.X1(), si.Y1()));
			}
		}
	}
}

void
CelObj::DrawTo(const Buffer& dest, const SOL_Rect& drawRect, SOL_Point p,
					const Ratio ratioX, const Ratio ratioY)
{   

	if (remapState == RemapInCel)
	{
		if (!ratioX.IsOne() || !ratioY.IsOne())
		{
			if (compressType)
				ScaleDrawMap(dest, ratioX, ratioY, drawRect, p);
			else
				ScaleDrawUncompMap(dest, ratioX, ratioY, drawRect, p);
		}
		else
		{
			if (compressType)
			{
				if (horzFlip)
					DrawHzFlipMap(dest, drawRect, p);
				else
					DrawNoFlipMap(dest, drawRect, p);
			}
			else
			{
				if (horzFlip)
					DrawUncompHzFlipMap(dest, drawRect, p);
				else
					DrawUncompNoFlipMap(dest, drawRect, p);
			}
      }
	}
	else
	{ 
		if (!ratioX.IsOne() || !ratioY.IsOne())
		{
			if (compressType)
				ScaleDrawNoMD(dest, ratioX, ratioY, drawRect, p);
			else
				ScaleDrawUncompNoMD(dest, ratioX, ratioY, drawRect, p);
		}
		else
		{
			if (compressType)
			{
				if (horzFlip)
					DrawHzFlipNoMD(dest, drawRect, p);
				else
					DrawNoFlipNoMD(dest, drawRect, p);
			}
			else
			{
				if (!hasSkip)
				{
					if (horzFlip)
						DrawUncompHzFlipNoMDNoSkip(dest, drawRect, p);
					else
						DrawUncompNoFlipNoMDNoSkip(dest, drawRect, p);
				}
				else
				{
					if (horzFlip)
						DrawUncompHzFlipNoMD(dest, drawRect, p);
					else
						DrawUncompNoFlipNoMD(dest, drawRect, p);
				}
			}
      }
	}
}




Bool
CelObjView::AnalyzeForRemap()
{
	// ***********************************************************************
	// Non scaled drawing.
	// ***********************************************************************
	// Get a pointer to the drawing area(a static buffer).

	int hRun = width;
	int vRun = height;
	uchar* resBuffer = ResPointer();

	CelHeader* celPtr = (CelHeader*) (resBuffer + celOffset);
	uchar* controlData = resBuffer + celPtr->controlOffset;
	uchar* colorData = resBuffer + celPtr->colorOffset;

	long* controlTable = (long *) (resBuffer + celPtr->rowTableOffset);
	long* colorTable = controlTable + height;

	// Draw the inset rectangle
	for (int i = 0;i < vRun; i++) {
		// Find the row data
		uchar* controlPtr = controlData + controlTable[i];
		uchar* colorPtr = colorData + colorTable[i];

		// Draw a line
		for (int j = 0;j < hRun;) {
			// Get the control character
			uchar control = *controlPtr++;
			if (!(control & REPEATC)) {
				for (int k = 0; k < control; ++k) {
					uchar color = *colorPtr++;
					if (color >= REMAP_START_COLOR  &&  color != skip  &&  color <= REMAP_END_COLOR)  {
						return True;
					}
				}
				j += control;
			}
			else {
				if (control & REPSKIP) {
					// Do a run of repeated skip
					control &= 0x3f;
					j += control;
				}
				else {
					// Do a run of repeated color
					uchar color = *colorPtr++;
					if (color >= REMAP_START_COLOR && color <= REMAP_END_COLOR)
						return True;
					control &= 0x3f;
					j += control;
				}
			}
		}
	}
	return False;
}



Bool
CelObjView::AnalyzeUncompForRemap()
{
	uchar* resBuffer = ResPointer();
	CelHeader* celPtr = (CelHeader*) (resBuffer + celOffset);
	int hRun = width;
	int vRun = height;

	uchar* colorData = resBuffer + celPtr->controlOffset;	//  Correct !
	for (int y = 0; y < vRun; y++)
	{
		uchar* src = colorData;
		for (int x = 0; x < hRun; x++)
		{
			uchar color = *src++;
			if (color >= REMAP_START_COLOR  &&  color != skip  &&  color <= REMAP_END_COLOR)
				return True;
		}
		colorData += width;
	}
	return False;
}




Bool
CelObjPic::AnalyzeForSkip()
{
	uchar* resBuffer = ResPointer();

	CelHeader* celPtr = (CelHeader*) (resBuffer + celOffset);
	uchar* controlData = resBuffer + celPtr->controlOffset;
	uchar* colorData = resBuffer + celPtr->colorOffset;

	long* controlTable = (long *) (resBuffer + celPtr->rowTableOffset);
	long* colorTable = controlTable + height;

	for (int i = 0; i < height; i++) {
		// Find the row data
		uchar* controlPtr = controlData + controlTable[i];
		uchar* colorPtr = colorData + colorTable[i];

		// Draw a line
		for (int j = 0; j < width;) {
			// Get the control character
			uchar control = *controlPtr++;
			if (!(control & REPEATC)) {
				for (int k = 0; k < control; ++k) {
					if (*colorPtr++ == skip)
						return True;
				}
				j += control;
			}
			else {
				if (control & REPSKIP) {
					// Do a run of repeated skip
					return True;
				}
				else {
					// Do a run of repeated color
					colorPtr++;
					control &= 0x3f;
					j += control;
				}
			}
		}
	}
	return False;
}



Bool
CelObjPic::AnalyzeUncompForSkip()
{
	uchar* resBuffer = ResPointer();
	CelHeader* celPtr = (CelHeader*) (resBuffer + celOffset);

	uchar* colorData = resBuffer + celPtr->controlOffset;	//  Correct !
	for (int y = 0; y < height; y++)
	{
		uchar* src = colorData;
		for (int x = 0; x < width; x++)
			if (*src++ == skip)
				return True;
		colorData += width;
	}
	return False;
}


void
CelObj::ScaleTables(const Ratio& ratioX, const Ratio& ratioY)
{
	static int lastused = -1;
	static Ratio lastScaleX[2], lastScaleY[2];
	static int scaleXa[2][maxScaledLine];
	static int scaleYa[2][maxScaledLine];

	assert(ratioX.numer > 0);
	assert(ratioX.denom >= 0);
	assert(ratioY.numer > 0);
	assert(ratioY.denom >= 0);

	//  If uninitialized, init all arrays to no scaling
	if (lastused == -1)
	{
		lastused = 0;
		for (int i = 0; i < maxScaledLine; i++)
		{
			scaleXa[0][i] = i;
			scaleXa[1][i] = i;
			scaleYa[0][i] = i;
			scaleYa[1][i] = i;
		}
	}

	assert(lastused >= 0);
	assert(lastused < 2);

	//  If we already have scaling cached, point to correct array
	for (int i = 0; i < 2; i++)
	{
		if (lastScaleX[i] == ratioX  &&  lastScaleY[i] == ratioY)
		{
			lastused = i;
			scaleX = scaleXa[i];
			scaleY = scaleYa[i];
			return;
		}
	}

	//  Need to calculate scaling array.  Use the least recently used array
	i = 1 - lastused;

	//  If X ratio scaling is different, recalculate table
	if (lastScaleX[i] != ratioX)
	{
		DoTable(scaleXa[i], ratioX, graphMgr->Xdim());
		lastScaleX[i] = ratioX;
	}

	//  If Y ratio scaling is different, recalculate table
	if (lastScaleY[i] != ratioY)
	{
		DoTable(scaleYa[i], ratioY, graphMgr->Ydim());
		lastScaleY[i] = ratioY;
	}

	//  Set pointers to proper table, init lastused
	scaleX = scaleXa[i];
	scaleY = scaleYa[i];
	lastused = i;
}



void
CelObj::DoTable(int* table, const Ratio& scale, int size)
{
	assert(table);
	assert(size > 0);
	assert(size <= maxScaledLine);
	assert(scale.numer > 0);
	assert(scale.denom >= 0);

	int total = 0;
	int value = 0;
	int numer = scale.numer;
	int denom = scale.denom;

	for (int i = 0; i < size; i++)
	{
		*table++ = value;
		total += denom;
		if (total >= numer)
		{
			value += total / numer;
			total %= numer;
		}
	}
}


uchar
CelObj::ReadPix(int x, int y, Bool mirrFlag)
{
	if (mirrFlag)  {
		int maxx = (width - 1);
		x = maxx - x;
	}

	int hBuild = x + 1;
	uchar* resBuffer = ResPointer();
	CelHeader* celPtr = (CelHeader*) (resBuffer + celOffset);

	if (compressType)
	{
		// Find the row data
		uchar* controlData = resBuffer + celPtr->controlOffset;
		uchar* colorData = resBuffer + celPtr->colorOffset;
		long* controlTable = (long *) (resBuffer + celPtr->rowTableOffset);
		long* colorTable = controlTable + height;
		uchar* controlPtr = controlData + controlTable[y];
		uchar* colorPtr = colorData + colorTable[y];

		// Build a line
		for (int j = 0; j < hBuild; ) {
			// Get the control character
			uchar control = *controlPtr++;
			if (!(control & REPEATC)) {
				// Do a run of unique bytes
				memcpy(&line[j], colorPtr, control);
				colorPtr += control;
				j += control;
			}
			else {
				if (control & REPSKIP) {
					// Do a run of repeated skip
					control &= 0x3f;
					memset(&line[j],skip,control);
					j += control;
				}
				else {
					// Do a run of repeated color
					control &= 0x3f;
					uchar color = *colorPtr++;
					memset(&line[j],color,control);
					j += control;
				}
			}
		}
		return line[x];
	}
	else  {
		uchar* colorData = resBuffer + celPtr->controlOffset;	//  Correct !
		colorData += (y * width) + x;
		return *colorData;
	}
}

SOL_Rect
CelObj::ReadLineSpan(int y)
{

// This routine finds the leftmost and rightmost nonskip pixels in a line

   SOL_Rect   pts;

   uchar* resBuffer = ResPointer();
   CelHeader* celPtr = (CelHeader*) (resBuffer + celOffset);

   pts.A.x = -1;
   if (compressType)
   {
      // Find the row data
      uchar* controlData = resBuffer + celPtr->controlOffset;
      uchar* colorData = resBuffer + celPtr->colorOffset;
      long* controlTable = (long *) (resBuffer + celPtr->rowTableOffset);
      long* colorTable = controlTable + height;
      uchar* controlPtr = controlData + controlTable[y];
      uchar* colorPtr = colorData + colorTable[y];

      // Build a line
      for (int j = 0; j < width; ) {
         // Get the control character
         uchar control = *controlPtr++;
         if (!(control & REPEATC)) {
            // Do a run of unique bytes
            if (pts.A.x == -1)
            pts.A.x   = j;
            colorPtr += control;
            j += control;
            pts.B.x  = j -1;
         }
         else {
            if (control & REPSKIP) {
               // Do a run of repeated skip
               control &= 0x3f;
               j += control;
            }
            else {
               // Do a run of repeated color
               if (pts.A.x == -1)
               pts.A.x = j;
               control &= 0x3f;
               uchar color = *colorPtr++;
               j += control;
               pts.B.x = j-1;
            }
         }
      }

      return pts;
   }
   else  {
      uchar* colorData = resBuffer + celPtr->controlOffset;   //  Correct !

      colorData += (y * width) ;

      for(int j=0;j<width;j++)   {
         if((*colorData) != Skip()) {
            if (pts.A.x == -1)
               pts.A.x = j;

            pts.B.x = j;

         }
         colorData++;
      }

      return pts;
   }
}

int 
CompareLinkArray(const void * ptr1, const void * ptr2)
{
	LinkPoint * p1 = (LinkPoint *) ptr1;
	LinkPoint * p2 = (LinkPoint *) ptr2;

	if((*p1).priority < (*p2).priority)
		return -1;
	else if((*p1).priority > (*p2).priority)
		return 1;

	return 0;

}

SCIWord 
CelObj::GetFirstLink()
{
	MemID viewID = resMgr->Get(MemResView, bitmap.View());
	ViewHeader* viewPtr = (ViewHeader*) *viewID;

	if (viewPtr->version < 0x84)
			msgMgr->Fatal(SrcLoc, "Unsupported version for Links");

	
	uchar* resBuffer = ResPointer();
	CelHeaderView* celPtr = (CelHeaderView*) (resBuffer + celOffset);

	if (celPtr->linkTableCount == 0)
	return(-1);

	struct LinkPoint linkArray[10];
	struct LinkPoint *linkPoint = (LinkPoint*)(resBuffer 
                                     + celPtr->linkTableOffset);

	for(int i=0;i<celPtr->linkTableCount;i++,linkPoint++)
		linkArray[i] = *linkPoint;
	

	qsort(linkArray,celPtr->linkTableCount,sizeof(LinkPoint),CompareLinkArray);

	return (linkArray[0].positionType);
}
 
SCIWord 
CelObj::GetNextLink(int type)
{
	MemID viewID = resMgr->Get(MemResView, bitmap.View());
	ViewHeader* viewPtr = (ViewHeader*) *viewID;

	if (viewPtr->version < 0x84)
			msgMgr->Fatal(SrcLoc, "Unsupported version for Links");

	uchar* resBuffer = ResPointer();
	CelHeaderView* celPtr = (CelHeaderView*) (resBuffer + celOffset);

	if (celPtr->linkTableCount == 0)
	return(-1);

	struct LinkPoint linkArray[10];
	struct LinkPoint *linkPoint = (LinkPoint*)(resBuffer 
                                     + celPtr->linkTableOffset);

	for(int i=0;i<celPtr->linkTableCount;i++,linkPoint++)
		linkArray[i] = *linkPoint;

	qsort(linkArray,celPtr->linkTableCount,sizeof(LinkPoint),CompareLinkArray);

	for(int j=0;j<celPtr->linkTableCount;j++) {
	 	if (linkArray[j].positionType == type)
			break;
	}

	if (j>=(celPtr->linkTableCount-1))
		return(-1);		

	return (linkArray[j+1].positionType);
}

SCIWord 
CelObj::GetLinkPoint(int type, int which)
{
	MemID viewID = resMgr->Get(MemResView, bitmap.View());
	ViewHeader* viewPtr = (ViewHeader*) *viewID;

	if (viewPtr->version < 0x84)
			msgMgr->Fatal(SrcLoc, "Unsupported version for Links");

	uchar* resBuffer = ResPointer();
	CelHeaderView* celPtr = (CelHeaderView*) (resBuffer + celOffset);

	if (celPtr->linkTableCount == 0)
	return(-1);

	struct LinkPoint linkArray[10];
	struct LinkPoint *linkPoint = (LinkPoint*)(resBuffer 
                                     + celPtr->linkTableOffset);

	for(int i=0;i<celPtr->linkTableCount;i++,linkPoint++)
		linkArray[i] = *linkPoint;
		
	for(int j=0;j<celPtr->linkTableCount;j++)
		if (linkArray[j].positionType == type)	{
			if (which == 1)
				if (mirrored)
					return(celPtr->xDim - linkArray[j].x);
				else
					return(linkArray[j].x);
			else
				return(linkArray[j].y);
		}	
			
	return(-1);
}


uchar*
CelObjView::ResPointer()
{
	MemID viewID = resMgr->Get(MemResView, bitmap.View());
	return (uchar*) *viewID;
}



void
CelObjView::Draw(const Buffer& dest, const ScreenItem& si, const SOL_Rect& drawRect, Bool hflip)
{
	horzFlip = hflip;
	CelObj::Draw(dest,si,drawRect);
}

void
CelObjView::Draw(const Buffer& dest, const SOL_Rect& toBuffer, SOL_Point p, Bool hflip)
{
	horzFlip = hflip;
	CelObj::DrawTo(dest, toBuffer, p);
}

void
CelObjView::Draw(const Buffer& dest, const SOL_Rect& toBuffer, SOL_Point p, Bool hflip,
					 const Ratio ratioX,const Ratio ratioY)
{
	horzFlip = hflip;
	CelObj::DrawTo(dest, toBuffer, p, ratioX, ratioY);
}


CelObjView::CelObjView(int v, int l, int c) : CelObj(v, l, c)
{
	int index;
	int i = SearchCache(bitmap, &index);
	if (i != -1)  {
		*this = * (CelObjView*) array[i].cel;
		array[i].timeStamp = ++timeStamp;
		return;
	}

	// Load the view into memory
	MemID viewID = resMgr->Get(MemResView, bitmap.View());
	ViewHeader* viewPtr = (ViewHeader*) *viewID;

	// Find the pointer to the cel if it exists
	if (viewPtr) {
			resx = viewPtr->resX;
			resy = viewPtr->resY;
			if (!resx  &&  !resy)  {
				switch (viewPtr->resolution) {

					case 0:
						resx = LOWRESX;
						resy = LOWRESY;
						break;

					case 1:
						resx = 640;
						resy = 480;
						break;

					case 2:
						resx = 640;
						resy = 400;
						break;
				}
			}

		if (viewPtr->loopCount <= bitmap.Loop())
			bitmap.SetLoop(viewPtr->loopCount - 1);
		if (l < 0)
			msgMgr->Fatal(SrcLoc, "Loop is less than 0!");

		LoopHeader* loopPtr =
			(LoopHeader*) ((char*) viewPtr + viewPtr->loopHeaderSize * bitmap.Loop() + viewPtr->viewHeaderSize + 2);
		if (loopPtr->altLoop != -1) {
			if (loopPtr->flags == 1) {
				mirrored = True;
			}
			loopPtr = (LoopHeader*) ((char*) viewPtr + viewPtr->loopHeaderSize*loopPtr->altLoop + viewPtr->viewHeaderSize + 2);
		}
		if (loopPtr->numCels <= bitmap.Cel())
			bitmap.SetCel(loopPtr->numCels - 1);
		paletteOffset = viewPtr->paletteOffset;
		celOffset = loopPtr->celOffset + bitmap.Cel() * viewPtr->celHeaderSize;
		CelHeaderView* celPtr = (CelHeaderView *) ((char *) viewPtr + celOffset);
		width = celPtr->xDim;
		height = celPtr->yDim;
		xorg = (celPtr->xDim >> 1) - celPtr->xHot;
// Thsi needs to be changed after the Windows VIew Editor
// allows either type of mirroring. Each loop/view will have
// a flag setting the type of mirroring to take place for the
// loop/view
#ifdef NETWORK
		if (mirrored)
			xorg = celPtr->xDim - xorg - 1;
#endif
		yorg = celPtr->yDim - celPtr->yHot - 1;
		skip = celPtr->skip;
		compressType = celPtr->compressType;
		if (compressType  &&  compressType != 0x8A)
			msgMgr->Fatal(SrcLoc, "Compression type not supported - V: %d  L: %d  C: %d\n",
				bitmap.View(), bitmap.Loop(), bitmap.Cel());

		if (celPtr->dataFlags & 0x80)  {
			hasSkip = celPtr->dataFlags & SKIP_PRESENT;
			if (celPtr->dataFlags & REMAP_PRESENT)
				remapState = RemapInCel;
			else
				remapState = NoRemapInCel;
		}
		else  {
			if (compressType)  {
				if (AnalyzeForRemap())
					remapState = RemapInCel;
				else
					remapState = NoRemapInCel;
			}
			else  {
				if (AnalyzeUncompForRemap())
					remapState = RemapInCel;
				else
					remapState = NoRemapInCel;
			}
		}

		PutCopyInCache(index);
	}
}


CelObjMem::CelObjMem(MemID handle) : CelObj(handle)
{
	// Check for valid handle
	if (!handle.IsValid()) {
		msgMgr->Alert(SrcLoc,"Invalid Text bitmap %x",handle);
#ifdef DEBUG
		width = 0;
		height = 0;
#endif
		return;
	}
	CelHeader* celPtr = (CelHeader*) *handle;
	if (celPtr->colorOffset != CELHEADERSIZE)	{
		msgMgr->Alert(SrcLoc,"Invalid Text bitmap %x",handle);
#ifdef DEBUG
		width = 0;
		height = 0;
#endif
		return;
	}
	width = celPtr->xDim;
	height = celPtr->yDim;
	xorg = celPtr->xHot;
	yorg = celPtr->yHot;
	skip = celPtr->skip;
	resx = celPtr->xRes;
	resy = celPtr->yRes;
	paletteOffset = celPtr->paletteOffset;
	celOffset = 0;
	compressType = 0;
	mirrored = False;
	remapState = (celPtr->dataFlags & REMAP_PRESENT) ? RemapInCel : NoRemapInCel;
}

//  Points to cel header

uchar*
CelObjMem::ResPointer()
{
	return (uchar*) *bitmap.Handle();
}


void
CelObjMem::Draw(const Buffer& dest, const ScreenItem& si, const SOL_Rect& drawRect, Bool hflip)
{
	horzFlip = hflip;
	CelObj::Draw(dest,si,drawRect);
}

void
CelObjMem::Draw(const Buffer& dest, const SOL_Rect& toBuffer, SOL_Point p, Bool hflip,
					 const Ratio ratioX,const Ratio ratioY)
{
	horzFlip = hflip;
	CelObj::DrawTo(dest, toBuffer, p, ratioX, ratioY);
}

void
CelObjMem::Draw(const Buffer& dest, const SOL_Rect& toBuffer, SOL_Point p, Bool hflip)
{
	horzFlip = hflip;
	CelObj::DrawTo(dest, toBuffer, p);
}

CelObjColor::CelObjColor(uchar theColor,int theWidth,int theHeight) : CelObj(theColor)
{
	width = theWidth;
	height = theHeight;
	xorg = 0;
	yorg = 0;
	resx = SCIRESX;
	resy = SCIRESY;
	paletteOffset = 0;
	mirrored = False;
	remapState = NoRemapInCel;
}

void
CelObjColor::Draw(const Buffer& dest,const ScreenItem&, const SOL_Rect& drawRect,Bool hflip)
{
	horzFlip = hflip;
	Draw(dest,drawRect);
}

void
CelObjColor::Draw(const Buffer& dest,const SOL_Rect& drawRect)
{
	// Start draw pointer
	uchar* drawPtr = dest.Addr(drawRect.A.x,drawRect.A.y);
	uint vRun = drawRect.Ylen();
	uint hRun = drawRect.Xlen();
	uint bump = dest.XDim();
	for (int i = 0;i < vRun;++i) {
		memset(drawPtr,bitmap.Color(),hRun);
		drawPtr+= bump;
	}
}

void
CelObjColor::Draw(const Buffer&, const SOL_Rect&, SOL_Point, Bool)
{
	msgMgr->Fatal(SrcLoc,"Unsupported method");
}

uchar*
CelObjColor::ResPointer()
{
	msgMgr->Fatal(SrcLoc,"Unsupported method");
	return NULL;
}

CelObjPic::CelObjPic(int p, int c) : CelObj(p, c)
{
	int index;
	int i = SearchCache(bitmap, &index);
	if (i != -1)  {
		*this = * (CelObjPic*) array[i].cel;
		array[i].timeStamp = ++timeStamp;
		return;
	}

	MemID picID = resMgr->Get(MemResPic, bitmap.Pic());
	PicHeader * picBuffer = (PicHeader *) *picID;

	if (picBuffer) {
		if (picBuffer->celCount <= bitmap.Cel())
			msgMgr->Fatal(SrcLoc, Msg_CelTooLargeForPic);

		else {
			paletteOffset = picBuffer->paletteOffset;
			celOffset = picBuffer->picHeaderSize + bitmap.Cel() * picBuffer->celHeaderSize;
			CelHeaderPic* celPtr = (CelHeaderPic *) ((char *) picBuffer + celOffset);
			width = celPtr->xDim;
			height = celPtr->yDim;
			xorg = celPtr->xHot;
			yorg = celPtr->yHot;
			skip = celPtr->skip;
			pos = SOL_Point(celPtr->xpos, celPtr->ypos);
			priority = celPtr->priority;
			celCount = picBuffer->celCount;
			compressType = celPtr->compressType;
			remapState = NoRemapInCel;
			if (picBuffer->resY == 0)  {
				switch (picBuffer->resX) {
					case 0:
						resx = LOWRESX;
						resy = LOWRESY;
						break;

					case 1:
						resx = 640;
						resy = 480;
						break;

					case 2:
						resx = 640;
						resy = 400;
						break;
	
				}
			}
			else  {
				resx = picBuffer->resX;
				resy = picBuffer->resY;
			}

			if (celPtr->dataFlags & 0x80)  {
				hasSkip = celPtr->dataFlags & SKIP_PRESENT;
				if (celPtr->dataFlags & REMAP_PRESENT)
					remapState = RemapInCel;
				else
					remapState = NoRemapInCel;
			}
			else  {
				if (!compressType)
					hasSkip = AnalyzeUncompForSkip();
				else
					hasSkip = True;

				if (compressType  &&  compressType != 0x8A)
					msgMgr->Fatal(SrcLoc, "Compression type not supported - P: %d  C: %d\n",
						bitmap.Pic(), bitmap.Cel());
			}
		}
		PutCopyInCache(index);
	}
}



CelObjPic::CelObjPic(const CelObjPic& src) : CelObj(src)
{
	pos = src.pos;
	priority = src.priority;
	celCount = src.celCount;
}



void
CelObjPic::Draw(const Buffer& dest, const ScreenItem& si, const SOL_Rect& drawRect, Bool hflip)
{
	horzFlip = hflip;
	CelObj::Draw(dest,si,drawRect);
}

void
CelObjPic::Draw(const Buffer& dest, const SOL_Rect& toBuffer, SOL_Point p, Bool hflip)
{
	horzFlip = hflip;
	CelObj::DrawTo(dest, toBuffer, p);
}


uchar*
CelObjPic::ResPointer()
{
	MemID picID = resMgr->Get(MemResPic, bitmap.Pic());
	return (uchar*) *picID;
}

