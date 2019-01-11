#include <string.h>
#include <stdarg.h>

#include "sol.hpp"

#include "array.hpp"
#include "config.hpp"
#include "dialog.hpp"
#include "event.hpp"
#include "font.hpp"
#include "errormgr.hpp"
#include "graphmgr.hpp"
#include "kernel.hpp"
#ifdef WINDOWS
#include "mbox.hpp"
#endif
#include "memmgr.hpp"
#include "msg.hpp"
#include "object.hpp"
#include "pmachine.hpp"
#include "selector.hpp"
#include "string.hpp"
#include "textid.hpp"
#include "time.hpp"
#include "window.hpp"
#include "cursor.hpp"
#include "line.hpp"

#ifndef NETWORK
static long    flash;
static short   insertMode = 1;
static short   clearEditLine;
#endif

void
KCreateTextBitmap(argList)
{
	// Create a bitmap containing text
	// Input
	// 	arg1-----function code
   //       FBMWithBitmap ----- Background is a bitmap
	// 	      arg2-----DText instance
   //          
   //       FBMWithoutBitmap -- Background is a solid color
	// 	      arg2-----Width of bitmap
	// 	      arg3-----Height of bitmap
	// 	      arg4-----DText instance
   //
   // Output
   //    Acc contains the MemID for the bitmap
   //

   enum FunctionCode
      {
      FBMWithoutBitmap,
      FBMWithBitmap
      };

   SOL_Rect rect;
	ObjectID DText = arg(4);

   switch (arg(1)) {
      case FBMWithoutBitmap:
			rect.A.x = DText.GetProperty(s_textLeft);
			rect.A.y = DText.GetProperty(s_textTop);
			rect.B.x = DText.GetProperty(s_textRight);
			rect.B.y = DText.GetProperty(s_textBottom);

         pm.acc = fontMgr->BuildFBM
            (arg(2),                                      //Width
             arg(3),                                      //Height
             rect,
				 DText.GetProperty(s_text),
				 DText.GetProperty(s_fore),
				 DText.GetProperty(s_back),
				 DText.GetProperty(s_skip),
				 DText.GetProperty(s_font),
				 DText.GetProperty(s_mode),
				 DText.GetProperty(s_borderColor),
				 DText.GetProperty(s_dimmed));
         break;
      case FBMWithBitmap:
         // Build bitmap
	      DText = arg(2);

			rect.A.x = DText.GetProperty(s_textLeft);
			rect.A.y = DText.GetProperty(s_textTop);
			rect.B.x = DText.GetProperty(s_textRight);
			rect.B.y = DText.GetProperty(s_textBottom);

			Bitmap bitmap(DText.GetProperty(s_view),
							  DText.GetProperty(s_loop),
							  DText.GetProperty(s_cel));
         pm.acc = fontMgr->BuildFBM
				(bitmap,
				 rect,
				 DText.GetProperty(s_text),
				 DText.GetProperty(s_fore),
				 DText.GetProperty(s_back),
				 DText.GetProperty(s_font),
				 DText.GetProperty(s_skip),
				 DText.GetProperty(s_borderColor),
				 DText.GetProperty(s_dimmed));
			break;
	}
}

#ifndef NETWORK

void
KEditText(argList)
{
	// call the kernel to edit an DEdit item
	// Input
	// 	arg1-----the DEdit instance
	
	SOL_Event	evt;
	int 			lastKey = 0;
	int			done = False;
	ObjectID    DEdit = (ObjectID) arg(1);
   Bool        changed = False;
	TextID      editTextID;
	TextID      textID;

   // Make a copy of the text
	textID = DEdit.GetProperty(s_text);
	editTextID.Add(textID);

	// build the edit structure
	EditStructID editStructID;
	editStructID.Get(sizeof(EditStruct),LOCKED);
	EditStruct* editStruct = *editStructID;
	editStruct->text = editTextID;
	editStruct->fore = DEdit.GetProperty(s_fore);
	editStruct->back = DEdit.GetProperty(s_back);
	editStruct->skip = DEdit.GetProperty(s_skip);
	editStruct->font = DEdit.GetProperty(s_font);
	editStruct->maxLength = DEdit.GetProperty(s_width);
	editStruct->BM = DEdit.GetProperty(s_bitmap);
	editStruct->cursor = 0;
	editStruct->cursorOn = 0;
	editStruct->borderColor = DEdit.GetProperty(s_borderColor);

	// Make sure the strings can handle maxLength
	memMgr->Realloc(editTextID,editStruct->maxLength +1);
	memMgr->Realloc(textID,editStruct->maxLength +1);

	// Calculate height for the title
   int titleHeight = 0;
	TextID title = DEdit.GetProperty(s_title);
	int titleFont = DEdit.GetProperty(s_titleFont);
	if ((int) title) {
	   fontMgr->SetFont(titleFont);
      titleHeight = fontMgr->PointSize() + 1;
      if (editStruct->borderColor != -1) titleHeight += 2;
   }

	// Calculate the dimensions for the BM and the textbox
	fontMgr->SetFont(editStruct->font);

	// The textHeight is just the pointSize + 1 for the cursor
	int textHeight = fontMgr->PointSize() + 1;
	// Assume the character 'M' is the widest character
	int textWidth = editStruct->maxLength*fontMgr->CharWidth('M') + 1;
   if (editStruct->borderColor != -1) {
		textWidth += 4;
		textHeight += 2;
	}

	int planeWidth =  textWidth;
	int planeHeight =  textHeight + titleHeight;

	SOL_Rect window(0,0,planeWidth-1,planeHeight-1);
	window.Offset(DEdit.GetProperty(s_x),DEdit.GetProperty(s_y));
   SOL_Point point(0,0);
	LocalGameToGlobal(DEdit.GetProperty(s_planeID),point);
	window.Offset(point.x,point.y);

	SOL_Rect insetRect(2,titleHeight + 2,planeWidth - 2,planeHeight - 2);
	editStruct->textBox = insetRect;		
	editStruct->BMWidth = planeWidth;		

   // If there is already a bitmap then assume it's still valid.
	// See if a bit map has been built
   if (!editStruct->BM) {
		// build a bitmap
		if ((int) title) {
			// Titled bitmap
			editStruct->BM = fontMgr->TitledFBM
									(
                           planeWidth,
									planeHeight,
                           insetRect,
									editStruct->text,
									editStruct->fore,
									editStruct->back,
									editStruct->skip,
									editStruct->font,
									DEdit.GetProperty(s_mode),
                           editStruct->borderColor,
                           title,
									DEdit.GetProperty(s_titleFore),
									DEdit.GetProperty(s_titleBack),
                           titleFont
									);
		}
		else {
			editStruct->BM = fontMgr->BuildFBM
									(
                           planeWidth,
									planeHeight,
                           insetRect,
									editStruct->text,
									editStruct->fore,
									editStruct->back,
									editStruct->skip,
									editStruct->font,
									DEdit.GetProperty(s_mode),
                           editStruct->borderColor,
									DEdit.GetProperty(s_dimmed)
									);
		}
	}

	// Put the cursor in
	DrawCursor(editStructID);

	// Show the edit dialog
   // ********************
   // Need a plane
	Plane* windowPlane = New Plane(window);
	// force no plane background drawing
// HOOK
// Since the bitmap may not fit the plane
//	windowPlane->SetPicNum(-3); 
	windowPlane->SetPicNum(-2); 
	int planeID = windowPlane->Id();
	graphMgr->AddPlane(windowPlane);

   // Need a screen item
	Bitmap bitmap(editStruct->BM);
	SOL_Point pt(0,0);
	ScaleInfo scaleInfo;
	ScreenItem* si = New ScreenItem(planeID,bitmap,pt,scaleInfo);

	graphMgr->AddScreenItem(si);
	graphMgr->FrameOut();
   // ********************

	// Clear the edit line on certain keystrokes
	clearEditLine = True;
	// This is the edit phase
	while (True) {
		eventMgr->Get(&evt,SOL_Event::All);
		switch(evt.type) {
			case SOL_Event::KeyDown:
         	lastKey = evt.message;
				switch (evt.message)	{
					case Kbd::Esc:
						done = True;
						break;
					case Kbd::Enter:
						done = True;
						break;
					case Kbd::Up:
						done = True;
						break;
					case Kbd::Down:
						done = True;
						break;
					case Kbd::Tab:
               	if (evt.modifiers == Kbd::Shift) lastKey = Kbd::Shift;
						done = True;
						break;
				}
				break;
			case SOL_Event::MouseDown:
				if (!window.Intersect(evt.where))
					done = True;
				break;
		}
		if (done) break;
		if (EditText(evt,editStructID)) changed = True;
		si->SetUpdateCount(graphMgr->ScreenCount());
		graphMgr->FrameOut();
	}

	graphMgr->DeletePlane(windowPlane);
	if (DEdit.GetProperty(s_frameOut))
	   graphMgr->FrameOut();
	memMgr->Free(editStruct->BM);
	memMgr->Free(editStructID);
   if (changed) {
	   StrTrim(*editTextID);
	   memMgr->Realloc(textID,strlen(*editTextID)+1);
	   strcpy(*textID,*editTextID);
   }
	editTextID.Free();

	eventMgr->Post(&evt);
   pm.acc = changed;
}

