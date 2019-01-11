#ifndef LINE_HPP
#define LINE_HPP

#ifndef RECT_HPP
#include "rect.hpp"
#endif

#ifndef MEMID_HPP
#include "memid.hpp"
#endif

class SOL_Line
{
	// The line goes from (x1,y1) to (x2,y2)
	// NOTE!	**** A.x <= B.x for all lines ****
	public:

	enum {tSolid, tDashed, tPattern};

	SOL_Point A;		//Left most point of the line
	SOL_Point B;		//Right most point of the line

	//  Constructors
	SOL_Line(int x1,int y1,int x2,int y2);
	SOL_Line(SOL_Point pt1,SOL_Point pt2);

	//  Const Methods
	void	DrawThin(uchar color,
				  		int style,
				  		int pattern,
				  		MemID BM,
				  		int BMWidth);

	void	DrawThick(uchar color,
				  		 int thickness,
				  		 int style,
				  		 int pattern,
				  		 MemID BM,
				  		 int BMWidth);
#define NOINTERSECT
#ifndef NOINTERSECT
	Bool	RectInterior(SOL_Rect& r);
	Bool	IsEmpty()		{return (A.x > B.x);}
	Bool 	LineIntersect(SOL_Line& L);
	Bool	RectIntersect(SOL_Rect& theRect);

	protected:
	Bool	HorizontalIntersect(int rectAx,int rectBx,int recty);
	Bool	VerticleIntersect(int rectAy,int rectBy,int rectx);
#endif
};

inline
SOL_Line::SOL_Line(int x1,int y1,int x2,int y2)
{
	// All lines have A.x <= B.x
	if (x1 <= x2) {
		A.x=x1;
		A.y=y1;
		B.x=x2;
		B.y=y2;
	}
	else {
		A.x=x2;
		A.y=y2;
		B.x=x1;
		B.y=y1;
	}
}

inline
SOL_Line::SOL_Line(SOL_Point pt1,SOL_Point pt2)
{
	if (pt1.x <= pt2.x) {
		A = pt1;
		B = pt2;
	}
	else {
		A = pt2;
		B = pt1;
	}
}

#ifndef NOINTERSECT
inline Bool
SOL_Line::RectInterior(SOL_Rect& r)
{
	// Determine if this line lines completely
	// interior to the given rectangle
	if ((A.x < r.A.x) ||
		 (B.x > r.B.x) || 
		 (A.y < r.A.y) || 
		 (A.y > r.B.y) ||
		 (B.y < r.A.y) ||
		 (B.y > r.B.y))

		 return False;
	return True;
}
#endif

#endif
