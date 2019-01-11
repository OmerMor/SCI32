#ifndef BUFFERD_HPP
#define BUFFERD_HPP

#include <string.h>

#ifndef BUFFER_HPP
#include "buffer.hpp"
#endif

class BufferStd : public Buffer
{
 public:
	BufferStd(int xdim, int ydim, uchar*);
	BufferStd(const BufferStd&);
	BufferStd& operator=(const BufferStd&);

   void  Clear(uchar c) const {memset(start, c, xDim*yDim);}
	Bool	IsNull() const 		{return start == NULL;}

	uchar*	Addr(const int x, const int y) const;
};

inline uchar*
BufferStd::Addr(const int x, const int y) const
{
	return start + (y * xDim) + x;
}

#endif
