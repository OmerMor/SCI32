#ifndef WINDOW_HPP
#define WINDOW_HPP

#ifndef FONT_HPP
#include "font.hpp"
#endif

#ifndef PALETTE_HPP
#include "palette.hpp"
#endif

#ifndef POINT_HPP
#include "point.hpp"
#endif

#ifndef RECT_HPP
#include "rect.hpp"
#endif

#ifndef TEXTID_HPP
#include "textid.hpp"
#endif

class Plane;
class ScreenItem;
class SOL_Event;
#define SCROLLWINDOW 0xF0CC

struct ScrollEntry {
	TextID 	textID;
	int 		font;
	int 		color;
	int 		mode;
};

FakeTemplateID(FakeScrollEntryID, ScrollEntry,MemWindowEntry)
class ScrollEntryID : public FakeScrollEntryID
{
   public:
	ScrollEntryID() {}
	ScrollEntryID(MemID& id) : FakeScrollEntryID(id) {}
	ScrollEntryID(const ScrollEntryID& id) : FakeScrollEntryID((MemID&) id) {}
	ScrollEntryID(SOL_Handle h) : FakeScrollEntryID(h) {}


	void
	Get(size_t size, MemAttrs attrs = MOVEABLE, unsigned num = 0,
			MemClient* client = 0, char initChar = 0,
			ReservedHandle h = IllegalHandle)
	{
		FakeScrollEntryID::Get(size * sizeof(ScrollEntry), attrs, num, client, initChar, h);
	}

	void
	Get(size_t size, ReservedHandle h, MemAttrs attrs = 0)
	{
		FakeScrollEntryID::Get(size * sizeof(ScrollEntry), h, attrs);
	}

	void
	Get(MemID id, size_t ofs, size_t size)
	{
		FakeScrollEntryID::Get(id, ofs, size * sizeof(ScrollEntry));
	}

	ScrollEntry& operator[](size_t s) const
	{
		return *(ScrollEntry*)(&FakeScrollEntryID::operator[](s * sizeof(ScrollEntry)));
	}

};

class SOL_Window
{
	public:

	SOL_Window() {}

	// Window with a title 
	SOL_Window(SOL_Rect& theWindow,	 // main info
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
				  Bool 		makeVisible);

	// Window without a title 
	SOL_Window(SOL_Rect& theWindow, 
				  SOL_Point thePos,	
				  int			thePlaneID,
				  char* 		theText,
				  ushort 	theFore,
				  ushort 	theBack,
				  ushort 	theFont,
				  short	 	theMode,
              short		theBorder,
				  Bool 		makeVisible);

	virtual ~SOL_Window();

	virtual void Hide();
	virtual void Move(SOL_Point pt);
	virtual void ResizeWindow(SOL_Rect& newWindow);
	virtual void SetText(char* theText,Bool showIt = True, int theStartChar = 0);
	virtual void Show();
	virtual void PageUp();
	virtual void PageDown();
	virtual void Home();
	virtual void End();
	virtual void UpArrow();
	virtual void DownArrow();

	protected:

	FontMgr*			fntMgr;		// The font manager for this window
	MemID				fontBitmap;	// the bitmap
	TextID	  		text;			// the text associated with the fontBox
	uint				textSize;	// the length in characters of the given text
	SOL_Rect			textBox;		// the inset rectangle for drawing text
	TextID	  		title;		// the text associated with the title

	int				startLine;	// the first line in the textbox	(1 relative)
	int				lastLine;	// the last line in the textbox (1 relative)
	int				linesPerPage; // the maximum lines per page
	int				maxLines;	// the number of lines in the text
	int				startChar;	// the first character in the textbox
	int				lastChar;	// the last character in the textbox
	int				xRes;			// Resolution of final bitmap
	int				yRes;       // Used to avoid round off errors

	Bool				visible;		// TRUE if in animate
	TextID	  		workText;	// the text currently in the fontBitmap

	ScreenItem*    si;			// The sitem for the fontBitmap
	SOL_Rect			window;  	// the planes coordinates screen relative
	SOL_Rect			scaleWindow;// the bitmaps coordinates in resolution of font
	Plane*	  		plane;		// plane associated with the fontBitmap
   SOL_Point		pos;			// plane relative position

	SOL_Point		mouseStart;	// The last mouse position
	MemID				lineIndicesID;// The indices into the text 
										// corresponding to a line start
	MemID				fontIndicesID;// in the lines above fonts default may differ
										// from one line to another
	MemID				colorIndicesID;// in the lines above colors default may differ
										// from one line to another
	MemID				modeIndicesID;// in the lines above modes default may differ
										// from one line to another
	int				font;			// Got to know which font the window was
										// Created with for meta char fonts
	int				fore;			// Got to know which color the window was
										// Created with for meta char fonts
	int				mode;			// Got to know which mode the window was
										// Created in for meta char fonts
	int				border;		// the border color if it exists

