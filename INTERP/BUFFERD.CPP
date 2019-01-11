#include "sol.hpp"
#include "bufferd.hpp"
#include "video.hpp"
#include "graphmgr.hpp"

uchar*
BufferStd::AddrSimRes(int x, int y) const
{
	y = (y * realResY) / simResY;
	x = (x * realResX) / simResX;
	return start + (y * xDim) + x;
}


BufferStd::BufferStd(int xdim, int ydim, uchar* addr) : Buffer(xdim, ydim)
{
	start = addr;
	simResX = LOWRESX;
	simResY = LOWRESY;
	realResX = xdim;
	realResY = ydim;
}


BufferStd::BufferStd(const BufferStd& src) : Buffer(src)
{
	start = src.start;
	simResX = src.simResX;
	simResY = src.simResY;
	realResX = src.realResX;
	realResY = src.realResY;
}

BufferStd&
BufferStd::operator=(const BufferStd& src)
{
	start = src.start;
	simResX = src.simResX;
	simResY = src.simResY;
	realResX = src.realResX;
	realResY = src.realResY;
	return *this;
}

