//	window.cpp
#include <string.h>

#include "sol.hpp"

#include "font.hpp"
#include "event.hpp"
#include "graph.hpp"
#include "graphmgr.hpp"
#include "kernel.hpp"
#include "mouse.hpp"
#include "msg.hpp"
#include "pmachine.hpp"
#include "si.hpp"
#include "stampver.hpp"
#include "window.hpp"

void
ScrollWindowID::Get(SOL_Rect& theWindow,
						SOL_Point thePos,
						int		thePlaneID,
						ushort 	theFore,
						ushort 	theBack,
						ushort 	theFont,
						short	 	theMode,
						short		theBorder,
						int		theSize)
{
	FakeScrollWindowID::Get(sizeof(ScrollWindow), LOCKED | LOCKEDNOSAVE);
	new(handle) ScrollWindow(theWindow,	thePos, thePlaneID, theFore,
									theBack,theFont,theMode,theBorder,theSize);
}

void
ScrollWindowID::Free()
{
	if(handle)
		(**this)->~ScrollWindow();
	FakeScrollWindowID::Free();
}

// Window with a title
SOL_Window::SOL_Window(SOL_Rect& theWindow,	 // main info
							  SOL_Point thePos,	
							  int			thePlaneID,
							  char* 		theText,
							  ushort 	theFore,
							  ushort 	theBack,
							  ushort 	theFont,
							  short	 	theMode,
                       short		theBorder,
							  char*		theTitle,	 // Title info
							  ushort 	theTitleFore,
							  ushort 	theTitleBack,
							  ushort 	theTitleFont,
							  Bool 		makeVisible) :
	si(0)
{

	if (!theTitle)
		msgMgr->Fatal(SrcLoc,Msg_NoWindowTitle);

	// Make copies of the character strings
	text.Add(theText);
	title.Add(theTitle);
 
	// Get a font manager for this window
	fntMgr = New FontMgr(theTitleFont);

	// Get resolution
	xRes = fntMgr->GetXRes();
	yRes = fntMgr->GetYRes();
	
	// Determine the title size
	int titleSize = fntMgr->PointSize(False) + 3;

	visible = False;
	startChar = 0;
	startLine = 1;

	plane = graphMgr->GetPlaneNext(thePlaneID);
	pos = thePos;

	// Determine a skip color that will not be hit
	for (int skip=0;skip<255;++skip) {
		if 
			(
			(skip != theFore)		  &&
			(skip != theBack)
			)
			break;
	}

	// Save the parameters
	scaleWindow = window = theWindow;
   scaleWindow.Scale(xRes,LOWRESX,yRes,LOWRESY);
   border = theBorder;

	textBox.A.x = 2;
	textBox.A.y = titleSize + 1;
	textBox.B.x = scaleWindow.Xlen() - 3;
	textBox.B.y = scaleWindow.Ylen() - 3;

	// Determine the string to draw
   
	font = theFont;
	fore = theFore;
	mode = theMode;
	fntMgr->SetFont(theFont);
	pointSize = fntMgr->PointSize(False);
	lastChar = fntMgr->TextCount(text,0,textBox,False) - 1;

	textSize = strlen(*text);
	workText.Get(textSize+1);
	strncpy(*workText,*text,lastChar+1);
	workText[lastChar+1] = 0;

	// Get the line indices for the text
	GetLineIndices();
		
	// Create a fontBitmap
	fontBitmap = fntMgr->TitledFBM(scaleWindow.Xlen(),
											 scaleWindow.Ylen(),
											 textBox,
											 workText,
											 theFore,
											 theBack,
                                  skip,
											 theFont,
											 theMode,
                                  theBorder,
											 title,
											 theTitleFore,
											 theTitleBack,
											 theTitleFont,
											 False);
	// Show if visible
	if (makeVisible) Show();
}

// Window without a title
SOL_Window::SOL_Window(SOL_Rect& theWindow, 
							  SOL_Point thePos,	
							  int			thePlaneID,
							  char* 		theText,
							  ushort 	theFore,
							  ushort 	theBack,
							  ushort 	theFont,
							  short	 	theMode,
                       short		theBorder,
							  Bool 		makeVisible) :
	si(0)
{
	// Make a copy of the character string
	text.Add(theText);
 
	// Get a font manager for this window
	fntMgr = New FontMgr(theFont);

	// Get resolution
	xRes = fntMgr->GetXRes();
	yRes = fntMgr->GetYRes();

	visible = False;
	startChar = 0;
	startLine = 1;

	// Save the parameters 
	plane = graphMgr->GetPlaneNext(thePlaneID);
	pos = thePos;
	scaleWindow = window = theWindow;
   scaleWindow.Scale(xRes,LOWRESX,yRes,LOWRESY);
   border = theBorder;
	textBox.A.x = 2;
	textBox.A.y = 2;
	textBox.B.x = scaleWindow.Xlen() - 3;
	textBox.B.y = scaleWindow.Ylen() - 3;

	// Determine the string to draw
	font = theFont;
	fore = theFore;
	mode = theMode;
	lastChar = fntMgr->TextCount(text,0,theFont,textBox,False) - 1;
	pointSize = fntMgr->PointSize(False);

	textSize = strlen(*text);
	workText.Get(strlen(*text)+1);
	strncpy(*workText,*text,lastChar+1);
	workText[lastChar+1] = 0;

	// Get the line indices for the text
	GetLineIndices();

	// Determine a skip color that will not be hit
	for (int skip=0;skip<255;++skip) {
		if 
			(
			(skip != theFore)		  &&
			(skip != theBack)
			)
			break;
	}

	// Create a fontBitmap
	fontBitmap = fntMgr->BuildFBM(scaleWindow.Xlen(),
								  			scaleWindow.Ylen(),
								  			textBox,
								  			workText,
								  			theFore,
								  			theBack,
								  			skip,
								  			theFont,
								  			theMode,
                                 border,
								  			0,False);
	// Show if visible
	if (makeVisible) Show();
}

SOL_Window::~SOL_Window()
{
	if (visible) Hide();
	memMgr->Free(fontBitmap);
	memMgr->Free(workText);
	memMgr->Free(text);
	memMgr->Free(lineIndicesID);
	memMgr->Free(fontIndicesID);
	if ((int) title) memMgr->Free(title);
	delete fntMgr;
}