void
KInputText(argList)
{
   // Generic input text routine for SCI
	// Input
	// 	arg1-----MemID for the edit text
	// 	arg2-----MemID for the title text
	// 	arg3-----Width for input box

   // Output
   //    True-----Text modified
   //    False----Text not modified

   // The priority of this plane is less than interperter created planes
	int priority = graphMgr->TopSCIPlane() + 1;
   pm.acc = GetInputText((MemID) arg(1),(MemID) arg(2),arg(3),priority);

}

int
GetInput(char* theText,char* theTitle,int boxWidth)
{
	// Gets input from a titled font box
	// input
	//		theText is the default input string
	//		theTitle is the title string
	//		boxWidth is the width of the input box	in characters

	TextID theTextID;
	TextID theTitleID;

	// Make copies of the character strings
	theTextID.Add(theText);
	theTitleID.Add(theTitle);

   // The priority of this plane will be higher than SCI planes
	int priority = graphMgr->TopPlane();
   if (priority < PLANEPRIORITYSTART)
      priority = PLANEPRIORITYSTART;
   else
      ++priority;

	int retVal = GetInputText(theTextID,theTitleID,boxWidth,priority);

	if (retVal) strcpy(theText,*theTextID);

	theTextID.Free();
	theTitleID.Free();
	return retVal;
}

int
GetInputText(TextID theTextID,TextID theTitleID,int boxWidth,int priority)
{
	// Gets input from a titled font box
	// input
	//		theText is the default input string
	//		theTitle is the title string
	//		boxWidth is the width of the input box	in characters

	SOL_Event	evt;
	int 			retVal = False;
	int			done = False;

	if (boxWidth <= 0) return retVal;

	// Make sure the edit string can handle boxWidth
	memMgr->Realloc(theTextID,boxWidth +1);

	// Need to build an edit structure
	EditStructID editStructID;

	editStructID.Get(sizeof(EditStruct),LOCKED);
	EditStruct* editStruct = *editStructID;
	editStruct->text = theTextID;
	editStruct->fore = SOL_BLACK;			
	editStruct->back = SOL_WHITE;			
	editStruct->skip = SOL_SKIP;			
	editStruct->font = systemFont; 		
	editStruct->maxLength = boxWidth;
	editStruct->borderColor = SOL_BLACK;
	editStruct->cursor = 0;			
	editStruct->cursorOn = 0;			

	// set the font to calculate height and width for the BM
	fontMgr->SetFont(editStruct->font);

	// Calculate the dimensions for the BM and the textbox
	// Assume the character 'm' is the widest character
	int maxPixelLen = boxWidth * fontMgr->CharWidth('M');
	int titleWidth = fontMgr->TextWidth(theTitleID,0,strlen(*theTitleID));
	if (maxPixelLen < titleWidth) maxPixelLen = titleWidth;
	int fontHeight = fontMgr->PointSize();

	int planeWidth =  maxPixelLen+4;
	int planeHeight =  2*fontHeight+7;
	SOL_Rect insetRect(1,planeHeight/2 + 1,planeWidth-2,planeHeight-2);
	SOL_Rect window(0,0,planeWidth-1,planeHeight-1);
	CenterWindow(window);
	editStruct->textBox = insetRect;		
	editStruct->BMWidth = planeWidth;		
	// build a bitmap
	editStruct->BM = fontMgr->TitledFBM
							(
							planeWidth,
							planeHeight,
							insetRect,
							theTextID,
							SOL_BLACK,
							SOL_WHITE,
	                  SOL_SKIP,			
							systemFont,
							TEJUSTLEFT,
                     SOL_BLACK,

							theTitleID,
							SOL_WHITE,
							SOL_BLACK,
							systemFont
							);
	// Put the cursor in
	DrawCursor(editStructID);

	// Show the edit dialog

	Plane* windowPlane = New Plane(window);
	windowPlane->SetPriority(priority);
	// force no plane background drawing
// HOOK
// Since the bitmap may not fit the plane
//	windowPlane->SetPicNum(-3); 
	windowPlane->SetPicNum(-2); 
	int planeID = windowPlane->Id();
	graphMgr->AddPlane(windowPlane);

	Bitmap bitmap(editStruct->BM);
	SOL_Point pt(0,0);
	ScaleInfo scaleInfo;
	ScreenItem* si = New ScreenItem(planeID,bitmap,pt,scaleInfo);

	graphMgr->AddScreenItem(si);
	graphMgr->FrameOut();

	// Clear the edit line on certain keystrokes
	clearEditLine = True;
	// This is the edit phase
	while (True)
      {
		eventMgr->Get(&evt,SOL_Event::All);
		if (evt.type == SOL_Event::KeyDown)
         {
			switch (evt.message)
            {
				case Kbd::Esc:
					retVal = False;
					done = True;
					break;
				case Kbd::Enter:
					retVal = True;
					done = True;
					break;
				}
			}
			if (done) break;
			EditText(evt,editStructID);
			si->SetUpdateCount(graphMgr->ScreenCount());
			graphMgr->FrameOut();
		}

	graphMgr->DeletePlane(windowPlane);
	graphMgr->FrameOut();
	memMgr->Free(editStruct->BM);
	memMgr->Free(editStructID);
	StrTrim(*theTextID);
	return retVal;
}

