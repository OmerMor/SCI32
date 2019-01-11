//      graph.cpp

#include <string.h>

#include "sol.hpp"

#include "array.hpp"
#include "graph.hpp"
#include "graphmgr.hpp"
#include "kernel.hpp"
#include	"line.hpp"
#include	"magnify.hpp"
#include "msg.hpp"
#include "newroom.hpp"
#include "object.hpp"
#include "pmachine.hpp"
#include "rect.hpp"
#include "selector.hpp"
#ifdef WIN32S
#include "shiftpal.hpp"
#endif
#include "resource.hpp"
#include "time.hpp"
#include "rect.hpp"
#include "palette.hpp"
#include "buffer.hpp"
#include "math.hpp"
#include "palmgr.hpp"
#include "shwstyle.hpp"
#include "textid.hpp"
#include "font.hpp"
#include "celobj.hpp"
#include "debug.hpp"

void RemapBitmap(MemID, ObjectID);
int  AddLinkedBitmap(ObjectID, ObjectID, int);

#ifdef DEBUG
#ifndef WINDOWS
void
KSaveScreen(argList)
{
	TextID id = (TextID) arg(1);
	graphMgr->SnapShot(*id);
}
#endif
#endif

void
KMorphOn(argList)
{
	int i = arg(1);
	graphMgr->morphOn = 1;
}


void
KFrameOut(kArgs)
{
#ifdef NOT_SHIPPING_VERSION
	if (doThrashReport)
		memMgr->ThrashReport();
#endif
#ifdef DEBUG
	trackingFrameOut = True;
#endif

	if (showStyles)
		ProcessShowStyles();
	else if (graphMgr->morphOn)	{
		graphMgr->PalMorphFrameOut(palMorphArray);
		graphMgr->morphOn = 0;
	}
	else {
		if (planeScrolls)
			ProcessScrolls();
		graphMgr->FrameOut();
	}
}

void
KAddScreenItem(argList)
{
	// Input
	//      arg1-----the SCI Object

	ObjectID id = (ObjectID) arg(1);

	int planeId = id.GetIndexedProperty(actPlaneID);
	id->SetInSIList();
	ScreenItem* si = graphMgr->NextScreen()->GetScreenItem(planeId,id);
	if (si == NULL) {
		si = New ScreenItem(id);
		// add the screen item
		graphMgr->AddScreenItem(si);
	}
	else {
		// Change to update
		*si = id;
	}
}

void
KAddLine(argList)
{
	// Input
	//      arg1-----Plane ID
	//      arg2-----Point A.x
	//      arg3-----Point A.y
	//      arg4-----Point B.x
	//      arg5-----Point B.y
	//		  The following arguments are optional (all or none)
	//      arg6-----z (priority)
	//      arg7-----color
	//      arg8-----style  Solid=0,Dashed=1,Pattern=2
	//      arg9-----pattern
	//      arg10----thickness = 2n+1 (1->1,2->1,3->3,4->3,5->5 etc.)
	// Output
	//		  pm.acc = id for line screen item

	int	z = 1000;
	uchar	color = SOL_WHITE;
	int	style = SOL_Line::tSolid;
	int	pattern = 0;
	int	thickness = 1;

	SOL_Rect rect;
	if (argCount == 10) {
		// No default arguments
		z = arg(6);
		color = arg(7);
		style = arg(8);
		pattern = arg(9);
		thickness = arg(10);
	}
		
	// Convert 16 bit pattern to 32 bit pattern
	pattern |= pattern << 16;

	MemID BM = BuildLineBitmap(arg(2),
										arg(3),
										arg(4),
										arg(5),
										color,
										style,
										pattern,
										thickness,
										rect);

	Bitmap bitmap(BM);
	ScreenItem* si = New ScreenItem(arg(1),bitmap,rect);
	si->SetPriority(z);
	si->color = color;

	// add the screen item
	graphMgr->AddScreenItem(si);
	pm.acc = si->Id();
}

void
KAddPolygon(argList)
{
	// Input
	//      arg1-----Plane ID
	//      arg2-----Polygon list MemID
	//		  The following arguments are optional (all or none)
	//      arg3-----z (priority)
	//      arg4-----color
	//      arg5-----style  Solid=0,Dashed=1,Pattern=2
	//      arg6-----pattern
	//      arg7-----thickness = 2n+1 (1->1,2->1,3->3,4->3,5->5 etc.)
	//      arg8-----close (True close polygon, False leave open)
	// Output
	//		  pm.acc = id for polygon screen item

	int	z = 1000;
	uchar	color = SOL_WHITE;
	int	style = SOL_Line::tSolid;
	int	pattern = 0;
	int	thickness = 1;
	Bool	close = True;

	if (argCount == 8) {
		// No default arguments
		z = arg(3);
		color = arg(4);
		style = arg(5);
		pattern = arg(6);
		thickness = arg(7);
		close = arg(8);
	}
		
	// Convert 16 bit pattern to 32 bit pattern
	pattern |= pattern << 16;

	SOL_Rect rect(100000,100000,-100000,-100000);

   ObjectID id = arg(2);
	SOL_ListID theList = id.GetProperty(s_elements);

	MemID BM = BuildPolygonBitmap(theList,
										color,
										style,
										pattern,
										thickness,
										rect,
										close);

	Bitmap bitmap(BM);
	ScreenItem* si = New ScreenItem(arg(1),bitmap,rect);
	si->SetPriority(z);
	si->color = color;

	// add the screen item
	graphMgr->AddScreenItem(si);
	pm.acc = si->Id();
}