void
SOL_Window::Show()
{
	if (visible) return;
	visible = True;
	Bitmap bitmap(fontBitmap);
	ScaleInfo scaleInfo;
	if (!si) {
   	if(plane)
			si = New ScreenItem(plane->Id(),bitmap,pos,scaleInfo);
		else  
			msgMgr->Fatal(SrcLoc,"No Plane defined");
   }
	graphMgr->AddScreenItem(si);
}

void
SOL_Window::PageUp()
{
	// Up one page
	if (startLine > 1) { 
		uint* indices = (uint*) *lineIndicesID;
		lastLine = startLine - 1;
		if (lastLine < linesPerPage) lastLine = linesPerPage;
		startLine -= linesPerPage;
		if (startLine < 1) startLine = 1;
		startChar = indices[startLine];
		lastChar = indices[lastLine+1] - 1;
		ReDrawText();
	}
}

void
SOL_Window::PageDown()
{ 
	// Down one page
	if (lastLine < maxLines) { 
		uint* indices = (uint*) *lineIndicesID;
		startLine = lastLine + 1;
		startChar = indices[startLine];
		lastLine += linesPerPage;
		if (lastLine > maxLines) lastLine = maxLines;
		lastChar = indices[lastLine+1] - 1;
		ReDrawText();
	}
}

void
SOL_Window::Home()
{  
	// Top of document
	if (startLine > 1) {
		uint* indices = (uint*) *lineIndicesID;
		startChar = 0;
		startLine = 1;
		lastLine = linesPerPage;
		if (lastLine > maxLines) lastLine = maxLines;
		lastChar = indices[lastLine+1] - 1;
		ReDrawText();
	}
}

void
SOL_Window::End()
{ 
	// Last page of document
	if (lastLine < maxLines) {
		uint* indices = (uint*) *lineIndicesID;
		lastLine = maxLines;
		startLine = lastLine - linesPerPage + 1;
		if (startLine < 1) startLine = 1;
		startChar = indices[startLine];
		lastChar = indices[lastLine+1] - 1;
		ReDrawText();
	}
}

void
SOL_Window::UpArrow()
{
  // Up one line
	if (startLine > 1) {
		uint* indices = (uint*) *lineIndicesID;
		uint* fonts = (uint*) *fontIndicesID;
		uint* colors = (uint*) *colorIndicesID;
		uint* modes = (uint*) *modeIndicesID;
		startLine--;
		lastLine--;
		if (lastLine - startLine + 1 < linesPerPage) lastLine = maxLines;
		startChar = indices[startLine];
		lastChar = indices[lastLine+1] - 1;
		int length = lastChar - startChar + 1;
		strncpy(*workText,&text[startChar],length);
		// Insert terminator
		workText[length] = 0;
		int drawCount = indices[startLine+1]-indices[startLine];
		if (text[indices[startLine+1]-1] == Kbd::LineFeed)
			drawCount--;
		fntMgr->SetColor(colors[startLine]);
		fntMgr->SetMode(modes[startLine]);
		fntMgr->DownOneLine(drawCount,linesPerPage,fonts[startLine]);
      if (visible) {
	      graphMgr->UpdateScreenItem(si);
		   graphMgr->FrameOut();
      }
	}
}

void
SOL_Window::DownArrow()
{ 
	// Down one line
	if (startLine < maxLines) {
		uint* indices = (uint*) *lineIndicesID;
		uint* fonts = (uint*) *fontIndicesID;
		uint* colors = (uint*) *colorIndicesID;
		uint* modes = (uint*) *modeIndicesID;
		startLine++;
		lastLine++;
		if (lastLine > maxLines) lastLine = maxLines;
		startChar = indices[startLine];
		lastChar = indices[lastLine+1] - 1;
		if (text[lastChar] == Kbd::LineFeed) lastChar--;
		int length = lastChar - startChar + 1;
		strncpy(*workText,&text[startChar],length);
		// Insert terminator
		workText[length] = 0;
		fntMgr->SetColor(colors[lastLine]);
		fntMgr->SetMode(modes[lastLine]);
		if ((lastLine - startLine + 1) == linesPerPage)	{
			fntMgr->UpOneLine(indices[lastLine]-indices[startLine],
									lastChar-indices[lastLine] + 1,
									linesPerPage,
									fonts[lastLine]);
		}
		else {
			fntMgr->UpOneLine(0,
									0,
									linesPerPage,
									fonts[lastLine]);
		}
      if (visible) {
		   graphMgr->UpdateScreenItem(si);
		   graphMgr->FrameOut();
      }
   }
}

Bool 
SOL_Window::BackAWord()
{
	// Back up a word
	Bool returnValue = True;

	if (((char*) *text)[startChar] != ' ') {
		while (((char*) *text)[startChar] != ' ') {
			if (!startChar) {
				returnValue = False;
				break;
			}
			startChar--;
		}
	}
	while (((char*) *text)[startChar] == ' ') {
		if (!startChar) {
			returnValue = False;
			break;
		}
		startChar--;
	}
	while (((char*) *text)[startChar-1] != ' ') {
		if (!startChar) {
			break;
		}
		startChar--;
	}
	return returnValue;
}

Bool 
SOL_Window::ForwardAWord()
{
	// Back up a word
	Bool returnValue = True;

	if (((char*) *text)[startChar] != ' ') {
		while (((char*) *text)[startChar] != ' ') {
			if (startChar > (textSize-1)) {
				returnValue = False;
				break;
			}
			startChar++;
		}
	}
	while (((char*) *text)[startChar] == ' ') {
		if (startChar > (textSize-1)) {
			returnValue = False;
			break;
		}
		startChar++;
	}
	return returnValue;
}

