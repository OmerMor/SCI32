#ifndef RECT_HPP
#define RECT_HPP

#include "point.hpp"

class SOL_Rect
{
	public:
	SOL_Point A;		//Left most point of the rectangle
	SOL_Point B;		//Right most point of the rectangle

	//  Constructors
	SOL_Rect()  {}
	SOL_Rect(SOL_Point& left,SOL_Point& right);
	SOL_Rect(int xa, int ya, int xb, int yb);

	//  Const Methods
	int	IsEmpty() const;
	int	Xlen() const;
	int	Ylen() const;
	long	Area() const;
	Bool	Intersect(SOL_Point P) const;
	Bool	Intersect(const SOL_Rect&) const;
	Bool	Intersect(const SOL_Rect&, SOL_Rect* clip) const;
	int	operator==(const SOL_Rect&) const;
	int	operator!=(const SOL_Rect&) const;
	int   Split(SOL_Rect& super,SOL_Rect* rectangles);

	//  Normal methods
	void	MakeEmpty();
	void	Set(int xa, int ya, int xb, int yb);
	void	MoveTo(int xa, int ya);
	void	Adjust();
	void	Clip(const SOL_Rect&);
	void	Both(const SOL_Rect&);
	void	Include(const int x, const int y);
	Bool	Includes(const SOL_Rect&);
	void	ToGlobal(const SOL_Rect&);
	void	ToLocal(const SOL_Rect&);
	void	Offset(int dx, int dy);
	void	Inset(int dx, int dy);
	void	Scale(int nx, int dx, int ny, int dy);
};


//  Constructor  -  Allows normal creation
//  --------------------------------------
inline
SOL_Rect::SOL_Rect(int xa, int ya, int xb, int yb)
{
	A.x = xa;
	A.y = ya;
	B.x = xb;
	B.y = yb;
}

inline
SOL_Rect::SOL_Rect(SOL_Point& left,SOL_Point& right)
{
	A = left;
	B = right;
}

//  Set  -  Allows user to initialize rectangle area
//  ------------------------------------------------
inline void
SOL_Rect::Set(int xa, int ya, int xb, int yb)
{
	A.x = xa;
	A.y = ya;
	B.x = xb;
	B.y = yb;
}

//  Adjust  -  Corrects coordinates so A is upper left, B lower right
//  ---------------------------------------------------------------------
inline void
SOL_Rect::Adjust()
{
	if (A.x > B.x) {
		int temp = B.x;
		B.x = A.x;
		A.x = temp;
	}

	if (A.y > B.y) {
		int temp = B.y;
		B.y = A.y;
		A.y = temp;
	}
}

//  Empty  -  Test to see if rectangle is empty
//  -------------------------------------------
inline int
SOL_Rect::IsEmpty() const
{
	if (B.x < A.x  ||  B.y < A.y)
		return True;
	return False;
}

//  MakeEmpty  -  Sets rectangle to emtpy state
//  -------------------------------------------
inline void
SOL_Rect::MakeEmpty()
{
	B.x = -1;
	B.y = -1;
	A.x = 0;
	A.y = 0;
}

//  Xlen  -  Returns X length of rectangle
//  --------------------------------------
inline int
SOL_Rect::Xlen() const
{
	return B.x - A.x + 1;
}

//  Ylen  -  Returns Y length of rectangle
//  --------------------------------------
inline int
SOL_Rect::Ylen() const
{
	return B.y - A.y + 1;
}

//  Area  -  Returns area of rectangle
//  ----------------------------------
inline long
SOL_Rect::Area() const
{
	return (long) Xlen() * Ylen();
}

//  Intersect  -  Tests to see if a point is within this rectangle
//  --------------------------------------------------------------
inline Bool
SOL_Rect::Intersect(SOL_Point P) const
{
	if (P.x < A.x  ||  P.x > B.x)
		return False;
	if (P.y < A.y  ||  P.y > B.y)
		return False;
	return True;
}

