#include "sol.hpp"

#include "celobj.hpp"
#include "graph.hpp"
#include "line.hpp"
#include "math.hpp"
#include "palette.hpp"

void
SOL_Line::DrawThin(uchar color,int style,int pattern,MemID BM,int BMWidth)
{
	// Mid Point algorithm used
	// See:
	//		Foley,van Dam,Feiner,Hughes 
	//		"Computer Graphics principles and practice" Second Edition
	//

	// To insure the slope is between 0 and 1, one of the following
	//	transformations will be used
	/*

															 
					  *				 |					*
					   *				 |				  *
					    *	  x = y	 |	x = y		 *
					  	  *  y = -x	 |	y = x		*
					  	   *			 |	        *
					  	    *		    |		    *
					        *		 |       *
					     	   *		 |      *
					  		    *	    |	    *
					x = -x  	  *	 |    *	 x = x
					y = y	  		*	 |	  *	 y = y	
					  			    *  |  *
					  				  * | *
					  				   *|*
	           ------------------------------------
										*|*
									  * |	*
					x = -x		 *	 |	 *		 x = x
					y = -y		*	 |	  *	 y = -y	
								  *	 |		*
								 *		 |		 *
								*		 |		  *
							  *		 |			*
							 *			 |			 *
							*			 |			  *
						  *			 |				*
						 *	  x = -y	 |	x = -y	 *
						*	  y = -x	 |	y = x		  *
					  *				 |					*
	*/
	// All lines are draw from left to right so four
	// of the above transformations will not occur

	// Set pattern	if not given
	switch (style) {
		case tSolid:
			pattern = -1;
			break;
		case tDashed:
			pattern = 0xff00ff00;
			break;
		case tPattern:
			break;
	}
	int P[32];
	int pIndex = 0;
	for (;pIndex<32;++pIndex) {
		if (pattern < 0 )
			P[pIndex] = 1;
		else
			P[pIndex] = 0;
		pattern = pattern << 1;
	}

	// Calculate the first bitmap pixel address
	uchar* BMAddr = ((uchar*) memMgr->GetAddr(BM)) + CELHEADERSIZE;

	// Optimize for horizontal and vertical lines
	if ((A.x-B.x) == 0) {
		// Vertical line
		pIndex = 0;
		int yStart = Min(A.y,B.y);
		int yEnd = Max(A.y,B.y);
		// Calculate the first line pixel address
		uchar* pixelAddr = BMAddr + yStart*BMWidth + A.x;
		while (yStart <= yEnd) {
			if (P[(pIndex++ % 32)])
				*pixelAddr = color;
			++yStart;
			pixelAddr += BMWidth;
		}
		return;
	}
	if ((A.y-B.y) == 0) {
		// Vertical line
		pIndex = 0;
		// Calculate the first line pixel address
		uchar* pixelAddr = BMAddr + A.y*BMWidth + A.x;
		int xStart = A.x;
		while (xStart <= B.x) {
			if (P[(pIndex++ % 32)])
				*pixelAddr = color;
			++xStart;
			++pixelAddr;
		}
		return;
	}

	// Calculate the first line pixel address
	uchar* pixelAddr = BMAddr + A.y*BMWidth + A.x;

	// Set up the transformation
	int T[2][2];
	T[0][0] = 1;
	T[0][1] = 0;
	T[1][0] = 0;
	T[1][1] = 1;

	if ((B.y-A.y) < 0)
		T[1][1] = -1;
	if (((B.y-A.y)*T[1][1]) > (B.x-A.x)) {
		int t00 = T[0][0];
		int t10 = T[1][0];
		T[0][0] = T[0][1];
		T[1][0] = T[1][1];
		T[0][1] = t00;
		T[1][1] = t10;
	}

	// transform so that 0 <= Slope <= 1
	SOL_Point start(T[0][0]*A.x + T[1][0]*A.y,T[0][1]*A.x + T[1][1]*A.y);
	SOL_Point end(T[0][0]*B.x + T[1][0]*B.y,T[0][1]*B.x + T[1][1]*B.y);


	// Slope is positive less equal to 1
	int dx = end.x-start.x;
	int dy = end.y-start.y;

	int d = 2*dy - dx;		// initial value of d
	int incrE =2*dy;			// increment for move East
	int incrNE = 2*(dy-dx);	// increment for move North East
	int x = start.x;
	int y = start.y;

	int xOffset = T[0][0] + T[1][0]*BMWidth;
	int yOffset = T[0][1] + T[1][1]*BMWidth;

	// Write first pixel
	if (P[0])
		*pixelAddr = color;
	// Write the remainder of the line
	pIndex = 1;
	while (x < end.x) {
		if (d <= 0) {
			d += incrE;
			++x;
			pixelAddr += xOffset;
		}
		else {
			d += incrNE;
			++x;
			++y;
			pixelAddr += yOffset + xOffset;
		}
		// Output new point
		if (P[(pIndex++ % 32)])
			*pixelAddr = color;
	}
}

