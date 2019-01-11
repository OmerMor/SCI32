#ifndef GRAPHMGR_HPP
#define GRAPHMGR_HPP

#ifndef PLANE_HPP
#include "plane.hpp"
#endif

#ifndef SHWSTYLE_HPP
#include "shwstyle.hpp"
#endif

#ifndef SCREEN_HPP
#include "screen.hpp"
#endif

#ifndef SAVEABLE_HPP
#include "saveable.hpp"
#endif

#ifndef REMAP_HPP
#include "remap.hpp"
#endif

#ifndef BUFFERD_HPP
#include "bufferd.hpp"
#endif

#ifndef ROBOT_HPP
#include "robot.hpp"
#endif

class PaletteMgr;
class SOL_Cursor;
class RectList;
class MagnifyView;


#ifdef HI_RES
const SCIRESX = 640;
const SCIRESY = 480;
const XSCALE  = 2;
const YSCALE  = 2.4;
#else
const SCIRESX = 320;
const SCIRESY = 200;
const XSCALE  = 1;
const YSCALE  = 1;
#endif

const LOWRESX = 320;
const LOWRESY = 200;

class GraphicsMgr : public Saveable
{
 public:
	// Constructor and destructor
	GraphicsMgr(int xdim, int ydim);
	virtual 	~GraphicsMgr();

	static 	Bool	drawToPriMap;
   MagnifyView* 	magHead;
	Bool				vmapValid;
	Bool           morphOn;

	void		AddPlane(Plane* plane)				{next.AddPlane(plane);}
	void		AddScreenItem(ScreenItem* si)		{next.AddScreenItem(si);}
	int		AddPic(long planeID, int picNum, SOL_Point& p,int mirror)
					{return next.AddPic(planeID,picNum,p,mirror);}
	void		DeleteScreenItem(ScreenItem* si);
	void		DeleteScreenItem(ScreenItem* si,int planeId);
	void		DeletePic(long planeID,int oldPic,int newPic)	
					{next.DeletePic(planeID,oldPic,newPic);}
	void		DeletePlane(int planeId);
	void		DeletePlane(Plane*);
	Plane* 	GetPlaneNext(int planeId) 			{return next.GetPlane(planeId);}
	Remap&	GRemapper() 							{return remapper;}
	Robot&	GRobot();
	void		InitRobot(int resNum, long planeID, int priority = -1, int offx = 0, int offy = 0);
	Bool		IsOnMe(ScreenItem&,Plane&,const SOL_Point&,Bool checkSkip);
	void		KillRobot();
	Screen*	NextScreen() 							{return &next;}
	void		NewScreenDim(int xdim, int ydim);
	void		NowSeen(int id, int planeID, SOL_Rect& nowSeen) 
					{next.NowSeen(id,planeID,nowSeen);}
	void		PlaneRect(int planeID, SOL_Rect& planeRect) 	
					{next.PlaneRect(planeID,planeRect);}
	void		RemapMarkRedraw()						{next.RemapMarkRedraw();}
	int		RobotStatus() const;
	void		SaveRestore(SaveGame*, Bool save);
	void		SaveRestoreDo(Saveable::Func, SaveGame*, Bool save);
	void		ScrollScreenItems(long planeID, int xStep, int yStep,Bool movePic = True)
					{next.ScrollScreenItems(planeID,xStep,yStep,movePic);}
	void		TimingTest();
	int		TopPlane() 								{return next.TopPlane();}
	int		TopSCIPlane() 							{return next.TopSCIPlane();}
	int		TopScreenItem(int planeID) 		{return next.TopScreenItem(planeID);}
	void		UpdatePlane(Plane*);
	void		UpdateScreenItem(ScreenItem*);
	int		Xdim() const							{return next.Xdim();}
	int		Ydim() const							{return next.Ydim();}

	virtual  void     	AlterVMAP(SOLPalette* myPal,SOLPalette* newPal,int palIndex, char* palArray=0) = 0;
	virtual	void			ClearLastScreen() = 0;
	virtual	Buffer*		CurrBuffer() = 0;
	virtual	void			FrameOut(Bool showBits = True,
									SOL_Rect dirtyRect = SOL_Rect(0,0,-1,-1));
	virtual	SOL_Cursor&	GCursor() = 0;
	virtual	Plane* 		GetPlaneCurrent(int planeId) = 0;
	virtual	PaletteMgr&	GPalette() = 0;
	virtual	void			MovieFrameOut(uchar*	Bmp,
												  int	bitmapWidth,
												  int bitmapX1,
												  int bitmapY1,
												  int bitmapX2,
												  int bitmapY2,
												  int screenX1,
												  int screenY1) = 0;
	virtual	void			MovieFrameOutDbl(uchar*	Bmp,
												  	  int	bitmapWidth,
												  	  int bitmapX1,
												  	  int bitmapY1,
												  	  int bitmapX2,
												  	  int bitmapY2,
												  	  int screenX1,
												  	  int screenY1) = 0;
	virtual  void	 		PalMorphFrameOut(char* palArray=0,PlaneShowStyle* style=0);
	virtual	Buffer& 		PriMap() = 0;
	virtual	void			PlayMovie(char* filename,int tickCount,int xOff=0,int yOff=0) = 0;
	virtual	void			Repaint(const SOL_Rect&) = 0;
	virtual	int			ScreenCount() const = 0;
	virtual	void			SetVideoForMovies() = 0;
	virtual	void			SetVideoForSpeed() = 0;
	virtual	void			ShakeScreen(int, int) = 0;
#ifdef DEBUG
#ifndef WINDOWS
	virtual	char*			SnapShot(char* filename) = 0;
#endif
#endif
	virtual	void			ShowBits() = 0;  // for showstyles only
	virtual	Screen&		VisibleScreen() = 0;

 protected:
	void	CreatePriMap();
	void	DeletePriMap();
	void	DrawEraseList(const RectList&, const Plane&);
	void	DrawScreenItemList(DrawList&);
	void	MergeToShowList(const SOL_Rect&, RectList&, int callOverhead = 0);

	virtual	void	CalcLists(DrawList** drawLists, RectList** eraseLists,SOL_Rect& dirtyRect) = 0;
	virtual	void	ResizeVmap(int xdim, int ydim) = 0;
	virtual	void	UpdateScreen() = 0;

	Bool			lastRemapFlag;
	Screen		next;
	int			pixelsPerCall;		//	Used for animation tuning
   Remap			remapper;
	Bool			resizeFlag;
	short    	timingFlag;			// Used for animation tuning
	Robot*		robot;
};

// The graphpics manager
extern GraphicsMgr* graphMgr;

#endif
