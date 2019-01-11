/*



	 The purpose of the PlaneDef class is to be an object that
	 the callers of GraphManager can use to create or change a plane.

	 It includes this information:
			Resolution of plane
			Size and position of plane on screen
			Background pic number  (if any)
			Background color
			Vanishing point

	 The class user must also use the plane ID in the ScreenItemDef
	 object to associate a particular ScreenItem with a plane.

*/

#ifndef PLANEDEF_HPP
#define PLANEDEF_HPP

#ifndef RECT_HPP
#include "rect.hpp"
#endif

#ifndef POINT_HPP
#include "point.hpp"
#endif

#ifndef CELOBJ_HPP
#include "celobj.hpp"
#endif



class ScreenItem;
class ObjectID;

class PlaneDef
{
	public:

	enum {coloredPlane,picturePlane,transparentPlane,opaquePlane};

	//  Constructors, etc
	PlaneDef(ObjectID);
	PlaneDef(int xdim, int ydim);
	PlaneDef(const SOL_Rect&);
	virtual	~PlaneDef()  {}

	//  Const Methods
	long			Id() const									{return planeId;}
	int			Priority() const							{return priority;}
	uchar			Color() const								{return color;}
	Bool			operator==(const PlaneDef&) const;
	Bool			operator!=(const PlaneDef& src)const {return !operator==(src);}
	SOL_Point	GetVanish() const							{return vanish;}
	int			PicResNum() const							{return picResNum;}
	const SOL_Rect&	DefRect() const					{return plane;}
	const	SOL_Rect&	GameRect() const					{return gameRect;}

	//  Normal methods
	PlaneDef&	operator=(const PlaneDef&);
	void	SetColor(uchar c)							{color = c;}
	void	SetPriority(int pri)						{priority = pri;}
	void	SetVanish(SOL_Point pt)					{vanish = pt;}
	virtual	void	SetPicNum(int num)			{picResNum = num;SetType();}
	virtual	void	SetType();
	virtual	void	SetPicMirror(Bool f)			{picMirror = f;}
	virtual	void	Resize(int xdim, int ydim);
	virtual	void	MoveTo(SOL_Point ul);
	virtual	void	MoveDelta(int x, int y);

	protected:
	friend class Plane;
	void		ConvGameRectToPlaneRect();
	static long nextId;

	long			planeId;
	int			type;					//	coloredPlane
											// picturePlane
											// transparentPlane
											// opaquePlane
	SOL_Rect		plane, gameRect;

	SOL_Point	vanish;				//  Vanishing point (scale is local)
	int			picResNum;        //  
	int			priority;
	uchar 		color;				//  Background color if no pic associated
	Bool			picMirror;			//  Mirror state for planedef
};


inline void
PlaneDef::MoveDelta(int x, int y)
{
	gameRect.Offset(x, y);
	ConvGameRectToPlaneRect();
}


#endif
