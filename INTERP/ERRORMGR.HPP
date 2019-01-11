#ifndef ERRORMGR_HPP
#define ERRORMGR_HPP

#ifndef RECT_HPP
#include "rect.hpp"
#endif

#ifndef FONT_HPP
#include "font.hpp"
#endif


#define MAXAREA 20000

class ErrorMgr
{
	public:

	ErrorMgr() {BuildFontTable();}

	void           Display(char* msg, char* title = 0);
	void           Clear();

	protected:

	uchar* BuildFBM
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
		);

   uchar* TitledFBM
	   (
	   int 		   theWidth,	 
	   int 		   theHeight,
	   SOL_Rect&   theTextBox,
	   char* 		theText,
	   int 		   theFore,
	   int 		   theBack,
      int         theSkip,
	   int 		   theMode,
      int         theBorderColor,
	   char* 		theTitleText  
	   );

	unsigned char  CharWidth(char theChar);
	unsigned char  CharHeight(char theChar);
	void           DownOneLine(uint drawCount,uint maxLines);
	void           DrawTextBox();
	void           Erase(SOL_Rect&);
	void           Frame(SOL_Rect& theRect,int thickness,char theColor);
	int            PointSize() {return pointSize;}
	int            StringWidth(char* theString);
	void           SetText(char* theText) {text = theText;}
	int            TextCount(char* theText,int theStart,SOL_Rect& rect);
	ushort         TextWidth(char* text,int first,int count);
	void           UpOneLine(uint startDraw,uint drawCount,uint maxLines);
	void           TextSize(char* theString,int maxSize,SOL_Rect& theRect);

	ushort   BMWidth;    // the width of the bitmap
	ushort   BMHeight;   // the height of the bitmap

	ushort   fore;       // text fore ground color
	ushort   back;       // text back ground color
	ushort   titleFore;  // title fore ground color
	ushort   titleBack;  // title back ground color
	// if the foreground or background color is the
	// same as the skip color then it will be skip
	ushort   skip;       // bitmap skip color

	char*    text;       // the text associated with the fontBox
	SOL_Rect textBox;    // the inset rectangle for drawing text
	SOL_Rect windowRect;

	ushort   borderColor;// the border color (-1 indicates no border)
	ushort   dim;        // true if text to be dimmed
	short    mode;       // the alignment mode

	// calculated properties

	ushort   penX;       // pen horizontal location relative to the bitmap
	ushort   penY;       // pen vertical location relative to the bitmap
	ushort   pointSize;  // the font verticle point size
	static   uchar    bitmap[MAXAREA + CELHEADERSIZE];

	// the font info
	int      lowChar;    // the first valid character
	int      highChar;   // the last valid character
	// the font widths for the current font
	char     fontWidths[MAXFONTSIZE];

	void BuildFontTable();

	void DrawText(int index,int count);

	void DrawChar(char);

	ushort TextWidth(int first,int count);
	ushort GetLongest(int* strOffset,int maxPixels);
};

// The font manager  (declared in font.cpp)
extern ErrorMgr* errorMgr;

#endif  //ErrorMgr