void
SOL_Line::DrawThick(uchar color,int thickness,int style,int pattern,MemID BM,int BMWidth)
{
	// Mid Point algorithm used
	// See:
	//		Foley,van Dam,Feiner,Hughes 
	//		"Computer Graphics principles and practice" Second Edition
	//

	// To insure the slope is between 0 and 1, one of the following
	//	transformations will be used
	/*

															 
					  *				 |					*
					   *				 |				  *
					    *	  x = y	 |	x = y		 *
					  	  *  y = -x	 |	y = x		*
					  	   *			 |	        *
					  	    *		    |		    *
					        *		 |       *
					     	   *		 |      *
					  		    *	    |	    *
					x = -x  	  *	 |    *	 x = x
					y = y	  		*	 |	  *	 y = y	
					  			    *  |  *
					  				  * | *
					  				   *|*
	           ------------------------------------
										*|*
									  * |	*
					x = -x		 *	 |	 *		 x = x
					y = -y		*	 |	  *	 y = -y	
								  *	 |		*
								 *		 |		 *
								*		 |		  *
							  *		 |			*
							 *			 |			 *
							*			 |			  *
						  *			 |				*
						 *	  x = -y	 |	x = -y	 *
						*	  y = -x	 |	y = x		  *
					  *				 |					*
	*/
	// All lines are draw from left to right so four
	// of the above transformations will not occur

	// The line will be 2*thickness+1 pixels thick

	// Set pattern	if not given
	switch (style) {
		case tSolid:
			pattern = -1;
			break;
		case tDashed:
			pattern = 0xff00ff00;
			break;
		case tPattern:
			break;
	}
	int P[32];
	int pIndex = 0;
	for (;pIndex<32;++pIndex) {
		if (pattern < 0 )
			P[pIndex] = 1;
		else
			P[pIndex] = 0;
		pattern = pattern << 1;
	}

	// Calculate the first bitmap pixel address
	uchar* BMAddr = ((uchar*) memMgr->GetAddr(BM)) + CELHEADERSIZE;

	// Optimize for horizontal and vertical lines
	if ((A.x-B.x) == 0) {
		// Vertical line
		pIndex = 0;
		int yStart = Min(A.y,B.y);
		int yEnd = Max(A.y,B.y);
		// Calculate the first line pixel address
		uchar* pixelAddr = BMAddr + yStart*BMWidth + A.x;
		while (yStart <= yEnd) {
			if (P[(pIndex++ % 32)]) {
				// horizontally thick
				uchar* writeAddr = pixelAddr - thickness;
				int pixels = 2*thickness + 1;
				for (;pixels>0;--pixels) {
					*(writeAddr++) = color;
				}	
			}
			++yStart;
			pixelAddr += BMWidth;
		}
		return;
	}
	if ((A.y-B.y) == 0) {
		// Horizontal line
		pIndex = 0;
		// Calculate the first line pixel address
		uchar* pixelAddr = BMAddr + A.y*BMWidth + A.x;
		int thickOffset = thickness*BMWidth;
		int xStart = A.x;
		while (xStart <= B.x) {
			if (P[(pIndex++ % 32)]) {
				// vertically thick
				uchar* writeAddr = pixelAddr - thickOffset;
				int pixels = 2*thickness + 1;
				for (;pixels>0;--pixels) {
					*(writeAddr) = color;
					writeAddr += BMWidth;
				}	
			}
			++xStart;
			++pixelAddr;
		}
		return;
	}

	// Aspect ratio
	#define AspectRatio 3/2
	
	Bool thickInX = True;
	int thickOffset = thickness;

	if (Abs(B.y-A.y)*AspectRatio/Abs(B.x-A.x)	< 1) {
		thickOffset = thickness*BMWidth;
		thickInX = False;
	}

	// Calculate the first line pixel address
	uchar* pixelAddr = BMAddr + A.y*BMWidth + A.x;

	// Set up the transformation
	int T[2][2];
	T[0][0] = 1;
	T[0][1] = 0;
	T[1][0] = 0;
	T[1][1] = 1;

	if ((B.y-A.y) < 0)
		T[1][1] = -1;
	if (((B.y-A.y)*T[1][1]) > (B.x-A.x)) {
		// thickness is horizontal
		thickness = -thickness;
		int t00 = T[0][0];
		int t10 = T[1][0];
		T[0][0] = T[0][1];
		T[1][0] = T[1][1];
		T[0][1] = t00;
		T[1][1] = t10;
	}

	// transform so that 0 <= Slope <= 1
	SOL_Point start(T[0][0]*A.x + T[1][0]*A.y,T[0][1]*A.x + T[1][1]*A.y);
	SOL_Point end(T[0][0]*B.x + T[1][0]*B.y,T[0][1]*B.x + T[1][1]*B.y);


	// Slope is positive less equal to 1
	int dx = end.x-start.x;
	int dy = end.y-start.y;

	int d = 2*dy - dx;		// initial value of d
	int incrE =2*dy;			// increment for move East
	int incrNE = 2*(dy-dx);	// increment for move North East
	int x = start.x;
	int y = start.y;

	int xOffset = T[0][0] + T[1][0]*BMWidth;
	int yOffset = T[0][1] + T[1][1]*BMWidth;

	// Write first pixel
	if (P[0]) {
		int pixels = 2*thickness + 1;
		if (thickInX) {
			// horizontally thick
			uchar* writeAddr = pixelAddr - thickOffset;
			for (;pixels>0;--pixels) {
				*(writeAddr++) = color;
			}	
		}
		else {
			// vertically thick
			uchar* writeAddr = pixelAddr - thickOffset;
			for (;pixels>0;--pixels) {
				*(writeAddr) = color;
				writeAddr += BMWidth;
			}	
		}
	}
	// Write the remainder of the line
	pIndex = 1;
	while (x < end.x) {
		if (d <= 0) {
			d += incrE;
			++x;
			pixelAddr += xOffset;
		}
		else {
			d += incrNE;
			++x;
			++y;
			pixelAddr += yOffset + xOffset;
		}
		// Output new point
		if (P[(pIndex++ % 32)]) {
			int pixels = 2*thickness + 1;
			if (thickInX) {
				// horizontally thick
				uchar* writeAddr = pixelAddr - thickOffset;
				for (;pixels>0;--pixels) {
					*(writeAddr++) = color;
				}	
			}
			else {
				// vertically thick
				uchar* writeAddr = pixelAddr - thickOffset;
				for (;pixels>0;--pixels) {
					*(writeAddr) = color;
					writeAddr += BMWidth;
				}	
			}
		}
	}
}