void
KBitmap(argList)
{
	// Bitmap functions
	// Input
	//      arg1-----function code
	//					
	enum {
		bCreate,			// Create an empty bitmap
		bDispose,		// Dispose of a bitmap
		bAddLine,		// Add a line to a bitmap
		bAddCel,			// Add a cel to a bitmap
		bAddText,		// Add text to a bitmap
		bAddRect,		// Add a colored rectangle to a bitmap
		bAddBitmap,		// Add another bitmap to a bitmap
		bInvertRect, 	// Invert a bitmap
		bSetOrigin,	 	// set origin point of the bitmap
		bCreateWithCel, // create a new bitmap from cel data
		bRemap,			// remap an existing bitmap 
		bDuplicate		// duplicate an existing bitmap 
	};
	// The arguments vary from function to function
	// The return value varys from function to function

	MemID			BM;
	int 			BMWidth;
	int 			BMHeight;
	CelHeader* 	celHeader;
	int 			function = arg(1);

	switch (function) {
		case bCreate: {
			// Create bitmap
			//	--------------
			// Input
			//      arg2-----bitmap width
			//      arg3-----bitmap height
			//      arg4-----skip color
			//      arg5-----background color
			//					  If background color = skip the bitmap is transparent
			//      arg6-----x resolution
			//      arg7-----y resolution
			//		  arg8-----remap state
			// Output
			//		  pm.acc = id for bitmap

#ifdef WIN32S
	arg(4) = (SCIWord)AddPalShift((int)arg(4));
	arg(5) = (SCIWord)AddPalShift((int)arg(5));
#endif

			BMWidth  = arg(2);
			BMHeight = arg(3);

			int BMresX = (argCount > 5) ? arg(6) : fontMgr->GetXRes();
			int BMresY = (argCount > 6) ? arg(7) : fontMgr->GetYRes();

			BM.Get(MemBitmap,BMWidth*BMHeight+CELHEADERSIZE);

			// Fill the bitmap with background color
			memset((uchar*) memMgr->GetAddr(BM)+CELHEADERSIZE,arg(5),BMWidth*BMHeight);

			BuildHeader(BM,BMWidth,BMHeight,arg(4),
							0,
							0,
							BMresX,
							BMresY,
							0,
							argCount >= 8 ? arg(8) : False
						  );

			pm.acc = BM;
			break;
		}

		case bCreateWithCel: {
			// Create bitmap with a cel
			//	------------------------
			// Input
			//      arg2-----view
			//      arg3-----loop
			//		  arg4-----cel
			//      arg5-----skip color
			//      arg6-----background color
			//					  If background color = skip the bitmap is transparent
			//      arg7-----remap state
			//      arg8-----clut data
			// Output
			//		  pm.acc = id for bitmap

			CelObjView celObj(arg(2), arg(3), arg(4));
			BMWidth  = celObj.Width();
			BMHeight = celObj.Height();
			BM.Get(MemBitmap,BMWidth*BMHeight+CELHEADERSIZE);

			int theSkip = (argCount < 5 || arg(5) == -1) ? celObj.Skip() : arg(5);
			int theBack = (argCount < 6 || arg(6) == -1) ? celObj.Skip() : arg(6);

			// Fill the bitmap with background color
			memset((uchar*) memMgr->GetAddr(BM)+CELHEADERSIZE,theBack,BMWidth*BMHeight);

			BuildHeader(BM,BMWidth,BMHeight,theSkip,
							0,
							0,
							celObj.ResX(),
							celObj.ResY(),
							0,
							argCount >= 7 ? arg(7) : False
						);

			// make sure the view is not purged by BM.Get() allocation
			resMgr->Get(MemResView, arg(2));

			// Create a buffer that points to bitmap
			BufferStd buffer(BMWidth, BMHeight, (uchar*) memMgr->GetAddr(BM) + CELHEADERSIZE);

			// draw the cel object into the buffer
			// theTextBox: relative to upper left. textBox >= pt coords
			// pt: upper left of cel
			SOL_Rect drawRect(0,0,BMWidth-1,BMHeight-1);
			SOL_Point pt(0, 0);
			celObj.Draw(buffer, drawRect, pt, celObj.Mirrored());

			if (argCount >= 8 && arg(8))
				RemapBitmap(BM, arg(8));

			pm.acc = BM;
			break;
		}

		case bDispose:
			// Dispose bitmap
			//	--------------
			// Input
			// 	  arg2-----the MemID for the bitmap

			if (memMgr->IsValid((SOL_Handle) arg(2)))
				memMgr->Free((SOL_Handle) arg(2));
			break;

		case bAddLine:	{
			// Add line to bitmap
			//	--------------
			// Input
			//      arg2-----MemID for the bitmap
			//      arg3-----Point A.x
			//      arg4-----Point A.y
			//      arg5-----Point B.x
			//      arg6-----Point B.y
			//      arg7-----color
			//      arg8-----style  Solid=0,Dashed=1,Pattern=2
			//      arg9-----pattern
			// Output
			// 	  Line added to bitmap
			//		  Accumulator not modified

#ifdef WIN32S
	arg(7) = (SCIWord)AddPalShift((int)arg(7));
#endif

			// Convert 16 bit pattern to 32 bit pattern
			int pattern = (argCount > 8) ? arg(9) : 0;
			int left = pattern << 16;
			pattern |= left;
			int style = (argCount > 7) ? arg(8) : SOL_Line::tSolid;

			BM = arg(2);
			// Get the width of the bitmap
			celHeader = ((CelHeader*) memMgr->GetAddr(BM));
			BMWidth = celHeader->xDim;

			SOL_Line 	line(arg(3),arg(4),arg(5),arg(6));

#ifdef DEBUG
			// Integrity checks
			BMHeight = celHeader->yDim;
			if ((line.A.x < 0)			||
				 (line.A.y < 0)			||
				 (line.B.y < 0)			||
				 (line.B.x > BMWidth)	||
				 (line.A.y > BMHeight)	||
				 (line.B.y > BMHeight))

	 			msgMgr->Fatal(SrcLoc,"Invalid line for given bitmap");
#endif

			line.DrawThin(arg(7),style,pattern,BM,BMWidth);
			break;
		}
		case bAddCel: {
			// Add cel to bitmap
			//	-----------------
			// Input
			//      arg2:  MemID for the bitmap
			//      arg3:  view
			//      arg4:  loop
			//      arg5:  cel
			//      arg6:  x of where to position cel [-1 = origin point]
			//      arg7:  y of where to position cel [-1 = origin point]
			//		  arg8:  dontClip [clip=0, union=1]
			//		  arg9:  x of cel to match with [-1 = origin point]
			//		  arg10: y of cel to match with [-1 = origin point]

			MemID			viewBM;
			SOL_Rect    theTextBox, BMRect;
			ObjectID    obj;

			BM = arg(2);
			celHeader = ((CelHeader*) memMgr->GetAddr(BM));
			BMWidth  = celHeader->xDim;
			BMHeight = celHeader->yDim;

			// Determine size of object were adding
		   CelObjView celObj(arg(3),arg(4),arg(5));
   		int objWidth = celObj.Width();
   		int objHeight= celObj.Height();

			int posX = (argCount > 5) ? arg(6) : 0;
			int posY = (argCount > 6) ? arg(7) : 0;
			int matchX = (argCount > 8) ? arg(9) : -1;
			int matchY = (argCount > 9) ? arg(10) : -1;

			SOL_Point pt(posX, posY);
			if (posX == -1) 
				pt.x = celHeader->xHot;
			if (posY == -1) 
				pt.y = celHeader->yHot;

			if (matchX == -1)
				pt.x -= celObj.XOrg();
			else
				pt.x -= matchX;

			if (matchY == -1)
				pt.y -= celObj.YOrg();
			else
				pt.y -= matchY;

			// Figure out the actual draw region into the cel. Make sure it
			// does not get bigger than existing bitmap.
			theTextBox.Set(pt.x,pt.y,pt.x+objWidth-1,pt.y+objHeight-1); 
			BMRect.Set(0, 0, BMWidth-1, BMHeight-1);
			theTextBox.Clip(BMRect);

			// make sure the view is not purged by BM.Get() allocation
			resMgr->Get(MemResView, arg(3));

			// Create a buffer that points to bitmap
			BufferStd buffer(BMWidth, BMHeight, (uchar*) memMgr->GetAddr(BM) + CELHEADERSIZE);

			// draw the cel object into the buffer
			// theTextBox: relative to upper left. textBox >= pt coords
			// pt: upper left of cel
			if (!theTextBox.IsEmpty())
				celObj.Draw(buffer, theTextBox, pt, celObj.Mirrored());

			break;
		}
		case bAddText: {
			// Add text to bitmap
			//	------------------
			// Input
			//      arg2:  MemID for the bitmap
			//      arg3:  text

			//		  arg4:  textRect.A.x
			//		  arg5:  textRect.A.y
			//		  arg6:  textRect.B.x
			//		  arg7:  textRect.B.y
			//      arg8:  fore
			//      arg9:  back
			//      arg10:  skip
			//      arg11:  font
			//      arg12:  mode
			//      arg13: borderColor
			//      arg14: dimmed

			BM = arg(2);
			celHeader = ((CelHeader*) memMgr->GetAddr(BM));
			BMWidth = celHeader->xDim;
			BMHeight = celHeader->yDim;

			SOL_Rect theTextBox(arg(4),arg(5),arg(6),arg(7));
			SOL_Rect BMRect(0, 0, BMWidth - 1, BMHeight - 1);

			theTextBox.Clip(BMRect);
			SOL_Point pt(theTextBox.A);

         SOL_Rect textBox(theTextBox);
			textBox.MoveTo(0,0);

			// create a text bitmap
			MemID textBM =
				fontMgr->BuildFBM(
               textBox.Xlen(),
               textBox.Ylen(),
					textBox,
					arg(3),
					arg(8),	// fore
					arg(9),  // back
					arg(10),	// skip
					arg(11),	// font
					arg(12),	// mode
					arg(13),	// borderColor
					arg(14),	// dimit
               False
				);

			// merge the text bitmap with passed in bitmap
			BufferStd buffer(BMWidth,
                          BMHeight,
                          (uchar*) memMgr->GetAddr(BM) + CELHEADERSIZE);

			CelObjMem celObj(textBM);

			celObj.Draw(buffer,theTextBox,pt,0);

			// get rid of the text bitmap
			memMgr->Free(textBM);

			break;
		}
		case bAddRect: {
			// Add rectangle to bitmap
			//	-----------------------
			// Input
			//      arg2-----MemID for the bitmap
			//      arg3-----Point rect.x1	left
			//      arg4-----Point rect.y1	top
			//      arg5-----Point rect.x2	right
			//      arg6-----Point rect.y2	bottom
			//      arg7-----color
			// Output
			// 	  Rectangle added to bitmap
			//		  Accumulator not modified

#ifdef WIN32S
	arg(7) = (SCIWord)AddPalShift((int)arg(7));
#endif

			BM = arg(2);

			// The color
			uchar color = arg(7);

			// Get the width of the bitmap
			celHeader = ((CelHeader*) memMgr->GetAddr(BM));
			BMWidth = celHeader->xDim;

			uchar* pixelAddr = ((uchar*) celHeader) + CELHEADERSIZE;
			pixelAddr += BMWidth*arg(4) + arg(3);

			int hRun = arg(5) - arg(3) + 1;
			int vRun = arg(6) - arg(4) + 1;

#ifdef DEBUG
			// Integrity checks
			BMHeight = celHeader->yDim;
			if ((hRun <= 0)			||
				 (vRun <= 0)			||
				 (hRun > BMWidth)		||
				 (vRun > BMHeight))

	 			msgMgr->Fatal(SrcLoc,"Invalid rectangle for given bitmap");
#endif

			for (;vRun > 0;--vRun) {
				memset(pixelAddr,color,hRun);
				pixelAddr += BMWidth;
			}
			break;
		}
		case bAddBitmap: {
			// Add bitmap to bitmap
			//	--------------------
			// Input
			//      arg2:  MemID for the bitmap
			//      arg3:  MemID for bitmap to merge
			//      arg4:  left X [==-1 match origin]
			//      arg5:  top Y	 [==-1 match origin]
			//		  arg6:  don't match origin points override

			MemID			mergeBM;
			SOL_Rect    mergeRect, BMRect;
			int			mergeBMHeight, mergeBMWidth;
			CelHeader* 	mergeCelHeader;

			BM = arg(2);
			celHeader = ((CelHeader*) memMgr->GetAddr(BM));
			BMWidth = celHeader->xDim;
			BMHeight = celHeader->yDim;

			mergeBM = arg(3);
			mergeCelHeader = ((CelHeader*) memMgr->GetAddr(mergeBM));
			mergeBMWidth  = mergeCelHeader->xDim;
			mergeBMHeight = mergeCelHeader->yDim;

			int posX = (argCount > 3) ? arg(4) : 0;
			int posY = (argCount > 4) ? arg(5) : 0;

			// merge the text bitmap with passed in bitmap
			BufferStd buffer(BMWidth,BMHeight,(uchar*) memMgr->GetAddr(BM) + CELHEADERSIZE);
			SOL_Point pt(posX,posY);
			if (argCount < 6 || !arg(6)) {
				if (posX == -1) 
					pt.x = celHeader->xHot - mergeCelHeader->xHot;
				if (posY == -1) 
					pt.y = celHeader->yHot - mergeCelHeader->yHot;
			}

			mergeRect.Set(pt.x, pt.y, pt.x + mergeBMWidth - 1, pt.y + mergeBMHeight - 1);

			CelObjMem celObj(mergeBM);
			BMRect.Set(0, 0, BMWidth - 1, BMHeight - 1);
			mergeRect.Clip(BMRect);
			celObj.Draw(buffer, mergeRect, pt, 0);

			break;
		}
		case bInvertRect: {
			// Invert a bitmap
			//	--------------------
			// Input
			//      arg2:  MemID for the bitmap
			//      arg3:  rectangle x1
			//      arg4:  rectangle y1
			//      arg5:  rectangle x2
			//      arg6:  rectangle y2
			//      arg7:  foreground color
			//      arg8:  background color
			//

#ifdef WIN32S
	arg(7) = (SCIWord)AddPalShift((int)arg(7));
	arg(8) = (SCIWord)AddPalShift((int)arg(8));
#endif

			BM = arg(2);
			celHeader = ((CelHeader*) memMgr->GetAddr(BM));
			BMWidth = celHeader->xDim;
			BMHeight = celHeader->yDim;

			SOL_Rect r(arg(3), arg(4), arg(5), arg(6));

			fontMgr->InvertRect(BM,BMWidth,r,arg(7),arg(8),False);

			break;
		}
		case bSetOrigin: {
			BM = arg(2);
			celHeader = ((CelHeader*) memMgr->GetAddr(BM));
			celHeader->xHot = arg(3);
			celHeader->yHot = arg(4);

			break;
		}
		case bRemap: {
			// Create a bitmap with remapped colors from a bitmap and a CLUT
			//---------------------
			// Input
			//			arg2:	bitmapID
			//			arg3: ArrayID
			// Output
			//		  pm.acc = id for bitmap

			RemapBitmap(arg(2), arg(3));

			break;
		}
		case bDuplicate: {
			// Duplicate an existing bitmap
			//	--------------
			// Input
			//      arg2-----bitmap to copy from
			// Output
			//		  pm.acc = id for bitmap

 			BM = arg(2);
			MemID newBM;

			// Get the width of the bitmap
			celHeader = ((CelHeader*) memMgr->GetAddr(BM));
			BMWidth  = celHeader->xDim;
			BMHeight = celHeader->yDim;

			int BMresX = (argCount > 5) ? arg(6) : fontMgr->GetXRes();
			int BMresY = (argCount > 6) ? arg(7) : fontMgr->GetYRes();

			newBM.Get(MemBitmap,BMWidth*BMHeight+CELHEADERSIZE);

			// copy the old bitmap into the new one
			memcpy((uchar*) memMgr->GetAddr(newBM)+CELHEADERSIZE,(uchar*) memMgr->GetAddr(BM)+CELHEADERSIZE,BMWidth*BMHeight);

			BuildHeader(newBM,BMWidth,BMHeight,celHeader->skip,
							celHeader->xHot,
							celHeader->yHot,
							celHeader->xRes,
							celHeader->yRes,
							0,
							False
						  );

			pm.acc = newBM;
			break;
		}

		default:
			break;
	}
}

