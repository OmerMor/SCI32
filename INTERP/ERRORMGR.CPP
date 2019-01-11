
#include <string.h>
#include <stdio.h>

#include "sol.hpp"

#include "array.hpp"
#include "celobj.hpp"
#include "errormgr.hpp"
#include "kernel.hpp"
#include "msg.hpp"
#include "palette.hpp"
#include "resource.hpp"
#include "bufferd.hpp"
#include "graph.hpp"  
#include "graphmgr.hpp"
#include "video.hpp"
#include "object.hpp"

uchar ErrorMgr::bitmap [MAXAREA + CELHEADERSIZE];


uchar * 
ErrorMgr::BuildFBM
	(
	int         theWidth,
	int         theHeight,
	SOL_Rect&   theTextBox,
	char*       theText,
	int         theFore,
	int         theBack,
	int         theSkip,
	int         theMode,
	int         theBorderColor,
	Bool        dimIt
	)
{
	int frameSize = 1;

	borderColor = theBorderColor;

	BMWidth = theWidth;
	BMHeight = theHeight;
	textBox = theTextBox;
	text = theText;
	fore = theFore;
	back = theBack;
	skip = theSkip;
	mode = theMode;
	dim = dimIt;

	// clip the textbox by the FBM
	SOL_Rect rect(0,0,BMWidth-1,BMHeight-1);
	textBox.Clip(rect);

	// build the header for the BM
	BuildHeader(bitmap,BMWidth,BMHeight,skip);

	// fill the bitmap with the back ground color
	Erase(rect);

	if (theBorderColor != -1) {
		// frame the box
		Frame(rect,frameSize,theBorderColor);
	   textBox.Inset(frameSize+1,frameSize+1);
	}

	// put the text into the bitmap
	DrawTextBox();
	
	// return the handle to the BM
	return bitmap;
}


uchar* 
ErrorMgr::TitledFBM
	(
	int 		   theWidth,		// main info
	int 		   theHeight,
	SOL_Rect&   theTextBox,
	char* 		theText,
	int 		   theFore,
	int 		   theBack,
   int         theSkip,
	int 		   theMode,
   int         theBorderColor,

	char* 		theTitleText	// Title info
	)
{
	int frameSize = 1;

	BMWidth = theWidth;
	BMHeight = theHeight;
	skip = theSkip;
   borderColor = theBorderColor;

   // calculate the title size
	int titleHeight = PointSize()+1;
   if (theBorderColor != -1) {
      titleHeight += 2;
   }

	// Draw the title
	text = theTitleText;
	back = theBack;
	fore = theFore;
	mode = TEJUSTCENTER;
	dim = False;

	// build the header for the BM
	BuildHeader(bitmap,BMWidth,BMHeight,skip);

	// fill the bitmap with the back ground color
	textBox.A.x = 0;
	textBox.A.y = 0;
	textBox.B.x = BMWidth - 1;
	textBox.B.y = BMHeight - 1;

	Erase(textBox);

	back = titleBack;
	textBox.B.y = titleHeight - 1;

   if (theBorderColor != -1) {
	   // frame the title box
	   Frame(textBox,frameSize,borderColor);
	   // reduce the title box by the frame
	   textBox.Inset(frameSize+1,frameSize+1);
   }

	// put the text into the bitmap
	DrawTextBox();

	text = theText;
	mode = theMode;
	textBox = theTextBox;

	// clip the textbox by the non title area
	SOL_Rect rect(0,titleHeight-1,BMWidth-1,BMHeight-1);
	textBox.Clip(rect);

   if (theBorderColor != -1) {
	   // frame the main box
	   Frame(textBox,1,borderColor);
	   // reduce the main box by the frame
	   textBox.Inset(frameSize+1,frameSize+1);
   }

	if (textBox.Ylen() >= pointSize) {
		// put the text into the bitmap
		DrawTextBox();
	}

	// return the handle to the BM
	return bitmap;
}

int
ErrorMgr::StringWidth(char* theString)
{
	text = theString;
	return (TextWidth(0,10000));
}