Bool
EditText(SOL_Event& evt,MemID editStructID)
{
	SOL_Point mousept;
	int	msg;

	UpdateFontMgr(editStructID);

	EditStruct* editStruct = (EditStruct*) *editStructID;
	char* str = (char *) memMgr->GetAddr(editStruct->text);

	int textLen = strlen(str);
	int changed = False;
	int deleteChar = False;
	int oldCursor = editStruct->cursor;

	switch(evt.type) {
		case SOL_Event::KeyDown:
			switch (msg = evt.message) {
				case Kbd::Home:
					// beginning of line
					clearEditLine = False;
					editStruct->cursor = 0;			
					break;

				case Kbd::End:
					// end of line
					clearEditLine = False;
					editStruct->cursor = textLen;			
					break;

				case CtrlKey('C'):	
					// control C
					// clear line
					editStruct->cursor = 0;			
					*str = 0;
					changed = True;
					break;
					
				case Kbd::BackSpace:
					// destructive backspace
					clearEditLine = False;
					deleteChar = True;
					if (editStruct->cursor)	--editStruct->cursor;			
					break;

				case Kbd::Left:
					// non-destructive backspace
					clearEditLine = False;
					if (editStruct->cursor)	--editStruct->cursor;			
					break;
		
				case Kbd::Insert:
					// toggle insert mode
					clearEditLine = False;
					if (insertMode)
						insertMode = 0;
					else
						insertMode = 1;
					changed = True;
					break;
		
				case Kbd::Del:
					// delete at cursor
					clearEditLine = False;
					if (editStruct->cursor != textLen)
						deleteChar = True;
					break;
		
				case Kbd::Right:
					clearEditLine = False;
					if (editStruct->cursor < textLen)
						++editStruct->cursor;
					break;

				default:
					if ((msg >= ' ') && (msg < 257)) {
						// If this is the first key, clear the input
						if (clearEditLine) {
						clearEditLine = False;
						*str = 0;
						textLen = 0;
						}
						// insert this key and advance cursor
						// if we have room in buffer AND we won't try to
						// draw outside of our enclosing rectangle
						if (
							(
							(!insertMode)  &&	
							(editStruct->cursor < editStruct->maxLength)
							)

							||

							(
							(textLen < editStruct->maxLength) &&
							(
							(fontMgr->CharWidth((char) msg) + fontMgr->StringWidth(editStruct->text)) 
							< 
							(editStruct->textBox.Xlen())
							)
							)
							) {
							changed = True;
							if (insertMode) {
								// shift it up one
								for(int i = textLen;i >= editStruct->cursor;i--)
									*(str + i + 1) = *(str + i);
							}
							else {
								if (editStruct->cursor == textLen)
									*(str + textLen + 1) = 0;
							}
							*(str + editStruct->cursor) = (char) msg;
							++(editStruct->cursor);
						}
					}
					break;
				}

			// if delete, we delete the character at cursor
			if (deleteChar) {
				changed = True;
				// collapse the string from cursor on
				for (int i = editStruct->cursor;i < textLen;i++)
					*(str + i) = *(str + i + 1);
				}
			break;

		case SOL_Event::MouseDown:
			// move cursor to closest character division
//			clearEditLine = False;
//			mousept.h = evt.where.h;
//			mousept.v = evt.where.v;
//			SOL_Rect rect = editStruct->textBox;
//			rect.Offset(editStruct->x,editStruct->y);
//
//			if (rect.Intersect(mousept.h,mousept.v))
//				for 
//					(
//					editStruct->cursor = textLen;
//					editStruct->cursor && (rect.A.x + fontMgr->TextWidth(text,0,cursor) - 1 > mousept.h) ;
//					--editStruct->cursor
//					);
			break; 												
		}

	if (changed) {
		// if we have changed we redraw the entire field in the text box
		EraseCursor(editStructID);
		// erase the text area of the FBM
		fontMgr->Erase(editStruct->textBox);
//      if (editStruct->borderColor != -1)
//			fontMgr->Frame(editStruct->textBox,1,editStruct->borderColor);
		fontMgr->DrawTextBox();
		DrawCursor(editStructID);
//		graphMgr->FrameOut();
	}
	else {
		if (oldCursor == editStruct->cursor) {
			// cursor is in the same place -- keep flashing
			FlashCursor(editStructID);
		}
		else {
			// cursor has moved -- ensure it is on at new position
			EraseCursor(editStructID);
			DrawCursor(editStructID);
		}
	}
   return changed;
}


void
UpdateFontMgr(MemID editStructID)
{
	// Gets the fontMgr in sync with the editStruct
	EditStruct* editStruct = (EditStruct*) memMgr->GetAddr(editStructID);
//	char * str = (char *) memMgr->GetAddr(editStruct->text);
	fontMgr->SetBM(editStruct->BM);
	fontMgr->SetFont(editStruct->font);
}