void
SOL_Window::ReDrawText()
{
  	uint* fonts = (uint*) *fontIndicesID;
  	uint* colors = (uint*) *colorIndicesID;
	uint* modes = (uint*) *modeIndicesID;

	// Redraw with new text
	int length = lastChar - startChar + 1; 
	strncpy(*workText,&text[startChar],length);
	// Insert terminator
	workText[length] = 0;

	fntMgr->Erase(textBox,False);
	fntMgr->SetColor(colors[startLine]);
	fntMgr->SetMode(modes[startLine]);
	fntMgr->SetFont(fonts[startLine]);
	fntMgr->DrawTextBox();
   if (visible) {
	   graphMgr->UpdateScreenItem(si);
	   graphMgr->FrameOut();
   }
}


void
SOL_Window::Hide()
{
	if (!visible) return;
	visible = False;
	graphMgr->DeleteScreenItem(si,plane->Id());
   si = NULL;
	graphMgr->FrameOut();
}
	 

void
SOL_Window::Move(SOL_Point pt)
{
	window.Offset(pt.x - window.A.x,pt.y - window.A.y);
   pos = pt;
   if (visible) {
	   si->pos = pt;
	   graphMgr->UpdateScreenItem(si);
	   graphMgr->FrameOut();
   }
}

void 
SOL_Window::ResizeWindow(SOL_Rect& newWindow)
{
	if (!newWindow.IsEmpty())	{

	   SOL_Rect tempWindow = newWindow;
 		// The new window will work
  	   newWindow.Scale(xRes,LOWRESX,yRes,LOWRESY);

		// Calculate the new textBox
		SOL_Rect newTextBox(textBox.A.x,
								  textBox.A.y,
								  newWindow.Xlen()-3,
								  newWindow.Ylen()-3
								  );
		// Don't let the window get too small or large
 		if (
			(newTextBox.Ylen() > (fntMgr->PointSize(False) + 3))		&&
			(newTextBox.Ylen() < yRes)								&&
			(newTextBox.Xlen() > (5*fntMgr->CharWidth('M',False)))	&&
			(newTextBox.Xlen() < xRes)
			) {

         window = tempWindow;
			scaleWindow = newWindow;
			textBox = newTextBox;

			// Get new line indices
			memMgr->Free(lineIndicesID);
			memMgr->Free(fontIndicesID);
			memMgr->Free(colorIndicesID);
			memMgr->Free(modeIndicesID);
			GetLineIndices();

			// Calculate the workText so that the old Start
			// character is still visible in the new window
			uint* indices = (uint*) *lineIndicesID;
			uint* fonts = (uint*) *fontIndicesID;
			uint* colors = (uint*) *colorIndicesID;
			uint* modes = (uint*) *modeIndicesID;
			startLine = 0;
			while (indices[startLine+1] <= startChar) startLine++;
			lastLine = startLine + linesPerPage - 1;
			if (lastLine > maxLines) lastLine = maxLines;
			startChar = indices[startLine];
			lastChar = indices[lastLine+1] - 1;
			int length = lastChar - startChar + 1;
			strncpy(*workText,&text[startChar],length);
			// insert terminator
			workText[length] = 0;
			// Rebuild the fontBitmap
			fntMgr->SetFont(fonts[startLine]);
			fntMgr->SetColor(colors[startLine]);
			fntMgr->SetMode(modes[startLine]);
			fntMgr->ResizeBitMap(scaleWindow,False);
			// Show the new bit map
         if (visible) {
			   graphMgr->UpdateScreenItem(si);
			   graphMgr->FrameOut();
         }
		}
	}
}

void 
SOL_Window::SetText(char* theText,Bool showIt, int theStartChar)
{
	if(theText != NULL)
		text = theText;
	textSize = strlen(*text); 
	memMgr->Realloc(workText,textSize+1);
	// Get the line indices for the text
	memMgr->Free(lineIndicesID);
	memMgr->Free(fontIndicesID);
	memMgr->Free(colorIndicesID);
	memMgr->Free(modeIndicesID);
	GetLineIndices();
	uint* indices = (uint*) *lineIndicesID;
	uint* fonts = (uint*) *fontIndicesID;
	uint* colors = (uint*) *colorIndicesID;
	uint* modes = (uint*) *modeIndicesID;
	startLine = 1;
	while (indices[startLine+1] <= theStartChar) startLine++;
	lastLine = startLine + linesPerPage - 1;
	if (lastLine > maxLines) lastLine = maxLines;
	startChar = indices[startLine];
	lastChar = indices[lastLine+1] - 1;
	int length = lastChar - startChar + 1;
	strncpy(*workText,&text[startChar],length);
	// insert terminator
	workText[length] = 0;
	fntMgr->SetFont(fonts[startLine]);
   if (pointSize != fntMgr->PointSize(False))
	  	msgMgr->Fatal(SrcLoc,"Illegal font size font = %d pointSize = %d, should be %d.",
                    fntMgr->GetFont(),fntMgr->PointSize(False),pointSize);

	colors = (uint*) *colorIndicesID;
	modes = (uint*) *modeIndicesID;

	fntMgr->SetColor(colors[startLine]);
	fntMgr->SetMode(modes[startLine]);
	fntMgr->SetText(workText);
	fntMgr->Erase(textBox,False);
	fntMgr->DrawTextBox();
	if (visible) {
		graphMgr->UpdateScreenItem(si);
		if (showIt)	graphMgr->FrameOut();
	}
}

