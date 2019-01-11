#include "sol.hpp"
#include "rect.hpp"

//  Intersect
//  Test to see if a rectangle conflicts with this one.
//  If there is an intersect return the intersect rectangle
//  -------------------------------------------------------
Bool
SOL_Rect::Intersect(const SOL_Rect& r, SOL_Rect* clip) const
{
	if ((B.x<r.A.x) || (B.y<r.A.y) || (A.x>r.B.x) || (A.y>r.B.y))
		return False;
	*clip = *this;
	// Calculate overlap
	if (A.x < r.A.x)
		clip->A.x = r.A.x;
	if (A.y < r.A.y)
		clip->A.y = r.A.y;
	if (B.x > r.B.x)
		clip->B.x = r.B.x;
	if (B.y > r.B.y)
		clip->B.y = r.B.y;
	return True;
}



int
SOL_Rect::Split(SOL_Rect& super,SOL_Rect* rectangles)
{
	/* This routine breaks apart this rectangle into multiple rectangles based
	 	upon the overlap of the super rectangle. Resulting rectangles are not
	 	overlaped by the super rectangle. 
	 	The return value is:
			-1 if no overlap.
		 	0 if this rectangle is completely overlaped.
		 	1,2,3 or 4 if this rectangle is partially overlaped.
	*/

	int	i = -1;		

	if (Intersect(super)) {
		SOL_Rect r = *this;
		++i;

		// Check top and bottom first because the run length
		// encoding has row pointers
		if (r.A.y < super.A.y)	{
			rectangles[i].A.y = A.y;
			rectangles[i].A.x = A.x;
			rectangles[i].B.y = super.A.y - 1;
			rectangles[i++].B.x = B.x;
			r.A.y = super.A.y;
		}

		if (r.B.y > super.B.y)	{
			rectangles[i].A.y = super.B.y + 1;
			rectangles[i].A.x = r.A.x;
			rectangles[i].B.y = r.B.y;
			rectangles[i++].B.x = r.B.x;
			r.B.y = super.B.y;
		}

		if (r.A.x < super.A.x)	{
			rectangles[i].A.y = r.A.y;
			rectangles[i].A.x = r.A.x;
			rectangles[i].B.y = r.B.y;
			rectangles[i++].B.x = super.A.x - 1;
			r.A.x = super.A.x;
		}

		if (r.B.x > super.B.x)	{
			rectangles[i].A.y = r.A.y;
			rectangles[i].A.x = super.B.x + 1;
			rectangles[i].B.y = r.B.y;
			rectangles[i++].B.x = r.B.x;
//			r.B.x = super.B.x;
		}
	}
	return (i);
}




Bool
SOL_Rect::IsWithin(int xdim, int ydim) const
{
	if (A.x < 0  ||  A.y < 0)
		return False;

	if (B.x >= xdim  ||  B.y >= ydim)
		return False;

	return True;
}
