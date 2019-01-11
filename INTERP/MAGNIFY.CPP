
#include <string.h>

#include "sol.hpp"

#include "bufferd.hpp"
#include "celobj.hpp"
#include "graphmgr.hpp"
#include "graph.hpp"
#include "magnify.hpp"
#include "selector.hpp"
#ifdef WIN32S
#include "shiftpal.hpp"
#endif
#include "si.hpp"

/* 
   Magnify Object constructor
   Creates a node on a list of MagnifyView objects stores the bitmap
   information of both forground and background cels and creates a screen
   item def of the first location of the forground
*/

MagnifyView::MagnifyView(ObjectID ID,MagnifyView* theNext):
	id(ID),
	magID(ID.GetProperty(s_magnify)),
	view(ID.GetIndexedProperty(actView)),
	loop(ID.GetIndexedProperty(actLoop)),
	cel(ID.GetIndexedProperty(actCel))
{
   // prepare forground and background info
	CelObjView forground(view, loop, cel);
	CelObjView background(	magID.GetProperty(s_view),
									magID.GetProperty(s_loop),
									magID.GetProperty(s_cel)
				 				);

   // calculate boundy rectangle on screen
	boundry.A.x = magID.GetProperty(s_x);
	boundry.A.y = magID.GetProperty(s_y);
	power = magID.GetProperty(s_power);
	boundry.B.x = boundry.A.x + (background.Width() - 1) / power;
	boundry.B.y = boundry.A.y + (background.Height() - 1) / power;

   // store skip and remap colors
#ifndef WIN32S
	remapColor = magID.GetProperty(s_skip);
#else
	remapColor = (uchar)AddPalShift((int)magID.GetProperty(s_skip));
#endif
	skip = forground.Skip();

   // set next node value
   next = theNext;

	// store width and height of forground(BM) and background(BG) cels
	BMWidth = forground.Width();
	BMHeight = forground.Height();
	BGWidth = background.Width();
	BGHeight = background.Height();

	// allocate memory for the bitmaps
	BMData.Get(MemBitmap, BMWidth*BMHeight+CELHEADERSIZE);
	FGData.Get(MemBitmap, BMWidth*BMHeight);
	BGData.Get(MemBitmap, BGWidth*BGHeight);

   // store origen point of forground cel
   org.x = forground.XOrg();
   org.y = forground.YOrg();

	// build the header for the BMData
	BuildHeader(BMData,BMWidth,BMHeight,skip,org.x,org.y); 

	// inset is size of data
	SOL_Rect rect(0,0,BMWidth - 1,BMHeight - 1);

	// draw the cel into the bitmap
	SOL_Point pt(0,0);

   // fill background with skip of mem cels
   memset((uchar *)*BMData + CELHEADERSIZE, skip, BMWidth*BMHeight);
   memset((uchar *)*FGData, skip, BMWidth*BMHeight);

   // actual screen cel space, background added later
	BufferStd buffer(BMWidth, BMHeight, (uchar *)*BMData + CELHEADERSIZE);
	forground.Draw(buffer,rect,pt,False);

   // load forground pixel information						
	BufferStd bufferFG(BMWidth, BMHeight, (uchar *)*FGData);
	forground.Draw(bufferFG,rect,pt,False);

   // fill background with skip of background mem cel
   memset((uchar *)*BGData, background.Skip(), BGWidth*BGHeight);

   // load background pixel information
	rect.B.x = BGWidth - 1;
	rect.B.y = BGHeight - 1;
	BufferStd bufferBG(BGWidth, BGHeight, (uchar *)*BGData);
	background.Draw(bufferBG,rect,pt,False);

   // where does this get drawn
	pt.x = id.GetIndexedProperty(actX);
	pt.y = id.GetIndexedProperty(actY);
   // created cel to be displayed
	FillData(pt);

	// make bitmap for created cel
	Bitmap bitmap(BMData);
	ScaleInfo scaleInfo;

   // created screen item
	si = New ScreenItem(id.GetIndexedProperty(actPlaneID),bitmap,pt,scaleInfo,id);

   // add to picture
	graphMgr->AddScreenItem(si);
}
	
MagnifyView::~MagnifyView()
{
	BGData.Free();
	BMData.Free();
	FGData.Free();
}

/*
   FillData creates the screen cel from the forground and background
   note: the cel is preloaded with the original forground info only back 
   groundwill be added
*/