void
RemapBitmap(MemID BM, ObjectID clutData)
{
	if ((ObjectID) clutData.IsObject())
		clutData = clutData.GetProperty(s_data);
	if (!clutData)
		return;

	CelHeader* celHeader = ((CelHeader*) memMgr->GetAddr(BM));
	int BMWidth  = celHeader->xDim;
	int BMHeight = celHeader->yDim;

	// setup pointers to BM and data block
	uchar* startColor = (uchar*) (memMgr->GetAddr(BM)) +CELHEADERSIZE;
	uchar* data = (uchar*) ((ArrayID) clutData).Data();

	// Go through each pixel and remap the color to CLUT 
	for(int i=0;i<BMWidth*BMHeight;i++)
		*startColor++ = data[*startColor];

}


void
KDeleteScreenItem(kArgs args)
{
	// Input
	//      arg1-----the SCI Object
 
	ObjectID id = arg(1);

	id->ClearInSIList();

	int planeId = id.GetIndexedProperty(actPlaneID);
	ScreenItem* si = graphMgr->NextScreen()->GetScreenItem(planeId,id);
	if (si == NULL)
      // ignore
      return;
	graphMgr->DeleteScreenItem(si,planeId);
}

void
KDeleteLine(argList)
{
	// Input
	//      arg1-----the ScreenLine ID
	//      arg2-----the plane ID
 
	// Get the existing screen item
	ScreenItem* si = graphMgr->NextScreen()->GetScreenItem(arg(2),arg(1));
	if (si == NULL) 
		return;

	// Get rid of old bitmap
	if (si->bitmap.Type() == Bitmap::tMem) {
		si->bitmap.Handle().Free();
	}

	graphMgr->DeleteScreenItem(si,(int) arg(2));
}

