#ifndef PDEVICE_HPP
#define PDEVICE_HPP

class SOL_Cursor;
class SOL_Point;
class SOL_Rect;

class PointerDevice
{
	public:
	virtual	~PointerDevice() {}
	virtual	Bool	Exists() const = 0;
	virtual	int	GlobalPos(SOL_Point*) const = 0;

	virtual	void	SetRestrictRect(const SOL_Rect&) = 0;
	virtual	void	ClearRestrictRect() = 0;
	virtual	void	SetPos(const SOL_Point&) = 0;
	virtual	void	Setup(SOL_Cursor&) = 0;
};

#endif
