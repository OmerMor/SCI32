//	graph.hpp

#ifndef GRAPH_HPP
#define GRAPH_HPP

#ifndef MEMID_HPP
#include "memid.hpp"
#endif

#ifndef GRAPHMGR_HPP
#include "graphmgr.hpp"
#endif

#ifndef POINT_HPP
#include "point.hpp"
#endif

#ifndef LIST_HPP
#include "list.hpp"
#endif

class SOL_Rect;
class ScreenItem;
class RectList;
					  
void		BuildHeader(uchar* bmAddr, int width, int height, int skip,
							int xHot = 0,
							int yHot = 0,
							int xRes = SCIRESX,
							int yRes = SCIRESY,
							Bool palFlag = False,
							Bool remap = False);

inline void		BuildHeader(MemID bm, int width, int height, int skip,
									int xHot = 0,
									int yHot = 0,
									int xRes = SCIRESX,
									int yRes = SCIRESY,
									Bool palFlag = False,
									Bool remap = False)
{
	BuildHeader((uchar*) *bm, width, height, skip, xHot, yHot, xRes, yRes, palFlag, remap);
}

MemID		BuildLineBitmap(int x1,
								 int y1,
								 int x2,
								 int y2,
								 uchar color,
								 int style,
								 int pattern,
								 int thickness,
								 SOL_Rect& rect);
MemID		BuildPolygonBitmap(SOL_ListID theList,
									 uchar color,
									 int style,
									 int pattern,
									 int thickness,
									 SOL_Rect& rect,
									 Bool close);
int		GetNumLoops(MemID);
int		GetNumCels(MemID,int);

#endif
