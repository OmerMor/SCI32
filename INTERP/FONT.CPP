//      font.cpp

#include <string.h>

#ifndef MACINTOSH

#include <mem.h>

#endif

#include "sol.hpp"

#include "array.hpp"
#include "celobj.hpp"
#include "font.hpp"
#include "kernel.hpp"
#include "msg.hpp"
#include "palette.hpp"
#include "resource.hpp"
#include "bufferd.hpp"
#include "graph.hpp"
#include "graphmgr.hpp"
#include "object.hpp"
#include "pmachine.hpp"
#include "textid.hpp"
#include "text.hpp"
#ifdef WIN32S
#include "shiftpal.hpp"
#endif

int	FontMgr::xRes = LOWRESX;
int	FontMgr::yRes = LOWRESY;

MemID
FontMgr::BuildFBM
	(
	Bitmap&     bitmap,
	SOL_Rect&   theTextBox,
	MemID       theText,
	int         theFore,
	int         theBack,
	int         theFont,
	int         theSkip,
	int         theBorderColor,
	Bool        dimIt
	)
{
	int frameSize = 1;

	// No title
	titleSize = 0;
   borderColor = theBorderColor;

	textBox = theTextBox;
   textBox.Scale(xRes,LOWRESX,yRes,LOWRESY);
	text = theText;
	fore = theFore;
	dim = dimIt;
	// Build the font table containing the font widths
	BuildFontTable(theFont);

	// create a cel object
	CelObjView celObj(bitmap.View(),bitmap.Loop(),bitmap.Cel());
   skip = celObj.Skip();

	// the cel object should have the dimension
	BMWidth = celObj.Width() * xRes / celObj.ResX();
	BMHeight = celObj.Height() * yRes / celObj.ResY();


	// clip the textbox by the FBM
	SOL_Rect rect(0,0,BMWidth - 1,BMHeight - 1);
	textBox.Clip(rect);

	// allocate memory for the bitmap
	BM.Get(MemBitmap,BMWidth*BMHeight+CELHEADERSIZE);

	// build the header for the BM
	BuildHeader(BM,BMWidth,BMHeight,skip,0,0,xRes,yRes);

	// draw the cel into the bitmap
	SOL_Point pt(0,0);

   memset((char*)memMgr->GetAddr(BM) + CELHEADERSIZE,11,BMWidth*BMHeight);

	BufferStd buffer(BMWidth,BMHeight,(uchar*) memMgr->GetAddr(BM) + CELHEADERSIZE);

	// fill the back ground with skip
	back = skip;
	Erase(rect,False);
	back = theBack;

	celObj.Draw(buffer,rect,pt,False,Ratio(xRes,celObj.ResX()),Ratio(yRes,celObj.ResY()));

	// fill the textBox with the back ground color (unless it's skip)
   if ((back != theSkip)        && (fore != theSkip))
	   Erase(textBox,False);

	if (!text)
		return BM;

   if (fore == theSkip) {
		// Build a FBM with the textBox as the rectangle
		int overlayWidth = textBox.Xlen();
		int overlayHeight = textBox.Ylen();
		SOL_Rect overlayTextBox(0,0,overlayWidth - 1,overlayHeight - 1);
		FontMgr fntMgr;
		MemID overlay = fntMgr.BuildFBM(overlayWidth,
												  overlayHeight,
												  overlayTextBox,
												  text,
												  fore,
												  back,
												  skip,
												  font,
												  mode,
												  borderColor,
												  0,
												  False);
		// Now copy this into the other bitmap treating skip as skip
		uchar* drawTo = (uchar*) memMgr->GetAddr(BM) + 
												 BMWidth*textBox.A.y + 
												 textBox.A.x +
												 CELHEADERSIZE;
		uchar* drawFrom = (uchar*) memMgr->GetAddr(overlay) + CELHEADERSIZE;
		int skipToNextRow = BMWidth - overlayWidth;
		for (int row = 0;row < overlayHeight;++row) {
			for (int col = 0;col < overlayWidth;++col) {
				uchar color = *drawFrom++;
				if (color == fore)
					++drawTo;
				else
					*drawTo++ = color;
			}
			drawTo += skipToNextRow;
		}
		memMgr->Free(overlay);
	}
	else {

      if (theBorderColor != -1) {
	      // frame the title box
	      Frame(rect,frameSize,borderColor,False);
      }
		// put the text into the bitmap
		if ((int)text)
		   DrawTextBox();
	}
	// return the handle to the BM
	return BM;
}