void
SOL_Window::GetLineIndices()
{
	// Get the line indices for text using the fntMgr

	fntMgr->SetFont(font);
	fntMgr->SetColor(fore);
	fntMgr->SetMode(mode);

	// Allocate some memory
	uint lineCount = 100;
	MemID indexMemory;
	indexMemory.Get(MemBitmap,(lineCount + 2)*sizeof(uint));
	MemID fontIndexMemory;
	fontIndexMemory.Get(MemBitmap,(lineCount + 2)*sizeof(uint));
	MemID colorIndexMemory;
	colorIndexMemory.Get(MemBitmap,(lineCount + 2)*sizeof(uint));
	MemID modeIndexMemory;
	modeIndexMemory.Get(MemBitmap,(lineCount + 2)*sizeof(uint));

	uint* indices = (uint*) *indexMemory;
	uint* fonts = (uint*) *fontIndexMemory;
	uint* colors = (uint*) *colorIndexMemory;
	uint* modes = (uint*) *modeIndexMemory;
	uint line = 0;
	fonts[line] = 0;
	colors[line] = 0;
	modes[line] = 0;
	indices[line++] = 0;

	// Make a text rectangle that will hold just one line
   if (pointSize != fntMgr->PointSize(False))
	  	msgMgr->Fatal(SrcLoc,"Illegal font size font = %d pointSize = %d, should be %d",
                    fntMgr->GetFont(),fntMgr->PointSize(False),pointSize);
	SOL_Rect r(0,0,textBox.Xlen()-1,fntMgr->PointSize(False)+2);
	for (uint start = 0;start < textSize;line++) {
      if (pointSize != fntMgr->PointSize(False))
	  		msgMgr->Fatal(SrcLoc,
							  "Illegal font size font = %d pointSize = %d, should be %d",
         					fntMgr->GetFont(),
								fntMgr->PointSize(False),
								pointSize);
		indices[line] = start;
		indices[0]++;
		fonts[line] = font;
		fonts[0]++;
		colors[line] = fore;
		colors[0]++;
		modes[line] = mode;
		modes[0]++;
		start = fntMgr-> TextCount(text,start,r,False) + start;
		if (line == lineCount) {
			if (lineCount > 1000)
				msgMgr->Fatal(SrcLoc,Msg_WindowTextTooBig);
			lineCount += 100;

			indexMemory.Realloc((lineCount + 2)*sizeof(uint));
			fontIndexMemory.Realloc((lineCount + 2)*sizeof(uint));
			colorIndexMemory.Realloc((lineCount + 2)*sizeof(uint));
			modeIndexMemory.Realloc((lineCount + 2)*sizeof(uint));

			indices = (uint*) *indexMemory;
			fonts = (uint*) *fontIndexMemory;
			colors = (uint*) *colorIndexMemory;
			modes = (uint*) *modeIndexMemory;
		}
 	}
	indices[line] = start; 
	fonts[line] = font;
	colors[line] = fore;
	modes[line] = mode;
	maxLines = indices[0];

	lineIndicesID = indexMemory;
	fontIndicesID = fontIndexMemory;
	colorIndicesID = colorIndexMemory;
	modeIndicesID = modeIndexMemory;

	lastChar = fntMgr->TextCount(text,0,font,textBox,False) - 1;

	lastLine = 0;
	while (lastLine < indices[0]) {
		if (indices[lastLine+1] < lastChar)
			lastLine++;
		else
			break;
	}
	linesPerPage = lastLine;
	if (lastLine > indices[0])
		msgMgr->Fatal(SrcLoc,Msg_InternalErrorOnLastLine);
}   


EventWindow::~EventWindow()
{
	if (plane)
		graphMgr->DeletePlane(plane);
}



Bool
EventWindow::HandleEvent(SOL_Event& theEvent)
{
	// Handles events
	// 	Events handled:
	//			PageUp
	//			PageDown
	//			Home
	//			End
	//       UpArrow
	//       DownArrow
	//			MouseDown followed by a MouseUp (causes a drag or resize)
	// If the event is handled the return
	// value is True, else it's False.

	int returnValue = False;

	switch(theEvent.type) {
		case SOL_Event::KeyDown:
			switch (theEvent.message) {
				case Kbd::PgDn:
            	PageDown();
					returnValue = True;
					break;
				case Kbd::PgUp:
            	PageUp();
					returnValue = True;
					break;
				case Kbd::Home:
            	Home();
					returnValue = True;
					break;
				case Kbd::End:
            	End();
					returnValue = True;
					break;
				case Kbd::Up:
            	UpArrow();
					returnValue = True;
					break;
				case Kbd::Down:
            	DownArrow();
					returnValue = True;
					break;
			}
			break;

		case SOL_Event::MouseDown:
			// If the mouse is in the title area move the window else
			// if the mouse is in the textBox resize the window from
			// the bottom right corner	relative to the mouse move vector.
											 
			uint* indices = (uint*) *lineIndicesID;
			// Check for right mouse click
			if (theEvent.modifiers == Kbd::Shift)
				break;
			if (!window.Intersect(theEvent.where))
				break;
			mouseStart = theEvent.where;
			SOL_Rect titleRect = window;
			titleRect.B.y -= textBox.Ylen() * LOWRESY / yRes;
			if (titleRect.Intersect(mouseStart))	{
				// mouseDown in the title bar
				// move the dialog until a mouse up
				while (True) {
					// Poll the mouse position
					SOL_Point mousePt;
					mouse->GlobalPosSci(&mousePt);
					if (mousePt != mouseStart) {
						SOL_Point delta(mousePt.x - mouseStart.x,mousePt.y - mouseStart.y);
						// Move the window
						window.Offset(delta.x,delta.y);
						plane->MoveDelta(delta.x,delta.y);
						graphMgr->UpdatePlane(plane);
						graphMgr->FrameOut();
						mouseStart = mousePt;
					}
					SOL_Event evt;
					eventMgr->Get(&evt,SOL_Event::All);
					if (evt.type == SOL_Event::MouseUp) break;
				}
			}
			else {
				// mouseDown in the text area
				// resize the dialog until a mouse up

				while (True) {
					// Poll the mouse position
					SOL_Point mousePt;
					mouse->GlobalPosSci(&mousePt);
					if (mousePt != mouseStart) {
						SOL_Point delta(mousePt.x - mouseStart.x,mousePt.y - mouseStart.y);
						// Resize the window
						SOL_Rect newWindow(window.A.x,window.A.y,
												 window.B.x+delta.x,
												 window.B.y+delta.y);
						ResizeWindow(newWindow);
						mouseStart = mousePt;
					}
					SOL_Event evt;
					eventMgr->Get(&evt,SOL_Event::All);
					if (evt.type == SOL_Event::MouseUp)	break;
				}
			}
			returnValue = True;
			break; 									
			
	}
	return returnValue;
}

void
EventWindow::Hide()
{
	if (!visible) return;
	visible = False;
	graphMgr->DeletePlane(plane);
   si = NULL;
	plane = NULL;
	graphMgr->FrameOut();
}

