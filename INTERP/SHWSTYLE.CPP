
// shwstyle.cpp

#include "sol.hpp"
#include "array.hpp"
#include "shwstyle.hpp"
#include "event.hpp"
#include "buffer.hpp"
#include "graph.hpp"
#include "graphmgr.hpp"
#include "kernel.hpp"
#include "object.hpp"
#include "palmgr.hpp"
#include "pmachine.hpp"
#include "time.hpp"
#include "integ.hpp"
#include <string.h>

char palMorphArray[256] = {
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1,
	1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int  randArray[12288][2];
long randMask[20] = { 0, 0, 0x03, 0x06, 0x0c, 0x14, 0x30,
	0x60,	0xB8, 0x0110, 0x0240, 0x0500, 0x0CA0, 0x1b00,
   0x3500, 0x6000, 0xB400, 0x12000, 0x20400, 0x72000};

void ProcessEffects(PlaneShowStyle* style)
{

	switch(style->style) {
			case DISSOLVENOMORPH:
				PixelDissolveEffect(style);
				break;
			case PIXELDISSOLVE:
				PixelDissolveEffect(style);
				break;
			case IRISOUT:
				IrisOutEffect(style);
				break;
			case IRISIN:
				IrisInEffect(style);
				break;
 			case WIPERIGHT:
				WipeRightEffect(style);
				break;
			case WIPELEFT:
				WipeLeftEffect(style);
				break;
			case WIPEDOWN:
				WipeDownEffect(style);	
				break;
			case WIPEUP:
				WipeUpEffect(style);
				break;
			case HSHUTTEROUT:
				HShutterOutEffect(style);
				break;				
			case HSHUTTERIN:
				HShutterInEffect(style);
				break;
			case VSHUTTERIN:
				VShutterInEffect(style);
				break;
			case VSHUTTEROUT:
				VShutterOutEffect(style);
				break;
	}
}

void
IrisOutEffect(PlaneShowStyle* style)
{
// This starts spiraling out from the center dissolving between two images

	int x,y,l,m,width,height,modx,mody;
	int x_l,x_r,y_t,y_b;
	x = style->divisions;
	Plane*  p =	graphMgr->VisiblePlane(style->planeID);
	SOL_Rect& r = p->OnScreen();		
 	SOL_Rect	B(0,0,x-1,x-1);
	// Since SOL_Rect includes all points specified, this is an x by x rect


	height 	= r.B.y - r.A.y + 1;
	width		= r.B.x - r.A.x + 1;
	
	y = (x * height)/width;

	mody = height%y;
	

	modx = width%x;

	x_l 	= width/(2*x);

	x_r	= width/(2*x);

	y_t	= height/(2*y) ;

	y_b	= height/(2*y);

// r.A.x and r.A.y are added to all rectangles to adjust for planes not at 0,0

	// First start with a center block

		B.Set(x_r*x+r.A.x,y_t*y+r.A.y,x_r*x+x-1+r.A.x,y_t*y+y-1+r.A.y);
		graphMgr->NextScreen()->ShowList().Clear();
		graphMgr->NextScreen()->ShowList().Add(B);
		graphMgr->ShowBits();

		x_l -= 1;
		x_r += 1;
		y_t -= 1;
		y_b += 1;



		while((x_l>=0 )&&(y_t>=0)) {
			for(l=x_l+1;l<=x_r;l++) {  //draw top	
				m = y_t;
				B.Set(l*x+r.A.x,m*y+r.A.y,l*x+x-1+r.A.x,m*y+y-1+r.A.y);
				graphMgr->NextScreen()->ShowList().Clear();
				graphMgr->NextScreen()->ShowList().Add(B);
				graphMgr->ShowBits();
			}

			for(m=y_t;m<=y_b;m++) {  //draw right	

 				l = x_r;
				B.Set(l*x+r.A.x,m*y+r.A.y,l*x+x-1+r.A.x,m*y+y-1+r.A.y);
				graphMgr->NextScreen()->ShowList().Clear();
				graphMgr->NextScreen()->ShowList().Add(B);
				graphMgr->ShowBits();
			}
			m--;

			for(l=x_r;l>=x_l;l--) {  //draw bottom	
				B.Set(l*x+r.A.x,m*y+r.A.y,l*x+x-1+r.A.x,m*y+y-1+r.A.y);
				graphMgr->NextScreen()->ShowList().Clear();
				graphMgr->NextScreen()->ShowList().Add(B);
				graphMgr->ShowBits();
			}
			l++;

			for(m=y_b;m>=y_t;m--) {  //draw left	
				B.Set(l*x+r.A.x,m*y+r.A.y,l*x+x-1+r.A.x,m*y+y-1+r.A.y);
				graphMgr->NextScreen()->ShowList().Clear();
				graphMgr->NextScreen()->ShowList().Add(B);
				graphMgr->ShowBits();
			}

			x_l -= 1;
			
			x_r += 1;

			y_t -= 1;
		
			y_b += 1;
		}

		// Clear entire screen 

		B.Set(r.A.x,r.A.y,r.B.x,r.B.y);
		graphMgr->NextScreen()->ShowList().Clear();
		graphMgr->NextScreen()->ShowList().Add(B);
		graphMgr->ShowBits();
}

void
IrisInEffect(PlaneShowStyle* style)
{
// This starts spiraling in from the outside between two images

	int x,y,l,m,width,height,modx,mody,modyy;
	int x_l,y_t;

	x = style->divisions;
	Plane*  p =	graphMgr->VisiblePlane(style->planeID);
	SOL_Rect& r = p->OnScreen();		
	height 	= r.B.y - r.A.y + 1;
	width		= r.B.x - r.A.x + 1;
	y = (x * height)/width;
 	SOL_Rect	B(0,0,x-1,x-1);
	// Since SOL_Rect includes all points specified, this is an x by x rect

	mody = height%y;

	modx = width%x;

	if (mody)
		modyy = 1;
	else
		modyy = 0;

	x_l = 0;
	y_t = 0;

// r.A.x and r.A.y are added to all rectangles to adjust for planes not at 0,0

		if (width%x) {
		// width is not an even multiple of x so adjust the first rightside draw
		
			for(m=0;m<height/y;m++) {  //draw right	
				l = width/x  ;
				B.Set(l*x+r.A.x,m*y+r.A.y,l*x+-1+(width%x)+r.A.x,m*y+y-1+r.A.y);
				graphMgr->NextScreen()->ShowList().Clear();
				graphMgr->NextScreen()->ShowList().Add(B);
				graphMgr->ShowBits();
			}
		}

		if (height%y) {
		// height is not an even multiple of y so adjust the first bottom draw

			m = height/y;
			for(l=width/x-1;l>=0;l--) {  //draw bottom	
				B.Set(l*x+r.A.x,m*y+r.A.y,l*x+x-1+r.A.x,m*y+-1+(height%y)+r.A.y);
				graphMgr->NextScreen()->ShowList().Clear();
				graphMgr->NextScreen()->ShowList().Add(B);
				graphMgr->ShowBits();
			}
		}

		while((x_l < width/x) || (y_t < height/y)) {
			for(m=height/y-1;m>=y_t;m--) {  //draw left	
				l = x_l;
				B.Set(l*x+r.A.x,m*y+r.A.y,l*x+x-1+r.A.x,m*y+y-1+r.A.y);
				graphMgr->NextScreen()->ShowList().Clear();
				graphMgr->NextScreen()->ShowList().Add(B);
				graphMgr->ShowBits();
			}

			for(l=x_l;l<width/x;l++) {  //draw top	
				m = y_t;
				B.Set(l*x+r.A.x,m*y+r.A.y,l*x+x-1+r.A.x,m*y+y-1+r.A.y);
				graphMgr->NextScreen()->ShowList().Clear();
				graphMgr->NextScreen()->ShowList().Add(B);
				graphMgr->ShowBits();
			}

			l--;
			for(m=y_t;m<height/y;m++) {  //draw right	
				B.Set(l*x+r.A.x,m*y+r.A.y,l*x+x-1+r.A.x,m*y+y-1+r.A.y);
				graphMgr->NextScreen()->ShowList().Clear();
				graphMgr->NextScreen()->ShowList().Add(B);
				graphMgr->ShowBits();
			}
			m--;
			for(l=width/x-1;l>=x_l;l--) {  //draw bottom	
				B.Set(l*x+r.A.x,m*y+r.A.y,l*x+x-1+r.A.x,m*y+y-1+r.A.x);
				graphMgr->NextScreen()->ShowList().Clear();
				graphMgr->NextScreen()->ShowList().Add(B);
				graphMgr->ShowBits();
			}

			x_l  += 1;
			width	 -= x;
			y_t	 += 1;
			height -= y;
		}
		// Clear entire screen 

		B.Set(r.A.x,r.A.y,r.B.x,r.B.y);
		graphMgr->NextScreen()->ShowList().Clear();
		graphMgr->NextScreen()->ShowList().Add(B);
		graphMgr->ShowBits();

}

void
VShutterInEffect(PlaneShowStyle* style)
{
// This wipes vertically from the edges to the middle between two images

	int x,l,m,n,width,height,mody;
	x = style->divisions;
	Plane*  p =	graphMgr->VisiblePlane(style->planeID);
	SOL_Rect& r = p->OnScreen();		
	height 	= r.B.y - r.A.y + 1;
	width		= r.B.x - r.A.x + 1;

 	SOL_Rect	B(0,0,x,x);

	if (height%(2*x))
 		mody = 1;
 	else
 		mody = 0;


	n = height/x-1;

// r.A.x and r.A.y are added to all rectangles to adjust for planes not at 0,0

	if (height%x) {
	// if height is not an even multiple of x adjust for the first draw

		for(l=0;l<width/x;l++) // Drawing Up	
		{
			B.Set(l*x+r.A.x,n*x+r.A.y,l*x+x-1+r.A.x,n*x+x-1+(height%x)+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}

		if ((l==width/x) && (width%x)) {
		// if width is not an even multiple of x adjust for last draw

			B.Set(l*x+r.A.x,n*x+r.A.y,l*x+-1+(width%x)+r.A.x,n*x+x-1+(height%x)+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
	}

	for(m=0;m<height/(2*x)+mody;m++) {
	// Since we draw from the top and bottom the outer loop is height/(2*x)
	// mody is a check to see if height is an even multiple of (2*x)
 
		for(l=0;l<width/x;l++)// Drawing Down 	
		{
			B.Set(l*x+r.A.x,m*x+r.A.y,l*x+x-1+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}

		if ((l==width/x) && (width%x)) {
		// if width is not an even multiple of x adjust for last draw

			B.Set(l*x+r.A.x,m*x+r.A.y,l*x+-1+(width%x)+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}

		for(l=0;l<width/x;l++) // Drawing Up	
		{
			B.Set(l*x+r.A.x,(n-m)*x+r.A.y,l*x+x-1+r.A.x,(n-m)*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}

		if ((l==width/x) && (width%x)) {
		// if width is not an even multiple of x adjust for last draw

			B.Set(l*x+r.A.x,(n-m)*x+r.A.y,l*x+-1+(width%x)+r.A.x,(n-m)*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}

	}
		// Clear entire screen 

		B.Set(r.A.x,r.A.y,r.B.x,r.B.y);
		graphMgr->NextScreen()->ShowList().Clear();
		graphMgr->NextScreen()->ShowList().Add(B);
		graphMgr->ShowBits();
}


void
VShutterOutEffect(PlaneShowStyle* style)
{
// This wipes vertically from the middle to the edges between two images

	int x,l,m,n,height,width,mody;
	x = style->divisions;
	Plane*  p =	graphMgr->VisiblePlane(style->planeID);
	SOL_Rect& r = p->OnScreen();	

	
	height 	= r.B.y - r.A.y + 1;
	width		= r.B.x - r.A.x + 1;

 	SOL_Rect	B(0,0,x,x);

	if (height%(2*x))
		mody = 1;
	else
		mody = 0;


	n = height/x-1;

// r.A.x and r.A.y are added to all rectangles to adjust for planes not at 0,0

	for(m=height/(2*x);m>=0;m--) {	
		for(l=0;l<width/x;l++) 	// Draw up from middle
		{
			B.Set(l*x+r.A.x,m*x+r.A.y,l*x+x-1+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}

		if ((l==width/x) && (width%x)) {
		// if width is not an even multiple of x adjust for last draw

			B.Set(l*x+r.A.x,m*x+r.A.y,l*x+-1+(width%x)+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}

	  	for( l=0;l<width/x;l++)  // Draw down from middle	
		{
			B.Set(l*x+r.A.x,(n-m)*x+r.A.y,l*x+x-1+r.A.x,(n-m)*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}

		if ((l==width/x) && (width%x)) {
		// if width is not an even multiple of x adjust for last draw

			B.Set(l*x+r.A.x,(n-m)*x+r.A.y,l*x+-1+(width%x)+r.A.x,(n-m)*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}

	}

	if (height%x) {
	// if heigth is not an even muliple of x adjust for last line
	  	for( l=0;l<width/x;l++) 	
		{
			B.Set(l*x+r.A.x,n*x+r.A.y,l*x+x-1+r.A.x,n*x+x+-1+(height%x)+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}

		if ((l==width/x) && (width%x)) {
		// if width is not an even multiple of x adjust for last draw

			B.Set(l*x+r.A.x,n*x+r.A.y,l*x+-1+(width%x)+r.A.x,n*x+x-1+(height%x)+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
	}
		// Clear entire screen 

		B.Set(r.A.x,r.A.y,r.B.x,r.B.y);
		graphMgr->NextScreen()->ShowList().Clear();
		graphMgr->NextScreen()->ShowList().Add(B);
		graphMgr->ShowBits();

}

void
HShutterOutEffect(PlaneShowStyle* style)
{
// This wipes horizontally from the middle to the edges between two images

	int x,l,m,n,height,width,mody;
	x = style->divisions;
 	SOL_Rect	B(0,0,x-1,x-1);
	// Since SOL_Rect includes all points specified, this is an x by x rect

	Plane*  p =	graphMgr->VisiblePlane(style->planeID);
	SOL_Rect& r = p->OnScreen();		
	height 	= r.B.y - r.A.y + 1;
	width		= r.B.x - r.A.x + 1;

	if (width%(2*x))
		mody = 1;
	else
		mody = 0;


		n = width/x-1;

// r.A.x and r.A.y are added to all rectangles to adjust for planes not at 0,0

	for(l=width/(2*x);l>=0;l--) {
		for(m=0;m<height/x;m++)// Draw from middle to left 	
		{
			B.Set(l*x+r.A.x,m*x+r.A.y,l*x+x-1+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
		if (height%x) {
		// if height is not an even multiple of x adjust for last draw

 			B.Set(l*x+r.A.x,m*x+r.A.y,l*x+x-1+r.A.x,m*x+-1+(height%x)+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
 		for( m=0;m<height/x;m++) // Draw from middle to right	
		{
			B.Set((n-l)*x+r.A.x,m*x+r.A.y,(n-l)*x+x-1+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}

		if (height%x) {
 		// if height is not an even multiple of x adjust for last draw

			B.Set((n-l)*x+r.A.x,m*x+r.A.y,(n-l)*x+x-1+r.A.x,m*x+-1+(height%x)+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();

		}
	}

	if (width%x) {
	// if width is not an even multiple of x adjust for the last draw

 		for( m=0;m<height/x;m++) // Draw from middle to right	
		{
			B.Set(n*x+r.A.x,m*x+r.A.y,n*x+x-1+(width%x)+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}

		if (height%x) {
 		// if height is not an even multiple of x adjust for last draw
			B.Set(n*x+r.A.x,m*x+r.A.y,n*x+x-1+(width%x)+r.A.x,m*x+-1+(height%x)+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
	}
		// Clear entire screen just to be sure

		B.Set(r.A.x,r.A.y,r.B.x,r.B.y);
		graphMgr->NextScreen()->ShowList().Clear();
		graphMgr->NextScreen()->ShowList().Add(B);
		graphMgr->ShowBits();

}

void
HShutterInEffect(PlaneShowStyle* style)
{
// This wipes horizontally from the edges to the middle between two images

	int x,l,m,n,height,width,mody;
	x = style->divisions;
 	SOL_Rect	B(0,0,x,x);
	// Since SOL_Rect includes all points specified, this is an x by x rect

	Plane*  p =	graphMgr->VisiblePlane(style->planeID);
	SOL_Rect& r = p->OnScreen();		
	height 	= r.B.y - r.A.y + 1;
	width		= r.B.x - r.A.x + 1;

		n = width/x-1;

	if (width%(2*x))
		mody = 1;
	else
		mody = 0;

// r.A.x and r.A.y are added to all rectangles to adjust for planes not at 0,0

	if (width%x) {
	// if width is not an even multiple of x adjust for the first draw

		for( m=0;m<height/x;m++) // Draw from right to left	
		{
			B.Set((n+1)*x+r.A.x,m*x+r.A.y,(n+1)*x-1+(width%x)+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
		if (height%x) {
 		// if height is not an even multiple of x adjust for last draw

			B.Set((n+1)*x+r.A.x,m*x+r.A.y,(n+1)*x+-1+(width%x)+r.A.x,m*x+-1+(height%x)+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
	}

	for(l=0;l<width/(2*x);l++) {
		for(m=0;m<height/x;m++) // Draw from left to right	
		{
			B.Set(l*x+r.A.x,m*x+r.A.y,l*x+x-1+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}

		if (height%x) {
		// if height is not an even multiple of x adjust for last draw

 			B.Set(l*x+r.A.x,m*x+r.A.y,l*x+x-1+r.A.x,m*x+-1+(height%x)+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}

		for( m=0;m<height/x;m++) // Draw from right to left	
		{
			B.Set((n-l)*x+r.A.x,m*x+r.A.y,(n-l)*x+x-1+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
		if (height%x) {
 		// if height is not an even multiple of x adjust for last draw

			B.Set((n-l)*x+r.A.x,m*x+r.A.y,(n-l)*x+x-1+r.A.x,m*x+-1+(height%x)+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();

		}

	}


		// Clear entire screen just to be sure

		B.Set(r.A.x,r.A.y,r.B.x,r.B.y);
		graphMgr->NextScreen()->ShowList().Clear();
		graphMgr->NextScreen()->ShowList().Add(B);
		graphMgr->ShowBits();

}


void
WipeLeftEffect(PlaneShowStyle* style)
{
// This function wipes from right to left between two images

	int x,height,width,l,m;
	x = style->divisions;

 	SOL_Rect	B(0,0,x-1,x-1);
	// Since SOL_Rect includes all points specified, this is an x by x rect

	Plane*  p =	graphMgr->VisiblePlane(style->planeID);
	SOL_Rect& r = p->OnScreen();		
	height 	= r.B.y - r.A.y + 1;
	width		= r.B.x - r.A.x + 1;

// r.A.x and r.A.y are added to all rectangles to adjust for planes not at 0,0

	if (width%x) {
	// if width is not an even multiple of x adjust for first draw
		l = width/x ;
		for(m=0;m<height/x;m++) 	
		{
			B.Set(l*x+r.A.x,m*x+r.A.y,l*x-1+(width%x)+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
		if ((m == height/x) && (height % x)) {	
		// if height is not an even multiple of x adjust for last draw

			B.Set(l*x+r.A.x,(m-1)*x+r.A.y,l*x-1+(width%x)+r.A.x,(m-1)*x+x+(height%x)-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}		
	}


	for(l=width/x-1;l>=0;l--) {
		for(m=0;m<height/x;m++) 	
		{
			B.Set(l*x+r.A.x,m*x+r.A.y,l*x+x-1+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
		if ((m == height/x) && (height % x)) {	
		// if height is not an even multiple of x adjust for last draw

			B.Set(l*x+r.A.x,(m-1)*x+r.A.y,l*x+x-1+r.A.x,(m-1)*x+x+(height%x)-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}		
	}
		// Clear entire screen to be sure

		B.Set(r.A.x,r.A.y,r.B.x,r.B.y);
		graphMgr->NextScreen()->ShowList().Clear();
		graphMgr->NextScreen()->ShowList().Add(B);
		graphMgr->ShowBits();

}

void
WipeRightEffect(PlaneShowStyle* style)
{
// This function wipes from left to right between two images

	int x,height,width,l,m;
	x = style->divisions;

 	SOL_Rect	B(0,0,x-1,x-1); 
	// Since SOL_Rect includes all points specified, this is an x by x rect

	Plane*  p =	graphMgr->VisiblePlane(style->planeID);
	SOL_Rect& r = p->OnScreen();		
	height 	= r.B.y - r.A.y + 1;
	width		= r.B.x - r.A.x + 1;

// r.A.x and r.A.y are added to all rectangles to adjust for planes not at 0,0

	for(l=0;l<width/x;l++) {
		for(m=0;m<height/x;m++) 	
		{
			B.Set(l*x+r.A.x,m*x+r.A.y,l*x+x-1+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
		if ((m == height/x) && (height % x)) {	
		// if height is not an even multiple of x adjust for last draw

			B.Set(l*x+r.A.x,(m-1)*x+r.A.y,l*x+x-1+r.A.x,(m-1)*x+x+(height%x)-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}		
	}

	if (width%x) {
	// if width is not an even multiple of x adjust for last draw
		for(m=0;m<height/x;m++) 	
		{
			B.Set((l-1)*x+r.A.x,m*x+r.A.y,(l-1)*x+x-1+(width%x)+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
		if ((m == height/x) && (height % x)) {	
		// if height is not an even multiple of x adjust for last draw

			B.Set((l-1)*x+r.A.x,(m-1)*x+r.A.y,(l-1)*x+x-1+(width%x)+r.A.x,(m-1)*x+x+(height%x)-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}		
	}
		// Clear entire screen just to be sure

	B.Set(r.A.x,r.A.y,r.B.x,r.B.y);
	graphMgr->NextScreen()->ShowList().Clear();
	graphMgr->NextScreen()->ShowList().Add(B);
	graphMgr->ShowBits();

}

void
WipeDownEffect(PlaneShowStyle* style)
{
// This function wipes from top to bottom between two images

	int x,height,width;
	x = style->divisions;

 	SOL_Rect	B(0,0,x-1,x-1);
	// Since SOL_Rect includes all points specified, this is an x by x rect

	Plane*  p =	graphMgr->VisiblePlane(style->planeID);
	SOL_Rect& r = p->OnScreen();		
	height 	= r.B.y - r.A.y + 1;
	width		= r.B.x - r.A.x + 1;

	
// r.A.x and r.A.y are added to all rectangles to adjust for planes not at 0,0

	for(int m=0;m<height/x;m++) {	
		for(int l=0;l<width/x;l++) 
		{
			B.Set(l*x+r.A.x,m*x+r.A.y,l*x+x-1+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
		if ((l == width/x) && (width%x)){
		// if width is not an even multiple of x adjust for last draw

			B.Set((l-1)*x+r.A.x,m*x+r.A.y,(l-1)*x+x-1+(width%x)+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
	}
	
	if (height%x) {
	// if height is not an even multiple of x adjust for last draw

		for(int l=0;l<width/x;l++) 
		{
			B.Set(l*x+r.A.x,(m-1)*x+r.A.y,l*x+x-1+r.A.x,(m-1)*x+x-1+(height%x)+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
		if ((l == width/x) && (width%x)){
		// if width is not an even multiple of x adjust for last draw

			B.Set((l-1)*x+r.A.x,(m-1)*x+r.A.y,(l-1)*x+x-1+(width%x)+r.A.x,(m-1)*x+x-1+(height%x)+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
	}


		// Clear entire screen just to be sure

		B.Set(r.A.x,r.A.y,r.B.x,r.B.y);
		graphMgr->NextScreen()->ShowList().Clear();
		graphMgr->NextScreen()->ShowList().Add(B);
		graphMgr->ShowBits();

}

void
WipeUpEffect(PlaneShowStyle* style)
{
// This function wipes from bottom to top between two images

	int x,height,width,m;
	x = style->divisions;

 	SOL_Rect	B(0,0,x-1,x-1);
	// Since SOL_Rect includes all points specified, this is an x by x rect

	Plane*  p =	graphMgr->VisiblePlane(style->planeID);
	SOL_Rect& r = p->OnScreen();		
	height 	= r.B.y - r.A.y + 1;
	width		= r.B.x - r.A.x + 1;

// r.A.x and r.A.y are added to all rectangles to adjust for planes not at 0,0

	if (height%x) {
	// if height is not an even multiple of x adjust for the first draw
		m = height/x;
		for(int l=0;l<width/x;l++) 
		{
			B.Set(l*x+r.A.x,m*x+r.A.y,l*x+x-1+r.A.x,m*x-1+(height%x)+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
		if ((l == width/x) && (width%x)){
		// if width is not an even multiple of x adjust for last draw

			B.Set((l-1)*x+r.A.x,m*x+r.A.y,(l-1)*x+x-1+(width%x)+r.A.x,m*x-1+(height%x)+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
   }

	for(m=height/x-1;m>=0;m--) {	
		for(int l=0;l<width/x;l++) 
		{
			B.Set(l*x+r.A.x,m*x+r.A.y,l*x+x-1+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}
		if ((l == width/x) && (width%x)){
		// if width is not an even multiple of x adjust for last draw

			B.Set((l-1)*x+r.A.x,m*x+r.A.y,(l-1)*x+x-1+(width%x)+r.A.x,m*x+x-1+r.A.y);
			graphMgr->NextScreen()->ShowList().Clear();
			graphMgr->NextScreen()->ShowList().Add(B);
			graphMgr->ShowBits();
		}

	}
		// Clear entire screen 

		B.Set(r.A.x,r.A.y,r.B.x,r.B.y);
		graphMgr->NextScreen()->ShowList().Clear();
		graphMgr->NextScreen()->ShowList().Add(B);
		graphMgr->ShowBits();

}



int
Bitwidth(int number)
{
	int width = 0;

	while (number != 0) {
		number = number >> 1;
		width += 1;
	}
	return(width);

}

// This dissolve random number generator was taken from Graphics Gems
// pp 224 - 226;

void
PixelDissolveEffect(PlaneShowStyle* style)
{
	int x = style->divisions;
	int width,height,org_width,org_height;
	int row,col;
	Plane*  p =	graphMgr->VisiblePlane(style->planeID);
	SOL_Rect& r = p->OnScreen();		

	org_width  = (r.B.x - r.A.x + 1);
	org_height = (r.B.y - r.A.y + 1);

	if (width%x)
		width 	= org_width/x + 1;
	else
		width 	= org_width/x;

	if (height%x)
		height	= org_height/x + 1;
	else
		height	= org_height/x;	

	long mask ;
	int seq;
 	SOL_Rect	B(0,0,x,x);
   int y = style->style;
	seq = 1;	

   mask = randMask[Bitwidth(width*height-1)];

		do
  		{
			row = seq/width;
			col = seq%width;
			
 			if (row < height) {
				if ((row==(height-1))&&(org_height%x)) { 
				// if height is not an even multiple of x adjust for last row
					if ((col==(width-1))&&(org_width%x)) {
				// if width is not an even multiple of x ajust for last column
						B.Set(col*x,row*x,col*x+(org_width%x)-1,row*x+(org_height%x)-1);
						graphMgr->NextScreen()->ShowList().Clear();
						graphMgr->NextScreen()->ShowList().Add(B);
				 		graphMgr->ShowBits();
					}
					else {
						B.Set(col*x,row*x,col*x+x-1,row*x+(org_height%x)-1);
						graphMgr->NextScreen()->ShowList().Clear();
						graphMgr->NextScreen()->ShowList().Add(B);
				 		graphMgr->ShowBits();
					}
				}
				else {
					if ((col==(width-1))&&(org_width%x)) {
				// if width is not an even multiple of x ajust for last column
						B.Set(col*x,row*x,col*x+(org_width%x)-1,row*x+x-1);
						graphMgr->NextScreen()->ShowList().Clear();
						graphMgr->NextScreen()->ShowList().Add(B);
			  	 		graphMgr->ShowBits();
					}
					else {
						B.Set(col*x,row*x,col*x+x,row*x+x);
						graphMgr->NextScreen()->ShowList().Clear();
						graphMgr->NextScreen()->ShowList().Add(B);
			  	 		graphMgr->ShowBits();
					}
				}
			}

			if (seq & 1) {
				seq = (seq >> 1) ^ mask;
			}
			else
				seq = (seq >> 1);
		} while(seq != 1);
		
		// be sure and do the first draw at 0,0
		B.Set(r.A.x,r.A.y,x-1,x-1);
		graphMgr->NextScreen()->ShowList().Clear();
		graphMgr->NextScreen()->ShowList().Add(B);
 		graphMgr->ShowBits();


		// Clear entire screen 

		B.Set(r.A.x,r.A.y,r.B.x,r.B.y);
		graphMgr->NextScreen()->ShowList().Clear();
		graphMgr->NextScreen()->ShowList().Add(B);
		graphMgr->ShowBits();

}

void
KSetPalStyleRange(argList)
{
	int i = arg(1);
	int j = arg(2);

	for(int k=i;k<=j;k++)
		palMorphArray[k] = 0;


}

void
KSetScroll(argList)
{
	// Scroll for a plane
	//
	// Input
	// 	arg1-----planeID
	// 	arg2-----directionX
	// 	arg3-----directionY
	//    arg4-----pic number
	//		arg5-----True with animate
	//					False without animate
	//		arg6-----cycle speed	(in ticks)
	//		arg7-----mirror

	if (!arg(2) && !arg(3)) 
		// No motion
		return;

	// Make sure this plane is not already
	// in the scroll list.
	int planeID = arg(1);
	PlaneScroll* scroll = planeScrolls;
	while (scroll) {
		if (scroll->planeID == planeID)
			msgMgr->Fatal(SrcLoc,Msg_ScrollAlreadyInProgress,planeID);
		scroll = scroll->next;
	}

	if (arg(2) && arg(3))
			msgMgr->Fatal(SrcLoc,Msg_ScrollInXAndY,planeID);

	scroll = New PlaneScroll();
	scroll->xStep = arg(2);
	scroll->yStep = arg(3);
	scroll->newPic = arg(4);
	scroll->clock = timeMgr->GetTickCount();
	scroll->planeID = planeID;
	scroll->animate = arg(5);
	if (argCount > 5)
		scroll->speed = arg(6);

	// Add the new pic just off the plane
	scroll->yDim = 0;
	scroll->xDim = 0;

	int index = graphMgr->VisibleScreen().Planes().Search(planeID);
	if (index == -1)
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,index);
	SOL_Rect r = graphMgr->VisibleScreen().Planes()[index].GameRect();

	SOL_Point pt;
	if (scroll->xStep) {
 		if (scroll->xStep > 0) {
			pt.x = -r.Xlen();
			pt.y = 0;
			scroll->xDim = -r.Xlen();
 		}
		else {
			pt.x = r.Xlen();
			pt.y = 0;
			scroll->xDim = r.Xlen();
 		}
	}
	else {
 		if (scroll->yStep > 0) {
			pt.x = 0;
			pt.y = -r.Ylen();
			scroll->yDim = -r.Ylen();
 		}
		else {
			pt.x = 0;
			pt.y = r.Ylen();
			scroll->yDim = r.Ylen();
 		}
	}

	scroll->oldPic = graphMgr->AddPic(planeID,scroll->newPic,pt,arg(7));

	if (scroll->animate) {
		scroll->next = planeScrolls;
		planeScrolls = scroll;
	}
	else {
		Bool done = False;
		while (!done) {
			done = ProcessScroll(scroll);
			graphMgr->FrameOut();
			AsyncEventCheck();
		}
		delete scroll;
	}
}


void
ProcessScrolls()
{
	PlaneScroll* lastScroll = NULL;
	PlaneScroll* scroll = planeScrolls;
	while (scroll) {
		Bool done = ProcessScroll(scroll);
		if (done) {
			// Rechain styles
			if (lastScroll) {
				lastScroll->next = scroll->next;
				delete scroll;
				scroll = lastScroll->next;
			}
			else {
				planeScrolls = scroll->next;
				delete scroll;
				scroll = planeScrolls;
			}
		}
		else {
			lastScroll = scroll;
			scroll = scroll->next;
		}
	}
}

Bool 
ProcessScroll(PlaneScroll* scroll)
{
	Bool done = False;
	uint time = timeMgr->GetTickCount();
	if (scroll->clock >= time)
		return False;
	int xStep = scroll->xStep;
	int yStep = scroll->yStep;
	if (((scroll->xDim + xStep)*scroll->xDim) <= 0)
		xStep = - scroll->xDim;
	if (((scroll->yDim + yStep)*scroll->yDim) <= 0)
		yStep = - scroll->yDim;
	scroll->xDim += xStep;
	scroll->yDim += yStep;
	if ((scroll->xDim == 0) && (scroll->yDim == 0)) {
		// At the end of the scroll
		graphMgr->DeletePic(scroll->planeID,scroll->oldPic,scroll->newPic);
		done = True;
	}
	graphMgr->ScrollScreenItems(scroll->planeID,xStep,yStep);
	
	return done;
}

void
KShowStylePercent(argList)
{
	// Percent complete of a show style for a plane
	//
	// Input
	// 	arg1-----planeID
	// Output
	//    acc -----the percent complete of the plane's show style
	if (!showStyles)
		pm.acc = 100;
	else {
		PlaneShowStyle* style = showStyles;
		while (style) {
			if (style->planeID == arg(1)) {
				pm.acc = (100*style->percent)/style->divisions;
				return;
			}
			style = style->next;
		}
		pm.acc = 100;
	}
}

void
KSetShowStyle(argList)
{
	// Show style for a plane
	//
	// Input
	// 	arg1-----showStyle
	// 	arg2-----planeID
	//		arg3-----speed	(seconds to complete)
	// 	arg4-----color (-1 is skip color)
	//    arg5-----priority of show style
	//		arg6-----True with animate
	//					False without animate
	//		arg7-----Reference frame (may be needed to remove a screen item)
	//    arg8-----Fade Array for use with FADE styles
	//		arg9-----Number of divisions for a show style (optional)

	// Planes not allowed to move or be overlaped by a
	// higher priority plane while show style in progress.
	//
	// Make sure this plane is not already
	// in the showStyles list.

	Bool 		entry;
	uchar 	color;
	int		thisStyle;

	thisStyle = arg(1);
	if (arg(4) == -1)	{
		entry = True;
		color = SOL_BLACK;
	}
	else {
		entry = False;
		color = arg(4);
	}


	if (thisStyle > LASTSTYLE)
		msgMgr->Fatal(SrcLoc,"Illegal show style for plane ",arg(2));
	int planeID = arg(2);

	// Save current plane image
	int index = graphMgr->NextScreen()->Planes().Search(planeID);
	if (index == -1)
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,index);
	SOL_Rect planeRect = graphMgr->NextScreen()->Planes()[index].GameRect();

	int priority = arg(5);
	Bool newStyle = True;
	PlaneShowStyle* style = showStyles;
	while (style) {
		if (style->planeID == planeID) {
			if (
				(
				((argCount > 8) && (style->divisions == arg(9))) ||
				((argCount <= 8) && (style->divisions == showStyleDivisions[thisStyle]))
				)
				&&
				(style->form == showStyleForms[thisStyle])
				) {
					newStyle = False;
					// The reverse of the exit style
					style->percent = 0;
					entry = True;
			}
			else {
				// Need the color!
				entry = True;
				color = style->color;
				KillStyle(style);
				break;
			}
			break;
		}	
		style = style->next;
	}

//#ifndef WINDOWS
	if (thisStyle <= PLAINFORM)  return;
//#endif

	if (newStyle) {
		style = New PlaneShowStyle();
		style->XRangeArray = NULL;
		style->form = showStyleForms[thisStyle];
		style->deleteMe = False;
		if (argCount > 8)
			style->divisions = arg(9);
		else
			style->divisions = showStyleDivisions[thisStyle];
		style->planeID = planeID;
		if (argCount > 7)	{
			style->XRangeSize   = ((ArrayID) arg(8))->size;

			style->XRangeArray  = New SCIWord [style->XRangeSize];
			for(int i=0;i<style->XRangeSize;i++)
				style->XRangeArray[i] =	((SCIWord*) ((ArrayID) arg(8)).Data()) [i];	
 
		}
		else  {
			style->XRangeSize   = 0;
		 }
	}
	style->entry = entry;
	style->color = color;
	style->clock = timeMgr->GetTickCount();
	style->style = thisStyle;
	style->animate = arg(6);

	// calculate the ticks per processing
	style->speed = (60*arg(3)+style->divisions-1)/style->divisions;
	if (style->speed == 0) {
		if (style->XRangeArray != NULL)
			delete style->XRangeArray;
		delete style;
		msgMgr->Alert(SrcLoc,Msg_ShowStyleSpeed);
		return;
	}

	if (arg(7)) {
		// Generate a reference frame
		graphMgr->FrameOut(False);
	}

	if (newStyle) {
		// Switch on show style
		switch (style->style) {
			case PLAINFORM:
				break;
			case DISSOLVENOMORPH:
				break;
			case PIXELDISSOLVE:
				break;
			case IRISOUT:
			case IRISIN:
				break;
			case WIPERIGHT:
			case WIPELEFT:
				break;
			case WIPEDOWN:
			case WIPEUP:
				break;
			case HSHUTTEROUT:
			case HSHUTTERIN:
				break;
			case VSHUTTEROUT:
			case VSHUTTERIN:
				break;
			case FADEOUT:
				break;
			case FADEIN:
				break;
			case PALETTEMORPH:
				break;
		}
		// Store for processing durring frameout
		style->next = showStyles;
		showStyles = style;
	}
}

void
ProcessShowStyles()
{
	Bool notDone;
	Bool withOutAnimate;
	uint time = timeMgr->GetTickCount();
	do {
		notDone = False;
		withOutAnimate = False;
		PlaneShowStyle* lastStyle = NULL;
		PlaneShowStyle* style = showStyles;
		while (style) {
			if (!style->animate)	++withOutAnimate;
			Bool done = ProcessShowStyle(style,time);
			if (!done) notDone = True;
			if (done && style->deleteMe) {
				style = ShutDownStyle(style);
			}
			else {
				lastStyle = style;
				style = style->next;
			}
		}
		if (withOutAnimate ) {
			graphMgr->FrameOut();
		}
	} while (notDone && withOutAnimate);

}

Bool 
ProcessShowStyle(PlaneShowStyle* style,uint time)
{
	Bool done = False;
	
	if ((style->clock >= time) && style->animate)
		return False;

	// Switch on show style
	switch (style->style) {
		case PLAINFORM:
			done = Plain(style);
			break;
		case DISSOLVENOMORPH:
			done = PalMorph(style);
			break;
		case PIXELDISSOLVE:
			done = PalMorph(style);
			break;
		case FADEOUT:
			done = Fade(-1,style);
			break;
		case FADEIN:
			done = Fade(1,style);
			break;
		case PALETTEMORPH:
#ifdef MACINTOSH
			done = Plain(style); //KGN PalMorph(style);
#else
			done = PalMorph(style);
#endif
			break;
		case HSHUTTEROUT:
		case IRISOUT:
		case VSHUTTEROUT:
		case WIPELEFT:
		case WIPEUP:
			done = PalMorph(style);
			break;
		case HSHUTTERIN:
		case IRISIN:
		case VSHUTTERIN:
		case WIPERIGHT:
		case WIPEDOWN:
			done = PalMorph(style);
			break;
	}
	return (done);
}

Bool
PalMorph(PlaneShowStyle* style)
{
			graphMgr->PalMorphFrameOut(palMorphArray,style);
			style->deleteMe = True;
			return True;
}


PlaneShowStyle*
ShutDownStyle(PlaneShowStyle* style)
{
	// Switch on show style
	switch (style->style) {
		case DISSOLVENOMORPH:
			break;
		case PIXELDISSOLVE:
			break;
		case HSHUTTEROUT:
		case HSHUTTERIN:
		case IRISOUT:
		case IRISIN:
		case VSHUTTEROUT:
		case VSHUTTERIN:
		case WIPERIGHT:
		case WIPELEFT:
		case WIPEDOWN:
		case WIPEUP:
			break;
		case PLAINFORM:
		case FADEOUT:
		case FADEIN:
			break;
	}
	// Rechain styles
	PlaneShowStyle* lastStyle = NULL;
	PlaneShowStyle* search = showStyles;
	while (search) {
		if (search == style) break;
		lastStyle = search;
		search = search->next;
	}

	if (lastStyle) {
		lastStyle->next = style->next;
		if (style->XRangeArray != NULL)
			delete style->XRangeArray;
		delete style;
		style = lastStyle->next;
	}
	else {
		showStyles = style->next;
		if (style->XRangeArray != NULL)
			delete style->XRangeArray;
		delete style;
		style = showStyles;
	}
	return style;
}


void
KillStyle(PlaneShowStyle* style)
{
	// Switch on show style
	switch (style->style) {
		case DISSOLVENOMORPH:
		case PIXELDISSOLVE:
		case HSHUTTEROUT:
		case HSHUTTERIN:
		case IRISOUT:
		case IRISIN:
		case VSHUTTEROUT:
		case VSHUTTERIN:
		case WIPERIGHT:
		case WIPELEFT:
		case WIPEDOWN:
		case WIPEUP:
		case PLAINFORM:
		case FADEOUT:
		case FADEIN:
			break;
	}
	// Rechain styles
	PlaneShowStyle* lastStyle = NULL;
	PlaneShowStyle* search = showStyles;
	while (search) {
		if (search == style) break;
		lastStyle = search;
		search = search->next;
	}

	if (lastStyle) {
		lastStyle->next = style->next;
		if (style->XRangeArray != NULL)
			delete style->XRangeArray;
		delete style;
		style = lastStyle->next;
	}
	else {
		showStyles = style->next;
		if (style->XRangeArray != NULL)
			delete style->XRangeArray;
		delete style;
		style = showStyles;
	}
}

Bool
Plain(PlaneShowStyle* style)
{
	if (style->entry) {
		graphMgr->GPalette().SetFade(100);
	}
	else {
		graphMgr->GPalette().SetFade(0);
	}
	style->deleteMe = True;
	return True;
}

Bool
Fade(int direction,PlaneShowStyle* style)
{
	Bool noChange = True;
	uint percent;

	if (style->percent < style->divisions) {
		// calculate the starting percent
		if (direction > 0)
			percent = style->percent;
		else
			percent = style->divisions - style->percent - 1;

		if (style->XRangeSize == 0)
			graphMgr->GPalette().SetFade(percent*100/(style->divisions-1));

		else {
			for(int i=0;i<style->XRangeSize;i+=2) {
				graphMgr->GPalette().SetFade(percent*100/(style->divisions-1),style->XRangeArray[i],style->XRangeArray[i+1]);
			}
		}

		style->clock += style->speed;
		style->percent++;
		noChange	= False;
	}
	if ((style->percent >= style->divisions) && noChange)	{
		if (direction > 0) style->deleteMe = True;
		return True;
	}
	else
		return False;
}



int showStyleDivisions[16] =
	{1,		// PLAIN,		  -form PLAIN
	 20,		// HSHUTTEROUT	  -form HSHUTTER
	 20,		// HSHUTTERIN	  -form HSHUTTER	
	 20,		// VSHUTTEROUT	  -form VSHUTTER
	 20,		// VSHUTTERIN	  -form VSHUTTER	
	 10,		// WIPELEFT		  -form HWIPE
	 10,		// WIPERIGHT	  -form HWIPE	
	 10,		// WIPEUP		  -form VWIPE	
	 10,		// WIPEDOWN		  -form VWIPE
	 20,		// IRISOUT		  -form IRIS	
	 20,		// IRISIN		  -form IRIS	
	 6,		// DISSOLVENOMORPH	  -form PLAIN
	 10,		// PIXELDISSOLVE -form PLAIN
	 101,		// FADEOUT		  -form PLAIN
	 101,		// FADEIN		  -form PLAIN
	  	2};		// PALETTEMORPH
	  
int showStyleForms[16] =
	{PLAINFORM,	 
	 HSHUTTER,
	 HSHUTTER,
	 VSHUTTER,
	 VSHUTTER,
	 HWIPE,	 
	 HWIPE,
	 VWIPE,
	 VWIPE,	 
	 IRIS,	 
	 IRIS,	 
	 PLAINFORM,	 
	 PLAINFORM,	 
	 FADEFORM,	 
	 FADEFORM,	 
	 PLAINFORM};	 



// List of planes that have a show style in progress
PlaneShowStyle* showStyles;

// List of scrolling planes
PlaneScroll* planeScrolls;