void
DrawCursor(MemID editStructID)
{
	EditStruct* editStruct = (EditStruct*) memMgr->GetAddr(editStructID);
	char * str = (char *) memMgr->GetAddr(editStruct->text);

	int left;
	int frameSize = 3;
	if (editStruct->borderColor == -1) frameSize = 0;

	if (!editStruct->cursorOn) {
		// The cursor box 
		if (editStruct->borderColor == -1) {
			left = editStruct->textBox.A.x+fontMgr->TextWidth(editStruct->text,0,editStruct->cursor);
			editStruct->cursorRect.A.x = left;
			if (insertMode) {
				editStruct->cursorRect.A.y = editStruct->textBox.A.y+fontMgr->PointSize()-1;
				editStruct->cursorRect.B.y = editStruct->cursorRect.A.y;
			}
			else {
				editStruct->cursorRect.A.y = editStruct->textBox.A.y;
				editStruct->cursorRect.B.y = editStruct->cursorRect.A.y+fontMgr->PointSize()-1;
			}
		}
		else {
			left = editStruct->textBox.A.x+fontMgr->TextWidth(editStruct->text,0,editStruct->cursor);
			editStruct->cursorRect.A.x = left;
			if (insertMode) {
				editStruct->cursorRect.A.y = editStruct->textBox.A.y+fontMgr->PointSize()-1;
				editStruct->cursorRect.B.y = editStruct->cursorRect.A.y;
			}
			else {
				editStruct->cursorRect.A.y = editStruct->textBox.A.y;
				editStruct->cursorRect.B.y = editStruct->cursorRect.A.y+fontMgr->PointSize()-1;
			}
		}

		// we flash the character cel unless we are at end of string
		str += editStruct->cursor;
		if (*str) {
			editStruct->cursorRect.B.x = left + fontMgr->CharWidth(*str) - 1;
		}
		else {
			editStruct->cursorRect.B.x = left;
			editStruct->cursorRect.B.x = left + fontMgr->CharWidth(' ') - 1;
		}
		// toggle cursor
		fontMgr->InvertRect(
					  editStruct->BM,
					  editStruct->BMWidth,
					  editStruct->cursorRect,
					  editStruct->fore,
					  editStruct->back);
	}
	editStruct->cursorOn = True;
	SetFlash();
}


void
EraseCursor(MemID editStructID)
{
	EditStruct* editStruct = (EditStruct*) memMgr->GetAddr(editStructID);

	if (editStruct->cursorOn)
		fontMgr->InvertRect(
					  editStruct->BM,
					  editStruct->BMWidth,
					  editStruct->cursorRect,
					  editStruct->fore,
					  editStruct->back);
	editStruct->cursorOn = False;
	SetFlash();
}


void
FlashCursor(MemID editStructID)
{
	EditStruct* editStruct = (EditStruct*) memMgr->GetAddr(editStructID);

	if (flash < timeMgr->GetTickCount()) {
		fontMgr->InvertRect(
					  editStruct->BM,
					  editStruct->BMWidth,
					  editStruct->cursorRect,
					  editStruct->fore,
					  editStruct->back);
		editStruct->cursorOn = !editStruct->cursorOn;
		SetFlash();
		}
}


void
SetFlash()
{
	flash = 30L + timeMgr->GetTickCount();
}

void
CenterWindow(SOL_Rect& window)
{
	// Centers a given rectangle to the screen
	// Assume window.A.x = window.A.y = 0
	
	int xOff = (320 -window.Xlen())/2;
	int yOff = (200 -window.Ylen())/2;
	window.Offset(xOff,yOff);
}

Bool
MsgBox(char* title, char* fmt, ...)
{
	char buf[MsgMgr::MaxStr];
	Bool escape;

	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

/*
	const width = 150;
	const x = (320 - width) / 2;
	const y = (200 - 10) / 2;
	DebugWindow(buf, title, width, SOL_Point(x, y), True, &escape);
*/

	if (graphMgr)
		graphMgr->GCursor().Hide();
	
#ifdef DEBUG	
	if (configMgr && configMgr->Get(configMgr->MonoEcho)) {
		msgMgr->Mono("{");
		msgMgr->Mono(title);
		msgMgr->Mono("}");
		msgMgr->Mono(buf);
		msgMgr->Mono("<Enter or Esc>");
	} else
#endif
#ifndef WINDOWS
		if (errorMgr && graphMgr)
			errorMgr->Display(buf,title); 
		else {
			puts("MsgBox called before errorMgr initialized:");
			puts(buf);
			exit(1);
		}
	escape = WaitOnUser();
#else
	escape = MBox2(buf,title);
#endif

#ifdef DEBUG
	if (configMgr && configMgr->Get(configMgr->MonoEcho))
		msgMgr->Mono("\n");
#endif

#ifndef WINDOWS
#ifdef DEBUG
	else
#endif
		errorMgr->Clear(); 
#endif

	if (graphMgr)
		graphMgr->GCursor().UnHide();

	return !escape;
}

void 
InvertRect(SOL_Rect& theRect,MemID BM,int BMWidth,char fore,char back)
{
	// Switches the fore ground and background colors
	// Calculate the box address
	char* penAddr = (char*) memMgr->GetAddr(BM);
	penAddr += 	theRect.A.y*BMWidth + theRect.A.x + CELHEADERSIZE;
	// Setup
	int vRun = theRect.Ylen();
	int hRun = theRect.Xlen();

	while(vRun--) {
		for (int i = 0;i < hRun;++i) {
			if (penAddr[i] == fore)
				penAddr[i] = back;
			else
				penAddr[i] = fore;
		}
		penAddr += BMWidth;
	}
}

#else // #ifndef NETWORK

