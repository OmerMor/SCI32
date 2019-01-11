#ifndef CURSOR_HPP
#define CURSOR_HPP

#ifndef SAVEABLE_HPP
#include "saveable.hpp"
#endif

#ifndef CELOBJ_HPP
#include "celobj.hpp"
#endif

class SOL_Rect;
class SOL_Point;
class PointerDevice;


class SOL_Cursor : public Saveable
{
	public:
	virtual	~SOL_Cursor() {}
	virtual	void		Hide() = 0;
	virtual	void		UnHide() = 0;
	virtual	void		Show() = 0;
	virtual	void		Set(int v, int l, int c) = 0;
	virtual	Bitmap&	Get() = 0;
	virtual	void		DeviceMovedTo(const SOL_Point&) = 0;		//  Called via interrupts
	virtual	void		ForcePos(SOL_Point) = 0;
	virtual	void		GonnaPaint(SOL_Rect) = 0;
	virtual	void		PaintStarting() = 0;
	virtual	void		DonePainting() = 0;
	virtual	void		SetRestrictRect(const SOL_Rect&) = 0;
	virtual	void		ClearRestrictRect() = 0;
	virtual	void		SetDevice(PointerDevice&) = 0;
	virtual	int		GetHideCount() = 0;
};


#endif