	int            pointSize;	// the intial point size used to look for errors

	Bool 	BackAWord();
	Bool 	ForwardAWord();
	virtual void	GetLineIndices();
	virtual void 	ReDrawText();
};

class EventWindow : public SOL_Window
{

	public:
	// Window without a title
	EventWindow(SOL_Rect& theWindow,	 // main info
				  char* 		theText,
				  ushort 	theFore,
				  ushort 	theBack,
				  ushort 	theFont,
				  short	 	theMode,
				  char*		theTitle,	 // Title info
				  ushort 	theTitleFore,
				  ushort 	theTitleBack,
				  ushort 	theTitleFont,
				  Bool 		makeVisible):
		SOL_Window(theWindow,SOL_Point(0,0),0,theText,theFore,theBack,theFont,theMode,SOL_BLACK,
					  theTitle,theTitleFore,theTitleBack,theTitleFont,False)
                 {
	                  if (makeVisible) Show();
                 }

	// Window without a title 
	EventWindow(SOL_Rect& theWindow,
				  char* 		theText,
				  ushort 	theFore,
				  ushort 	theBack,
				  ushort 	theFont,
				  short	 	theMode,
				  Bool 		makeVisible):
		SOL_Window(theWindow,SOL_Point(0,0),0,theText,theFore,theBack,theFont,theMode,SOL_BLACK,
					  False)
                 {
	                  if (makeVisible) Show();
                 }

	~EventWindow();
	
	Bool HandleEvent(SOL_Event& theEvent);
	void Hide();
	void Move(SOL_Point pt);
	void ResizeWindow(SOL_Rect& newWindow);
	void Show();

};

class ScrollWindow : public SOL_Window
{
	public:
	int   refType;    // This value is checked at the kernel call
	ScrollWindow() {}

	ScrollWindow(SOL_Rect& theWindow, SOL_Point thePos, int thePlaneID,
					ushort theFore, ushort theBack, ushort theFont, short theMode,
					short theBorder, int theSize);
	 
	~ScrollWindow(); 
	TextID			Add(TextID textID,
							 int font,
							 int color,
							 int mode,
							 Bool jump = True);
	void				Delete(TextID);
	void				Clear();
	TextID			Insert(TextID key,
								 TextID textID,
								 int font,
								 int color,
								 int mode);
	TextID			Modify(TextID key,
								 TextID textID,
								 int font,
								 int color,
								 int mode,
								 Bool jump = True);
	TextID			Get(int index);
	int				Which(TextID);
	int				Where(int scale);
	void				Go(int index, int scale);
	ScrollEntryID  CopyEntries();
	void				Reconstruct(ScrollEntryID newEntries);
	ScrollEntryID	Entries()	{return entries;}

	protected:

	int	   		end;
   ScrollEntryID  entries;
	int	   		size; 
};

// Create ScrollWindowID Class for scroll window
FakeTemplateID(FakeScrollWindowID, ScrollWindow, MemWindow)
class ScrollWindowID : public FakeScrollWindowID
{
	public:
	ScrollWindowID() {}
	ScrollWindowID(const MemID& id) : FakeScrollWindowID(id) {}
	ScrollWindowID(const ScrollWindowID& id) : FakeScrollWindowID((MemID&) id) {}
	ScrollWindowID(SOL_Handle h) : FakeScrollWindowID(h) {}

	void Get(SOL_Rect& theWindow,
				SOL_Point thePos,
				int		thePlaneID,
				ushort 	theFore,
				ushort 	theBack,
				ushort 	theFont,
				short	 	theMode,
				short		theBorder,
				int		theSize);
	void Free();
};

enum {    // enums for KScrollWindow kernel call
	CREATE,
	ADD, 
	CLEAR,
	PAGEUP,  
	PAGEDOWN,
	UPARROW, 
	DOWNARROW,
	HOME,     
	END,
	RESIZE,
	WHERE,  
	GO,
   INSERT,
   DELETE,
   MODIFY,
	HIDE,
	SHOW,    
	DESTROY,
	TEXT,
	RECONSTRUCT
};


#ifdef DEBUG
	// external subroutines:
	EventWindow*	CenterXWindow(char* contents,
								  	char* title,
								  	int width,
								  	int top,
								  	Bool wait = False);

	EventWindow* DebugWindow(char* contents,
									char* title,
									int width,
									SOL_Point& loc,
									Bool wait = False,
									Bool* escape = NULL);
	void			MemoryWindow();
	void 			PaletteWindow ();
#endif

Bool			WaitOnUser(EventWindow* w = NULL);
//Bool			WaitOnUser(ScrollWindow* w);
// for scroll window testing only
//void        TestWindow();

#endif  //WINDOW_HPP