void
KDeletePolygon(argList)
{
	// Input
	//      arg1-----the ScreenPolygon item ID
	//      arg2-----the plane ID
 
	// Get the existing screen item
	ScreenItem* si = graphMgr->NextScreen()->GetScreenItem(arg(2),arg(1));
	if (si == NULL) 
		return;

	// Get rid of old bitmap
	if (si->bitmap.Type() == Bitmap::tMem) {
		si->bitmap.Handle().Free();
	}

	graphMgr->DeleteScreenItem(si,(int) arg(2));
}

void
KUpdateScreenItem(argList)
{
	// Input
	//      arg1-----the SCI Object

	ObjectID id = arg(1);

	if(!id.GetProperty(s_magnify)) {
		// UPDATE SCREEN ITEM
		int planeId = id.GetIndexedProperty(actPlaneID);
		ScreenItem* si = graphMgr->NextScreen()->GetScreenItem(planeId,id);
		if (si == NULL)
			msgMgr->Fatal(SrcLoc,Msg_ScreenItemNotExist);
		*si = id;
	} 
	else {
		// UPDATE MAGNIFIER
		// look for existing magnifier                                                  
	   MagnifyView *ptr = graphMgr->magHead; 
	   MagnifyView *last = NULL;
	   while(ptr && ptr->id != id) {
	   	last = ptr;
		   ptr = ptr->next;
	   }
		if(ptr) 
			// magnifer already in list update
			ptr->Update();
		else
	 		msgMgr->Fatal(SrcLoc,"Magnify not registered");

   }
}

void
KUpdateLine(argList)
{
	// Input
	//      arg1-----ScreenLine ID
	//      arg2-----Plane ID
	//      arg3-----Point A.x
	//      arg4-----Point A.y
	//      arg5-----Point B.x
	//      arg6-----Point B.y
	//		  The following arguments are optional (all or none)
	//      arg7-----z
	//      arg8-----color
	//      arg9-----style
	//      arg10----pattern  Solid=0,Dashed=1,Pattern=2};
	//      arg11----thickness = 2n+1 (1->1,2->1,3->3,4->3,5->5 etc.)

	// Get the existing screen item
	ScreenItem* si = graphMgr->NextScreen()->GetScreenItem(arg(2),arg(1));
	if (si == NULL)
		msgMgr->Fatal(SrcLoc,Msg_ScreenItemNotExist);

	int	z = si->Priority();
	uchar	color = si->color;
	int	style = SOL_Line::tSolid;
	int	pattern = 0;
	int	thickness = 1;

	if (argCount == 11) {
		// No default arguments
		z = arg(7);
		color = arg(8);
		style = arg(9);
		pattern = arg(10);
		thickness = arg(11);
	}
		
	// Convert 16 bit pattern to 32 bit pattern
	pattern |= pattern << 16;

	SOL_Rect rect;
	MemID BM = BuildLineBitmap(arg(3),arg(4),arg(5),arg(6),color,style,pattern,thickness,rect);

	// Get rid of old bitmap
	if (si->bitmap.Type() == Bitmap::tMem) {
		si->bitmap.Handle().Free();
	}

	// Update bitmap, position, priority and color
	si->bitmap.SetHandle(BM);
	si->pos.x = rect.A.x;
	si->pos.y = rect.A.y;
	si->SetPriority(z);
	si->color = color;

	// Set to redraw
	graphMgr->UpdateScreenItem(si);
}