void
EventWindow::Move(SOL_Point pt)
{
	if(plane) {
		window.Offset(pt.x - window.A.x,pt.y - window.A.y);
		plane->MoveTo(pt);
      if (visible) {
   		graphMgr->UpdatePlane(plane);
	   	graphMgr->FrameOut();
      }
   }
}

void
EventWindow::ResizeWindow(SOL_Rect& newWindow)
{
	if (!newWindow.IsEmpty())	{
		plane->ResizePlane(newWindow.Xlen(),newWindow.Ylen());
		SOL_Window::ResizeWindow(newWindow);
	}
}

void
EventWindow::Show()
{   
	if (visible) return;

	if (!plane)
		plane = New Plane(window);
	// force no plane background drawing
// HOOK
// Since the bitmap may not fit the plane
//	plane->SetPicNum(-3); 
	plane->SetPicNum(-2); 
	graphMgr->AddPlane(plane);
	SOL_Window::Show();
	graphMgr->FrameOut();
}

// ScrollWindow Code
// note ScrollWindows do not inherit from SOL_Windows but do contain a pointer to one

ScrollWindow::ScrollWindow(SOL_Rect& theWindow,
							  		SOL_Point thePos,	
									int		thePlaneID,
							  		ushort 	theFore,
							  		ushort 	theBack,
							  		ushort 	theFont,
							  		short	 	theMode,
									short		theBorder,
									int		theSize) :
					SOL_Window(theWindow, thePos, thePlaneID, "", theFore,
							  theBack, theFont, theMode, theBorder, False)
{
   refType = SCROLLWINDOW;
	end = 0;
   size = theSize;
	entries.Get(size);
	for (int i=0;i<size;++i) {
		entries[i].textID = (TextID)0;
		entries[i].font = SYSTEMFONT;
		entries[i].color = SOL_BLACK;
		entries[i].mode = TEJUSTCENTER;
	}
}

ScrollWindow::~ScrollWindow()
{
   for (int index = 0; index < end; index++) 
      entries[index].textID.Free();
	entries.Free();
}

TextID
ScrollWindow::Add(TextID textID, int font, int color, int mode, Bool jump)
{
	TextID newText;
	if(end < size) {
      end++;
	} else {
		text.Free();
      startChar -= strlen(*(entries[0].textID));
      entries[0].textID.Free();
		for(int i = 0; i < end - 1; i++) {
			entries[i].textID = entries[i + 1].textID;
			entries[i].font = entries[i + 1].font;     
			entries[i].color = entries[i + 1].color;
			entries[i].mode = entries[i + 1].mode;

			text.AddF("|s%d|",i);

			if (font != -1)    
				text.AddF("|f%d|",entries[i].font);

			if (color != -1)
				text.AddF("|c%d|",entries[i].color);

			if (mode != -1)
				text.AddF("|a%d|",entries[i].mode);
			
			text.Add(entries[i].textID);
		}
	}
	// Make a copy of the text
	TextID textCopy;
	textCopy.Add(textID);
	// Add a new entry
	entries[end - 1].textID = textCopy;
	entries[end - 1].font = font;
	entries[end - 1].color = color;
	entries[end - 1].mode = mode;

	// Put controls into new string
	newText.AddF("|s%d|",end - 1);
	if (font != -1)    
		newText.AddF("|f%d|",font);
	if (color != -1)
		newText.AddF("|c%d|",color);
	if (mode != -1)
		newText.AddF("|a%d|",mode);
	newText.Add(textID);
	text.Add(newText);

   if(jump)
      startChar = strlen(*text) - strlen(*newText);
   if (end)
	   SetText(NULL,True,startChar);
	newText.Free();
   return entries[end - 1].textID;
}

void
ScrollWindow::Delete(TextID key)
{
   int index;
   for (index = 0; index < end; index++) 
      if(entries[index].textID == key)
         break;

	if (index < end) {
		entries[index].textID.Free();
		end--;
		for(int i = index; i < end; i++) {
			entries[i].textID = entries[i + 1].textID;
			entries[i].font = entries[i + 1].font;     
			entries[i].color = entries[i + 1].color;
			entries[i].mode = entries[i + 1].mode;
		}
      entries[end].textID = (TextID)0;
		text.Free();
      int start = 0;
		for(i = 0; i < end; i++) {
			 
			if(i == index)
				start = strlen(*text);

			text.AddF("|s%d|",i);

			if (font != -1)    
				text.AddF("|f%d|",entries[i].font);

			if (fore != -1)
				text.AddF("|c%d|",entries[i].color);

			if (mode != -1)
				text.AddF("|a%d|",entries[i].mode);
			
			text.Add(entries[i].textID);
		}
		SetText(NULL,True,start);
	}

}
  
void
ScrollWindow::Clear()
{
	for(int i = 0; i < end; i++)
   	entries[i].textID.Free();
	end = 0;
   SetText("",True);
}

ScrollEntryID
ScrollWindow::CopyEntries()
{
	ScrollEntryID 	entriesCopy;
	TextID 			textCopy;
	char* 			target;
	char* 			source;
	size_t 			sizeMem;

	sizeMem = memMgr->GetSize((SOL_Handle) entries);

	entriesCopy.Get(size);
	target = (char*) memMgr->GetAddr((int) entriesCopy);
	source = (char*) memMgr->GetAddr((int) entries);
	memcpy(target,source,sizeMem);

	for (int i=0;i<size;++i) {
		if ((int) entries[i].textID) {
			// Make a copy
			sizeMem = memMgr->GetSize((SOL_Handle) entries[i].textID);
			textCopy.Get(sizeMem);

			target = (char*) memMgr->GetAddr((int) textCopy);
			source = (char*) memMgr->GetAddr((int) entries[i].textID);

			memcpy(target,source,sizeMem);

			entriesCopy[i].textID = textCopy;
		}
	}
	return entriesCopy;
}

void
ScrollWindow::Reconstruct(ScrollEntryID newEntries)
{
	entries.Free();
	entries = newEntries;
	for (int i=0;i<size;++i) {
		if (!(int) entries[i].textID) 
			break;
		text.Add(entries[i].textID);
      end++;
	}
	if ((int) entries[i].textID) 
	   SetText(NULL,True,startChar);
}

