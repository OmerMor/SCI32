#ifndef GRAPHMD_HPP
#define GRAPHMD_HPP

#ifndef GRAPHMGR_HPP
#include "graphmgr.hpp"
#endif

#ifndef VGA_HPP
#include "vga.hpp"
#endif

#ifndef PALMGRD_HPP
#include "palmgrd.hpp"
#endif

#ifndef CURSORD_HPP
#include "cursord.hpp"
#endif

#ifndef BUFFERD_HPP
#include "bufferd.hpp"
#endif

#ifndef TIMED_HPP
#include "timed.hpp"
#endif

#ifndef VIDEO_HPP
#include "video.hpp"
#endif

class GraphicsMgrDOS : public GraphicsMgr
{
	public:
	GraphicsMgrDOS(int xdim,int ydim,int vesaMode,Bool usePri);
	~GraphicsMgrDOS();

	void			ClearLastScreen()			{last.Clear();}
	SOL_Cursor&	GCursor()					{return *cursor;}
	Plane* 		GetPlaneCurrent(int planeId)
						{return last.GetPlane(planeId);}
	PaletteMgr&	GPalette()					{return palmgr;}
	void			NewScreenDim(int xdim,int ydim);
	void			PlayMovie(char* filename,int tickCount,int x = 0,int y = 0);
	Buffer& 		PriMap() 					{return pmap;}
	void			Repaint(const SOL_Rect&);
	int  			ScreenCount() const     {return 1;}
	void			SetVideoForMovies();
	void			SetVideoForSpeed();
	void			ShakeScreen(int num,int dir) 
						{::ShakeScreen(num,dir,(int *)&(((TimeMgrDOS *)timeMgr)->GetTickRef()));}
#ifdef DEBUG
	char*			SnapShot(char* filename);
#endif
	void			UpdateScreen()			   {last = next;}
	Screen&		VisibleScreen()   		{return last;}

	void     AlterVMAP(SOLPalette* myPal,int palIndex,char* palArray);

	protected:
	void			CalcLists(DrawList** drawLists,RectList** eraseLists)
						{next.CalcLists(last,drawLists,eraseLists);}
	void			CreatePriMap();
	Buffer*		CurrBuffer()            {return &buffer;}
	void			DeletePriMap();
	void			ResizeVmap(int xdim,int ydim);
	void			ShowBits();
	void			TimingTest(const BufferStd&);

	BufferStd		buffer;	// VMAP
	CursorDOS*		cursor;
	Screen			last;
	Bool				modeX;
	PaletteMgrDOS	palmgr;
	BufferStd		pmap;		// PMAP
	MemID				pmapID;
	Bool				vesa;
	Vga				vga;
	MemID				vmapID;
};

#endif