int
ErrorMgr::TextCount(char* theText,int theStart,SOL_Rect& rect)
{
	// returns the number of characters that will fit into a rectangle
	// starting at the given character of the text string.

	int count;

	char* oldText = text;
	text = theText;
	int maxPixels = rect.Xlen() - 4;
	// Adjust for frame
	int vRun =  (rect.Ylen()-2)/PointSize();
	int strOffset = theStart;

	for (;vRun>0;--vRun) {
		GetLongest(&strOffset,maxPixels);
	}
	count = strOffset - theStart;

	text = oldText;

	return count;
}

void
ErrorMgr::Erase(SOL_Rect& theRect)
{
	// Erase the given rectangle of the BM using the background color

	// Get the address of the BM
	CelHeader* fontBM;
	fontBM = (CelHeader*) bitmap;
	int hRun = theRect.Xlen();
	int vRun = theRect.Ylen();

	// Is the erase rectangle the whole BM?
	if 
		(
		(hRun == BMWidth)    &&
		(vRun == BMHeight)
		) {
		char * drawHere = (char * )fontBM + fontBM->colorOffset;
		memset(drawHere,back,hRun*vRun);
	}
	else {
		char * drawHere = (char * )fontBM + fontBM->colorOffset + BMWidth*theRect.A.y + theRect.A.x;
		while (vRun--) {
			memset(drawHere,back,hRun);
			drawHere += BMWidth;
		}
	}
}


void
ErrorMgr::Frame(SOL_Rect& theRect,int thickness,char theColor)
{
	// If the thickness is too large reduce it.
	if (thickness*2 > theRect.Ylen()) {
		thickness = theRect.Ylen()/2;
	}
	if (thickness*2 > theRect.Xlen()) {
		thickness = theRect.Xlen()/2;
	}
	// Calculate the box address
	char* penAddr = (char *)bitmap;
	penAddr +=  theRect.A.y*BMWidth + theRect.A.x + CELHEADERSIZE;
	// draw the top frame
	int vRun = thickness;
	int hRun = theRect.Xlen();
	while(vRun--) {
		memset(penAddr,theColor,hRun);
		penAddr += BMWidth;
	}
	// draw the side frames
	vRun = theRect.Ylen() - 2*thickness;
	hRun = thickness;
	int skip1 = theRect.Xlen() - thickness;
	int skip2 = BMWidth - theRect.Xlen() + thickness;
	while(vRun--) {
		memset(penAddr,theColor,hRun);
		penAddr += skip1;
		memset(penAddr,theColor,hRun);
		penAddr += skip2;
	}
	// draw the bottom frame
	vRun = thickness;
	hRun = theRect.Xlen();
	while(vRun--) {
		memset(penAddr,theColor,hRun);
		penAddr += BMWidth;
	}
}

void
ErrorMgr::BuildFontTable()
{
	// Build the table of font widths and initializes the font pointSize

	FontHeader * fontHeader;
	fontHeader = (FontHeader *) sciSystemFont;
	
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
		fontWidths[i] = *((char *) fontHeader + charRecsAddr[i]);
	}
}

ushort
ErrorMgr::TextWidth(int index,int count)
{
	// Adds the widths of the characters in the given string
	// from the character str[index] for a length of count up
	// to the end of the string
	
	// Set up to add the widths
	int width = 0;
	char* str = (char*) text + index;
	while (count-- && (*str != 0)) {
		width += fontWidths[*str++];
	}
	return width;
}

ushort
ErrorMgr::TextWidth(char* theText,int first,int count)
{
	// Adds the widths of the characters in the given string
	// from the character str[index] for a length of count up
	// to the end of the string
	text = theText;
	return (TextWidth(first,count));
}

unsigned char 
ErrorMgr::CharWidth(char theChar)
{
	// Determine the given character's width using the current font
	return fontWidths[theChar];
}