//  Intersect  -  Test to see if a rectangle conflicts with this one
//  ----------------------------------------------------------------
inline Bool
SOL_Rect::Intersect(const SOL_Rect &r) const
{
	if (B.x < r.A.x)
		return False;
	if (B.y < r.A.y)
		return False;
	if (A.x > r.B.x)
		return False;
	if (A.y > r.B.y)
		return False;
	return True;
}

//  operator==  -  Compare for equality
//  -----------------------------------
inline int
SOL_Rect::operator==(const SOL_Rect& r) const
{
	if (A.x == r.A.x  &&  A.y == r.A.y  &&  B.x == r.B.x  &&  B.y == r.B.y)
		return True;
	return False;
}

//  operator!=  -  Compare for inequality
//  -------------------------------------
inline int
SOL_Rect::operator!=(const SOL_Rect& r) const
{
	return !operator==(r);
}


//  Clip  -  Clips this rectangle with specified rectangle
//  ------------------------------------------------------
inline void
SOL_Rect::Clip(const SOL_Rect &r)
{
	if (Intersect(r))  {
		if (A.x < r.A.x)
			A.x = r.A.x;
		if (A.y < r.A.y)
			A.y = r.A.y;
		if (B.x > r.B.x)
			B.x = r.B.x;
		if (B.y > r.B.y)
			B.y = r.B.y;
	}
	else
		MakeEmpty();
}


//  Both  -  Enlarges this rectangle to also encompass arg rect
//  -----------------------------------------------------------
inline void
SOL_Rect::Both(const SOL_Rect &r)
{
	if (A.x > r.A.x)
		A.x = r.A.x;
	if (A.y > r.A.y)
		A.y = r.A.y;
	if (B.x < r.B.x)
		B.x = r.B.x;
	if (B.y < r.B.y)
		B.y = r.B.y;
}

//  Include  -  Enlarges this rectangle to also encompass arg point
//  ---------------------------------------------------------------
inline void
SOL_Rect::Include(const int x, const int y)
{
	if (A.x > x)
		A.x = x;
	if (A.y > y)
		A.y = y;
	if (B.x < x)
		B.x = x;
	if (B.y < y)
		B.y = y;
}


//  Includes  -  True if given rectangle completely 
// 				  encompassed by this rectangle
//  ---------------------------------------------------------------
inline Bool
SOL_Rect::Includes(const SOL_Rect& r)
{
	return  (Intersect(r.A) && Intersect(r.B));
}

//  ToGlobal  -  Convert this rectangle to global based on arg rect
//  ---------------------------------------------------------------
inline void
SOL_Rect::ToGlobal(const SOL_Rect &r)
{
	A.x += r.A.x;
	A.y += r.A.y;
	B.x += r.A.x;
	B.y += r.A.y;
}

//  MoveTo - Move rect so its upper left corner is at a given point
//  ---------------------------------------------------------------
inline void
SOL_Rect::MoveTo(int xa, int ya)
{
	B.x += xa - A.x;
	B.y += ya - A.y;
	A.x = xa;
	A.y = ya;
}

//  ToLocal  -  Convert this rectangle to local based on arg rect
//  -------------------------------------------------------------
inline void
SOL_Rect::ToLocal(const SOL_Rect &r)
{
	A.x -= r.A.x;
	A.y -= r.A.y;
	B.x -= r.A.x;
	B.y -= r.A.y;
}

//  Offset  -  Offset Rectangle by dx, dy
//  -------------------------------------
inline void
SOL_Rect::Offset(int dx, int dy)
{
	A.x += dx;
	B.x += dx;
	A.y += dy;
	B.y += dy;
}

//  Inset  -  Shrink rectangle in by dx, dy
//  ---------------------------------------
inline void
SOL_Rect::Inset(int dx, int dy)
{
	A.x += dx;
	B.x -= dx;
	A.y += dy;
	B.y -= dy;
}

//  Scale  -  scale each point by numerator and denominator
//  -------------------------------------------------------
inline void
SOL_Rect::Scale(int nx, int dx, int ny, int dy)
{
	A.x = A.x * nx / dx;
	B.x = B.x * nx / dx;
	A.y = A.y * ny / dy;
	B.y = B.y * ny / dy;
}

#endif
