//	screen.cpp

#include "sol.hpp"

#include "graphmgr.hpp"
#include "msg.hpp"
#include "plane.hpp"
#include "rectlist.hpp"
#include "screen.hpp"




void
Screen::AbortPlane(Plane* plane)
{
	int index = planes.Search(plane->Id());
	if (index == -1)
		msgMgr->Fatal("AbortPlane - plane not found!");

	planes.Delete(index);
}

void
Screen::AddPlane(Plane* plane)
{
	int index = planes.Search(plane->Id());
	if (index == -1) {
		// Clip plane's onScreen by the screen
		plane->OnScreen().Clip(screenRect);
		planes.Add(plane);
	}
	else {
		// This plane already exists, just update it.
		plane->SetDeleteCount(0);
		if (!plane->AddCount())
			plane->SetUpdateCount(graphMgr->ScreenCount());
		// The planes list is kept In priority order
		planes.Sort();
	}
}

int
Screen::AddPic(long planeID,int thePic,SOL_Point& thePoint,int mirror)
{
	int index = planes.Search(planeID);
	if (index == -1)
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,planeID);
	return planes[index].AddPic(thePic,thePoint,mirror);
}

void
Screen::DeletePic(long planeID,int oldPic,int newPic)
{
	int index = planes.Search(planeID);
	if (index == -1)
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,planeID);
	planes[index].DeletePic(oldPic,newPic);
}

void
Screen::ScrollScreenItems(long planeID,int xStep,int yStep,Bool movePic)
{
	int index = planes.Search(planeID);
	if (index == -1)
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,planeID);
	planes[index].ScrollScreenItems(xStep,yStep,movePic);
}

void
Screen::AddScreenItem(ScreenItem* si)
{
	int index = planes.Search(si->PlaneId());
	if (index == -1) {
		msgMgr->Fatal(SrcLoc,Msg_InvalidPlane,si->PlaneId());
	}

	planes[index].AddScreenItem(si);
}

void
Screen::NewScreenDim(int newXdim,int newYdim)
{
	xDim = newXdim;
	yDim = newYdim;
	screenRect.Set(0,0,xDim - 1,yDim - 1);
	int count = planes.Length();
	for (int i = 0;i < count;i++) {
		planes[i].NewScreenDim(screenRect);	
	}
}

void
Screen::Clear()
{
	planes.Clear();
	showList.Clear();
}

void
Screen::PlaneRect(int planeID,SOL_Rect& planeRect)
{
	int index = planes.Search(planeID);
	if (index == -1)
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,planeID);
	planeRect = planes[index].OnScreen();
}

int
Screen::TopScreenItem(int planeID)
{
	int index = planes.Search(planeID);
	if (index == -1)
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,planeID);
	return planes[index].TopScreenItem();
}

void
Screen::NowSeen(int id,int planeID,SOL_Rect& nowSeen)
{
	int index = planes.Search(planeID);
	if (index == -1)
		msgMgr->Fatal(SrcLoc,Msg_PlaneNotFound,planeID);
	planes[index].NowSeen(id,nowSeen);
}