void
ScrollWindow::Go(int index, int scale)
{
   if(!scale)
		msgMgr->Fatal("Range must be greater than 0");

	if(index > scale) 
		msgMgr->Fatal("Index is Out of Range in ScrollWindow");

	uint* indices = (uint*) *lineIndicesID;
	startLine = (index * maxLines / scale) + 1; // add 1 to make 1 relative not zero
	startChar = indices[startLine];
	lastLine = startLine + linesPerPage - 1;
	if (lastLine > maxLines) lastLine = maxLines;
	lastChar = indices[lastLine+1] - 1;
	ReDrawText();

}

int
ScrollWindow::Where(int scale)
{
   if(maxLines)
	   return (startLine - 1) * scale / maxLines;
   return 0;
}

TextID
ScrollWindow::Insert(TextID key, TextID textID, int font, int color, int mode)
{
   int index;
   for (index = 0; index < end; index++) 
      if(entries[index].textID == key)
         break;

	if(index < end) {
		if(end < size) {
			for(int i = index; i < end; i++) {
			   entries[i + 1].textID = entries[i].textID;
			   entries[i + 1].font = entries[i].font;     
			   entries[i + 1].color = entries[i].color;
				entries[i + 1].mode = entries[i].mode;
			}   
			end++;
		} else {
			entries[0].textID.Free();
			for(int i = 0; i < index; i++) { 
			   entries[i].textID = entries[i + 1].textID;
			   entries[i].font = entries[i + 1].font;     
			   entries[i].color = entries[i + 1].color;
			   entries[i].mode = entries[i + 1].mode;
			}
		} 

		// Make a copy of the text
		TextID textCopy;
		textCopy.Add(textID);

		// Replace the entry
		entries[index].textID = textCopy;
		entries[index].font = font;
		entries[index].color = color;
		entries[index].mode = mode;
  
		text.Free();
      int start = 0;
		for(int i = 0; i < end; i++) {
			if(i == index)
				start = strlen(*text);
			text.AddF("|s%d|",i);
			if (font != -1)    
				text.AddF("|f%d|",entries[i].font);
			if (color != -1)
				text.AddF("|c%d|",entries[i].color);
			if (mode != -1)
				text.AddF("|a%d|",entries[i].mode);
			text.Add(entries[i].textID);
		}

		SetText(NULL,True,start);
      return entries[index].textID;
	}
   return 0;
}

TextID
ScrollWindow::Modify(TextID key, TextID textID, int font, int color, int mode, Bool jump)
{
   int index;
   for (index = 0; index < end; index++) 
      if(entries[index].textID == key)
         break;

	if(index < end) {

   	entries[index].textID.Free();
		// Make a copy of the text
		TextID textCopy;
		textCopy.Add(textID);
		entries[index].textID = textCopy;
		// Update entry
		entries[index].font = font;
		entries[index].color = color;
		entries[index].mode = mode;  
		text.Free();
      int start = 0;
		for(int i = 0; i < end; i++) {
			if(i == index)
				start = strlen(*text);
			text.AddF("|s%d|",i);
			if (font != -1)    
				text.AddF("|f%d|",entries[i].font);
			if (color != -1)
				text.AddF("|c%d|",entries[i].color);
			if (mode != -1)
				text.AddF("|a%d|",entries[i].mode);
			text.Add(entries[i].textID);
		} 
      if(jump)
         startChar = start;
		SetText(NULL,True,startChar);
      return entries[index].textID;
	}
   return (TextID)0;
}

TextID
ScrollWindow::Get(int index)
{
	if(index < end)
		return (entries[index].textID);
	else
      return (TextID)0;
} 
  
int
ScrollWindow::Which(TextID theText)
{
	for(int i = 0; i< end; i++) {
		if(theText == entries[i].textID)
      	return i;
	}
   return -1;
}
/*
TextID
ScrollWindow::Get(int x, int y)
{
	return (TextID)0;
}
*/
#if defined(DEBUG) || defined(NOT_SHIPPING_VERSION)
EventWindow	*
DebugWindow(char* 		text,
				char* 		title,
				int 			size,
				SOL_Point& 	pt,
				Bool 			wait,
				Bool* 		escape)
{
	// text is the text for the main box
	// title is the text for the title
	// size is the maximum width in pixels
	// pt is the screen position
	// if wait == True wait for input

	SOL_Rect	sr;
	SOL_Event dbgEvent;
	TextID theText;
	TextID theTitle;

	if (escape)
		*escape = False;
	// Make copies of the character strings
	theText.Add(text);
	if (title)
		theTitle.Add(title);

	// Calculate the title size
	// Adjust for a 3 pixel border
	fontMgr->SetFont(SYSTEMFONT);
	// Check that the window size will fit the largest character
	if (fontMgr->CharWidth('M') > (size - 4))
		msgMgr->Fatal(SrcLoc,Msg_WindowSizeTooSmall);

	// check title will fit in the given size (adjust for border)
	int titleWidth = title ? fontMgr->StringWidth(theTitle) + 4 : 0;
	if (titleWidth > size)
		msgMgr->Fatal(SrcLoc,Msg_WindowTitleTooBig);
	int titleHeight = title ? fontMgr->PointSize() + 3 : 0;

	SOL_Rect windowRect;
	fontMgr->TextSize(theText,size - 4,windowRect);
	// Adjust for a border
	windowRect.B.x = size -1;
	windowRect.B.y += 4;
	// Adjust the window rect to enclose the title rect
	if (windowRect.Xlen() < titleWidth)
		windowRect.B.x = titleWidth - 1;
	windowRect.B.y += titleHeight + 1;
	if (!title)
		windowRect.B.y++;

	// Make the windowRect screen relative
	windowRect.Offset(pt.x,pt.y);

	// If the window is too long for the screen reduce its height
	if (
		(windowRect.B.y > LOWRESY) &&
		(windowRect.Ylen() > (fontMgr->PointSize() + titleHeight))
		)
		windowRect.B.y = LOWRESY - 15;

	EventWindow* w =
		title ? New EventWindow(windowRect,
											 text,
											 SOL_BLACK,
											 SOL_WHITE,
											 SYSTEMFONT,
											 TEJUSTLEFT,
											 title,
											 SOL_WHITE,
											 SOL_BLACK,
											 SYSTEMFONT,
											 True)
		: New EventWindow(windowRect,
											 text,
											 SOL_BLACK,
											 SOL_WHITE,
											 SYSTEMFONT,
											 TEJUSTLEFT,
											 True);

	// Get rid of string copies
	theText.Free();
	theTitle.Free();

	if (!wait)
		return w;

	Bool esc = WaitOnUser(w);
	if (escape)
		*escape = esc;
	delete w;
	return 0;
}
#endif