EditClass::EditClass(short id)
{
	visible = False;
	changed = False;

	ObjectID    EditItem = (ObjectID) id;

   // Make a copy of the text
	textID = EditItem.GetProperty(s_text);
	if (((ObjectID) textID).IsObject())
		textID = ((ObjectID) textID).GetProperty(s_data);
	editTextID.Add(*textID);
	
	text = editTextID;
	fore = EditItem.GetProperty(s_fore);
	back = EditItem.GetProperty(s_back);
	skip = EditItem.GetProperty(s_skip);
	font = EditItem.GetProperty(s_font);
	BM   = EditItem.GetProperty(s_bitmap);
	borderColor = EditItem.GetProperty(s_borderColor);

	// Get a font manager for this edit item
	fntMgr = New FontMgr(font); 

   // Calculate the dimensions for the BM and the textbox
	fntMgr->SetFont(font);   

	maxChars  		= EditItem.RespondsTo(s_max)?EditItem.GetProperty(s_max):0;
	maxHeight 		= EditItem.RespondsTo(s_numLines)?EditItem.GetProperty(s_numLines):1;
	maxCharWidth  	= (EditItem.GetProperty(s_right)  - EditItem.GetProperty(s_left)) / fntMgr->CharWidth('M',False);
	if (!maxHeight) 
		maxHeight = 1;
	if (!maxCharWidth)
		maxCharWidth= maxChars / maxHeight;
	if (!maxChars)
		maxChars = maxHeight * maxCharWidth;

	// get resolution from base fontMgr, not local one
	xRes = fntMgr->GetXRes();
	yRes = fntMgr->GetYRes();

	// Make sure the strings can handle maxChars
	editTextID.Realloc(maxChars+1);
	textID.Realloc(maxChars+1);

	// The textHeight is just the pointSize + 1 for the cursor
	int textHeight = fntMgr->PointSize(False) * maxHeight + 1;
	textHeight = (textHeight * SCIRESX + xRes - 1) / xRes;

	// Assume the character 'M' is the widest character
	int textWidth = maxCharWidth * fntMgr->CharWidth('M',False) + 1;
	textWidth = (textWidth * SCIRESY + yRes - 1) / yRes;

	// always do this because otherwise cursor has problems when hits edge
//   if (borderColor != -1) {
		textWidth += 4;
		textHeight += 2;
//	}


   // planes are in Low res coordinates
	int planeWidth  = textWidth;
	int planeHeight = textHeight;

	window = SOL_Rect(0,0,planeWidth-1,planeHeight-1);
	window.Offset(EditItem.GetProperty(s_x),EditItem.GetProperty(s_y));
   SOL_Point point(0,0);

	ObjectID thisWinID;
	if (thisWinID = EditItem.GetProperty(s_planeID)) {
		point.x = thisWinID.GetProperty(s_left);
		point.y = thisWinID.GetProperty(s_top);
	}
//	LocalGameToGlobal(EditItem.GetProperty(s_planeID),point);

	window.Offset(point.x,point.y);

	SOL_Rect insetRect(2, 2,textWidth-2,textHeight-2);
	textBox = insetRect;		
	BMWidth = textWidth;

   // If there is already a bitmap then assume it's still valid.
	// See if a bit map has been built
   if (!BM) {
		// build a bitmap
		BM = fntMgr->BuildFBM
								(
                        planeWidth,
								planeHeight,
                        insetRect,
								text,
								fore,
								back,
								skip,
								font,
								EditItem.GetProperty(s_mode),
                        borderColor,
								EditItem.GetProperty(s_dimmed)
								);
	}

	// Put the cursor in
	GetLineIndices();
	SetCursor(strlen(*editTextID));
	cursorOn = 0;
	DrawCursor();
}

EditClass::EditClass(TextID theTextID,TextID theTitleID,int boxWidth,int boxHeight):
	textID(theTextID), title(theTitleID), maxCharWidth(boxWidth), maxHeight(boxHeight)
{
	// Gets input from a titled font box
	// input
	//		theText is the default input string
	//		theTitle is the title string
	//		boxWidth is the width of the input box	in characters

	
	visible = False;
	changed = False;

	maxChars = maxCharWidth * maxHeight;

	if (maxHeight <= 0 || maxCharWidth <= 0)
		msgMgr->Fatal(SrcLoc,"illegal parameters to editclass");

	editTextID.Add(textID);

	// Make sure the strings can handle maxCharWidth
	memMgr->Realloc(editTextID, maxChars + 1);
	memMgr->Realloc(textID, maxChars + 1);

	text = editTextID;
	fore = SOL_BLACK;			
	back = SOL_WHITE;			
	skip = SOL_SKIP;			
	font = systemFont; 		
//	maxWidth = boxWidth;
	borderColor = SOL_BLACK;
	cursor = 0;			
	cursorOn = 0; 

	// Get a font manager for this edit item
	fntMgr = New FontMgr(font); 

	// set the font to calculate height and width for the BM
	fntMgr->SetFont(font);

	// get resolution from base fontMgr, not local one
	xRes = fntMgr->GetXRes();
   yRes = fntMgr->GetYRes();
		
	int titleHeight = 0;
	int titleWidth  = 0;
	// Calculate height for the title
	if ((int) title) {    
      titleHeight = fntMgr->PointSize(False) + 1;
		titleHeight = (titleHeight * SCIRESY + yRes - 1) / yRes;
      if (borderColor != -1) titleHeight += 2;
//		titleWidth = fntMgr->TextWidth(title,0,strlen(*title)); 
	}

	// The textHeight is just the pointSize + 1 for the cursor
	int textHeight = fntMgr->PointSize(False) * maxHeight + 1;
	textHeight = (textHeight * SCIRESX + xRes - 1) / xRes;

	// Assume the character 'M' is the widest character
	int textWidth = maxCharWidth * fntMgr->CharWidth('M',False) + 1;  
	textWidth = (textWidth * SCIRESY + yRes - 1) / yRes;

//	if (textWidth < titleWidth) textWidth = titleWidth;
   if (borderColor != -1) {
		textWidth += 4;
		textHeight += 2;
	}

   // planes are in Low res coordinates
	int planeWidth = textWidth;
	int planeHeight = textHeight + titleHeight;

	// This centers the window automatically
	window = SOL_Rect(0,0,planeWidth-1,planeHeight-1);   
	int xOff = (SCIRESX -window.Xlen())/2;
	int yOff = (SCIRESY -window.Ylen())/2;
	window.Offset(xOff,yOff);

	SOL_Rect insetRect(2,titleHeight + 2,planeWidth-2,planeHeight-2);
	textBox = insetRect;		
	BMWidth = textWidth;

   if ((int) title) 
		BM = fntMgr->TitledFBM
								(
								planeWidth,
								planeHeight,
								insetRect,
								theTextID,
								SOL_BLACK,
								SOL_WHITE,
	                  	SOL_SKIP,			
								systemFont,
								TEJUSTLEFT,
                     	SOL_BLACK,

								theTitleID,
								SOL_WHITE,
								SOL_BLACK,
								systemFont
								);
   else
		BM = fntMgr->BuildFBM
								(
                        planeWidth,
								planeHeight,
                        insetRect,
								theTextID,
							   SOL_BLACK,
							   SOL_WHITE,
	                     SOL_SKIP,			
							   systemFont,
							   TEJUSTLEFT,
                        SOL_BLACK,
								False
								);

	// Put the cursor in 
	GetLineIndices();

	SetCursor(strlen(*editTextID));
	DrawCursor();
}


EditClass::~EditClass()
{    
	BM.Free();
   if (changed) {
//	   StrTrim(*editTextID);
	   memMgr->Realloc(textID,strlen(*editTextID)+1);
	   strcpy(*textID,*editTextID);
   }
	editTextID.Free();
	delete fntMgr;
} 

void
EditClass::Show(int pri)
{    
	// Show the edit dialog
   // ********************
	// Need a plane
	if(visible)
		return;
	visible = True;
   if(pri < 0)
		pri = graphMgr->TopSCIPlane() + 1;
	windowPlane = New Plane(window); //Plane windowPlane(window);
	windowPlane->SetPriority(pri);
	// force no plane background drawing
	windowPlane->SetPicNum(-2); 
	graphMgr->AddPlane(windowPlane);

   // Need a screen item
	SOL_Point pt(0,0);
	ScaleInfo scaleInfo;
	si = New ScreenItem(windowPlane->Id(),(Bitmap) BM,pt,scaleInfo);

	graphMgr->AddScreenItem(si);
//	graphMgr->FrameOut(False);
}