void
KUpdatePolygon(argList)
{
	// Input
	//      arg1-----ScreenPolygon ID
	//      arg2-----Plane ID
	//      arg3-----Polygon list MemID
	//		  The following arguments are optional (all or none)
	//      arg4-----z (priority)
	//      arg5-----color
	//      arg6-----style  Solid=0,Dashed=1,Pattern=2
	//      arg7-----pattern
	//      arg8-----thickness = 2n+1 (1->1,2->1,3->3,4->3,5->5 etc.)


	// Get the existing screen item
	ScreenItem* si = graphMgr->NextScreen()->GetScreenItem(arg(2),arg(1));
	if (si == NULL)
		msgMgr->Fatal(SrcLoc,Msg_ScreenItemNotExist);

	int	z = si->Priority();
	uchar	color = si->color;
	int	style = SOL_Line::tSolid;
	int	pattern = 0;
	int	thickness = 1;
	Bool	close = True;

	if (argCount == 8) {
		// No default arguments
		z = arg(4);
		color = arg(5);
		style = arg(6);
		pattern = arg(7);
		thickness = arg(8);
	}
		
	// Convert 16 bit pattern to 32 bit pattern
	pattern |= pattern << 16;

	SOL_Rect rect(100000,100000,-100000,-100000);

   ObjectID id = arg(3);
	SOL_ListID theList = id.GetProperty(s_elements);

	MemID BM = BuildPolygonBitmap(theList,
										color,
										style,
										pattern,
										thickness,
										rect,
										close);

	// Get rid of old bitmap
	if (si->bitmap.Type() == Bitmap::tMem) {
		si->bitmap.Handle().Free();
	}

	// Update bitmap, position, priority and color
	si->bitmap.SetHandle(BM);
	si->pos.x = rect.A.x;
	si->pos.y = rect.A.y;
	si->SetPriority(z);
	si->color = color;

	// Set to redraw
	graphMgr->UpdateScreenItem(si);
}

void
KAddMagnify(argList)
{
	// Input
	//      arg1-----the SCI Object

	ObjectID id = arg(1);

	// look for existing magnifier to kill if needed                                                        
	MagnifyView *ptr = graphMgr->magHead; 
	MagnifyView *last = NULL;
	while(ptr && ptr->id != id) {
	last = ptr;
		ptr = ptr->next;
	}

	// if found a ptr to delete
	if (ptr) {
	// if last is null we're at the head of the list
		if (!last) { 
			graphMgr->magHead = ptr->next;
			delete ptr;
		} else {   
			last->next = ptr->next;
			delete ptr;
		}
	}
   // Now add new magnifier
	graphMgr->magHead = New MagnifyView(id,graphMgr->magHead);
}

void
KDeleteMagnify(argList)
{
	// Input
	//      arg1-----the SCI Object

	ObjectID id = arg(1);

	// look for existing magnifier                                                  
	MagnifyView *ptr = graphMgr->magHead; 
	MagnifyView *last = NULL;
	while(ptr && ptr->id != id) {
		last = ptr;
		ptr = ptr->next;
	}
	// if found a ptr to delete
	if (ptr) {
		// if last is null we're at the head of the list
		if (!last) { 
			graphMgr->magHead = ptr->next;
			delete ptr;
		} else {   
			last->next = ptr->next;
			delete ptr;
		}
	}

	ObjectID magID(id.GetProperty(s_client));
	id->ClearInSIList();

	int planeId = magID.GetIndexedProperty(actPlaneID);

	ScreenItem* si = graphMgr->NextScreen()->GetScreenItem(planeId,id);
	if (si == NULL)
      // ignore
      return;
	graphMgr->DeleteScreenItem(si,planeId);
}

void
KAddPlane(argList)
{
	// Input
	//      arg1-----an SCI plane Object
	// Ouput
	//      none

	ObjectID id = (ObjectID) arg(1);

	Plane* plane = graphMgr->GetPlaneNext(id);
	if (plane == NULL) {
		plane = New Plane((ObjectID) arg(1));
		// add the plane
		graphMgr->AddPlane(plane);
	}
	else {
		// Change add to update
		// Put new data into plane
		*plane = id;

		graphMgr->UpdatePlane(plane);
	}
}

void
KDeletePlane(argList)
{
	// Input
	//      arg1-----an SCI plane Object
	// Ouput
	//      none

	ObjectID id = arg(1);

	graphMgr->DeletePlane((int) arg(1));
}

void
KUpdatePlane(argList)
{
	// Input
	//      arg1-----an SCI plane Object
	// Ouput
	//      none

	ObjectID id = (ObjectID) arg(1);

	Plane* plane = graphMgr->GetPlaneNext(id);
	if (plane == NULL)
		msgMgr->Fatal(SrcLoc,Msg_ScreenItemNotExist);

	// Put new data into plane
	*plane = id;

	graphMgr->UpdatePlane(plane);
}

void
KAddPicAt(argList)
{
	// Input
	//      arg1-----an SCI plane Object
	//      arg2-----picture number
	//      arg3-----x postion plane relative of left corner of picture
	//      arg4-----y postion plane relative of top corner of picture
	//		  arg5-----True if mirrored, False for not mirrored
	// Ouput
	//      picture added to plane at specified postion

	// Currently the assumption is that the
	// plane is already a picture plane
	ObjectID id = (ObjectID) arg(1);
	Bool mirror = False;
	if (argCount > 4)
		mirror = arg(5);
	SOL_Point anchor(arg(3),arg(4));

	Plane* plane = graphMgr->GetPlaneNext(id);
	if (plane != NULL)
		plane->AddPic(arg(2),anchor,mirror);
	else
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,id);

}

void
KRepaintPlane(argList)
{
	// Input
	//      arg1-----an SCI plane Object
	// Ouput
	//      none
	
	ObjectID id = (ObjectID) arg(1);

	Plane* plane = graphMgr->GetPlaneCurrent(id);
	if (plane != NULL)
		graphMgr->Repaint(plane->OnScreen());
	else
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,id);
}

void 
KGetHighPlanePri(kArgs)
{
	pm.acc = graphMgr->TopSCIPlane();
}

void
KGetHighItemPri(argList)
{
	// Input
	//      arg1-----the plane's ID

	pm.acc = graphMgr->TopScreenItem(arg(1));
}

void
KCelHigh(argList)
{
   // Input is view,loop,cel
	//  Report height in game resolution
   CelObjView celObj(arg(1),arg(2),arg(3));
   pm.acc = celObj.Height() * Ratio(LOWRESY, celObj.ResY());
}

void
KCelWide(argList)
{
   // Input is view,loop,cel
   // Output is the given cels width
   CelObjView celObj(arg(1),arg(2),arg(3));
   pm.acc = celObj.Width() * Ratio(LOWRESX, celObj.ResX());
}

void
KNumLoops(argList)
{
	// Input
	//      arg1-----the SCI Object
	// Ouput
	//    acc------the number of loops in the view

	ObjectID id = arg(1);

	int view = id.GetIndexedProperty(actView);
	pm.acc = GetNumLoops(resMgr->Get(MemResView,view));
}

