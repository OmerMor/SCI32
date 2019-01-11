#ifndef FONT_HPP
#define FONT_HPP


#ifndef CELOBJ_HPP
#include "celobj.hpp"
#endif

#ifndef RECT_HPP
#include "rect.hpp"
#endif

#ifndef GRAPHMGR_HPP
#include "graphmgr.hpp"
#endif

#ifndef MEMID_HPP
#include "memid.hpp"
#endif

// text justification
#define TEJUSTLEFT	0
#define TEJUSTCENTER 1
#define TEJUSTRIGHT  2

//	character that introduces a control sequence in text
const char CTRL_CHAR	= '|';

#define SYSTEMFONT ((ushort) -1)

class FontHeader
{
	public:
	Int16	lowChar;	
	Int16	highChar;	
	Int16	pointSize;
	Int16	charRecs;	
};

class FontMgr
{
	public:

	FontMgr(ushort theFont = SYSTEMFONT) {font = theFont+1;BuildFontTable(theFont);}

	MemID BuildFBM
		(
	   Bitmap&     bitmap,
	   SOL_Rect&   theTextBox,
	   MemID       theText,
	   int         theFore,
	   int         theBack,
	   int         theFont,
		int			theSkip,
		int			theBorderColor,
	   Bool        dimIt
		);

	MemID BuildFBM
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
		Bool			scale = True
		);


	MemID TitledFBM
		(
	   int 		   theWidth,		// main info
	   int 		   theHeight,
	   SOL_Rect&   theTextBox,
	   MemID 		theText,
	   int 		   theFore,
	   int 		   theBack,
      int         theSkip,
	   int 		   theFont,
	   int 		   theMode,
		int         theBorderColor,

	   MemID 		theTitleText,	// Title info
	   int 		   theTitleFore,
	   int 		   theTitleBack,
		int 		   theTitleFont,
		Bool			scale = True
		);

	uchar 			CharWidth(uchar theChar,Bool scale = True);
	uchar 			CharHeight(uchar theChar,Bool scale = True);
	void				DeleteFBM(MemID theBM);
	void				DownOneLine(uint drawCount,uint maxLines,int theFont);
	void				DrawTextBox();
	void				Erase(SOL_Rect&, Bool scale = True);
	void 				Frame(SOL_Rect& theRect,int thickness,uchar theColor, Bool scale = True);
	void				InvertRect(MemID theBM,int theWidth,SOL_Rect theRect,char fore,char back,Bool scale = True);
	ushort			PenX() {return penX;}
	ushort			PenY() {return penY;}
	int 				PointSize(Bool scale = True) {if (scale) return (pointSize * SCIRESY + yRes - 1) / yRes; return pointSize;}
	void 				ResizeBitMap(SOL_Rect newRect,Bool scale = True);
	int				StringWidth(MemID theString);
	void 				SetBM(MemID theBM) {BM =theBM;}
	void 				SetFont(int theFont);
	void 				SetColor(int theFore) {fore = theFore;}
	void 				SetMode(int theMode) {mode = theMode;}
	void				SetRes(int xres, int yres) {xRes = xres; yRes = yres;}
	int 				GetFont() const {return font;}
	int 				GetColor() const {return fore;}
	int 				GetMode() const {return mode;}
	int 				GetXRes() const {return xRes;}
	int 				GetYRes() const {return yRes;}
	void 				SetText(MemID theText) {text = theText;}
	int				TextCount(MemID theText,int theStart,int theFont,SOL_Rect& rect,Bool scale = True);
	int				TextCount(MemID theText,int theStart,SOL_Rect& rect,Bool scale = True);
	void				TextSize(MemID theString,int size,SOL_Rect& theRect,Bool scale=True);
	ushort			TextWidth(MemID text,int first,int count);
	void				UpOneLine(uint startDraw,uint drawCount,uint maxLines,int theFont);

	protected:

	ushort 	BMWidth;		// the width of the bitmap
	ushort 	BMHeight;	// the height of the bitmap

	ushort 	fore;			// text fore ground color
	ushort 	back;			// text back ground color
	ushort 	titleFore;	// title fore ground color
	ushort 	titleBack;	// title back ground color
	// if the foreground or background color is the
	// same as the skip color then it will be skip
	ushort 	skip;			// bitmap skip color

	MemID	  	text;			// the text associated with the fontBox
	SOL_Rect	textBox;		// the inset rectangle for drawing text

	MemID    title;		// the title if any
	ushort	titleSize;	// the title height

	ushort	font; 		// the default font
	ushort	titleFont; 	// the title font

   ushort   borderColor;// the border color (-1 indicates no border)
	Bitmap	bitmap;		// bitmap for the background
	ushort	dim;			// true if text to be dimmed
	short		mode;			// the alignment mode

	// calculated properties

	ushort	penX;			// pen horizontal location relative to the bitmap
	ushort	penY;			// pen vertical location relative to the bitmap
	ushort	pointSize;	// the font verticle point size
	MemID    BM;			// the location of the bit map

	// the font info
	int		lowChar;		// the first valid character
	int		highChar;	// the last valid character

	static int xRes;     // current resolution of font bitmaps
	static int yRes;     // current resolution of font bitmaps

	// the font widths for the current font
	#define MAXFONTSIZE 256
	uchar		fontWidths[MAXFONTSIZE];

	void BuildFontTable(int theFont);

	void ClearChar(uchar);

	void DrawText(int index,int count);

	void DrawChar(uchar);

	ushort TextWidth(int first,int count);
	ushort GetLongest(int* strOffset,int maxPixels);
};

extern FontMgr*	fontMgr;
extern uchar 		sciSystemFont[];

#endif  //FONT_HPP