void
EditClass::Hide(Bool frameOut)
{
					
	if (!visible) return;
	visible = False;
	graphMgr->DeleteScreenItem(si, windowPlane->Id());
//   if(si) {
//	   delete si;
      si = NULL;
//	}
	graphMgr->DeletePlane(windowPlane);
	if (frameOut)
  		graphMgr->FrameOut();
}

void
EditClass::GetLineIndices()
{ 
	// Get the line indices for text using the fntMgr

	// Allocate some memory
	uint lineCount = 100;
	MemID indexMemory;
	indexMemory.Get(MemBitmap,(lineCount + 2)*sizeof(uint));
	uint* indices = (uint*) *indexMemory;
	uint line = 0;
	indices[line++] = 0;

   fntMgr->SetFont(font);
	// Make a text rectangle that will hold just one line
	SOL_Rect r(0,0,textBox.Xlen()-1,fntMgr->PointSize(False)+2);
//	r.B.x = (r.B.x * LOWRESX + xRes - 1) / xRes;
//	r.B.y = (r.B.y * LOWRESY + yRes - 1) / yRes;

   int textSize = strlen((char *)*text);
	for (uint start = 0;start < textSize;line++) {
		indices[line] = start;
		//msgMgr->Mono("indices[%d]=%d",line,start);
		indices[0]++;
//		start = fntMgr-> TextCount(text,start,r,False) + start;
		start = fntMgr-> TextCount(text,start,r,True) + start;
		if (line == lineCount) {
			if (lineCount > 1000)
				msgMgr->Fatal(SrcLoc,Msg_WindowTextTooBig);
			lineCount += 100;
			indexMemory.Realloc((lineCount + 2)*sizeof(uint));
			indices = (uint*) *indexMemory;
		}
 	}
	indices[line] = start;
	//msgMgr->Mono("indices[%d]=%d\r\n",line,start);
	maxLines = indices[0];

	lineIndicesID = indexMemory;

	int lastChar = fntMgr->TextCount(text,0,font,textBox,False) - 1;

	int lastLine = 0;
	while (lastLine < indices[0]) {
		if (indices[lastLine+1] < lastChar)
			lastLine++;
		else
			break;
	}
	if (lastLine > indices[0])
		msgMgr->Fatal(SrcLoc,Msg_InternalErrorOnLastLine);
}

Bool
EditClass::HandleEvent()
{
	int		done = False;
	int		retVal = False;
	SOL_Event evt;
	
	// Clear the edit line on certain keystrokes
	clearEditLine = True;
	// This is the edit phase
	while (True) {
		eventMgr->Get(&evt,SOL_Event::All);
		switch(evt.type) {
			case SOL_Event::KeyDown:
				switch (evt.message)	{
					case Kbd::Esc:
						done = True;
						retVal = False;
						break;

					case Kbd::Enter:
						done = True;
						retVal = True;
						break;
				}
				break;
			case SOL_Event::MouseDown:
				if (!window.Intersect(evt.where)) {
					done = True;
					retVal = False;
				}
				break;
		}
		if (done) 
			break;
		EditText(evt);

		graphMgr->UpdateScreenItem(si);
		graphMgr->FrameOut();
	}
	 
//	eventMgr->Post(&evt);

	return retVal;
}

Bool
EditClass::EditText(SOL_Event& evt)
{
	SOL_Point mousept;
	int	msg;

	UpdateFontMgr();

	char* str = (char *) *text;

	int i;
	int textLen = strlen(str);
	int charChanged = False;
	int deleteChar = False;
	int oldCursor = cursor;

	switch(evt.type) {
		case SOL_Event::KeyDown:
			switch (msg = evt.message) {
				case Kbd::Home:
					// beginning of line
					clearEditLine = False;
					cursor = 0;			
					break;

				case Kbd::End:
					// end of line
					clearEditLine = False;
					cursor = textLen;			
					break;

				case CtrlKey('C'):	
					// control C
					// clear line
					cursor = 0;			
					*str = 0;
					charChanged = True;
					break;
					
				case Kbd::BackSpace:
					// destructive backspace
					clearEditLine = False;
					deleteChar = True;
					if (cursor)	--cursor;			
					break;

				case Kbd::Left:
					// non-destructive backspace
					clearEditLine = False;
					if (cursor)	--cursor;			
					break;
		
				case Kbd::Insert:
					// toggle insert mode
					clearEditLine = False;
					if (insertMode)
						insertMode = 0;
					else
						insertMode = 1;
					charChanged = True;
					break;
		
				case Kbd::Del:
					// delete at cursor
					clearEditLine = False;
					if (cursor != textLen)
						deleteChar = True;
					break;
		
				case Kbd::Right:
					clearEditLine = False;
					if (cursor < textLen)
						++cursor;
					break;

				case Kbd::Up:
					// determine if not on top line, then move up
					//
					if	(cursorY > 0) {
         			uint* indices = (uint*) *lineIndicesID;
      				for(i = 1; cursor >= indices[i+1] && i < indices[0]; i++);
						cursor=indices[i-1]+cursorX;
						SetCursor(cursor);
						EraseCursor();
						DrawCursor();
					}

					break;

				case Kbd::Down:
					// determine if not on bottom line, then move down
					//
					if	(cursorY < maxLines) {
         			uint* indices = (uint*) *lineIndicesID;
      				for(i = 1; cursor >= indices[i+1] && i < indices[0]; i++);
						cursor=indices[i+1]+cursorX;
						SetCursor(cursor);
						EraseCursor();
						DrawCursor();
					}

					break;

				default:
					if ((msg >= ' ') && (msg < 257)) {
						// If this is the first key, clear the input
						if (clearEditLine) {
   						clearEditLine = False;
	   					*str = 0;
		   				SetCursor(0);
						}
						// insert this key and advance cursor
						// if we have room in buffer AND we won't try to
						// draw outside of our enclosing rectangle
						if ( ((!insertMode)  &&  (cursor < maxCharWidth))
							   ||
							  (textLen < maxChars)
							) {
							charChanged = True;
							if (insertMode) {
								// shift it up one
								for(int i = textLen;i >= cursor;i--)
									*(str + i + 1) = *(str + i);
							}
							else {
								if (cursor == textLen)
									*(str + textLen + 1) = 0;
							}
							*(str + cursor) = (char) msg;
							++(cursor);
						}
					}
					break;
				}

			// if delete, we delete the character at cursor
			if (deleteChar) {
				charChanged = True;
				// collapse the string from cursor on
				for (int i = cursor;i < textLen;i++)
					*(str + i) = *(str + i + 1);
				}
			break;      

		case SOL_Event::MouseDown:
			// move cursor to closest character division
			clearEditLine = False;
			mousept.x = evt.where.x;
			mousept.y = evt.where.y;

         int ps = fntMgr->PointSize(False);
         SOL_Rect intersectRect(textBox);
         intersectRect.Offset(window.A.x, window.A.y);

			if (intersectRect.Intersect(mousept)) {
         	uint* indices = (uint*) *lineIndicesID;
            for (int line = 0; line < indices [0]; line++) {
               SOL_Rect r(0,0,textBox.Xlen()-1,ps);
               r.Offset(intersectRect.A.x,intersectRect.A.y+(ps*line));
               if (r.Intersect(mousept)) {
                  char *str = (char *) *text;
                  int lineSize = (indices[0] > line+1) ? indices[line+2] - indices[line+1] : strlen(str)-indices[line+1];

                  int xPos = intersectRect.A.x;
                  for (int loop = 0; loop < lineSize; loop++) {
                     int cWide = fntMgr->CharWidth(*(str+loop));
                     if (mousept.x > xPos && mousept.x < xPos+cWide-1) {
                        EraseCursor();
                        cursor = indices[line+1]+loop;
                        DrawCursor();
                     }
                     xPos += cWide;
                  }
               }
            }
         }
			break; 												
		}

	if (charChanged) {
		// if we have changed we redraw the entire field in the text box
		EraseCursor();
		// erase the text area of the FBM
		fntMgr->Erase(textBox);
      //if (editStruct->borderColor != -1)
			//fntMgr->Frame(editStruct->textBox,1,editStruct->borderColor);
		fntMgr->DrawTextBox();
		GetLineIndices();
		DrawCursor();
		changed = True;
	   memMgr->Realloc(textID,strlen(*editTextID)+1);
	   strcpy(*textID,*editTextID);
	}
	else {
		if (oldCursor == cursor) {
			// cursor is in the same place -- keep flashing
			FlashCursor();
		}
		else {
			// cursor has moved -- ensure it is on at new position
			EraseCursor();
			DrawCursor();
		}
	}
   graphMgr->UpdateScreenItem(si);
//	graphMgr->FrameOut();
   return changed;
}


