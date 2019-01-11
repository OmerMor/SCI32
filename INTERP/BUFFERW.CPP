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

uchar*
BufferWin::AddrSimRes(int x, int y) const
{
	y = (y * realResY) / simResY;
	x = (x * realResX) / simResX;
	return start + ((yDim - y - 1) * xDim) + x;
}


BufferWin::BufferWin(int xdim, int ydim, uchar* addr) : Buffer(xdim, ydim)
{
	start = addr;
	simResX = SCIRESX;
	simResY = SCIRESY;
	realResX = xdim;
	realResY = ydim;
}


BufferWin::BufferWin(const BufferWin& src) : Buffer(src)
{
	start = src.start;
	simResX = src.simResX;
	simResY = src.simResY;
	realResX = src.realResX;
	realResY = src.realResY;
}

BufferWin&
BufferWin::operator=(const BufferWin& src)
{
	start = src.start;
	simResX = src.simResX;
	simResY = src.simResY;
	realResX = src.realResX;
	realResY = src.realResY;
	return *this;
}