int
GetNumLoops(MemID id)
{
	return ((ViewHeaderID) id)->loopCount;
}


void
KNumCels(argList)
{
	// Input
	//    arg1-----the SCI Object
	// Ouput
	//    acc------the number of cels in the object's view,loop

	ObjectID id = arg(1);

	int view = id.GetIndexedProperty(actView);
	int loop = id.GetIndexedProperty(actLoop);

	pm.acc = GetNumCels(resMgr->Get(MemResView,view),loop);
}


int
GetNumCels(MemID viewID,int loop)
{
	ViewHeader * viewPtr = (ViewHeader *) *viewID;
	if (viewPtr->loopCount < loop) 
		return 0;
	else {
		LoopHeader * loopPtr =
			(LoopHeader *) ((char *) viewPtr + viewPtr->loopHeaderSize * loop + viewPtr->viewHeaderSize + 2);
		if (loopPtr->altLoop != -1)
			loopPtr = (LoopHeader*) ((char*) viewPtr + viewPtr->loopHeaderSize*loopPtr->altLoop + viewPtr->viewHeaderSize + 2);
		return loopPtr->numCels;
	}
}

void
KShakeScreen(argList)
{
	graphMgr->ShakeScreen(arg(1), arg(2));
}

void
KIsOnMe(argList)
{
	// Input
	//		arg(1) = x coordinate of point to check (plane relative)
	//		arg(2) = y   "
	// 	arg(3) = obj
	//		arg(4) = check skip or not
	//
	// Returns -
	// 	True or False

	ObjectID id = arg(3);

	int planeId = id.GetIndexedProperty(actPlaneID);

	ScreenItem* si = graphMgr->VisibleScreen().GetScreenItem(planeId,id);
	if (si == NULL) {
		pm.acc = False;
		return;
	}

	Plane* plane = graphMgr->GetPlaneCurrent(planeId);
	if (plane == NULL) {
		pm.acc = False;
		return;
	}

	ScreenItem siCopy(si);

	pm.acc = graphMgr->IsOnMe(siCopy,*plane,SOL_Point(arg(1), arg(2)),(Bool) arg(4));
}

void
KSetNowSeen(argList)
{
	// Input
	//      arg1-----the SCI Object
	// Ouput
	//       -----the nowSeen rectangle is set

	ObjectID id = arg(1);

	int planeID = id.GetIndexedProperty(actPlaneID);
	SOL_Rect nowSeen;
	graphMgr->NowSeen((int) id,planeID,nowSeen);
#ifdef NETWORK
	id.SetIndexedProperty(actLeft, nowSeen.A.x);
	id.SetIndexedProperty(actTop, nowSeen.A.y);
	id.SetIndexedProperty(actRight, nowSeen.B.x);
	id.SetIndexedProperty(actBottom, nowSeen.B.y);
#else
	id.SetIndexedProperty(actNSLeft, nowSeen.A.x);
	id.SetIndexedProperty(actNSTop, nowSeen.A.y);
	id.SetIndexedProperty(actNSRight, nowSeen.B.x);
	id.SetIndexedProperty(actNSBottom, nowSeen.B.y);
#endif
}

void
KObjectIntersect(argList)
{
	// Input
	//      arg1-----the SCI Object
	//      arg2-----the other SCI Object
	// Ouput
	//          -----The AREA of the object intersection.

	ObjectID id = arg(1);
	int planeID = id.GetIndexedProperty(actPlaneID);
	SOL_Rect nowSeen;
	graphMgr->NowSeen(arg(1),planeID,nowSeen);

	ObjectID id2 = arg(2);
	int planeID2 = id2.GetIndexedProperty(actPlaneID);
	SOL_Rect nowSeen2;
	graphMgr->NowSeen(arg(2),planeID2,nowSeen2);
	nowSeen.Clip(nowSeen2);
	long intersectArea = nowSeen.Area();
	pm.acc = intersectArea;

}

void
KPriority(argList)
{
	int i = arg(1);
//	graphMgr->ShowPriority(arg(1));
}

void
BuildHeader(uchar* bmAddr,
            int width,
            int height,
            int skip,
            int xHot,
            int yHot,
            int xRes,
            int yRes,
				Bool palFlag,
				Bool remap)
{
	// initialize the header
	CelHeader* celHeader = (CelHeader*) bmAddr;
	celHeader->xDim = width;
	celHeader->yDim = height;
	celHeader->xHot = xHot;
	celHeader->yHot = yHot;
	celHeader->skip = skip;
	celHeader->compressType = 0;     //HOOK Needs to be fixed
	celHeader->dataFlags = 0;
	celHeader->dataByteCount = width * height;
	celHeader->controlByteCount = 0;
	celHeader->xRes = xRes;
	celHeader->yRes = yRes;

	if (palFlag) 
		celHeader->paletteOffset = CELHEADERSIZE + (width * height);
	else
		celHeader->paletteOffset = 0;

	// HOOK 
	// the pics have the color offset in the controlOffset field
	// when that is corrected then this can be corrected.
// celHeader->controlOffset = 0;
	celHeader->controlOffset = CELHEADERSIZE;
	celHeader->colorOffset = CELHEADERSIZE;
	celHeader->rowTableOffset = 0;

	if (remap)
		celHeader->dataFlags |= 2; //2=REMAP_PRESENT
}

MemID
BuildLineBitmap(int x1, int y1, int x2, int y2, uchar color,
					 int style, int pattern, int thickness, SOL_Rect& rect)
{
	MemID    BM;

	thickness = Max(1,thickness);
	if (!(thickness & 1))
		thickness -= 1;
	thickness = thickness >> 1;

	// Find the rectangle thats fits the line
	rect.A.x = (x1 < x2)? x1 - thickness:x2 - thickness;
	rect.A.y = (y1 < y2)? y1 - thickness:y2	- thickness;
	rect.B.x = (x2 < x1)? x1 + thickness:x2 + thickness;
	rect.B.y = (y2 < y1)? y1 + thickness:y2	+ thickness;

	// Get memory for bitmap
	int BMWidth = rect.Xlen();
	int BMHeight = rect.Ylen();
	BM.Get(MemBitmap,BMWidth*BMHeight+CELHEADERSIZE);

	// Chose a skip
	uchar skip = 0;
	if (color != SOL_SKIP)
		skip = SOL_SKIP;
	// build the header for the BM
	BuildHeader(BM,BMWidth,BMHeight,skip);

	// Fill the bitmap with skip
	memset((uchar*) memMgr->GetAddr(BM)+CELHEADERSIZE,skip,BMWidth*BMHeight);

	SOL_Line line(x1 - rect.A.x,
					  y1 - rect.A.y,
					  x2 - rect.A.x,
					  y2 - rect.A.y);

	if (!thickness)
		line.DrawThin(color,style,pattern,BM,BMWidth);
	else
//	 	msgMgr->Fatal(SrcLoc,"Thick line not currently supported");
		line.DrawThick(color,thickness,style,pattern,BM,BMWidth);
	return BM;
}