void
EditClass::UpdateFontMgr()
{
//	char * str = (char *) memMgr->GetAddr(editStruct->text);
	fntMgr->SetBM(BM);
	fntMgr->SetFont(font);
}


void
EditClass::DrawCursor()
{
	char * str = (char *) *text;

	int left;
	int frameSize = 3;
	if (borderColor == -1) frameSize = 0;

//	if (!cursorOn) {
//		// The cursor box 
//		if (borderColor == -1) {
//			left = textBox.A.x+fontMgr->TextWidth(text,0,cursor);
//			cursorRect.A.x = left;
//			if (insertMode) {
//				cursorRect.A.y = textBox.A.y+fontMgr->PointSize()-1;
//				cursorRect.B.y = cursorRect.A.y;
//			}
//			else {
//				cursorRect.A.y = textBox.A.y;
//				cursorRect.B.y = cursorRect.A.y+fontMgr->PointSize()-1;
//			}
//		}
//		else {
//			left = textBox.A.x+fontMgr->TextWidth(text,0,cursor);
//			cursorRect.A.x = left;
//			if (insertMode) {
//				cursorRect.A.y = textBox.A.y+fontMgr->PointSize()-1;
//				cursorRect.B.y = cursorRect.A.y;
//			}
//			else {
//				cursorRect.A.y = textBox.A.y;
//				cursorRect.B.y = cursorRect.A.y+fontMgr->PointSize()-1;
//			}
//		}
//
//		// we flash the character cel unless we are at end of string
//		str += cursor;
//		if (*str) {
//			cursorRect.B.x = left + fontMgr->CharWidth(*str) - 1;
//		}
//		else {
//			cursorRect.B.x = left;
//			cursorRect.B.x = left + fontMgr->CharWidth(' ') - 1;
//		}
//		// toggle cursor
//		fontMgr->InvertRect(
//					  BM,
//					  BMWidth,
//					  cursorRect,
//					  fore,
//					  back);
//	}

	if (!cursorOn) {
		// The cursor box
		SetCursor();
      int tWidth = fntMgr->TextWidth(text,0,cursor); //cursor - cursorX,cursorX);
      int ps=fntMgr->PointSize(False)-1;
		ps = (ps * SCIRESY + yRes - 1) / yRes;

		cursorRect.A.x = left = textBox.A.x+tWidth;
		if (insertMode) {
         cursorRect.A.y = textBox.A.y+ps*(cursorY+1);
			cursorRect.B.y = cursorRect.A.y;
		} else {
			cursorRect.A.y = textBox.A.y+ps*cursorY;
			cursorRect.B.y = cursorRect.A.y+ps-1;
		}

		// we flash the character cel unless we are at end of string
		str += cursor;
		if (*str) {
			cursorRect.B.x = left + fntMgr->CharWidth(*str) - 1;
		}
		else {
			cursorRect.B.x = left + fntMgr->CharWidth(' ') - 1;
		}

		//msgMgr->Mono("cursorX=%d,Y=%d cursorRect.B.y=%d",cursorX,cursorY,cursorRect.B.y);

		// toggle cursor
		fntMgr->InvertRect(
					  BM,
					  BMWidth,
					  cursorRect,
					  fore,
					  back);
	}
	cursorOn = True;
	SetFlash();
}


void
EditClass::EraseCursor()
{

	if (cursorOn)
		fntMgr->InvertRect(
					  BM,
					  BMWidth,
					  cursorRect,
					  fore,
					  back);
	cursorOn = False;
	SetFlash();
}


void
EditClass::FlashCursor()
{

	if (flash < timeMgr->GetTickCount()) {
		fntMgr->InvertRect(
					  BM,
					  BMWidth,
					  cursorRect,
					  fore,
					  back);
		cursorOn = !cursorOn;
		SetFlash();
		}
}


void
EditClass::SetFlash()
{
	flash = 30L + timeMgr->GetTickCount();
}  

void
EditClass::InvertRect(SOL_Rect& theRect,int BMWidth,char fore,char back)
{
	// Switches the fore ground and background colors
	// Calculate the box address
	char* penAddr = (char*) memMgr->GetAddr(BM);
	penAddr += 	theRect.A.y*BMWidth + theRect.A.x + CELHEADERSIZE;
	// Setup
	int vRun = theRect.Ylen();
	int hRun = theRect.Xlen();

	while(vRun--) {
		for (int i = 0;i < hRun;++i) {
			if (penAddr[i] == fore)
				penAddr[i] = back;
			else
				penAddr[i] = fore;
		}
		penAddr += BMWidth;
	}
}

