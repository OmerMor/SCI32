//	mouse.hpp

#ifndef MOUSE_HPP
#define MOUSE_HPP

#ifndef PTRDEV_HPP
#include "ptrdev.hpp"
#endif


class SOL_Point;
class SOL_Rect;

class SOL_Mouse : public PointerDevice
{
 protected:
	SCI_Rect * hotRectangles;
	int hotRectangleCount;
	int inRect;
	int hotOn;

 public:
 	SOL_Mouse();
	virtual ~SOL_Mouse() {}

	virtual	Bool	Exists() const = 0;
	virtual	int	GlobalPos(SOL_Point*) const = 0;
	virtual	int	GlobalPosSci(SOL_Point*) const = 0;
	virtual	int	GetSwiftInfo(int*, int*, int*, int*) const {return 0;}

	virtual	void	SetRestrictRect(const SOL_Rect&) = 0;
	virtual	void	ClearRestrictRect() = 0;
	virtual	void	SetPos(const SOL_Point&) = 0;
	virtual	void	Setup(SOL_Cursor&) = 0;
	virtual	Bool	IsSwift()const {return False;}
	virtual	void	Vibrate(int , int = 1, int = 1) const {}
	virtual void	SetHotRectangleStatus(int status);
	virtual void	SetupHotRectangles(int count,MemID handle);
	virtual void	CheckHotRectangles(SOL_Point & pos);
};

extern SOL_Mouse* mouse;

#endif