MemID
BuildPolygonBitmap(SOL_ListID theList,uchar color,int style,
						 int pattern,int thickness,SOL_Rect& rect,Bool close)
{
	MemID    BM;

	thickness = Max(1,thickness);
	if (!(thickness & 1))
		thickness -= 1;
	thickness = thickness >> 1;

   SOL_List* list = *theList;

	for (KNodeID node = list->First();(int) node;node = node->Next()) {
      ObjectID id = (ObjectID) node->val;

		ObjectID arrayID = id.GetProperty(s_points);
		MemID pointsID = arrayID.GetProperty(s_data);

      SCI_Point* points = (SCI_Point*) memMgr->GetAddr(pointsID);

		int size = id.GetProperty(s_size);
	
		for (int i=1;i<=size;++i) {
			if (rect.A.x > points[i].x) 
				rect.A.x = points[i].x;
			if (rect.B.x < points[i].x) 
				rect.B.x = points[i].x;

			if (rect.A.y > points[i].y) 
				rect.A.y = points[i].y;
			if (rect.B.y < points[i].y) 
				rect.B.y = points[i].y;
		}
	}

	rect.A.x -= thickness;
	rect.A.y -= thickness;
	rect.B.x += thickness;
	rect.B.y += thickness;

	// Get memory for bitmap
	int BMWidth = Max(0,rect.Xlen());
	int BMHeight = Max(0,rect.Ylen());
	BM.Get(MemBitmap,BMWidth*BMHeight+CELHEADERSIZE);

	// Chose a skip
	uchar skip = 0;
	if (color != SOL_SKIP)
		skip = SOL_SKIP;
	// build the header for the BM
	BuildHeader(BM,BMWidth,BMHeight,skip);

	// Fill the bitmap with skip
	memset((uchar*) memMgr->GetAddr(BM)+CELHEADERSIZE,skip,BMWidth*BMHeight);

	// Draw the lines into the bitmap
	for (node = list->First();node;node = node->Next()) {
      ObjectID id = (ObjectID) node->val;

		ObjectID arrayID = id.GetProperty(s_points);
		MemID pointsID = arrayID.GetProperty(s_data);

      SCI_Point* points = (SCI_Point*) memMgr->GetAddr(pointsID);

		int size = id.GetProperty(s_size);
	


		for (int i=1;i<size;++i) {
			SOL_Line line(points[i].x - rect.A.x,
							  points[i].y - rect.A.y,
							  points[i+1].x - rect.A.x,
							  points[i+1].y - rect.A.y);
			if (!thickness)
				line.DrawThin(color,style,pattern,BM,BMWidth);
			else
//	 	      msgMgr->Fatal(SrcLoc,"Thick line not currently supported");
				line.DrawThick(color,thickness,style,pattern,BM,BMWidth);
		}
		if (close) {
			SOL_Line line(points[1].x - rect.A.x,
							  points[1].y - rect.A.y,
							  points[size].x - rect.A.x,
							  points[size].y - rect.A.y);
			if (!thickness)
				line.DrawThin(color,style,pattern,BM,BMWidth);
			else
//	 	      msgMgr->Fatal(SrcLoc,"Thick line not currently supported");
				line.DrawThick(color,thickness,style,pattern,BM,BMWidth);
		}
	}

	return BM;
}

void	KMovePlaneItems(argList)
{
	// Shifts all the screen items in the plane
	// Input
	//      arg1-----the SCI Object plane id
	//      arg2-----the delta x 
	//      arg3-----the delta y 
	//      arg4-----Bool shift the picture items also
	// Ouput
	//      none

	int	planeID = arg(1);
	int	deltaX = arg(2);
	int	deltaY = arg(3);
	Bool 	movePic = False;
	if (argCount > 3)
		movePic = arg(4);

	graphMgr->ScrollScreenItems(planeID,deltaX,deltaY,movePic);

	// Update position of all SCI objects in plane

	ScreenItemList& siArray = graphMgr->GetPlaneNext(planeID)->SIList();
	int count = siArray.Length();
	for (int i=0;i<count;++i) {
		ObjectID  siID = siArray[i].Id();
		if ((int) siID < 20000)	{
			// Ajdust it's position
			if (deltaX) {
				int newX = siID.GetIndexedProperty(actX) + deltaX;
				siID.SetIndexedProperty(actX,newX);
			}
			if (deltaY) {
				int newY = siID.GetIndexedProperty(actY) + deltaY;
				siID.SetIndexedProperty(actY,newY);
			}
		}
	}
}



void
KBaseLineSpan(argList)
{
	//	   Input:
	//		arg(1) = ArrayID to array for rect
	//		arg(2) = view
	//		arg(3) = loop
	//		arg(4) = cel
	//		arg(5) = [x]
	//		arg(6) = [y]
	//		Returns in ArrayID:
	//			A.x = leftmost non-skip of cel
	//			B.x = rightmost non-skip of cel
	//			A.y = B.y = bottom line of non skip

	CelObjView vlc(arg(2),arg(3),arg(4));
	
	SOL_Rect ptz,ptemp;

	SCIWord* data = (SCIWord*) ((ArrayID) arg(1)).Data();

	int	begin,end;

	begin	= data[1];
	end	= data[3];

	if (end == 0)
		end = vlc.Height() - 1;

	// Find beginning xy and ending xy of cel's base line span

	ptemp.A.x =	 1000;
	ptemp.B.x = -1;

	for (int i = begin; i <= end  ; i++) {
		ptz	= vlc.ReadLineSpan(i);
		
		if (ptz.A.x != -1)  {
			if (ptz.A.x < ptemp.A.x) {
				ptemp.A.x = ptz.A.x;
				ptemp.A.y = i;
			}

			if (ptz.B.x > ptemp.B.x) {
				ptemp.B.x = ptz.B.x;
				ptemp.B.y = i;
			}
		}
	}

	// Make the xy points relative to the cel origin and the passed in point

	int cWidth = vlc.Width();
	if (vlc.Mirrored()) {
		int ptempA = ptemp.A.x;
		ptemp.A.x = cWidth - ptemp.B.x -1;
		ptemp.B.x = cWidth - ptempA - 1;
	}
	int xOff = ((argCount > 4) ? arg(5) : 0);
	int yOff = ((argCount > 5) ? arg(6) : 0);


	//	Store the info in the data which can be read back from SCI

	// left
	data[0] = (ptemp.A.x - vlc.XOrg()) * Ratio(LOWRESX, vlc.ResX()) + xOff;
	// right
	data[2] = (ptemp.B.x - vlc.XOrg()) * Ratio(LOWRESX, vlc.ResX()) + xOff;
	// top
	data[1] = (ptemp.A.y - vlc.YOrg()) * Ratio(LOWRESY, vlc.ResY()) + yOff;
	// bottom
	data[3] = (ptemp.B.y - vlc.YOrg()) * Ratio(LOWRESY, vlc.ResY()) + yOff;

}