unsigned char
ErrorMgr::CharHeight(char theChar)
{
	// Determine the given character's height using the current font
			
	FontHeader * fontHeader;
	fontHeader = (FontHeader *) sciSystemFont;
	Int16* charRecsAddr = (Int16*) &fontHeader->charRecs;
	unsigned char height = *((unsigned char *) fontHeader + charRecsAddr[theChar] +1);

	return height;
}

void
ErrorMgr::DrawTextBox()
{
	// put the text to the textBox in mode requested 
	int start = 0;
	int strOffset = 0;
	char* str;
	int length,wide,drawCount;

	// Set the penY into the textBox at the draw position
	penY = textBox.A.y;

	wide = textBox.Xlen();
	str = text;
	while (*str) {
		// Set the penX into the textBox at the draw position
		penX = textBox.A.x;
		//Getlongest assumes the string is text
		drawCount = GetLongest(&strOffset,wide);
		// drawCount is the count of characters to print
		// strOffset is where to resume printing from
		// GetLongest may pass over non-printable characters
		length = TextWidth(start,drawCount);
		// determine justification and draw the line
		switch (mode) {
			case TEJUSTCENTER:
				penX += (wide - length)/2;
				break;
			case TEJUSTRIGHT:
				penX += wide - length;
				break;
//       case TEJUSTLEFT:
//       default:
//          penX += 0;
		}
		DrawText(start,drawCount);
		start = strOffset;
		str = text + strOffset;
		// put the penY down one line
		penY += pointSize;
	}
}

void
ErrorMgr::DrawText(int index,int count)
{
	while(count-- > 0)
		{
		char* str = text;
		DrawChar(str[index++]);
		}
}

void
ErrorMgr::DownOneLine(uint drawCount,uint maxLines)
{
	// Shift down the textBox bits one line
	char* bits = ((char*) bitmap) + CELHEADERSIZE;
	char* target = bits + BMWidth*(textBox.A.y+maxLines*pointSize-1) + textBox.A.x;
	char* source = bits + BMWidth*(textBox.A.y+(maxLines-1)*pointSize-1) + textBox.A.x;
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
	Erase(r);
	penX = textBox.A.x;
	penY = textBox.A.y;
	DrawText(0,drawCount);
}

void
ErrorMgr::UpOneLine(uint startDraw,uint drawCount,uint maxLines)
{
	// Shift up the textBox bits one line
	char* bits = ((char*) bitmap) + CELHEADERSIZE;
	char* target = bits + BMWidth*textBox.A.y + textBox.A.x;
	char* source = bits + BMWidth*(textBox.A.y+pointSize) + textBox.A.x;
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
	Erase(r);
	penX = textBox.A.x;
	penY = textBox.A.y+(maxLines-1)*pointSize;
	DrawText(startDraw,drawCount);
}