MemID 
FontMgr::BuildFBM
	(
	int         theWidth,
	int         theHeight,
	SOL_Rect&   theTextBox,
	MemID       theText,
	int         theFore,
	int         theBack,
	int         theSkip,
	int         theFont,
	int         theMode,
   int         theBorderColor,
	Bool        dimIt,
   Bool        scale
	)
{

#ifdef MACINTOSH

	if	(theFore == 255)						// MAC 4/28
		theFore = 0;
	else if	(theFore == 0)
		theFore = 255;

	if	(theBack == 255)
		theBack = 0;
	else if	(theBack == 0)
		theBack = 255;

	if	(theBorderColor == 255)
		theBorderColor = 0;
	else if	(theBorderColor == 0)
		theBorderColor = 255;


#endif

#ifdef WIN32S
	theFore = AddPalShift(theFore);
	theBack = AddPalShift(theBack);
	if (theBorderColor != -1)
		theBorderColor = AddPalShift(theBorderColor);
#endif

	int frameSize = 1;

	// No title
	titleSize = 0;
   borderColor = theBorderColor;

	SetFont(theFont);

	int textSize = strlen(*(TextID)theText);

   // talk to LARRY on this stuff note TextSize is a scale call
	/*SOL_Rect textRect;
	TextSize(theText,theWidth,textRect);
	if ((textRect.Xlen() > theTextBox.Xlen()) || (textRect.Ylen() > theTextBox.Ylen()))
		msgMgr->Fatal(SrcLoc,Msg_WindowTextTooBig);
	*/
	BMWidth = theWidth;
	BMHeight = theHeight;
	textBox = theTextBox;
	if (scale) {
		BMWidth = BMWidth * xRes / LOWRESX;
		BMHeight = BMHeight * yRes /LOWRESY;
		textBox.Scale(xRes,LOWRESX,yRes,LOWRESY);
   }
	text = theText;
	fore = theFore;
	back = theBack;
	skip = theSkip;
	mode = theMode;
	dim = dimIt;

	// clip the textbox by the FBM
	SOL_Rect rect(0,0,BMWidth-1,BMHeight-1);
	textBox.Clip(rect);

	// allocate memory for the bitmap
	BM.Get(MemBitmap,BMWidth*BMHeight+CELHEADERSIZE);

	// build the header for the BM
	BuildHeader(BM,BMWidth,BMHeight,skip,0,0,xRes,yRes);

	// fill the bitmap with the background color
	Erase(rect,False);

   if (theBorderColor != -1) {
	   // frame the title box
	   Frame(rect,frameSize,borderColor,False);
   }

	// put the text into the bitmap
	DrawTextBox();

	// return the handle to the BM
	return BM;
}


MemID 
FontMgr::TitledFBM
	(
	int                theWidth,            // main info
	int                theHeight,
	SOL_Rect&   theTextBox,
	MemID           theText,
	int                theFore,
	int                theBack,
   int         theSkip,
	int                theFont,
	int                theMode,
   int         theBorderColor,

	MemID           theTitleText,   // Title info
	int                theTitleFore,
	int                theTitleBack,
	int                theTitleFont,
   Bool					scale
	)
{

#ifdef WIN32S
	theFore = AddPalShift(theFore);
	theBack = AddPalShift(theBack);
	if (theBorderColor != -1)
		theBorderColor = AddPalShift(theBorderColor);
#endif

	int frameSize = 1;

   if (scale) {
		BMWidth = theWidth * xRes / LOWRESX;
		BMHeight = theHeight * yRes / LOWRESY;
	} else {  
		BMWidth = theWidth;
		BMHeight = theHeight;
   }
	skip = theSkip;
   borderColor = theBorderColor;

   if (!theTitleText)
		msgMgr->Fatal(SrcLoc,Msg_NoWindowTitle);
   // calculate the title size
   SetFont(theTitleFont);
	int titleHeight = pointSize+1;
	// check title will fit in the given size (adjust for border)
   text = theTitleText;
	int titleWidth = TextWidth(0,10000) + 2;
   if (theBorderColor != -1) {
      titleWidth += 2;
      titleHeight += 2;
   }
	if (titleWidth > BMWidth)
	   msgMgr->Fatal(SrcLoc,Msg_WindowTitleTooBig);
	if (titleHeight > BMHeight)
		msgMgr->Fatal(SrcLoc,Msg_WindowTitleTooBig);
	title = theTitleText;
	titleSize = titleHeight;
	titleFont = theTitleFont;
	titleFore = theTitleFore;
	titleBack = theTitleBack;

	// Draw the title
	text = title;
	fore = titleFore;
	back = theBack;
	mode = TEJUSTCENTER;
	dim = False;

	// allocate memory for the bitmap
	BM.Get(MemBitmap,BMWidth*BMHeight+CELHEADERSIZE);

	// build the header for the BM
	BuildHeader(BM,BMWidth,BMHeight,skip,0,0,xRes,yRes);

	// fill the bitmap with the back ground color
	textBox.A.x = 0;
	textBox.A.y = 0;
	textBox.B.x = BMWidth - 1;
	textBox.B.y = BMHeight - 1;

	Erase(textBox,False);

	back = titleBack;
	textBox.B.y = titleSize - 1;
	Erase(textBox,False);

   if (theBorderColor != -1) {
	   // frame the title box
	   Frame(textBox,frameSize,borderColor,False);
	   // reduce the title box by the frame
	   textBox.Inset(frameSize+1,frameSize+1);
   }

	// put the text into the bitmap
	DrawTextBox();

	text = theText;
	back = theBack;
	fore = theFore;
	mode = theMode;
	textBox = theTextBox;
   if(scale)
		textBox.Scale(xRes,LOWRESX,yRes,LOWRESY);

	SetFont(theFont);

	// clip the textbox by the non title area
	SOL_Rect rect(0,titleSize-1,BMWidth-1,BMHeight-1);
	textBox.Clip(rect);

   if (theBorderColor != -1) {
	   // frame the main box
	   Frame(rect,1,borderColor,False);
   }

	if (textBox.Ylen() >= pointSize) {
		// put the text into the bitmap
		DrawTextBox();
	}

	// return the handle to the BM
	return BM;
}