Bool
WaitOnUser(EventWindow* w)
{
	if(!eventMgr)
		return False;

	SOL_Event userEvent;
	while (True) {
		eventMgr->Get(&userEvent,SOL_Event::MouseDown | SOL_Event::KeyDown);
		if (w && w->HandleEvent(userEvent)) 
			continue;
		if (
			(userEvent.type == SOL_Event::KeyDown) &&
			(
			(userEvent.message == Kbd::Enter) ||
			(userEvent.message == Kbd::Esc)
			)
			)
			return userEvent.message == Kbd::Esc;
		if (userEvent.type == SOL_Event::MouseDown)
			return userEvent.modifiers == Kbd::Shift;
		if (
			(userEvent.type == SOL_Event::KeyDown) &&
			(userEvent.message == 'S')
			)
			mapStamp = -1;
	}
}

void
KScrollWindow(argList)
{

	// arg(1) type of call      
   ObjectID obj;
	if(!arg(1)) { // call to create window, CREATE = 0

		ScrollWindowID scrollWindow;
        
		// arg(2) objID;
		// arg(3) is num text entries  
      if(arg(0) != 3)
         msgMgr->Fatal("Illegal number of arguments to ScrollWindow - CREATE");
		obj = arg(2);
		SOL_Rect window;
   	SOL_Point pt;
	
		pt.x = window.A.x = obj.GetProperty(s_nowSeenL);
		pt.y = window.A.y = obj.GetProperty(s_nowSeenT);
		window.B.x = obj.GetProperty(s_nowSeenR);
		window.B.y = obj.GetProperty(s_nowSeenB);

		scrollWindow.Get( window, pt,
						obj.GetProperty(s_planeID),
			  			obj.GetProperty(s_fore),
			  			obj.GetProperty(s_back),
			  			obj.GetProperty(s_font),
			  			obj.GetProperty(s_mode),
			  			obj.GetProperty(s_borderColor),
						arg(3) //size
					);
		pm.acc = scrollWindow; 

	} else {
		// arg(2) of all but the create call is the ScrollWindow ID
		ScrollWindowID scrollWindow((SOL_Handle) arg(2));

		if(scrollWindow->refType != SCROLLWINDOW)
         msgMgr->Fatal("Handle does not corispond to Scroll Window Type");
   
		SOL_Rect window;
		switch(arg(1)) {
			case ADD:  // add a text block to window
				// arg(3) is textID
				// arg(4) is font
				// arg(5) is color
            // arg(6) is allinment
            // arg(7) is optional Should Jump

            // HOOK
            // This statement should be surrounded by DEBUG
            // **********************************
		      if (!memMgr->IsValid(arg(3)))
			      msgMgr->Fatal(SrcLoc,Msg_InvalidHandle,arg(3));
            // **********************************

            if(arg(0) < 6)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - ADD\n numArgs = %d",arg(0));
            else if(arg(0) < 7)
					pm.acc = scrollWindow->Add(arg(3),arg(4),arg(5),arg(6));
            else
					pm.acc = scrollWindow->Add(arg(3),arg(4),arg(5),arg(6),arg(7));
				break;

			case CLEAR: // clear all entries to window
            if(arg(0) != 2)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - CLEAR");
				scrollWindow->Clear();
				break;

			case PAGEUP:
            if(arg(0) != 2)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - PAGEUP");
				scrollWindow->PageUp();
				break;

			case PAGEDOWN:
            if(arg(0) != 2)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - PAGEDOWN");
				scrollWindow->PageDown();
				break;

			case UPARROW:
            if(arg(0) != 2)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - UPARROW");
				scrollWindow->UpArrow();
				break;

			case DOWNARROW:
            if(arg(0) != 2)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - DOWNARROW");
				scrollWindow->DownArrow();
				break;

			case HOME:
            if(arg(0) != 2)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - HOME");
				scrollWindow->Home();
				break;

			case END:
            if(arg(0) != 2)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - END");
				scrollWindow->End();
				break;

			case RESIZE:   
            if(arg(0) != 3)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - RESIZE"); 
		      obj = arg(3);
				window.A.x = obj.GetProperty(s_nowSeenL);
				window.A.y = obj.GetProperty(s_nowSeenT);
				window.B.x = obj.GetProperty(s_nowSeenR);
				window.B.y = obj.GetProperty(s_nowSeenB);
				scrollWindow->ResizeWindow(window);
				break;

			case WHERE: // Given the range, Where in that range would the thumb lie
            if(arg(0) != 3)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - WHERE");
				pm.acc = scrollWindow->Where(arg(3));
				break;

			case GO: // Goto the window position corisponding to a given thumb and range
            if(arg(0) != 4)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - GO");
				scrollWindow->Go(arg(3), arg(4));
				break;

         case INSERT:
            if(arg(0) != 7)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - INSERT");
				pm.acc = scrollWindow->Insert(arg(3),arg(4),arg(5),arg(6),arg(7));
            break;

         case DELETE:
            if(arg(0) != 3)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - DELETE");
				scrollWindow->Delete(arg(3));
            break;

         case MODIFY:
            if(arg(0) == 7)
					pm.acc = scrollWindow->Modify(arg(3),arg(4),arg(5),arg(6),arg(7));
            else if(arg(0) == 8)
					pm.acc = scrollWindow->Modify(arg(3),arg(4),arg(5),arg(6),arg(7),arg(8));
            else
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - MODIFY");
            break;

			case HIDE: 
            if(arg(0) != 2)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - HIDE");
				scrollWindow->Hide();
				break;

			case SHOW: 
            if(arg(0) != 2)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - SHOW");
				scrollWindow->Show();
				break;

			case DESTROY:
            if(arg(0) != 2)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - DESTROY");
				scrollWindow->Hide();
				scrollWindow.Free();
				break;

			case TEXT: {
            if(arg(0) != 2)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - TEXT");
				pm.acc = (Acc) scrollWindow->CopyEntries();
				break;
			}

			case RECONSTRUCT:
            if(arg(0) != 3)
               msgMgr->Fatal("Illegal number of arguments to ScrollWindow - RECONSTRUCT");
				scrollWindow->Reconstruct(arg(3));
				break;

			default:
         	msgMgr->Alert("Illegal Procedure In ScrollWindow Kernel Call");
         	break;

		}
   }
}


