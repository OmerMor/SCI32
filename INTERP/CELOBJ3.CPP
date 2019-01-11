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





void
CelObj::DrawHzFlip(const Buffer& dest, const Buffer& destpri, SOL_Rect toBuffer, SOL_Point ul, uchar pri)
{
	// ***********************************************************************
	// Non scaled drawing mirrored.
	// ***********************************************************************
	// Get a pointer to the drawing area(a static buffer).

	int rowLen = toBuffer.Xlen();
	int srcCol = width - (toBuffer.A.x - ul.x + toBuffer.Xlen());
	int srcRow = toBuffer.A.y - ul.y;
	int hRun = width - toBuffer.A.x + ul.x;
	int vRun = toBuffer.Ylen();

	// The view can't move until the draw is done!
	// Get a pointer to the cel.
	uchar* resBuffer = ResPointer();
	CelHeader* celPtr = (CelHeader*) (resBuffer + celOffset);

	uchar* controlData = resBuffer + celPtr->controlOffset;
	uchar* colorData = resBuffer + celPtr->colorOffset;

	long* controlTable = (long *) (resBuffer + celPtr->rowTableOffset);
	long* colorTable = controlTable + height;

	// Draw the inset rectangle
	for (int i = 0;i < vRun;++i,++srcRow) {
		// Find the row data
		uchar* target = dest.Addr(toBuffer.A.x, toBuffer.A.y + i) + rowLen - 1;
		uchar* primap = destpri.Addr(toBuffer.A.x, toBuffer.A.y + i) + rowLen - 1;
		uchar* controlPtr = controlData + controlTable[srcRow];
		uchar* colorPtr = colorData + colorTable[srcRow];

		// Draw a line
		for (int j = 0;j <hRun;) {
			// Get the control character
			uchar control = *controlPtr++;
			if (!(control & REPEATC)) {
				// Do a run of unique bytes
				if ((j + control) < srcCol) {
					j += control;
					colorPtr += control;
				}
				else {
					if (j < srcCol) {
						control -= srcCol - j;
						colorPtr += srcCol - j;
						j = srcCol;
					}
					if (j + control > hRun) {
						control = hRun - j;
					}
					j += control;
					for (int k = 0;k<control;++k) {
						uchar color = *colorPtr++;
						if (color != skip  &&  color < REMAP_START_COLOR) {
							*target = color;
							*primap = pri;
						}
						target--;
						primap--;
					}
				}
			}
			else {
				if (control & REPSKIP) {
					// Do a run of repeated skip
					control &= 0x3f;
					if ((j + (int) control) < srcCol) {
						j += control;
					}
					else {
						if (j < srcCol) {
							control -= srcCol - j;
							j = srcCol;
						}
						if (j + control > hRun) {
							control = hRun - j;
						}
						j += control;
						target -= control;
						primap -= control;
					}
				}
				else {
					// Do a run of repeated color
					control &= 0x3f;
					if ((j + control) < srcCol) {
						j += control;
						colorPtr++;
					}
					else {
						if (j < srcCol) {
							control -= srcCol - j;
							j = srcCol;
						}
						if (j + control > hRun) {
							control = hRun - j;
						}
						uchar color = *colorPtr++;
						target -= control - 1;
						primap -= control - 1;
                  if (color < REMAP_START_COLOR)  {
						   memset(target,color,control);
							memset(primap,pri,control);
						}

						target--;
						primap--;
						j += control;
					}
				}
			}
		}
	}
}







