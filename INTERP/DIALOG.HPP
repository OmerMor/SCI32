#ifndef DIALOG_HPP
#define DIALOG_HPP

#include "sol.hpp"
#include "rect.hpp"
#include "event.hpp"

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

typedef ID<EditStruct, MemEditStruct> EditStructID;

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

#endif  //DIALOG_HPP
