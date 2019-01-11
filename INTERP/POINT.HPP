//	point.hpp

#ifndef POINT_HPP
#define POINT_HPP

typedef int	Coord;

struct SOL_Point {

	enum { DefaultCoord = 0x89ABCDEF };

	SOL_Point() : x(DefaultCoord), y(DefaultCoord) {}
	SOL_Point(Coord x, Coord y) : x(x), y(y) {}
	SOL_Point(const SOL_Point& p) : x(p.x), y(p.y) {}

	SOL_Point&	operator=(const SOL_Point&);

	SOL_Point&	operator+=(const SOL_Point&);
	SOL_Point&	operator-=(const SOL_Point&);
	Bool			operator==(const SOL_Point&) const;
	Bool			operator!=(const SOL_Point&) const;

	SOL_Point	operator+(const SOL_Point&) const;
	SOL_Point	operator-(const SOL_Point&) const;
	SOL_Point	operator-() const;
	SOL_Point	operator*(const int&) const;
	int         operator*(const SOL_Point&) const;
	int         operator^(const SOL_Point&) const;

	Coord	x;
	Coord	y;
};

inline SOL_Point&
SOL_Point::operator=(const SOL_Point& p)
{
	x = p.x;
	y = p.y;
	return *this;
}

inline SOL_Point
SOL_Point::operator+(const SOL_Point& p) const
{
   return SOL_Point(x+p.x,y+p.y);
}

inline SOL_Point
SOL_Point::operator-(const SOL_Point& p) const
{
   return SOL_Point(x-p.x,y-p.y);
}

inline SOL_Point
SOL_Point::operator-() const
{
   return SOL_Point(-x,-y);
}

// Scalar product
inline SOL_Point
SOL_Point::operator*(const int& p) const
{
   return SOL_Point(x*p,y*p);
}

// Dot product
inline int
SOL_Point::operator*(const SOL_Point& p) const
{
   return ((int)x*(int)p.x+(int)y*(int)p.y);
}

// Cross product
// Actually just returns the Z direction for (x1,y1,0) cross (x2,y2,0)
inline int
SOL_Point::operator^(const SOL_Point& p) const
{
   int direction;
   direction = (
               (((int) x) * ((int) p.y)) 
               - 
               (((int) y) * ((int) p.x))
               );
   if (direction < 0)
      return(-1);
   if (direction > 0)
      return(1);
   return(0);
}

inline SOL_Point&
SOL_Point::operator+=(const SOL_Point& p)
{
	x += p.x;
	y += p.y;
	return *this;
}

inline SOL_Point&
SOL_Point::operator-=(const SOL_Point& p)
{
	x -= p.x;
	y -= p.y;
	return *this;
}

inline Bool
SOL_Point::operator==(const SOL_Point& p) const
{
	return x == p.x  &&  y == p.y;
}

inline Bool
SOL_Point::operator!=(const SOL_Point& p) const
{
	return x != p.x  ||  y != p.y;
}

struct SCI_Point {

	enum { DefaultCoord = 0xABCD };

	SCI_Point() : x(DefaultCoord), y(DefaultCoord) {}
	SCI_Point(Coord x, Coord y) : x(x), y(y) {}
	SCI_Point(const SCI_Point& p) : x(p.x), y(p.y) {}

	SCI_Point&	operator=(const SCI_Point&);

	SCI_Point&	operator+=(const SCI_Point&);
	SCI_Point&	operator-=(const SCI_Point&);
	Bool			operator==(const SCI_Point&) const;
	Bool			operator!=(const SCI_Point&) const;

	SCI_Point	operator+(const SCI_Point&) const;
	SCI_Point	operator-(const SCI_Point&) const;
	SCI_Point	operator-() const;
	SCI_Point	operator*(const int&) const;
	int         operator*(const SCI_Point&) const;
	int         operator^(const SCI_Point&) const;

	//	a coordinate must fit in a property
	SCIWord	x;
	SCIWord	y;
};

inline SCI_Point&
SCI_Point::operator=(const SCI_Point& p)
{
	x = p.x;
	y = p.y;
	return *this;
}

inline SCI_Point
SCI_Point::operator+(const SCI_Point& p) const
{
   return SCI_Point(x+p.x,y+p.y);
}

inline SCI_Point
SCI_Point::operator-(const SCI_Point& p) const
{
   return SCI_Point(x-p.x,y-p.y);
}

inline SCI_Point
SCI_Point::operator-() const
{
   return SCI_Point(-x,-y);
}

// Scalar product
inline SCI_Point
SCI_Point::operator*(const int& p) const
{
   return SCI_Point(x*p,y*p);
}

// Dot product
inline int
SCI_Point::operator*(const SCI_Point& p) const
{
   return ((int)x*(int)p.x+(int)y*(int)p.y);
}

// Cross product
// Actually just returns the Z direction for (x1,y1,0) cross (x2,y2,0)
inline int
SCI_Point::operator^(const SCI_Point& p) const
{
   int direction;
   direction = (
               (((int) x) * ((int) p.y)) 
               - 
               (((int) y) * ((int) p.x))
               );
   if (direction < 0)
      return(-1);
   if (direction > 0)
      return(1);
   return(0);
}

inline SCI_Point&
SCI_Point::operator+=(const SCI_Point& p)
{
	x += p.x;
	y += p.y;
	return *this;
}

inline SCI_Point&
SCI_Point::operator-=(const SCI_Point& p)
{
	x -= p.x;
	y -= p.y;
	return *this;
}

inline Bool
SCI_Point::operator==(const SCI_Point& p) const
{
	return x == p.x  &&  y == p.y;
}

inline Bool
SCI_Point::operator!=(const SCI_Point& p) const
{
	return x != p.x  ||  y != p.y;
}

#endif