void
FontMgr::DeleteFBM(MemID theBM)
{
	memMgr->Free(theBM);
}

void 
FontMgr::ResizeBitMap(SOL_Rect newRect,Bool scale)
{
	int frameSize = 1;

   if (scale)
		newRect.Scale(xRes,LOWRESX,yRes,LOWRESY);

	// Assume the textBox is what's left 
	// after the title area is removed.
	// Assume dim is False.
	if (!newRect.IsEmpty()) {
		// ReAlloc the memory 
		BMWidth = newRect.Xlen();
		BMHeight = newRect.Ylen();

		if (titleSize >= BMHeight)
			msgMgr->Fatal(SrcLoc,Msg_WindowTitleTooBig);
		memMgr->Realloc(BM,BMWidth*BMHeight+CELHEADERSIZE);

		// build the header for the BM
		BuildHeader(BM,BMWidth,BMHeight,skip,0,0,xRes,yRes);
	   CelHeader* celPtr = (CelHeader*) *BM;

		// fill the bitmap with the back ground color
		Erase(newRect,False);

		// Is there a title
		if (titleSize) {
			// Save the text info
			MemID saveText = text;
			ushort saveFont = font;
			ushort saveFore = fore;
			ushort saveBack = back;
			short saveMode = mode;

			// Draw the title
			textBox.A.x = 0;
			textBox.A.y = 0;
			textBox.B.x = newRect.Xlen() - 1;
			textBox.B.y = titleSize - 1;
			text = title;
			SetFont (titleFont);
			fore = titleFore;
			back = titleBack;
			mode = TEJUSTCENTER;
			dim = False;

			Erase(textBox,False);

	 if (borderColor != (ushort)-1) {
			   // frame the title box
			   Frame(textBox,frameSize,SOL_BLACK,False);
			   // reduce the title box by the frame
				textBox.Inset(frameSize+1,frameSize+1);
	 }

			// put the title text into the bitmap
			DrawTextBox();

			text = saveText;
			back = saveBack;
			fore = saveFore;
			mode = saveMode;
			SetFont(saveFont);
		}

		// The text box is whats left after the title area is removed
		textBox.A.x = 0;
		textBox.A.y = titleSize;
		textBox.B.x = newRect.Xlen() - 1;
		textBox.B.y = newRect.Ylen() - 1;

      if (borderColor != (ushort)-1) {
		   // frame the main box
		   Frame(textBox,1,SOL_BLACK,False);
		   // reduce the main box by the frame
		   textBox.Inset(frameSize+1,frameSize+1);
      }

		if (textBox.Ylen() >= pointSize) {
			// put the text into the bitmap
			DrawTextBox();
		}
	}
}

void
FontMgr::SetFont(int theFont)
{
	// Build the font table containing the font widths
	BuildFontTable(theFont);
}

int
FontMgr::StringWidth(MemID theString)
{
	text = theString;
	return ((TextWidth(0,10000) * LOWRESX + xRes - 1) / xRes);
}

