//	shiftpal.cpp

#include "sol.hpp"

#include "celobj.hpp"
#include "memmgr.hpp"
#include "shiftpal.hpp"
#include "mbox.hpp"

#ifdef MACINTOSH
	Bool	WING=False; //KGN defined somewhere else in the dos world.
#endif

int
AddPalShift(int val)
{
	if (WING && (val < 246))
		return (val + PALSHIFT);
	else
		return val;
}

void
ShiftPaletteView(uchar* resBuffer)
{
	long celOffset;
	int  loop, cel;

	ViewHeader* viewPtr = (ViewHeader*) resBuffer;

	for (loop = 0; loop < viewPtr->loopCount; loop++) {
		LoopHeader* loopPtr =
			(LoopHeader*) ((char*) viewPtr + viewPtr->loopHeaderSize * loop
	                                   + viewPtr->viewHeaderSize + 2);
		if (loopPtr->altLoop == -1) {
			for (cel = 0; cel < loopPtr->numCels; cel++) {
				celOffset = loopPtr->celOffset + cel * viewPtr->celHeaderSize;
				ShiftPaletteCel(resBuffer, celOffset);
			}
		}
	}
}

void
ShiftPalettePic(uchar* resBuffer)
{
	long celOffset;
	int  cel;

	PicHeader* picPtr = (PicHeader *) resBuffer;
	cel	= picPtr->celCount;
	for(cel = 0; cel < picPtr->celCount; cel++) {
		celOffset = picPtr->picHeaderSize + cel * picPtr->celHeaderSize;
	  	ShiftPaletteCel(resBuffer, celOffset);
	}
}

void
ShiftPaletteCel(uchar* resBuffer, long celOffset)
{
	CelHeader* celPtr = (CelHeader*) (resBuffer + celOffset);
	int hRun = celPtr->xDim;
	int vRun = celPtr->yDim;

	uchar* controlData = resBuffer + celPtr->controlOffset;
	uchar* colorData = resBuffer + celPtr->colorOffset;
	long* controlTable = (long *) (resBuffer + celPtr->rowTableOffset);
	long* colorTable = controlTable + vRun;

	if (celPtr->skip < 246)
		celPtr->skip += PALSHIFT;
	if (celPtr->compressType) {
		for (int i = 0;i < vRun; i++) {
			// Find the row data
			uchar* controlPtr = controlData + controlTable[i];
			uchar* colorPtr = colorData + colorTable[i];
			for (int j = 0;j < hRun;) {

				// Get the control character
				uchar control = *controlPtr++;
				if (!(control & REPEATC)) {

				// Do a run of unique bytes
				  	for(int k=0;k<control;k++) {
						if (*colorPtr < 246)
							*colorPtr += PALSHIFT;
						colorPtr++;
               }
					j += control;

				} else {
					if (control & REPSKIP) {
						// Do a run of repeated skip
						control &= 0x3f;
						j += control;

					} else {
						// Do a run of repeated color
						control &= 0x3f;
						if (*colorPtr < 246)
						  	*colorPtr += PALSHIFT;
						colorPtr++;
						j += control;
					}
				}
			}				
		}
	} else {
		colorData = resBuffer + celPtr->controlOffset;
		for (int i = 0;i < vRun; i++) {
			uchar* src = colorData;
			for (int j = 0;j < hRun;j++) {
				if (*src < 246)
					*src += PALSHIFT;
				src++;
			}
			colorData += hRun;
		}
	}

#ifdef DEBUG
	#ifndef MACINTOSH
		memMgr->SetChecksum(resBuffer);
	#endif
#endif

}
