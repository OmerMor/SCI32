#ifndef EDIT_HPP
#define EDIT_HPP

#include "sol.hpp"
#include "rect.hpp"
#include "event.hpp"
#include "textid.hpp"

class FontMgr;
class ScreenItem;
class Plane;

int			GetInput(char*,char*,int);
Bool			MsgBox(char* title, char* fmt, ...);

#ifndef NETWORK

class TextID;

class EditStruct
	{
	public:

	// This structure looks like an SCI array
	// so that SCI can get at the values
	MemID    BM;			// the location of the bit map
	int		BMWidth;    // the width of the entire bitmap
	MemID	  	text;			// the text associated with the fontBox
	SOL_Rect	textBox;		// the inset rectangle for drawing text
	int 		borderColor;// the borderColor (-1 indicates no border)
	ushort	x;				// the left of the BM relative to it's plane
	ushort	y;				// the top of the BM relative to it's plane
	long		planeID;		// the plane for the BM
	ushort 	fore;			// bitmap fore ground color
	ushort 	back;			// bitmap back ground color
	ushort 	skip;			// bitmap skip color
	ushort	font; 		// the default font
	ushort	cursor;		// pen horizontal location relative to the bitmap in characters
	ushort	cursorOn;	// TRUE if the cursor is on
	SOL_Rect cursorRect; //	The cursor rectangle
	ushort   maxLength;	// maximum string allowed
	};

FakeTemplateID(EditStructID, EditStruct, MemEditStruct)

void 			CenterWindow(SOL_Rect&);
void 			DrawCursor(MemID editStructID);
Bool 			EditText(SOL_Event& evt,MemID);
void 			EraseCursor(MemID editStructID);
void 			FlashCursor(MemID editStructID);
int			GetInput(char*,char*,int);
int         GetInputText(TextID,TextID,int,int);
void 			InvertRect(SOL_Rect&,MemID,int,char,char);
Bool			MsgBox(char* title, char* fmt, ...);
void 			SetFlash();
void 			UpdateFontMgr(MemID);

#else

class EditClass
{
	public:
	EditClass(short id);
	EditClass(TextID theTextID,TextID theTitleID,int boxWidth,int boxHeight = 1);
	virtual ~EditClass();

	void			Show(int pri = -1);
	void			Hide(Bool frameOut = True);
	Bool			HandleEvent();
	Bool 			EditText(SOL_Event& evt);
	void			UpdateFontMgr();
	void 			DrawCursor();
	void 			EraseCursor();
	void 			FlashCursor();
	void 			InvertRect(SOL_Rect&,int,char,char);
	void 			SetFlash();
	void			Highlight(int);
	
	// This structure looks like an SCI array
	// so that SCI can get at the values
	MemID    BM;			// the location of the bit map
	int		BMWidth;    // the width of the entire bitmap
	MemID	  	text;			// the text associated with the fontBox
	SOL_Rect	textBox;		// the inset rectangle for drawing text
	int 		borderColor;// the borderColor (-1 indicates no border)
	ushort	x;				// the left of the BM relative to it's plane
	ushort	y;				// the top of the BM relative to it's plane
//	int		planeID;		// the plane for the BM
	ushort 	fore;			// bitmap fore ground color
	ushort 	back;			// bitmap back ground color
	ushort 	hiColor;		// edit bitmap highlight color
	ushort 	skip;			// bitmap skip color
	ushort	font; 		// the default font
	MemID		lineIndicesID;// The indices into the text  
								// corresponding to a line start
	ushort	cursor;		// pen absolute location relative to the bitmap in characters
	ushort	cursorX;		// pen horizontal location relative to the bitmap in characters
	ushort	cursorY;		// pen vertical location relative to the bitmap in characters
	ushort	cursorOn;	// TRUE if the cursor is on
	SOL_Rect cursorRect; //	The cursor rectangle
	ushort   maxCharWidth;// maximum characters that fit in one line of edit box
	ushort	maxHeight;	// the height of edit box
	ushort	maxChars;	// maximum number of characters allowed
	long    	flash;
	short   	insertMode;
	short   	clearEditLine;  
	int 		xRes;
	int 		yRes; 
	TextID   editTextID;
	TextID   textID;
	TextID   title;
	Bool		changed;
	Bool		visible;
	ScreenItem* 	si;			// The sitem for the fontBitmap
	SOL_Rect			window;  	// the planes coordinates screen relative
	int				maxLines;
	Plane*			windowPlane;
	FontMgr*			fntMgr;		// The font manager for this edit item

	virtual void	GetLineIndices();
	void		SetCursor(int where = -1);
};

#endif // #ifndef NETWORK

#endif  //DIALOG_HPP

