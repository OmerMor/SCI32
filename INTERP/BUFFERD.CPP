#include "sol.hpp"
#include "bufferd.hpp"
#include "video.hpp"
#include "graphmgr.hpp"



BufferStd::BufferStd(int xdim, int ydim, uchar* addr) : Buffer(xdim, ydim)
{
	start = addr;
}


BufferStd::BufferStd(const BufferStd& src) : Buffer(src)
{
	start = src.start;
}

BufferStd&
BufferStd::operator=(const BufferStd& src)
{
	start = src.start;
	return *this;
}