void
FontMgr::TextSize(MemID theString,int maxSize,SOL_Rect& theRect,Bool scale)
{
	// makes the rect large enough to hold the text str

	theRect.A.x = 0;
	theRect.B.x = 0;
	theRect.A.y = 0;
	theRect.B.y = 0;

   maxSize = maxSize * xRes/LOWRESX;

#define DEFAULTWIDE     192
	// set the width
	//      if maxSize = 0 use DEFAULTWIDE
	//      if maxSize > 0 use maxSize
	//    if maxSize < 0 put all the text on one line
	text = theString;
   int oldfont = font;
	int currentfont = font;
   int oldcolor = fore;
	int currentcolor = fore;
   int oldmode = mode;
	int currentmode = mode;
	if (maxSize < 0) {
		// we don't want word wrap
      
		theRect.B.x = TextWidth(0,10000) - 1;
	}
	else {
		if (!maxSize) maxSize = DEFAULTWIDE * xRes / LOWRESX;
		theRect.B.x = maxSize - 1;
		// get a local pointer to text
		uchar* str = (uchar*) memMgr->GetAddr(text);

		int longest = 0;
		int start = 0;
		int end = 0;
		while (*str){
			int count = GetLongest(&end,theRect.B.x + 1);
			SetFont(currentfont);
			mode = currentmode;
	      fore = currentcolor;
			int length = TextWidth(start,count);
			if (length > longest)
			longest = length;
			currentfont = font;
			currentmode = mode;
	      currentcolor = fore;
			start = end;
			str = (uchar*) memMgr->GetAddr(text) + start;
			theRect.B.y += pointSize;
		}
		theRect.B.y -= 1;
		if (maxSize > longest) theRect.B.x = longest - 1;
	}
   if (scale) {
      theRect.A.x = theRect.A.x * LOWRESX / xRes;
      theRect.A.y = theRect.A.y * LOWRESY / yRes;
      theRect.B.x = (theRect.B.x * LOWRESX + xRes - 1) / xRes;
      theRect.B.y = (theRect.B.y * LOWRESY + yRes - 1) / yRes;
   }
   //theRect.Scale(LOWRESX,xRes,LOWRESY,yRes);
	SetFont(oldfont);
	fore = oldcolor;
   mode = oldmode;
}

int
FontMgr::TextCount(MemID theText,int theStart,SOL_Rect& rect,Bool scale)
{
	// The font manager already has the font
	return TextCount(theText,theStart,font,rect,scale);
}

int
FontMgr::TextCount(MemID theText,int theStart,int theFont,SOL_Rect& rect,Bool scale)
{
	// returns the number of characters that will fit into a        rectangle
	// starting at the given character of the text string.

	int count;

	SOL_Rect newRect = rect;
	if (scale)
		newRect.Scale(xRes,LOWRESX,yRes,LOWRESY);

	SetFont(theFont);
	// Save the original text
	MemID   oldText = text;
	text = theText;
	int maxPixels = newRect.Xlen();
   //if (borderColor != (ushort)-1) maxPixels -= 4;
	// Adjust for frame
	int vRun =  (newRect.Ylen()-2)/pointSize;
	int strOffset = theStart;

	for (;vRun>0;--vRun) {
		GetLongest(&strOffset,maxPixels);
	}
	count = strOffset - theStart;

	text = oldText;

	return count;
}

void
FontMgr::Erase(SOL_Rect& theRect, Bool scale)
{
	// Erase the given rectangle of the BM using the background color

	SOL_Rect eraseRect = theRect;
	if (scale)
		eraseRect.Scale(xRes,LOWRESX,yRes,LOWRESY);

	// Get the address of the BM
	CelHeader* fontBM;
	fontBM = (CelHeader*) memMgr->GetAddr(BM);
	int hRun = eraseRect.Xlen();
	int vRun = eraseRect.Ylen();

	// Is the erase rectangle the whole BM?
	if 
		(
		(hRun == BMWidth)               &&
		(vRun == BMHeight)
		) {
		uchar* drawHere = (uchar*) fontBM + fontBM->colorOffset;
		memset(drawHere,back,hRun*vRun);
	}
	else {
		uchar * drawHere = (uchar*) fontBM + fontBM->colorOffset + BMWidth*eraseRect.A.y + eraseRect.A.x;
		while (vRun--) {
			memset(drawHere,back,hRun);
			drawHere += BMWidth;
		}
	}
}


void
FontMgr::Frame(SOL_Rect& theRect,int thickness,uchar theColor,Bool scale)
{
	SOL_Rect frameRect = theRect;
	if(scale)
		frameRect.Scale(xRes,LOWRESX,yRes,LOWRESY);
	// If the thickness is too large reduce it.
	if (thickness*2 > frameRect.Ylen()) {
		thickness = frameRect.Ylen()/2;
	}
	if (thickness*2 > frameRect.Xlen()) {
		thickness = frameRect.Xlen()/2;
	}
	// Calculate the box address
	uchar* penAddr = (uchar*) memMgr->GetAddr(BM);
	penAddr +=      frameRect.A.y*BMWidth + frameRect.A.x + CELHEADERSIZE;
	// draw the top frame
	int vRun = thickness;
	int hRun = frameRect.Xlen();
	while(vRun--) {
		memset(penAddr,theColor,hRun);
		penAddr += BMWidth;
	}
	// draw the side frames
	vRun = frameRect.Ylen() - 2*thickness;
	hRun = thickness;
	int skip1 = frameRect.Xlen() - thickness;
	int skip2 = BMWidth - frameRect.Xlen() + thickness;
	while(vRun--) {
		memset(penAddr,theColor,hRun);
		penAddr += skip1;
		memset(penAddr,theColor,hRun);
		penAddr += skip2;
	}
	// draw the bottom frame
	vRun = thickness;
	hRun = frameRect.Xlen();
	while(vRun--) {
		memset(penAddr,theColor,hRun);
		penAddr += BMWidth;
	}
}

