#ifndef BUFFER_HPP
#define BUFFER_HPP

class SOL_Rect;

class Buffer
{
 public:
	virtual	uchar*	Addr(const int x, const int y) const = 0;
   virtual  void     Clear(uchar) const = 0;
	virtual	Bool		IsNull() const = 0;
	int		XDim() const							{return xDim;}
	int		YDim() const							{return yDim;}
	#ifdef DEBUG
	Bool		Assert() const							{return xDim > 0  &&  yDim > 0;}
	#endif

	uchar*	start;

 protected:
 	Buffer(int xdim, int ydim)						{xDim = xdim; yDim = ydim;}
	Buffer(const Buffer& src)						{xDim = src.xDim; yDim = src.yDim;}
	int		xDim;
	int		yDim;
};

#endif
