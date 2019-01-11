#ifndef GRAPHMW_HPP
#define GRAPHMW_HPP

#ifndef GRAPHMGR_HPP
#include "graphmgr.hpp"
#endif

//#ifndef VGA_HPP
//#include "vga.hpp"
//#endif

#ifndef PALMGRW_HPP
#include "palmgrw.hpp"
#endif

#ifndef CURSORW_HPP
#include "cursorw.hpp"
#endif

#ifndef BUFFERW_HPP
#include "bufferw.hpp"
#endif

class GraphicsMgrWin : public GraphicsMgr
{
	public:
	GraphicsMgrWin(int xdim, int ydim);
	~GraphicsMgrWin();

	int	ScreenCount() const	{return 1;}
	const Screen&	VisibleScreen() const	{return last;}
	const Screen&   NextScreen() const   	{return next;}
	Screen*	CurrScreen() 						{return &last;}
	void	UpdateScreen();
	Screen&		VisibleScreen()   			{return last;}
//	Bool	PlaneExistsLast(int planeId)		{return last.PlaneExists(planeId);}
	void	PlayMovie(char* filename, int tickCount, int xoff = 0, int yoff = 0);
	void	SetVideoForMovies();
	void	SetVideoForSpeed(); 
	void	ShakeScreen(int, int)				{}
	void	Repaint(const SOL_Rect&);
	void  ClearLastScreen()						{}
//	PaletteMgr&	GPalette()						{return palmgr;}
	PaletteMgr&	GPalette()						{return *palmgr;}
	SOL_Cursor&	GCursor()						{return *cursor;}
	void		SCursorClr();
	void		SCursorWin();
	void		TimingTest();
	Buffer& 	PriMap() 							{return pmap;}
	void     AlterVMAP(const SOLPalette* myPal, const SOLPalette* newPal, int palIndex, char* palArray);

	protected:
	void			CalcLists(DrawList** drawLists,RectList** eraseLists,SOL_Rect& dirtyRect)
						{next.CalcLists(last,drawLists,eraseLists,dirtyRect);}
	void		ResizeVmap(int xdim, int ydim);
//	void		Repaint(SOL_Rect& theRect);
	void		ShowBits();
	void		ShowRect(const SOL_Rect *, uchar *);
	Buffer*	CurrBuffer()						{return &buffer;}

	Screen			last;
	BufferWin		buffer;			// VMAP
//	Vga				vga;
//	PaletteMgrWin	palmgr;
	PaletteMgr*		palmgr;
	CursorWin*		cursor;
	CursorWin*		clrCursor;
	CursorWin*		winCursor;
	Bool				modeX;
	MemID				vmapID;
	BufferWin		pmap;
	MemID				pmapID;
};

_Packed struct BMI {
	BITMAPINFOHEADER	bmih;
	union {
		WORD			colorix[256];
		RGBQUAD		rgbq[256];
	};
};

extern	BMI	bmiHdr;
extern	uint	colorUse;


inline void	
GraphicsMgrWin::UpdateScreen()
{
	last = next;
}
#endif