void
CelObj::DrawNoFlip(const Buffer& dest, const Buffer& destpri, SOL_Rect toBuffer, SOL_Point ul, uchar pri)
{
	// ***********************************************************************
	// Non scaled drawing.
	// ***********************************************************************
	// Get a pointer to the drawing area(a static buffer).

	int rowLen = toBuffer.Xlen();
	int srcCol = toBuffer.A.x - ul.x;
	int srcRow = toBuffer.A.y - ul.y;
	int hRun = toBuffer.B.x - ul.x + 1;
	int vRun = toBuffer.Ylen();

	// Get a pointer to the cel.
	uchar* resBuffer = ResPointer();
	CelHeader* celPtr = (CelHeader*) (resBuffer + celOffset);

	uchar* controlData = resBuffer + celPtr->controlOffset;
	uchar* colorData = resBuffer + celPtr->colorOffset;

	long* controlTable = (long *) (resBuffer + celPtr->rowTableOffset);
	long* colorTable = controlTable + height;

	// Draw the inset rectangle
	for (int i = 0;i < vRun;++i,++srcRow) {
		// Find the row data
		uchar* target = dest.Addr(toBuffer.A.x, toBuffer.A.y + i);
		uchar* primap = destpri.Addr(toBuffer.A.x, toBuffer.A.y + i);
		uchar* controlPtr = controlData + controlTable[srcRow];
		uchar* colorPtr = colorData + colorTable[srcRow];

		// Draw a line
		for (int j = 0;j <hRun;) {
			// Get the control character
			uchar control = *controlPtr++;
			if (!(control & REPEATC)) {
				// Do a run of unique bytes
				if ((j + control) < srcCol) {
					j += control;
					colorPtr += control;
				}
				else {
					if (j < srcCol) {
						control -= srcCol - j;
						colorPtr += srcCol - j;
						j = srcCol;
					}
					if (j + control > hRun) {
						control = hRun - j;
					}
					j += control;
					for (int k = 0;k<control;++k) {
						uchar color = *colorPtr++;
						if (color != skip  &&  color < REMAP_START_COLOR)  {  
							*target = color;
							*primap = pri;
						}
						target++;
						primap++;
					}
				}
			}
			else {
				if (control & REPSKIP) {
					// Do a run of repeated skip
					control &= 0x3f;
					if ((j + (int) control) < srcCol) {
						j += control;
					}
					else {
						if (j < srcCol) {
							control -= srcCol - j;
							j = srcCol;
						}
						if (j + control > hRun) {
							control = hRun - j;
						}
						j += control;
						primap += control;
						target += control;
					}
				}
				else {
					// Do a run of repeated color
					control &= 0x3f;
					if ((j + control) < srcCol) {
						j += control;
						colorPtr++;
					}
					else {
						if (j < srcCol) {
							control -= srcCol - j;
							j = srcCol;
						}
						if (j + control > hRun) {
							control = hRun - j;
						}
						uchar color = *colorPtr++;
						if (color < REMAP_START_COLOR)  {
							memset(target,color,control);
							memset(primap,pri,control);
						}
						j += control;
						target += control;
						primap += control;
					}
				}
			}
		}
	}
}

void
CelObj::DrawUncompNoFlip(const Buffer& dest, const Buffer& destpri, SOL_Rect toBuffer, SOL_Point ul, uchar pri)
{
	// ***********************************************************************
	// Non scaled drawing.
	// ***********************************************************************

	uchar* resBuffer = ResPointer();
	CelHeader* celPtr = (CelHeader*) (resBuffer + celOffset);
	int hRun = toBuffer.Xlen();
	int vRun = toBuffer.Ylen();
	int xSize = dest.XDim();

	uchar* colorData = resBuffer + celPtr->controlOffset;	//  Correct !
	int srcCol = toBuffer.A.x - ul.x;
	int srcRow = toBuffer.A.y - ul.y;
	colorData += (srcRow * width) + srcCol;


	// Draw the inset rectangle
	for (int y = 0; y < vRun; y++)
	{
		uchar* target = dest.Addr(toBuffer.A.x, toBuffer.A.y + y);
		uchar* primap = destpri.Addr(toBuffer.A.x, toBuffer.A.y + y);
		uchar* src = colorData;

		for (int x = 0; x < hRun; x++)
		{
			uchar color = *src++;
			if (color != skip  &&  color < REMAP_START_COLOR) {
				*target = color;
				*primap = pri;
			}
			target++;
			primap++;
		}
		colorData += width;
	}
}


void
CelObj::DrawUncompHzFlip(const Buffer& dest, const Buffer& destpri, SOL_Rect toBuffer, SOL_Point ul, uchar pri)
{
	// ***********************************************************************
	// Non scaled drawing mirrored.
	// ***********************************************************************

	uchar* resBuffer = ResPointer();
	CelHeader* celPtr = (CelHeader*) (resBuffer + celOffset);
	int hRun = toBuffer.Xlen();
	int vRun = toBuffer.Ylen();
	int xSize = dest.XDim();

	uchar* colorData = resBuffer + celPtr->controlOffset;	//  Correct !
	int srcCol = width - (toBuffer.A.x - ul.x + toBuffer.Xlen());
	int srcRow = toBuffer.A.y - ul.y;
	colorData += (srcRow * width) + srcCol;


	// Draw the inset rectangle
	for (int y = 0; y < vRun; y++)
	{
		uchar* target = dest.Addr(toBuffer.A.x, toBuffer.A.y + y) + hRun - 1;
		uchar* primap = destpri.Addr(toBuffer.A.x, toBuffer.A.y + y) + hRun - 1;
		uchar* src = colorData;

		for (int x = 0; x < hRun; x++)
		{
			uchar color = *src++;
			if (color != skip  &&  color < REMAP_START_COLOR)  {
				*target = color;
				*primap = pri;
			}
			target--;
			primap--;
		}
		colorData += width;
	}
}