#ifdef DEBUG
EventWindow*
CenterXWindow(char* contents,char* title,int width,int top,Bool wait)
{
	//	return a DebugWindow centered along x axis

   // for testing scrollwindow only
   //TestWindow();
	return DebugWindow(contents,
							 title,
							 width,
							 SOL_Point((320 - width)/2,top),
							 wait);
}

void
PaletteWindow()
{
	#define boxWidth	10
	#define boxHeight	5

	SOL_Event evt;

	// allocate memory for the bitmap
	MemID palID;
	palID.Get(MemBitmap,256*boxWidth*boxHeight+CELHEADERSIZE);

	// build the header for the BM
	BuildHeader(palID,16*boxWidth,16*boxHeight,SOL_SKIP);

	uchar* palAddr  = ((uchar*) memMgr->GetAddr(palID)) + CELHEADERSIZE;

	for (int i = 0;i < 16;i++) {
		for (int j = 0;j < boxHeight;j++) {
			for (int k = 0;k < 16;k++) {
				int color = i*16 + k;
				for (int l = 0;l < boxWidth;l++) {
					*palAddr++ = color;
				}
			}
		}
	}

	SOL_Rect window(0,0,16*boxWidth-1,16*boxHeight-1);
	Plane* plane = New Plane(window);
// HOOK
// Since the bitmap may not fit the plane
//	plane->SetPicNum(-3); 
	plane->SetPicNum(-2); 
	graphMgr->AddPlane(plane);

	Bitmap bitmap(palID);
	SOL_Point pt(0,0);
	ScaleInfo scaleInfo;
	ScreenItem* si = New ScreenItem(plane->Id(),bitmap,pt,scaleInfo);

	graphMgr->AddScreenItem(si);
	graphMgr->FrameOut();

	while (True) {
		eventMgr->Get(&evt,SOL_Event::MouseDown | SOL_Event::KeyDown);
  		if (
  			(evt.type == SOL_Event::KeyDown) &&
  			(
  			(evt.message	== Kbd::Enter) ||
  			(evt.message	== Kbd::Esc)
  			)
  			)

  			break;
  		if (evt.type ==	SOL_Event::MouseDown) {
			if (!window.Intersect(evt.where))
				break;
			SOL_Point mouseStart = evt.where;
			// mouseDown in the title bar
			// move the dialog until a mouse up
			while (True) {
				// Poll the mouse position
				SOL_Point mousePt;
				mouse->GlobalPosSci(&mousePt);
				if (mousePt != mouseStart) {
					SOL_Point delta(mousePt.x - mouseStart.x,mousePt.y - mouseStart.y);
					// Move the window
					window.Offset(delta.x,delta.y);
					plane->MoveDelta(delta.x,delta.y);
					graphMgr->UpdatePlane(plane);
					graphMgr->FrameOut();
					mouseStart = mousePt;
				}
				eventMgr->Get(&evt,SOL_Event::All);
				if (evt.type == SOL_Event::MouseUp) break;
			}
		}
  	}

	graphMgr->DeletePlane(plane);
	memMgr->Free(palID);
	graphMgr->FrameOut();
}

void
MemoryWindow()
{

	const	rows = 80;		 	// Matches the constants in memmgr.cpp
	const cols = 100;		 	// Matches the constants in memmgr.cpp
	const indexXPixels = 3;	// Matches the constants in memmgr.cpp
	const indexYPixels = 2;	// Matches the constants in memmgr.cpp
	SOL_Event evt;

	MemID memMapID = memMgr->MemoryBitMap();

	SOL_Rect window(0,0,cols*indexXPixels-1,(rows+1)*indexYPixels-1);
	Plane* plane = New Plane(window);
// HOOK
// Since the bitmap may not fit the plane
//	plane->SetPicNum(-3); 
	plane->SetPicNum(-2); 
	graphMgr->AddPlane(plane);

	Bitmap bitmap(memMapID);
	SOL_Point pt(0,0);
	ScaleInfo scaleInfo;
	ScreenItem* si = New ScreenItem(plane->Id(),bitmap,pt,scaleInfo);

	graphMgr->AddScreenItem(si);
	graphMgr->FrameOut();

	while (True) {
		eventMgr->Get(&evt,SOL_Event::MouseDown | SOL_Event::KeyDown);
  		if (
  			(evt.type == SOL_Event::KeyDown) &&
  			(
  			(evt.message	== Kbd::Enter) ||
  			(evt.message	== Kbd::Esc)
  			)
  			)

  			break;
  		if (evt.type ==	SOL_Event::MouseDown) {
			if (!window.Intersect(evt.where))
				break;
			SOL_Point mouseStart = evt.where;
			// mouseDown in the title bar
			// move the dialog until a mouse up
			while (True) {
				// Poll the mouse position
				SOL_Point mousePt;
				mouse->GlobalPosSci(&mousePt);
				if (mousePt != mouseStart) {
					SOL_Point delta(mousePt.x - mouseStart.x,mousePt.y - mouseStart.y);
					// Move the window
					window.Offset(delta.x,delta.y);
					plane->MoveDelta(delta.x,delta.y);
					graphMgr->UpdatePlane(plane);
					graphMgr->FrameOut();
					mouseStart = mousePt;
				}
				eventMgr->Get(&evt,SOL_Event::All);
				if (evt.type == SOL_Event::MouseUp) break;
			}
		}
  	}

	graphMgr->DeletePlane(plane);
	memMgr->Free(memMapID);
	graphMgr->FrameOut();
}

#endif