void
KInvertRect(argList)
{
	// Input
	//      arg1-----the bitmap memID
	//              arg2-----the rectangle x1
	//              arg3-----the rectangle y1
	//              arg4-----the rectangle x2
	//              arg5-----the rectangle y2
	//              arg6-----the bitmap width
	//              arg7-----the foreground color
	//              arg8-----the background color
	//              
	// Ouput
	//    acc------the number of cels in the object's view,loop

	SOL_Rect r(arg(2),arg(3),arg(4),arg(5));

	fontMgr->InvertRect(arg(1),
							  arg(6),
							  r,
							  arg(7),
							  arg(8));
}

void
FontMgr::InvertRect(MemID theBM,int theWidth,SOL_Rect theRect,char fore,char back,Bool scale)
{
	// Switches the fore ground and background 
	// colors for the given rectangles
   if (scale) {
	   theWidth = theWidth * xRes / LOWRESX;
	   theRect.Scale(xRes,LOWRESX,yRes,LOWRESY);
   }

	// Sanity check
	if ((theRect.B.y*theWidth + theRect.B.x + CELHEADERSIZE) >= memMgr->GetSize(theBM))
		msgMgr->Fatal(SrcLoc,Msg_WindowTextTooBig);

	// Calculate the box address
	char* penAddr = (char*) memMgr->GetAddr(theBM);
	penAddr +=      theRect.A.y*theWidth + theRect.A.x + CELHEADERSIZE;
	// Setup
	int vRun = theRect.Ylen();
	int hRun = theRect.Xlen();

	while(vRun--) {
		for (int i = 0;i < hRun;++i) {
			if (penAddr[i] == fore)
				penAddr[i] = back;
			else
				if (penAddr[i] == back)
					penAddr[i] = fore;
		}
		penAddr += theWidth;
	}
}

void
FontMgr::ClearChar(uchar theChar)
{
	// Clear the given character at the current pen location
   // MustRewrite for Imbedded font
	SOL_Rect theRect;

	theRect.A.y = penY;
	theRect.B.y = penY + pointSize;

	theRect.A.x = penX;
	theRect.B.x = penX + fontWidths[theChar];

	Erase(theRect,False);
}

void
FontMgr::BuildFontTable(int theFont)
{
	if (font == theFont)    return;
	font = theFont;
	// Build the table of font widths and initializes the font pointSize

	FontHeader * fontHeader;

#ifdef MACINTOSH

	MemID fontID = resMgr->Get(MemResFont,font);
	fontHeader = (FontHeader *) memMgr->GetAddr(fontID);

#else

	if(font == SYSTEMFONT) {
		fontHeader = (FontHeader *) systemFont;
	} else {
		MemID fontID = resMgr->Get(MemResFont,font);
		fontHeader = (FontHeader *) memMgr->GetAddr(fontID);
	}

#endif
	
	// initialize the font constants
	pointSize = fontHeader->pointSize;
	lowChar = fontHeader->lowChar;
	highChar = fontHeader->highChar;

	Int16* charRecsAddr = (Int16*) &fontHeader->charRecs;

	// fill in the font widths table
	for (int i = 0;i < lowChar;++i) {
		fontWidths[i] = 0;
	}
	for (i = highChar+1;i < MAXFONTSIZE;++i) {
		fontWidths[i] = 0;
	}
	for (i = lowChar;i <= highChar;++i) {
		fontWidths[i] = *((uchar *) fontHeader + charRecsAddr[i]);
	}
}

ushort
FontMgr::TextWidth(int index,int count)
{
	// Adds the widths of the characters in the given string
	// from the character str[index] for a length of count up
	// to the end of the string
	
	// Set up to add the widths
	int width = 0;
	uchar* str = (uchar*) memMgr->GetAddr(text) + index;
	while (count-- > 0 && (*str != 0)) {
		if(*str == CTRL_CHAR) {
			int newNum;
			newNum = 0;
			str++;
			if(!(count-- > 0)) break;

			if((*str == 'c' || *str == 'a' || *str == 'f') &&  count-- > 0) {
		int action = *str;
				str++;
				while(*str >= '0' && *str <= '9' && count) {
					newNum *= 10;
					newNum += *str++ - '0';
	       count--;
				}
	    if(count > 0)
				   switch(action) {
					   case 'c':
						   fore = newNum;
						   break;
					   case 'a': 
						   mode = newNum;
						   break;
					   case 'f':
						   SetFont(newNum);
						   break;
				   }
			}
	 
			while(count > 0 && *str != CTRL_CHAR) {
				count--;
				str++;
			}

		} else 
			width += fontWidths[*str];
      str++;
	}
	return width;
}