void
ErrorMgr::DrawChar(char theChar)
{
	// Draw a font character into the created BM at 
	// the given pen location with the given color

	// Address the character pattern
	FontHeader * fontHeader;
	fontHeader = (FontHeader *) sciSystemFont;

	// Check for a valid character
	if ((theChar >= highChar) || (theChar < lowChar))
		return;
	// Get the characters dimensions
	Int16* charRecsAddr = (Int16*) &fontHeader->charRecs;
	char * charAddr = (char *) fontHeader + charRecsAddr[theChar];
	int cWide = *charAddr++;
	int cHigh = *charAddr++;
	// charAddr now points to the characters line patterns

	// start drawing into the BM at:
	//    penY*width + penX + BMaddress
	char* penAddr = (char*) bitmap;
	penAddr +=  penY*BMWidth + penX + CELHEADERSIZE;

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
				*penAddr = fore;
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
ErrorMgr::GetLongest(int* strOffset,int maxPixels)
{
	// return count of chars that fit in the given pixel length

	char c;
	int count = 0;
	int lastCount = 0;

	int first = *strOffset;
	char* str = text + first;

	int last = first;

	// find a HARD terminator or LAST SPACE that fits on line

#define  LF       0x0a
#define  CR       0x0d
#define  SP       0x20
	while(True) {
		c = *str;
		if (c == CR) {
			if (str[1] == LF) {
				// so we don't see it later
				(*strOffset)++;
			}
			if (lastCount && (maxPixels < TextWidth(first,count))) {
				*strOffset = last;
				return(lastCount);
			} 
			else {
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
			} 
			else {
				*strOffset = last;
				// eliminate trailing spaces
				str = text + *strOffset;
				while (*str++ == ' ') {
					(*strOffset)++;
				}
				return(lastCount);
			}
		}

		// all is still cool
		++count;
		str++;
		(*strOffset)++;

		// we may never see a space to break on
		if (!lastCount &&  (TextWidth(first,count) > maxPixels)) {
			last += --count;
			*strOffset = last;
			break;
//       return(count);
		}
	}
	return(count);
} 

void
ErrorMgr::TextSize(char* theString,int maxSize,SOL_Rect& theRect)
{
	// makes the rect large enough to hold the text str

	theRect.A.x = 0;
	theRect.A.y = 0;
	theRect.B.y = 0;

#define DEFAULTWIDE  192
	// set the width
	//    if maxSize = 0 use DEFAULTWIDE
	//    if maxSize > 0 use maxSize
	//    if maxSize < 0 put all the text on one line
	text = theString;
	if (maxSize < 0) {
		// we don't want word wrap
		theRect.B.x = StringWidth(text);
	}
	else {
		if (!maxSize) maxSize = DEFAULTWIDE;
		theRect.B.x = maxSize;
		// get a local pointer to text
		char* str = text;

		int longest = 0;
		int start = 0;
		int end = 0;
		while (*str){
			int count = GetLongest(&end,theRect.B.x);
			int length = TextWidth(start,count);
			if (length > longest)
				longest = length;
			start = end;
			str = text + start;
			theRect.B.y += pointSize;
		}
		theRect.B.y -= 1;
		if (maxSize > longest) theRect.B.x = longest - 1;
	}

}

void
ErrorMgr::Display(char* msg, char* title)
{ 
	if (graphMgr) {
      int titleWidth = 0;

	   if (title && strlen(title))
         titleWidth = TextWidth(title, 0, strlen(title));

	   const int size = 150;
	   TextSize(msg,size - 4,windowRect);

      if(windowRect.B.x < titleWidth)
	      windowRect.B.x = titleWidth;

      if(titleWidth) // must be a title
	      windowRect.B.y += PointSize()+3;

	   // Adjust for a border
	   windowRect.B.x += 4;
	   windowRect.B.y += 4;

	   if(windowRect.Xlen() * windowRect.Ylen() > MAXAREA)
		   windowRect.B.y = MAXAREA/windowRect.Xlen();

	   SOL_Rect rect; 
      rect.A.x = 0;
	   rect.A.y = 0;
	   rect.B.x = windowRect.Xlen() - 1;
	   rect.B.y = windowRect.Ylen() - 1; 
     
      if(titleWidth)
	      rect.A.y = PointSize()+2;
   
	   SOL_Point pt((graphMgr->Xdim() - windowRect.Xlen()) / 2,
					   (graphMgr->Ydim() - windowRect.Ylen()) / 2);
	   // Make the windowRect screen relative
	   windowRect.Offset(pt.x,pt.y);

	   uchar* messageBox = titleWidth? //if title width the title bitmap

		   TitledFBM(windowRect.Xlen(), windowRect.Ylen(), rect, msg, SOL_BLACK,
								   SOL_WHITE, 50, 0, SOL_BLACK, title):

		   BuildFBM(windowRect.Xlen(), windowRect.Ylen(), rect, msg, SOL_BLACK,
								   SOL_WHITE, 50, 0, SOL_BLACK, False);

	   // Draw the rect
	   WriteVideoRect(&windowRect, messageBox + CELHEADERSIZE, 0);
	} else
		puts(msg);
}

void
ErrorMgr::Clear()
{ 
	if(graphMgr)
		graphMgr->Repaint(windowRect);
}

// The font manager
ErrorMgr*   errorMgr;
