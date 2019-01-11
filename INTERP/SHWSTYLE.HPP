//	shwstyle.hpp

#ifndef SHWSTYLE_HPP
#define SHWSTYLE_HPP

#ifndef MEMID_HPP
#include "memid.hpp"
#endif

#ifndef POINT_HPP
#include "point.hpp"
#endif

class SOL_Rect;
class ScreenItem;


class PlaneScroll
{
public:
	PlaneScroll()	{speed = 0;}
	int			  	planeID;		// the plane that has the show style
	int				xStep;		// the step size in the horizontal direction
	int				xDim;			// How far to scroll in the x direction
	int				yStep;		// the step size in the vertical direction
	int				yDim;			// How far to scroll in the y direction
	int				newPic;		// the picture scrolling in
	int				oldPic;		// the picture scrolling out
	Bool			  	animate;		// if true do processing on SCI frameouts
	int			  	speed;		// ticks between each process of the BM
	ulong			  	clock;	 	// next time to process the BM
	PlaneScroll*	next;			// another plane with a show style
};

class PlaneShowStyle
{
public:
	PlaneShowStyle() {percent = 0;si = NULL;}
	int					planeID;		// the plane that has the show style
	int					style;		// the show style
	Bool					entry;		// entry or exit style
	int					form;			// physical form type for the show style
	MemID					BM;			// the screen items bitmap
	int					BMWidth;		// the bitmap's width
	int					BMHeight;  	// the bitmap's height
	int					speed;		// ticks between each process of the BM
	Bool					animate;		// if true do processing on SCI frameouts
	ulong					clock;	 	// next time to process the BM
	uint					percent;		// percent of processing done
	uint					divisions;	// the number of divisions for this show style
	ScreenItem**		si;			// The array of screen item def pointers
	int					tilesPerFrame;	// Tiles changed on each frameout
	int					pDMask;     // Pixel dissolve mask (based on plane size)
	ulong					pixel;		// The pixel for pixel dissolve
	ulong					firstPixel;	// The start pixel for pixel dissolve
	uchar					color;		// The color for the style
	ScreenItem*			siPlane;		// screen item for the plane
	PlaneShowStyle*	next;			// another plane with a show style
	Bool					deleteMe;	// style no longer needed
};

// show pic styles
enum planeShowStyles	{
	PLAINFORM,
	HSHUTTEROUT,			
	HSHUTTERIN,			
	VSHUTTEROUT,			
	VSHUTTERIN,			
	WIPELEFT,			
	WIPERIGHT,		
	WIPEUP,			
	WIPEDOWN,			
	IRISOUT,			
	IRISIN,			
	DISSOLVE,			
	PIXELDISSOLVE,	
	FADEOUT,		
	FADEIN,
	PALETTEMORPH,		
	LASTSTYLE = PALETTEMORPH
};

// show pic forms
enum planeShowForms	{
	PLAIN,
	HSHUTTER,			
	VSHUTTER,			
	HWIPE,			
	VWIPE,		
	IRIS			
};

void		DissolveSetUp(PlaneShowStyle* style,int priority,SOL_Rect& planeRect);
Bool 		Dissolve(PlaneShowStyle* style);
void		DissolveShutDown(PlaneShowStyle* style) ;
Bool		Fade(int direction,PlaneShowStyle* style);
void 		FillPoint(char* celPtr,SOL_Point& pt,uchar color,int BMWidth);
void 		FillRect(char* celPtr,SOL_Rect& r,uchar color,int BMWidth);
void		GenericKill(PlaneShowStyle* style);
Bool		GenericProcess(int direction,PlaneShowStyle* style);
void		GenericShutDown(PlaneShowStyle* style);
Bool		Plain(PlaneShowStyle* style);
Bool 		ProcessScroll(PlaneScroll* scroll);
void		ProcessScrolls();
Bool 		ProcessShowStyle(PlaneShowStyle*,uint time);
void 		ProcessShowStyles();
void		HShutterSetUp(PlaneShowStyle* style,int priority) ;
void		HWipeSetUp(PlaneShowStyle* style,int priority);
void		IrisSetUp(PlaneShowStyle* style,int priority);
void		KillStyle(PlaneShowStyle* style);
void 		PixelDissolveSetUp(PlaneShowStyle* style,int priority,SOL_Rect& planeRect);
Bool 		PixelDissolve(PlaneShowStyle* style);
PlaneShowStyle* ShutDownStyle(PlaneShowStyle* style);
void		VShutterSetUp(PlaneShowStyle* style,int priority);
void		VWipeSetUp(PlaneShowStyle* style,int priority);
Bool		PalMorph(PlaneShowStyle* style);
void     ProcessEffects(PlaneShowStyle* style=0);
void		PixelDissolveEffect(PlaneShowStyle* style=0);
void     WipeDownEffect(PlaneShowStyle* style=0);
void     WipeUpEffect(PlaneShowStyle* style=0);
void     WipeRightEffect(PlaneShowStyle* style=0);
void     WipeLeftEffect(PlaneShowStyle* style=0);
void     HShutterInEffect(PlaneShowStyle* style=0);
void     HShutterOutEffect(PlaneShowStyle* style=0);
void     VShutterInEffect(PlaneShowStyle* style=0);
void     VShutterOutEffect(PlaneShowStyle* style=0);
void     IrisInEffect(PlaneShowStyle* style=0);
void     IrisOutEffect(PlaneShowStyle* style=0);
int      Bitwidth(int number);
// List of planes that have a show style in progress
extern 	PlaneScroll* 		planeScrolls;
extern 	PlaneShowStyle* 	showStyles;
extern 	int 					showStyleDivisions[];
extern 	int 					showStyleForms[];
extern 	int 					pixelDissolveTable[];
extern   char					palMorphArray[];
#endif