ushort
FontMgr::TextWidth(MemID theText,int first,int count)
{
	// Adds the widths of the characters in the given string
	// from the character str[index] for a length of count up
	// to the end of the string
	text = theText;
	return ((TextWidth(first,count) * LOWRESX + xRes - 1) / xRes);
}

uchar 
FontMgr::CharWidth(uchar theChar,Bool scale)
{
	// Determine the given character's width using the current font
	if(scale)
		return (fontWidths[theChar] * LOWRESX + xRes - 1) / xRes;

	return fontWidths[theChar];
}

uchar
FontMgr::CharHeight(uchar theChar,Bool scale)
{
	// Determine the given character's height using the current font
			
	FontHeader * fontHeader;

#ifdef MACINTOSH

	MemID fontID = resMgr->Get(MemResFont,font);
	fontHeader = (FontHeader *) memMgr->GetAddr(fontID);

#else

	if(font == SYSTEMFONT) {
		fontHeader = (FontHeader *) systemFont;
	} else {
		MemID fontID = resMgr->Get(MemResFont,font);
		fontHeader = (FontHeader *) memMgr->GetAddr(fontID);
	}

#endif

	Int16* charRecsAddr = (Int16*) &fontHeader->charRecs;
	uchar height = *((uchar*) fontHeader + charRecsAddr[theChar] +1);

	if(scale)
		return (height * LOWRESY + yRes - 1) / yRes;

	return height;
}

void
FontMgr::DrawTextBox()
{
	// Put the text to the textBox in mode requested 

	int wide = textBox.Xlen();
	uchar* str = (uchar*) memMgr->GetAddr(text);
   if (!*str)
      return;

	// Set the penY into the textBox at the draw position
	penY = textBox.A.y;

	int strOffset = 0;
   if (!GetLongest(&strOffset,wide))
		msgMgr->Fatal(SrcLoc,Msg_WindowTextTooBig);
	strOffset = 0;

	int start = 0;
	while (*str) { 
		int oldfont = font;
		int oldcolor = fore;
		// Set the penX into the textBox at the draw position
		penX = textBox.A.x;
		//Getlongest assumes the string is text
		int drawCount = GetLongest(&strOffset,wide);
		SetFont(oldfont);
		fore = oldcolor;
		// drawCount is the count of characters to print
		// strOffset is where to resume printing from
		// GetLongest may pass over non-printable characters
		int length = TextWidth(start,drawCount); 
		SetFont(oldfont);
		fore = oldcolor;
	   // determine justification and draw the line
		switch (mode) {
			case TEJUSTCENTER:
				penX += (wide - length)/2;
				break;
			case TEJUSTRIGHT:
				penX += wide - length;
				break;
//                      case TEJUSTLEFT:
//                      default:
//                              penX += 0;
		}
		DrawText(start,drawCount);
		start = strOffset;
		str = (uchar*) memMgr->GetAddr(text) + strOffset;
		// put the penY down one line
		penY += pointSize;
   }
}

void
FontMgr::DrawText(int index,int count)
{
	uchar* str = (uchar*) memMgr->GetAddr(text);
	while(count-- > 0) {
		if(str[index] == CTRL_CHAR) {
			int newNum;
			newNum = 0;
			index++;
			if(!count--) break;

			if((str[index] == 'c' || str[index] == 'a' || str[index] == 'f') &&  count--) {
		int action = str[index];
				index++;
				while(str[index] >= '0' && str[index] <= '9' && count) {
					newNum *= 10;
					newNum += str[index++] - '0';
	       count--;
				}
	    if(count)
				   switch(action) {
					   case 'c':
						   fore = newNum;
						   break;
					   case 'a': 
						   mode = newNum;
						   break;
					   case 'f':
						   SetFont(newNum);
						   break;
				   }
			}
	 
			while(count > 0 && str[index] != CTRL_CHAR) {
				count--;
				index++;
			}
		} else 
			DrawChar(str[index]);
      index++;
	}
}

