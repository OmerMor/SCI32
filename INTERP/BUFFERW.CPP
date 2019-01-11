#include "sol.hpp"
#include "bufferw.hpp"
#include "rect.hpp"
#include "video.hpp"
#include "graphmgr.hpp"

#ifdef WIN32S
extern	Bool	bDIBInverted;

uchar*
BufferWin::Addr(const int x, const int y) const
{
	if (bDIBInverted)
		return start + ((yDim - y - 1) * xDim) + x;
	else
		return start + (y * xDim) + x;
}
#endif


BufferWin::BufferWin(int xdim, int ydim, uchar* addr) : Buffer(xdim, ydim)
{
	start = addr;
}


BufferWin::BufferWin(const BufferWin& src) : Buffer(src)
{
	start = src.start;
}

BufferWin&
BufferWin::operator=(const BufferWin& src)
{
	start = src.start;
	return *this;
}