void
CelObj::ScaleDraw(const Buffer& dest, const Buffer& destpri, const Ratio& ratioX, const Ratio& ratioY, const SOL_Rect& drawRect, const SOL_Point& pt, uchar pri)
{
	// Calculate the scaling tables
	ScaleTables(ratioX,ratioY);

	// Get a pointer to the drawing area(a static buffer).
	int x1 = drawRect.A.x;
	int y1 = drawRect.A.y;
	int x2 = drawRect.B.x;
	int y2 = drawRect.B.y;

	int celX1 = pt.x;
	int celY1 = pt.y;

	int hBuild;
	int offx = celX1 / ratioX;
	if (horzFlip)  {
		int maxx = (width - 1);
		for (int x = x1; x <= x2; x++)
			scaleX1[x] = maxx - (scaleX[x] - offx);

		hBuild = scaleX1[x1];
	}
	else  {
		for (int x = x1; x <= x2; x++)
			scaleX1[x] = scaleX[x] - offx;

		hBuild = scaleX1[x2];
	}

	hBuild++;
//	hBuild = width;
	int offy = celY1 / ratioY;
	for (int y = y1; y <= y2; y++)
		scaleY1[y] = scaleY[y] - offy;

	uchar* resBuffer = ResPointer();
	CelHeader* celPtr = (CelHeader*) (resBuffer + celOffset);
	uchar* controlData = resBuffer + celPtr->controlOffset;
	uchar* colorData = resBuffer + celPtr->colorOffset;

	long* controlTable = (long *) (resBuffer + celPtr->rowTableOffset);
	long* colorTable = controlTable + height;

	// Draw the inset rectangle
	int lastyi = -1;
	for (y = y1; y <= y2; y++)
	{
		uchar* target = dest.Addr(x1, y);
		uchar* primap = destpri.Addr(x1, y);
		if (lastyi != scaleY1[y])
		{
			// Find the row data
			lastyi = scaleY1[y];
			uchar* controlPtr = controlData + controlTable[lastyi];
			uchar* colorPtr = colorData + colorTable[lastyi];

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
		}

		// Draw a line
		for (int x = x1; x <= x2; x++) {
			uchar color = line[scaleX1[x]];
			if (color != skip  &&  color < REMAP_START_COLOR)  {
				*target = color;
				*primap = pri;
			}
			target++;
			primap++;
		}
	}
}



void
CelObj::ScaleDrawUncomp(const Buffer& dest, const Buffer& destpri, const Ratio& ratioX, const Ratio& ratioY, const SOL_Rect& drawRect, const SOL_Point& pt, uchar pri)
{
	// Calculate the scaling tables
	ScaleTables(ratioX,ratioY);

	// Get a pointer to the drawing area(a static buffer).
	int x1 = drawRect.A.x;
	int y1 = drawRect.A.y;
	int x2 = drawRect.B.x;
	int y2 = drawRect.B.y;

	int celX1 = pt.x;
	int celY1 = pt.y;

	// Get a pointer to the cel.
	uchar* resBuffer = ResPointer();
	CelHeader* celPtr = (CelHeader*) (resBuffer + celOffset);
	uchar* colorData = resBuffer + celPtr->controlOffset;

	int offx = celX1 / ratioX;
	if (horzFlip)  {
		int maxx = (width - 1);
		for (int x = x1; x <= x2; x++)
			scaleX1[x] = maxx - (scaleX[x] - offx);
	}
	else
		for (int x = x1; x <= x2; x++)
			scaleX1[x] = scaleX[x] - offx;

	int offy = celY1 / ratioY;
	for (int y = y1; y <= y2; y++)
		scaleY1[y] = scaleY[y] - offy;

	// Draw the inset rectangle
	for (y = y1; y <= y2; y++)
	{
		uchar* target = dest.Addr(x1, y);
		uchar* primap = destpri.Addr(x1, y);
		uchar* colorPtr = colorData + (width * scaleY1[y]);

		// Draw a line
		for (int x = x1; x <= x2; x++) {
			uchar color = colorPtr[scaleX1[x]];
			if (color != skip  &&  color < REMAP_START_COLOR) {
				*target = color;
				*primap = pri;
			}
			target++;
			primap++;
		}
	}
}