void
FontMgr::DownOneLine(uint drawCount,uint maxLines,int theFont)
{
	// Shift down the textBox bits one line
	uchar* bits = ((uchar*) *BM) + CELHEADERSIZE;
	uchar* target = bits + BMWidth*(textBox.A.y+maxLines*pointSize-1) + textBox.A.x;
	uchar* source = bits + BMWidth*(textBox.A.y+(maxLines-1)*pointSize-1) + textBox.A.x;
	uint vRun = (maxLines-1)*pointSize;
	uint hRun = textBox.Xlen();
	while (vRun--) {
		memcpy(target,source,hRun);
		target -= BMWidth;
		source -= BMWidth;
	}
	// Now put in the first line
	SOL_Rect r(textBox);
	r.B.y = r.A.y + pointSize - 1;
	Erase(r,False);
	penX = textBox.A.x;
	penY = textBox.A.y; 
	SetFont(theFont);
	// determine justification and draw the line
	int oldfont = font;
	int oldcolor = fore;
	int length = TextWidth(0,drawCount);
	int wide = textBox.Xlen();
	switch (mode) {
		case TEJUSTCENTER:
			penX += (wide - length)/2;
			break;
		case TEJUSTRIGHT:
			penX += wide - length;
			break;
   }
   SetFont(oldfont);
   fore = oldcolor;
	DrawText(0,drawCount);
}

void
FontMgr::UpOneLine(uint startDraw,uint drawCount,uint maxLines,int theFont)
{
	// Shift up the textBox bits one line
	uchar* bits = ((uchar*) *BM) + CELHEADERSIZE;
	uchar* target = bits + BMWidth*textBox.A.y + textBox.A.x;
	uchar* source = bits + BMWidth*(textBox.A.y+pointSize) + textBox.A.x;
	uint vRun = (maxLines-1)*pointSize;
	uint hRun = textBox.Xlen();
	while (vRun--) {
		memcpy(target,source,hRun);
		target += BMWidth;
		source += BMWidth;
	}
	// Now put in the last line
	SOL_Rect r(textBox);
	r.A.y += pointSize*(maxLines - 1);
	r.B.y += 1;
	Erase(r,False);
	penX = textBox.A.x;
	penY = textBox.A.y+(maxLines-1)*pointSize;
	SetFont(theFont);
	// determine justification and draw the line
	int oldfont = font;
	int oldcolor = fore;
	int length = TextWidth(startDraw,drawCount);
	int wide = textBox.Xlen();
	switch (mode) {
		case TEJUSTCENTER:
			penX += (wide - length)/2;
			break;
		case TEJUSTRIGHT:
			penX += wide - length;
			break;
   }
   SetFont(oldfont);
   fore = oldcolor;
	DrawText(startDraw,drawCount);
}

void
FontMgr::DrawChar(uchar theChar)
{
	// Draw a font character into the created BM at 
	// the given pen location with the given color

	// Address the character pattern
	FontHeader * fontHeader;

#ifdef MACINTOSH

	MemID fontID = resMgr->Get(MemResFont,font);
	fontHeader = (FontHeader *) memMgr->GetAddr(fontID);

#else

	if(font == SYSTEMFONT) {
		fontHeader = (FontHeader *) systemFont;
	} else {
		MemID fontID = resMgr->Get(MemResFont,font);
		fontHeader = (FontHeader *) memMgr->GetAddr(fontID);
	}

#endif

	// Check for a valid character
	if ((theChar >= highChar) || (theChar < lowChar))
		return;
	// Get the characters dimensions
	Int16* charRecsAddr = (Int16*) &fontHeader->charRecs;
	uchar * charAddr = (uchar*) fontHeader + charRecsAddr[theChar];
	int cWide =     *charAddr++;
	int cHigh =     *charAddr++;
	// charAddr now points to the characters line patterns

	// start drawing into the BM at:
	//      penY*width + penX + BMaddress
	uchar* penAddr = (uchar*) memMgr->GetAddr(BM);
	penAddr +=      penY*BMWidth + penX + CELHEADERSIZE;

	// the skip to the next line of the character
	int skip = BMWidth - cWide;

	int inCharPenY = penY;
	while (cHigh) {
		// Draw a line of pattern
		signed char mask = 0xff;
		if (dim) { 
			if (inCharPenY & 1)
				mask = 0xaa;
			else
				mask = 0x55;
		}
		// the current dot position in the character
		int dotPos = 0;
		// get the next byte of pattern
		signed char pattern = (signed char) *charAddr++ & mask;
		while (dotPos < cWide) {
			if (pattern < 0)
				*penAddr        = fore;
			penAddr++;
			dotPos++;
			// shift the dot out the left
			pattern = pattern << 1;

			// Following line prevents skiping a 
			// byte of pattern when cWide == 8
			if (dotPos == cWide) break;

			// check for byte reload
			if (!(dotPos & 7))
				pattern = (signed char) *charAddr++ & mask;

		}
		// a line of pattern done get to the next line in the BM
		penAddr += skip;
		++inCharPenY;
		--cHigh;
	}
	// put the pen in place for the next character
	penX += cWide;
}