void
MagnifyView::FillData(SOL_Point pt)
{
   // if pt in rect then fill background in
   // else just fil remapColor with skip
	if(boundry.Intersect(pt)) {

		SOL_Point focus;
      // focus in upper left corner of forground compared to background.
      // calculate the point location of a screen coordinate in the 
      // background cel.
      //  +--------+  
      //  |        |  point in rect
      //  |        |  +---+
      //  |   *    |  | * |
      //  |        |  |   |
      //  |        |  |   |
      //  |        |  +---+
      //  |        |
      //  |        |
      //  +--------+
      // then correct the location be the offset of the origen of the
      // forground.
		focus.x = (pt.x - boundry.A.x) * power - org.x;
		focus.y = (pt.y - boundry.A.y) * power - org.y;

      // intialize forground and virtual cel
		uchar* ptrBM = (uchar *)*BMData + CELHEADERSIZE;
		uchar* ptrFG = (uchar *)*FGData;
		uchar* ptrBG;

      // for exery line in the forgroung
		for(int y = 0; y < BMHeight; y++) {
         // if this y location is in the background get the start of that 
         // line.
			if (y + focus.y >= 0 && y + focus.y < BGHeight)
				ptrBG = (uchar *)*BGData + (y + focus.y) * BGWidth;
			else
         	ptrBG = NULL;
         // for every pizel in this forground line
			for(int x = 0; x < BMWidth; x++) {
            // if this is remap color better change it to something
				if (*ptrFG == remapColor) {
               // if there is a current background line and our x location
               // is within the bound of that line get the background color
               // else it is skip.
					if (ptrBG && x + focus.x >= 0 && x + focus.x < BGWidth)
						*ptrBM = *((uchar*)(ptrBG + x + focus.x));
					else
						*ptrBM = skip;
				}
				ptrBM++;
				ptrFG++;
			}
		}
	} else {
      // fill all remap colors with skip
		uchar* ptrBM = (uchar *)*BMData + CELHEADERSIZE;
		uchar* ptrFG = (uchar *)*FGData;
		for(int y = 0; y < BMHeight; y++) {
			for(int x = 0; x < BMWidth; x++) {
				if (*ptrFG == remapColor) {
					*ptrBM = skip;
				}
				ptrBM++;
				ptrFG++;
			}
		}
	}
}

void
MagnifyView::Update()
{
	Bool update = False;

   // if the view loop or cel has changed record the change and flag that 
	// work must be done
	int holder;

	if((holder = id.GetIndexedProperty(actView)) != view){
		view = holder;
		update = True;
	}

	if((holder = id.GetIndexedProperty(actLoop)) != loop){
		loop = holder;
		update = True;
	}

	if((holder = id.GetIndexedProperty(actCel)) != cel){
		cel = holder;
		update = True;
	}
	 
	if(update) { 
      // get the new forground
	   CelObjView forground(view, loop, cel);

      // record old bitmap size
      long oldSize = BMWidth * BMHeight;

      // get new cel info
		BMWidth = forground.Width();
		BMHeight = forground.Height();
      skip = forground.Skip();
      org.x = forground.XOrg();
      org.y = forground.YOrg();

      // if the data size changed get new buffers after release of old.
		if (oldSize != BMWidth * BMHeight) {
			// Resize the memory allocations
		   BMData.Realloc(BMWidth*BMHeight+CELHEADERSIZE);
		   FGData.Realloc(BMWidth*BMHeight);
      }

      // build the header
		BuildHeader(BMData,BMWidth,BMHeight,skip,org.x,org.y); 

		// inset is size of data
		SOL_Rect rect(0,0,BMWidth - 1,BMHeight - 1);

		// draw the cel into the bitmap
		SOL_Point pt(0,0);

      // fill background with skip of mem cels
      memset((uchar *)*BMData + CELHEADERSIZE, skip, BMWidth*BMHeight);
      memset((uchar *)*FGData, skip, BMWidth*BMHeight);

      // read in new forground information to both virtual cel and forground 
      // cel.
		BufferStd buffer(BMWidth, BMHeight, (uchar *)*BMData + CELHEADERSIZE);
		forground.Draw(buffer,rect,pt,False);
											
		BufferStd bufferFG(BMWidth, BMHeight, (uchar *)*FGData);
		forground.Draw(bufferFG,rect,pt,False);

		si->SetInsetRect(SOL_Rect(0,0,BMWidth - 1,BMHeight - 1));

		si->pos.x =	id.GetIndexedProperty(actX);
		si->pos.y =	id.GetIndexedProperty(actY);

	} else {
      // location must be different
		si->pos.x = id.GetIndexedProperty(actX);
		si->pos.y = id.GetIndexedProperty(actY);
   }

   // calculate new virtual cel
	FillData(si->pos);
	
   // update it
	graphMgr->UpdateScreenItem(si);
}