#ifndef NOINTERSECT

Bool 
SOL_Line::RectIntersect(SOL_Rect& rect)
{
	// Determine if this line intersects the given rectangle.
	// Any part of the line interior or on the given
	// rectangle will be considered an intersection.

	if (IsEmpty() || rect.IsEmpty())
		return False;

	// Check for rectangle overlap
//	if (A.x < B.x) {
		if ((B.x < rect.A.x) || (A.x > rect.B.x))
			return False;
//	}
//	else {
//		if ((A.x < rect.A.x) || (B.x > rect.B.x))
//			return False;
//	}
	if (A.y < B.y) {
		if ((B.y < rect.A.y) || (A.y > rect.B.y))
			return False;
	}
	else {
		if ((A.y < rect.A.y) || (B.y > rect.B.y))
			return False;
	}

	// The rectangles overlap, determine if an endpoint
	// of this line lies interior to the given rectangle
	if ((rect.A.x <= A.x) && (A.x <= rect.B.x))
		return True;
	if ((rect.A.x <= B.x) && (B.x <= rect.B.x))
		return True;
	if ((rect.A.y <= A.y) && (A.y <= rect.B.y))
		return True;
	if ((rect.A.y <= B.y) && (B.y <= rect.B.y))
		return True;


	// Determine if any of the sides of the 
	// rectangle intersect this line.
	if (HorizontalIntersect(rect.A.x,rect.B.x,rect.A.y))
			return True;
	if (HorizontalIntersect(rect.A.x,rect.B.x,rect.B.y))
			return True;
	if (VerticleIntersect(rect.A.y,rect.B.y,rect.A.x))
			return True;
	if (VerticleIntersect(rect.A.y,rect.B.y,rect.B.x))
			return True;
	return False;
}


Bool
SOL_Line::HorizontalIntersect(int rectAx,int rectBx,int recty)
{
	// Determine if this line intersects the given horizontal line segement
	/*

							* B
						  *
						 *
						*
					  *
	 C ************************ D
					*
				  *
				 *
				*
			A *

	*/


	// Are B and A on opposite sides of CD?
	int test1 = (A.y-recty);
	int test2 = (B.y-recty);
	int test3 = test1*test2;
	if (test3 > 0)
		return False;
	if (test1 == 0) {
		if ((A.x-rectAx)*(A.x-rectBx) <= 0)
			return True;
		else
			return False;
	}
	if (test2 == 0) {
		if ((B.x-rectAx)*(B.x-rectBx) <= 0)
			return True;
		else
			return False;
	}

	// Are C and D on opposite sides of AB?
	test1 = (rectAx-A.x)*(B.y-A.y)-(B.x-A.x)*(recty-A.y);
	test2 = (rectBx-A.x)*(B.y-A.y)-(B.x-A.x)*(recty-A.y);
	if (test1*test2 <= 0)
		return True;
	// test1*test2 > 0
	return False;
}