void
EditClass::SetCursor(int where)
{
	if(where > -1)
   	cursor = where;
	int i;
	uint* indices = (uint*) *lineIndicesID;

   if (maxHeight == 1) i = cursorY = 0;
   else {
      for(i = 1; cursor >= indices[i+1] && i < indices[0]; i++);
   	cursorY = i-1;
   }
	cursorX = cursor - indices[i];
}

void
EditClass::Highlight(int col)
{
	
	int editBMWidth = BMWidth;

	hiColor = col;

	SOL_Rect r(textBox);

//	xRes = fntMgr->GetXRes();
//	yRes = fntMgr->GetYRes();
//	r.B.x = (r.B.x * xRes) / LOWRESX;
//	r.B.y = (r.B.y * yRes) / LOWRESY;
//	editBMWidth = (editBMWidth * xRes) / LOWRESX;

	r.Inset(-2,-2);
	r.B.x--;
	r.B.y--;
	SOL_Line line1(r.A.x, r.A.y, r.B.x, r.A.y);
	line1.DrawThin(hiColor,SOL_Line::tSolid,0,BM,editBMWidth);
	SOL_Line line2(r.B.x, r.A.y, r.B.x, r.B.y);
	line2.DrawThin(hiColor,SOL_Line::tSolid,0,BM,editBMWidth);
	SOL_Line line3(r.A.x, r.B.y, r.B.x, r.B.y);
	line3.DrawThin(hiColor,SOL_Line::tSolid,0,BM,editBMWidth);
	SOL_Line line4(r.A.x, r.A.y, r.A.x, r.B.y);
	line4.DrawThin(hiColor,SOL_Line::tSolid,0,BM,editBMWidth);

	if (col == back)
		EraseCursor();
	else
		DrawCursor();

   if (visible) {
		graphMgr->UpdateScreenItem(si);
//		graphMgr->FrameOut();
	}
}

void
KEdit(argList)
{
   enum {
      editModal,
      editCreate,
      editDestroy,
      editShow,
      editHide,
      editHandleEvent,
		editGetRect,
		editHighlight
   };

   switch (arg(1)) {
      case editModal: 
         if (argCount > 2) {
 	         int priority = graphMgr->TopSCIPlane() + 1;
 	         EditClass ec((TextID) arg(2),(TextID) arg(3),arg(4),argCount > 4 ? arg(5) : 1);
 	         ec.Show(priority);
 	         pm.acc = ec.HandleEvent();
            ec.Hide();
         } else {
	         EditClass ec(arg(2));
	         ec.Show();
	         pm.acc = ec.HandleEvent();
            ec.Hide(((ObjectID)arg(1)).GetProperty(s_frameOut));
         }
         break;
      case editCreate: 
         {
            MemID thunk;
            thunk.Get(MemEdit, sizeof(EditClass));
            if (argCount == 2) 
               	new(*thunk) EditClass(arg(2));
					else
               	new(*thunk) EditClass((TextID) arg(2), (TextID) arg(3), arg(4), argCount > 4 ? arg(5) : 1);
            pm.acc = thunk;
         }
         break;
      case editDestroy:
         {
            MemID thunk((SOL_Handle)arg(2));
            ((EditClass *)*thunk)->Hide();
            ((EditClass *)*thunk)->~EditClass();
            thunk.Free();
         }
         break;
      case editShow:
         {
            MemID thunk((SOL_Handle)arg(2));
            ((EditClass *)*thunk)->Show();
         }
         break;
      case editHide:
         {
            MemID thunk((SOL_Handle)arg(2));
            ((EditClass *)*thunk)->Hide();
         }
         break;
      case editHandleEvent:
         {
            MemID thunk((SOL_Handle)arg(2));
            SOL_Event evt((EventObjectID)arg(3));
            ((EditClass *)*thunk)->EditText(evt);
         }
         break;
		case editGetRect:
			{
				MemID thunk((SOL_Handle)arg(2));
				SCIWord* data = (SCIWord*) ((ArrayID) arg(3)).Data();
				EditClass *thisEdit = ((EditClass *)*thunk);

				SOL_Rect r = ((EditClass *)*thunk)->window;
				data[0] = r.A.x;
				data[1] = r.A.y;
				data[2] = r.B.x;
				data[3] = r.B.y;
			}
			break;
		case editHighlight:
			{
				MemID thunk((SOL_Handle)arg(2));
            ((EditClass *)*thunk)->Highlight(arg(3));
			}
			break;
   }
}

int
GetInput(char* theText,char* theTitle,int boxWidth)
{
	// Gets input from a titled font box
	// input
	//		theText is the default input string
	//		theTitle is the title string
	//		boxWidth is the width of the input box	in characters

	TextID theTextID;
	TextID theTitleID;

	// Make copies of the character strings
	theTextID.Add(theText);
	theTitleID.Add(theTitle);

   // The priority of this plane will be higher than SCI planes
	int priority = graphMgr->TopPlane() + 1;
   if (priority < PLANEPRIORITYSTART)
      priority = PLANEPRIORITYSTART;

	Bool retVal = 0;

	{
	EditClass ec(theTextID,theTitleID,boxWidth);
	ec.Show(priority);
	retVal = ec.HandleEvent();
   ec.Hide();
	}

	if (retVal) {
		strcpy(theText,*theTextID);
	}

	theTextID.Free();
	theTitleID.Free();
	return retVal;
}   

Bool
MsgBox(char* title, char* fmt, ...)
{
	char buf[MsgMgr::MaxStr];
	Bool escape;

	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);


	if (graphMgr)
		graphMgr->GCursor().Hide();
	
#ifdef DEBUG	
	if (configMgr && configMgr->Get(configMgr->MonoEcho)) {
		msgMgr->Mono("{");
		msgMgr->Mono(title);
		msgMgr->Mono("}");
		msgMgr->Mono(buf);
		msgMgr->Mono("<Enter or Esc>");
	} else
#endif
#ifndef WINDOWS
		if (errorMgr)
			errorMgr->Display(buf,title); 
		else {
			puts("MsgBox called before errorMgr initialized:");
			puts(buf);
			exit(1);
		}
	escape = WaitOnUser();
#else
	escape = MBox2(buf,title);
#endif

#ifdef DEBUG
	if (configMgr && configMgr->Get(configMgr->MonoEcho))
		msgMgr->Mono("\n");
#endif

#ifndef WINDOWS
#ifdef DEBUG
	else
#endif
		errorMgr->Clear(); 
#endif

	if (graphMgr)
		graphMgr->GCursor().UnHide();

	return !escape;
}

#endif // #ifndef NETWORK