ushort 
FontMgr::GetLongest(int* strOffset,int maxPixels)
{
	// return count of chars that fit in the given pixel length

	uchar c;
	int count = 0;
	int lastCount = 0;

	int first = *strOffset;
	uchar* str = (uchar*) memMgr->GetAddr(text) + first;

	int last = first;

	// find a HARD terminator or LAST SPACE that fits on line

#define LF                      0x0a
#define CR                      0x0d
#define SP                      0x20
	int oldFont = font;
	int oldcolor = fore;
	int oldmode = mode;
	int lastFont = font;
	int lastColor = fore;
	int lastMode = mode;
	while(True) {
		SetFont(oldFont);
		fore = oldcolor;
		mode = oldmode;
		c = *str;
		if (c == CR) {
			if (str[1] == LF)       {
				// so we don't see it later
				(*strOffset)++;
			}
			if (lastCount && (maxPixels < TextWidth(first,count))) {
				*strOffset = last;
				return(lastCount);
			} 
			else {
         	// verify the color and mode
				SetFont(oldFont);
				fore = oldcolor;
				mode = oldmode;
				TextWidth(first,count);
				// so we don't see it later
				(*strOffset)++;
				// caller sees end of string
				return(count);
			}
		}
		if (c == LF) {
			// by Corey for 68k
			if ((str[1] == CR) && (str[2] != LF))
				// so we don't see it later
				(*strOffset)++;
			if (lastCount && (maxPixels < TextWidth(first,count))) {
				*strOffset = last;
				return(lastCount);
			} 
			else {  
         	// verify the color and mode
				SetFont(oldFont);
				fore = oldcolor;
				mode = oldmode;
				TextWidth(first,count);
				// so we don't see it later
				(*strOffset)++;
				// caller sees end of string
				return(count);
			}
		}

		if (c == '\0') {
			if (lastCount && (maxPixels < TextWidth(first,count))) {
				*strOffset = last;
				return(lastCount);
			} 
			else {     
         	// verify the color and mode
				SetFont(oldFont);
				fore = oldcolor;
				mode = oldmode;
				TextWidth(first,count);
				// caller sees end of string
				return(count);
			}
		}


		if (c == SP) {
			// check word wrap
			if (maxPixels >= TextWidth(first,count)) {
				last = *strOffset;
				// so we don't see space again
				++last;
				lastCount = count;
				lastFont = font;
				lastColor = fore;
				lastMode = mode;
			} 
			else {
				*strOffset = last;
				// eliminate trailing spaces
				str = (uchar*) memMgr->GetAddr(text) + *strOffset;
				while (*str++ == ' ') {
					(*strOffset)++;
				}
				SetFont(lastFont);
				fore = lastColor;
				mode = lastMode;
				return(lastCount);
			}
		}

		// all is still cool
		++count;
		str++;
		(*strOffset)++;

		// we may never see a space to break on
		SetFont(oldFont); 
		fore = oldcolor;
		mode = oldmode;
		if (!lastCount &&  (TextWidth(first,count) > maxPixels)) {
			last += --count;
			*strOffset = last;
			break;
      }
	}
	return(count);
}

void
KTextSize(argList)

	// Input -
	//              arg(1) = ArrayID to array for rect
	//              arg(2) = TextID to string data
	//              arg(3) = font
	//              arg(4) = maximum width (0 is default of 192)
	//              arg(5) = Scale or not
	//
	//      Returns -
	//              Nothing.

{
	uint width = 0;
   Bool scale = True;
   if (argCount >=5)
      scale = arg(5);
	if (argCount >= 4)
		width = arg(4);
	SOL_Rect r(0,0,0,0);
	fontMgr->SetFont(arg(3));
	fontMgr->TextSize((MemID) arg(2),width,r,scale);

	SCIWord* data = (SCIWord*) ((ArrayID) arg(1)).Data();
	data[0] = r.A.x;
	data[1] = r.A.y;
	data[2] = r.B.x;
	data[3] = r.B.y;
}

void
KTextWidth(argList)
{
	// Input
	//      arg1-----the MemID
	//                arg2-----the font number

	MemID memID = arg(1);
	fontMgr->SetFont(arg(2));
	pm.acc = fontMgr->StringWidth(memID);
}

void
KPointSize(argList)
{
	// Input
	//                arg1-----the font number

	fontMgr->SetFont(arg(1));
	pm.acc = fontMgr->PointSize();
}

void
KSetFontRes(argList)
{ 
   fontMgr->SetRes(arg(1),arg(2));
}

// The font manager
FontMgr*        fontMgr;