Bool
SOL_Line::VerticleIntersect(int rectAy,int rectBy,int rectx)
{
	// Determine if this line intersects the given verticle line segement
	/*

					 * D	* B
					 *	  *
					 *	 *
					 *	*
					 **
	             *               
					**
				  * *
				 *	 *
				*	 *
			A *	 *	C

	*/


	// Are B and A on opposite sides of CD?
	int test1 = (A.x-rectx);
	int test2 = (B.x-rectx);
	int test3 = test1*test2;
	if (test3 > 0)
		return False;
	if (test1 == 0) {
		if ((A.y-rectAy)*(A.y-rectBy) <= 0)
			return True;
		else
			return False;
	}
	if (test2 == 0) {
		if ((B.y-rectAy)*(B.y-rectBy) <= 0)
			return True;
		else
			return False;
	}

	// Are C and D on opposite sides of AB?
	test1 = (rectAy-A.y)*(B.x-A.x)-(B.y-A.y)*(rectx-A.x);
	test2 = (rectBy-A.y)*(B.x-A.x)-(B.y-A.y)*(rectx-A.x);
	if (test1*test2 <= 0)
		return True;
	// test1*test2 > 0
	return False;
}


Bool 
SOL_Line::LineIntersect(SOL_Line& L)
{
	// Determine if this line intersects the given line.
	/*

						* D
					  *
					 *
	 A	*		   *
		 *	     *
		  *	 *
			*  *
			 **
			 **
			*	*
		  *	 *
	  C *		  *
					*
					 *	B

	*/

	if (IsEmpty() || L.IsEmpty())
		return False;

	// Check for rectangle overlap
//	if (A.x < B.x) {
		if (L.A.x < L.B.x) {
			if ((B.x < L.A.x) || (L.B.x < A.x))
				return False;
		}
		else {
			if ((B.x < L.B.x) || (L.A.x < A.x))
				return False;
		}
//	}
//	else {
//		if (L.A.x < L.B.x) {
//			if ((A.x < L.A.x) || (L.B.x < B.x))
//				return False;
//		}
//		else {
//			if ((A.x < L.B.x) || (L.A.x < B.x))
//				return False;
//		}
//	}


	if (A.y < B.y) {
		if (L.A.y < L.B.y) {
			if ((B.y < L.A.y) || (L.B.y < A.y))
				return False;
		}
		else {
			if ((B.y < L.B.y) || (L.A.y < A.y))
				return False;
		}
	}
	else {
		if (L.A.y < L.B.y) {
			if ((A.y < L.A.y) || (L.B.y < B.y))
				return False;
		}
		else {
			if ((A.y < L.B.y) || (L.A.y < B.y))
				return False;
		}
	}



	// Are B and A on opposite sides of CD?
	int CBxCD =	(B.x-L.A.x)*(L.B.y-L.A.y) - (L.B.x-L.A.x)*(B.y-L.A.y);
	int CAxCD =	(A.x-L.A.x)*(L.B.y-L.A.y) - (L.B.x-L.A.x)*(A.y-L.A.y);

	int test = CBxCD*CAxCD;
	if (test > 0)
		return False;
	if (test == 0) {
		if (CBxCD == 0) {
			if ((B.x-L.A.x)*(B.x-L.B.x) > 0)
				return False;
			else
				return True;
		}
		// CAxCD == 0
		if ((A.x-L.A.x)*(A.x-L.B.x) > 0)
			return False;
		else
			return True;
	}


	// Are C and D on opposite sides of AB?
	int ACxAB =	(L.A.x-A.x)*(B.y-A.y) - (B.x-A.x)*(L.A.y-A.y);
	int ADxAB =	(L.B.x-A.x)*(B.y-A.y) - (B.x-A.x)*(L.B.y-A.y);

	test = ACxAB*ADxAB;

	if (test > 0)
		return False;
	if (test == 0) {
		if (ACxAB == 0) {
			if ((L.A.x-A.x)*(L.A.x-B.x) > 0)
				return False;
			else
				return True;
		}
		// ADxAB == 0
		if ((L.B.x-A.x)*(L.B.x-B.x) > 0)
			return False;
		else
			return True;
	}

	// test < 0
	return True;
}

#endif