void
KCelInfo(argList)
{

	enum {
		getOrigX,
		getOrigY,
		getLinkX,
		getLinkY,
		getPixel
	};

	int func = arg(1);

	CelObjView celObj(arg(2),arg(3),arg(4));

	switch (func) {
		case getOrigX: {
			pm.acc = celObj.XOrg();
			break;
		}
		case getOrigY: {
			pm.acc = celObj.YOrg();
			break;
		}
		case getLinkX: {
			pm.acc = 0;
			break;
		}
		case getLinkY: {
			pm.acc = 0;
			break;
		}
		case getPixel: {
			pm.acc = celObj.ReadPix(arg(5), arg(6), celObj.Mirrored());
			break;
		}
	}
}


int
AddLinkedBitmap(ObjectID idBase, ObjectID	idAdd, int type)
{
	int planeId = idBase.GetIndexedProperty(actPlaneID);
	ScreenItem* siBase = graphMgr->NextScreen()->GetScreenItem(planeId,idBase);

	planeId = idAdd.GetIndexedProperty(actPlaneID);
	ScreenItem* siAdd = graphMgr->NextScreen()->GetScreenItem(planeId,idAdd);
	MemID			BM;
	int 			BMWidthA,BMWidthB,objWidthA,objWidthB;
	int 			BMHeightA,BMHeightB,objHeightA,objHeightB;
	SOL_Rect    theTextBox,BMRect;
	int 			viewB,loopB,celB,viewA,loopA,celA,resX,resY;
	
	int i = type;
	int x,y;
	int largerBitmap = 0;
	int xOffset = 0;
	int yOffset = 0;

	viewB = siBase->bitmap.View();
	loopB = siBase->bitmap.Loop();
	celB  = siBase->bitmap.Cel();
   CelObjView vCelObjB(viewB,loopB,celB); 

	viewA = siAdd->bitmap.View();
	loopA = siAdd->bitmap.Loop();
	celA  = siAdd->bitmap.Cel();
   CelObjView vCelObjA(viewA,loopA,celA); 

	x = vCelObjB.GetLinkPoint(type,1);
	if (x == -1) {
		msgMgr->Fatal(SrcLoc,"No type for CelLink x");
		return(0);	
		}	

	y = vCelObjB.GetLinkPoint(type,0);
	if (y == -1){
 		msgMgr->Fatal(SrcLoc,"No type for CelLink y");
		return(0);	
	}

	objWidthB  = vCelObjB.Width();
	objHeightB = vCelObjB.Height();

	objWidthA  = vCelObjA.Width();
	objHeightA = vCelObjA.Height();

	BMWidthB 		= siBase->GetCel()->Width() ;
	BMHeightB 		= siBase->GetCel()->Height();

	x -= vCelObjA.XOrg();
	y -= vCelObjA.YOrg();

	if ((x+objWidthA)>BMWidthB) {
		BMWidthB = x + objWidthA ;
		largerBitmap = 1;
	}

	if ((y+objHeightA)>BMHeightB) {
		BMHeightB = y + objHeightA ;
		largerBitmap = 1;
	}

	if (x<0) {
		BMWidthB += -(x);
		largerBitmap = 1;
		xOffset  = -(x);
		x = 0;
	}

	if (y<0) {
		BMHeightB += -(y);
		yOffset   = -(y);
		largerBitmap = 1;
		y = 0;
	}

	
	if ((siBase->bitmap.Type() != Bitmap::tMem)||largerBitmap) {  // create a bitmap

		BM.Get(MemBitmap,BMWidthB*BMHeightB+CELHEADERSIZE);

		resX = siBase->GetCel()->ResX();
		resY = siBase->GetCel()->ResY();
		memset((uchar*) memMgr->GetAddr(BM)+CELHEADERSIZE,0,BMWidthB*BMHeightB);

	   BuildHeader(BM,BMWidthB,BMHeightB,0,siBase->GetCel()->XOrg(),siBase->GetCel()->YOrg(),resX,resY);
   
		siBase->bitmap = Bitmap(BM);
		siBase->bitmap.SetType(Bitmap::tMem);
				
		theTextBox.Set(xOffset,yOffset,objWidthB+xOffset,objHeightB+yOffset); 
		BMRect.Set(xOffset, yOffset,xOffset+BMWidthB-1,yOffset+BMHeightB-1);
		theTextBox.Clip(BMRect);
		SOL_Point pt(xOffset,yOffset);

		BufferStd buffer(BMWidthB,BMHeightB,(uchar*) memMgr->GetAddr(BM) + CELHEADERSIZE);

		vCelObjB.Draw(buffer, theTextBox, pt, 0);
		idBase.SetIndexedProperty(actBitmap,BM);
	}
	// add object to bitmap
	BM	= siBase->bitmap.Handle();

	theTextBox.Set(x,y,x+objWidthA,y+objHeightA);
	BMWidthA 		= siAdd->GetCel()->Width();
	BMHeightA 		= siAdd->GetCel()->Height();
 
	BMRect.Set(x,y,x+BMWidthA-1,y+BMHeightA-1);
	theTextBox.Clip(BMRect);

	SOL_Point pt(x,y);

	BufferStd buffer(BMWidthB,BMHeightB,(uchar*) memMgr->GetAddr(BM) + CELHEADERSIZE);
	vCelObjA.Draw(buffer, theTextBox, pt, 0);
	 
	return(BM);	
}

void 
KCelLink(argList)
{
	//		arg(1) = function
	//		arg(2) = view
	//		arg(3) = loop
	//		arg(4) = cel
	//		arg(5) = type

	enum {
		GetFirstLink,
		GetNextLink,
		GetXLinkPoint,
		GetYLinkPoint,
		AddBitmapLink
		
	};

	int function = arg(1);
	
	if (function == 4) {
		pm.acc = AddLinkedBitmap(arg(2),arg(3),arg(4));
		return;
	}

	CelObjView	vlc(arg(2),arg(3),arg(4));

	switch(function) {
		case GetFirstLink:
		pm.acc =	vlc.GetFirstLink();
		break;

		case GetNextLink:
		pm.acc = vlc.GetNextLink(arg(5));
		break;

		case GetXLinkPoint:
		pm.acc = vlc.GetLinkPoint(arg(5),1);
		break;

		case GetYLinkPoint:
		pm.acc = vlc.GetLinkPoint(arg(5),0);
		break;

		case AddBitmapLink:
		// arg(2) = baseSI
		// arg(3) = addedSI
		// arg(4) = linkType
		pm.acc = AddLinkedBitmap(arg(2),arg(3),arg(4));
		break;		

	}
}