void
Screen::CalcLists(Screen& last,DrawList* drawLists[],RectList* eraseLists[])
{
	/*
	If a plane moves, then nextPlane must be intersected with the
	currPlane to created erase rectangles.


		 ____________________				     ____________________
		|currPlane           |				    |                    |
		|	                  |				    |	                   |
		|							|				    |	erase rect 1	    |
		|							|				    |						    |
		|			  ____________________	    |********* __________|
		|			 |nextPlane				 |	    |			  |
		|			 |                    |	    |		     |
		|			 |							 |	    |	erase	  |
		|			 |							 |	    |	rect	  |
		|			 |							 |	    |	2   	  |
		|			 |							 |	    |  		  |
		|			 |							 |	    |		     |
		|_________|							 |	    |_________|
					 |							 |
					 |							 |
					 |							 |
					 |							 |
					 |____________________|


	The erase rectangles created will be intersected with all other
	nextPlanes from highest priority to lowest priority. The overlaps
	will be added to the eraseList of the nextplane and then dumped if
	the plane is not transparent. Whats left after the overlap is 
	removed will be add to the list of erase rectangles to be processed
	againest all the nextPlanes of lower priority.
	*/

	SOL_Rect rects[4];
	RectList eraseRects;
	int nDeletedPlanes = 0;

	int nPlanes = planes.Length();
	PlaneList& lastPlanesList = last.Planes();

	// Go through the planes list resolving planes
	// that were moved, resized or deleted.
	// Also count the number of transparent planes
 	int transparentPlanesCount = 0;
	for (int i = 0; i < nPlanes;++i) {
		int passEraseRects = 0;
		int lastI = lastPlanesList.Search(planes[i].Id());
		if (planes[i].IsTransparent())
 			++transparentPlanesCount;

		if (planes[i].DeleteCount()) {
			if ((lastI != -1) && !lastPlanesList[lastI].OnScreen().IsEmpty()) {
				eraseRects.Add(lastPlanesList[lastI].OnScreen());
				++passEraseRects;
			}
			++nDeletedPlanes;
		}
		else {
			if ((lastI != -1) && planes[i].EraseCount())	{
				planes[i].DecEraseCount();
				// Check for the plane losing space
				int newRects = lastPlanesList[lastI].OnScreen().Split
				(
					planes[i].OnScreen(),
					rects
				);

				// rects is a list of the erase rectangles created by
				// the move or resize of a plane
				if (newRects != 0) {
					switch (newRects)	{

						case -1:
							// No overlap
							if (!lastPlanesList[lastI].OnScreen().IsEmpty())
								eraseRects.Add(lastPlanesList[lastI].OnScreen());
							break;

						default:
							// Multiple IRects not covered by nextPlane
							for (int i = 0; i < newRects; i++) {
								eraseRects.Add(rects[i]);
							}
							break;
					}
				++passEraseRects;
				}

				if (!planes[i].RedrawAllCount()) {
					// Check for the plane gaining space
					newRects = planes[i].OnScreen().Split
					(
						lastPlanesList[lastI].OnScreen(),
						rects
					);

					// rects is a list of the erase rectangles created by
					// the move or resize of a plane
					if (newRects != 0) {
						switch (newRects)	{

							case -1:
								// No overlap
								break;

							default:
								// Multiple IRects not covered by nextPlane
								for (int i = 0; i < newRects; i++) {
									eraseRects.Add(rects[i]);
								}
								break;
						}
					++passEraseRects;
					}
				}
			}
		}
		if (passEraseRects) {
			// Now eraseRects contains erase rectangle created by
			// the move, resize or delete. These erase rectangles 
			// are now to be distributed to other planes (assume 
			// that the	erase rectangles will be completely absorbed 
			// by	planes (an erase rectangle may go to it's own plane
			// if the plane got bigger)

			// Go through the planes from high to low priority
			// adding erase rectangles to the plane when there is
			// overlap

			// The planesList is sorted in low to high priority
			int rectsLastPri = lastPlanesList[lastI].Priority();

			for (int j = 0;j < eraseRects.Length();++j) {
				for (int k = nPlanes - 1;k >= 0;--k) {
					// eliminate the any deleted planes from this test
					// eliminate the i plane from this test (needed for deleted planes)
					// eliminate transparent planes (they will get passed erase rects
					// from non transparent planes


					if (!planes[k].DeleteCount() && !planes[k].IsTransparent()) {
						if (planes[k].OnScreen().Intersect(eraseRects[j],rects)) {
							// Add the overlap to the k plane if the k plane had
							// higher or equal priority than the i plane on the last frame out.
							if 
								(
								(!planes[k].RedrawAllCount())
								&&
								(rectsLastPri >= lastPlanesList[lastPlanesList.Search(planes[k].Id())].Priority())
								)
								eraseLists[k]->Add(rects[0]);
							int newRects = eraseRects[j].Split
								(
								planes[k].OnScreen(),
								rects
								);
							for (int l = 0;l < newRects;++l) {
								eraseRects.Add(rects[l]);
							}
							eraseRects.Delete(j);
							break;
						}
					}
				}
			}
			eraseRects.Pack();
		}
	}
	// All of the eraseRects should be absorbed 
	// by planes. EraseRects should be empty
#ifdef DEBUG
#ifndef WINDOWS
	if (eraseRects.Length())
		msgMgr->Fatal(SrcLoc,"Plane's erase rectangles not absorbed.");
#endif
#endif

	// If a deleted plane was encountered it may have to be deleted
	if (nDeletedPlanes) {
		for (i = nPlanes - 1; i >= 0; --i) {
			if (planes[i].DeleteCount())	{
				planes[i].DecDeleteCount();
				if (planes[i].DeleteCount() <= 0) {
					// delete the last screen's plane
					int lastI = lastPlanesList.Search(planes[i].Id());
					if (lastI != -1)
						lastPlanesList.Delete(lastI);

					// delete the next screen's plane
					planes.Delete(i);

					// Shift the erase and draw rectangles
					// down because deleting a plane shifts 
					// the planes list down
					delete eraseLists[i];
					for (int k = i;k < nPlanes - 1;++k) {
						eraseLists[k] = eraseLists[k+1];
					}
					eraseLists[nPlanes - 1]	= NULL;
					delete drawLists[i];
					for (k = i;k < nPlanes - 1;++k) {
						drawLists[k] = drawLists[k+1];
					}
					drawLists[nPlanes - 1]	= NULL;
				}
				if ((--nDeletedPlanes) <= 0) break;
			}
		}
	}

	// The number of planes may have changed because some were deleted
	nPlanes = planes.Length();

	// Go through the planes list resolving planes
	// that had their priority changed
	for (i = 0; i < nPlanes;++i) {
		if (planes[i].PriorityCount())	{
			planes[i].DecPriorityCount();
			int lastI = lastPlanesList.Search(planes[i].Id());
			// Use intersection because the plane 
			// may have been resized	or moved.
			planes[i].OnScreen().Intersect(lastPlanesList[lastI].OnScreen(),rects);
			eraseRects.Add(rects[0]);
			for (int j = nPlanes - 1;j >= 0;--j) {
				// Don't bother with transparent planes. They will 
				// get their erase rects from non transparent planes
				int lastJ = lastPlanesList.Search(planes[j].Id());
				int eraseRectsLength = eraseRects.Length();
				for (int k = 0; k < eraseRectsLength;++k) {
					int newRects = eraseRects[k].Split
					(
						planes[j].OnScreen(),
						rects
					);
					switch (newRects)	{
						case 0:
							// Completey overlaped
							// Has the relative priority changed?

							// Note: this check is not taking into account
							// different planes that overlap and have the
							// same priority.	If two planes have the same
							// priority and overlap then a change of priority
							// to one of these planes may cause an incorrect 
							// resolution for the priority change.

							if (lastJ != -1) {
								if
								(
								(
								(lastPlanesList[lastI].Priority() - lastPlanesList[lastJ].Priority())
								*
								(planes[i].Priority() - planes[j].Priority())
								)
								<= 0
								) {
									// Add to the erase rects of the higher priority plane
									if (planes[i].Priority() > planes[j].Priority()) {
										eraseLists[i]->Add(eraseRects[k]);
									}
									else {
										eraseLists[j]->Add(eraseRects[k]);
									}
								}
							}
							eraseRects.Delete(k);
							break;

						case -1:
							// No overlap
							break;

						default:
							// Multiple IRects not covered by plane[j]
							for (int l = 0; l < newRects; l++) {
								eraseRects.Add(rects[l]);
							}
							// Has the relative priority changed?

							// Note: this check is not taking into account
							// different planes that overlap and have the
							// same priority.	If two planes have the same
							// priority and overlap then a change of priority
							// to one of these planes may cause an incorrect 
							// resolution for the priority change.
							if (lastJ != -1) {
								if
								(
								(
								(lastPlanesList[lastI].Priority() - lastPlanesList[lastJ].Priority())
								*
								(planes[i].Priority() - planes[j].Priority())
								)
								<= 0
								) {
									// Add the overlap to the erase rects of the higher priority plane
									planes[i].OnScreen().Intersect(planes[j].OnScreen(),&eraseRects[k]);
									if (planes[i].Priority() > planes[j].Priority()) {
										eraseLists[i]->Add(eraseRects[k]);
									}
									else {
										eraseLists[j]->Add(eraseRects[k]);
									}
								}
							}
							eraseRects.Delete(k);
							break;
					}
				}
				eraseRects.Pack();
			}
		}
	}

	// Go through the planes list calling each 
	// to a calculate draw and erase list.
	for (i = 0;i < nPlanes;++i) {
		int lastI = lastPlanesList.Search(planes[i].Id());
		Plane* lastPlane = NULL;
		if (lastI != -1)
			lastPlane = lastPlanesList.Pointer(lastI);
		if (planes[i].RedrawAllCount())
			planes[i].RedrawAll(lastPlane,planes,*drawLists[i],*eraseLists[i]);
		else  {
			// If the plane did not exist on the last frame out,
			// it's redrawAllCount should be True
			if (lastPlane == NULL)
				msgMgr->Fatal(SrcLoc,Msg_PrevPlaneNotFound,planes[i].Id());
			planes[i].CalcLists(lastPlane,planes,*drawLists[i],*eraseLists[i]);
		}

		// Adjust the last screen planes list for added or updated planes
		if (planes[i].AddCount()) {
			Plane* lastPlane = New Plane(planes[i]);
			lastPlanesList.Add(lastPlane);
			planes[i].DecAddCount();
		}
		else {
			if (planes[i].UpdateCount()) {
				lastPlanesList[lastI] = planes[i];
				planes[i].DecUpdateCount();
			}
		}
	}

	// Now do some work for transparent planes
	if (transparentPlanesCount) {
		for (i = 0;i < nPlanes;++i) {
			int j;

			for (j = i+1;j < nPlanes;++j) {
				if (planes[j].IsTransparent()) {
					planes[j].FilterUpEraseRects(*drawLists[j],*eraseLists[i]);
				}
			}

			if (planes[i].IsTransparent()) {
				int j;
				for (j = i-1;j >= 0;--j) {
					planes[j].FilterDownEraseRects(*drawLists[j],*eraseLists[j],*eraseLists[i]);
				}
				// eraseList for the transparent plane should be empty
				if (eraseLists[i]->Length() > 0 )
					msgMgr->Fatal(SrcLoc,"Transparent plane's erase list not absorbed");
			}

			for (j = i+1;j < nPlanes;++j) {
				if (planes[j].IsTransparent())
					planes[j].FilterUpDrawRects(*drawLists[j],*drawLists[i]);
			}
		}
	}
}

void
Screen::RemapMarkRedraw()
{
	int nPlanes = planes.Length();
	for(int i = 0; i < nPlanes; ++i) {
		planes[i].RemapMarkRedraw();
   }
}

ScreenItem* 
Screen::GetScreenItem(int planeId, long siId)
{
	int index = planes.Search(planeId);
	if (index == -1)
		return NULL;

	ScreenItemList& siList = planes[index].SIList();
	index = siList.Search(siId);
	if (index == -1)
		return NULL;
	return &siList[index];
}

Plane* 
Screen::GetPlane(int planeId)
{
	int index = planes.Search(planeId);
	if (index == -1)
		return NULL;
	return &planes[index];
}
