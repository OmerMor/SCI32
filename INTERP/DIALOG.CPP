#include <string.h>
#include <stdarg.h>

#include "sol.hpp"

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
static long    flash;
static short   insertMode = 1;
static short   clearEditLine;

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
	editStruct->font = SYSTEMFONT; 		
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
							SYSTEMFONT,
							TEJUSTLEFT,
                     SOL_BLACK,

							theTitleID,
							SOL_WHITE,
							SOL_BLACK,
							SYSTEMFONT
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
		graphMgr->FrameOut();
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
	MBox(buf,title);
	escape = False;
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
