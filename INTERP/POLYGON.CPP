// NAME---POLYGON---
//
// Algorithms by Larry Scott 
//
//    POLYGONs must be closed and not self intersecting (JORDAN polygon). 
//
//    All integers are assumed to be at least 32 bits

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "sol.hpp"

#include "array.hpp"
#include "kernel.hpp"
#include "math.hpp"
#include "msg.hpp"
#include "object.hpp"
#include "pmachine.hpp"
#include "polygon.hpp"
#include "selector.hpp"

SCI_Point planePoly[4];

MemID 
BuildPolygonList(SOL_ListID theList,int listSize)
{
   int i = 0;
   MemID polygonListID;
	polygonListID.Get(MemPolygonList,(listSize+1)*sizeof(SOL_Polygon));
	SOL_Polygon* polygonList = (SOL_Polygon*) memMgr->GetAddr(polygonListID);

   SOL_List* list = *theList;
	for (KNodeID node = list->First();(int) node;node = node->Next(),++i) {
      ObjectID id = (ObjectID) node->val;

		polygonList[i].type = (uchar) id.GetProperty(s_type);
		if (polygonList[i].type == MBP) {
			// Don't add the polygons to be merged
			--i;
			continue;
		}
		
		ObjectID arrayID = id.GetProperty(s_points);
		MemID pointsID = arrayID.GetProperty(s_data);

      SCI_Point* points = (SCI_Point*) memMgr->GetAddr(pointsID);
		polygonList[i].polyPoints = &points[1];

		int size = id.GetProperty(s_size);
      if (size < 3)
		   msgMgr->Fatal(SrcLoc,Msg_NotAPolygon);
		polygonList[i].n = size;

		polygonList[i].info = 0; 
	}
	polygonList[i].polyPoints = NULL;
	polygonList[i].n = 0;
	polygonList[i].type = 0;
	polygonList[i].info = 0; 

   return(polygonListID);
}

void 
MergeMBPPolygons(SOL_ListID SCIList,MemID polyListID)
{
   int i = 0;

   SOL_List* list = *SCIList;
	for (KNodeID node = list->First();(int) node;node = node->Next(),++i) {
      ObjectID id = (ObjectID) node->val;

		if ((uchar) id.GetProperty(s_type) == MBP) {
			// Merge this polygon

			ObjectID arrayID = id.GetProperty(s_points);
			MemID pointsID = arrayID.GetProperty(s_data);

      	SCI_Point* points = (SCI_Point*) memMgr->GetAddr(pointsID);

			int size = id.GetProperty(s_size);
      	if (size < 3)
		   	msgMgr->Fatal(SrcLoc,Msg_NotAPolygon);
			MemID mergedPolygonID = MergePolygons(points,size,(SOL_Polygon*) memMgr->GetAddr(polyListID));

			SOL_Polygon* polyList = (SOL_Polygon*) memMgr->GetAddr(polyListID);

			// Eliminate any merged polygons MBP,MTP,MNP or MCP
   		for (int j=0;polyList[j].n != 0;++j)
      		{
      		if ((polyList[i].type >= MBP) ||
            	 (polyList[i].type == MTP) ||
            	 (polyList[i].type == MNP) ||
            	 (polyList[i].type == MCP))
         		{
         		DeletePolygon(polyList,j);
         		--j;
         		}
      		}

			// Add the merged polygon (MBP) as a barred access polygon (BAP)
			AddPolygon(polyListID,points,size,BAP);
		}
	}
}


void KAvoidPath(argList)
{	
   // Input
   // Arg1 = A.x
   // Arg2 = A.y
   // Arg3 = B.x
   // Arg4 = B.y
   // Arg5 = polygon list
   // Arg6 = Plane width
   // Arg7 = Plane height
   // Arg8 = optimize level (if no Arg8 then set to 1)

	SOL_ListID theList;
   int optimize = 1;
	int listSize = 0;

   SCI_Point A(arg(1),arg(2));
   SCI_Point B(arg(3),arg(4));

	if (arg(5)) {
      ObjectID id = arg(5);
		theList = id.GetProperty(s_elements);
		// the number of polygons
		listSize = id.GetProperty(s_size);
   }

	// handle no obstacle case... just act like a MoveTo
	if (!arg(5) || !listSize) {
      MemID pathID;
		pathID.Get(MemPointList,4*sizeof(SCI_Point));
      SCI_Point* path = (SCI_Point*) *pathID;
      // element size
      path[0].x = 2;
      // max elements
      path[0].y = 6;
		path[1] = A;
		path[2] = B;
		path[3].x = ENDOFPATH;
		path[3].y = ENDOFPATH;
		pm.acc = pathID;
		return;
	}

   // Plane dimensions
   SCI_Point plane(arg(6),arg(7));

	// optimize path or not?
	if (argCount >= 8)
		optimize	= arg(8);

   MemID polygonListID = BuildPolygonList(theList,listSize);

	// Merge any MBP polygons
   MergeMBPPolygons(theList,polygonListID);

	pm.acc = (Acc) GetPath(A,B,(SOL_Polygon*) memMgr->GetAddr(polygonListID),optimize,plane);
   polygonListID.Free();
}

void KInPolygon(argList)
{	
   // Input
   // Arg1 = A.x
   // Arg2 = A.y
   // Arg3 = polygon

   SCI_Point A(arg(1),arg(2));

   ObjectID polygonID = arg(3);

	ObjectID arrayID = polygonID.GetProperty(s_points);
	MemID pointsID = arrayID.GetProperty(s_data);
   SCI_Point* points = (SCI_Point*) memMgr->GetAddr(pointsID);
	int size =  polygonID.GetProperty(s_size);
	if (size < 3)
	   msgMgr->Fatal(SrcLoc,Msg_NotAPolygon);

	pm.acc = PointInterior(A,&points[1],size );
}

void KMergePoly(argList)
{
   // Input
   // Arg1 = SCI polygon object
   // Arg2 = polygon list

	SOL_ListID  theList;
   int         listSize;
   int         i = 0;

	if (arg(2)) {
      ObjectID id = arg(2);
		theList = id.GetProperty(s_elements);
		// the number of polygons
		listSize = id.GetProperty(s_size);
   }

	// handle no polygons case... just return the given polygon
	if (!arg(2) || !listSize) {
		pm.acc = arg(1);
		return;
	}

   MemID polygonListID = BuildPolygonList(theList,listSize);
	SOL_Polygon* polygonList = (SOL_Polygon*) memMgr->GetAddr(polygonListID);


   ObjectID SCIPolygonID = arg(1);
   int size = SCIPolygonID.GetProperty(s_size);
   MemID pointsID = ((ObjectID) SCIPolygonID.GetProperty(s_points)).GetProperty(s_data);
   SCI_Point* points = (SCI_Point*) ((char*) memMgr->GetAddr(pointsID) + sizeof(Array));

	pm.acc = MergePolygons(points,size,(SOL_Polygon*) memMgr->GetAddr(polygonListID));

	SOL_List* list = *theList;
	polygonList = (SOL_Polygon*) memMgr->GetAddr(polygonListID);
	for (KNodeID node = list->First(); (int) node;node = node->Next(),i++) {
		ObjectID id = node->val;
		id.SetProperty(s_type,polygonList[i].type);
	}
}

// GETPATH---PATH AROUND A SET OF OBSTACLE POLYGONS
MemID 
GetPath(SCI_Point& A,SCI_Point& B,SOL_Polygon* polylist,int opt,SCI_Point& planeDim)
{

   // Types of polygon exits and polygon entries
   enum PathStartType
      {
      fromA,
      fromAtoEXIT,
      fromBAPtoEXIT
      };

   enum PathEndType 
      {
      toB,
      toENTRYtoB,
      toENTRY
      };

   #define	INVERTED	1

   int         pathType;
   int         i;
	SCI_Point   exitPoint,entryPoint;
   SCI_Point   I1,I2;
   int         nodeI1,nodeI2;
   int         pathStart = fromA;
   int         pathEnd = toB;
   SCI_Point   path[MAXPOLYPATH];

#ifdef POLYTEST
   // Make polygons clockwise.
	// The following code, TestClockwise and InvertPolygon are
   // only needed if some polygons may be defined in a counter
   // clockwise direction. Clockwise refers to users view point
   // which is counter clockwise mathematically.

   for (i=0;polylist[i].n != 0;++i)
      {
      if (testClockwise(polylist[i].polyPoints,polylist[i].n) < 0)
         {
         if(polylist[i].type!=CAP)
            {
            InvertPolygon(polylist[i].polyPoints,polylist[i].n);
		      Message("Direction of !CAP polygon counter clockwise. Fixed!");
            }
         }
      else
         {
         if(polylist[i].type==CAP)
            {
            InvertPolygon(polylist[i].polyPoints,polylist[i].n);
		      Message("Direction of CAP polygon clockwise. Fixed!");
            }

         }
      // also test for adjacent colinear line segments
		if (int count = TestColinear(polylist[i].polyPoints,&polylist[i].n))
         {
		   Message("%d adjacent colinear line segments found. Fixed!",count);
         }
      }
#endif

   // set screen polygon
   planePoly[0].x = 0;
   planePoly[0].y = 0;
   planePoly[1].x = planeDim.x - 1;
   planePoly[1].y = 0;
   planePoly[2].x = planeDim.x - 1;
   planePoly[2].y = planeDim.y - 1;
   planePoly[3].x = 0;
   planePoly[3].y = planeDim.y - 1;

   // TAP polygons are invisable when optimization is off
   for (i=0;polylist[i].n != 0;++i)
      {
      if (
            (polylist[i].type >= MTP) ||
            ((polylist[i].type == TAP) && (opt == False))
         )
         {
         DeletePolygon(polylist,i);
         --i;
         }
      }
   // are we starting in a polygon?
   for (i=0;polylist[i].n != 0;++i)
      {
      if (
            PointInterior(A,polylist[i].polyPoints,polylist[i].n) ||
            (
               (polylist[i].type==CAP) &&
               (NearPoint(A,polylist[i].polyPoints,polylist[i].n,&I2,True)<2)
            )
         )
         {
         switch (polylist[i].type)
            {
            case TAP:
               DeletePolygon(polylist,i);
               --i;
               break;
            case NAP:
               pathStart = fromAtoEXIT;
               if (opt)
                  NearPoint(A,polylist[i].polyPoints,polylist[i].n,&exitPoint,True);
               else
                  DeletePolygon(polylist,i);
               break;
            case BAP:
               pathStart = fromBAPtoEXIT;
               NearPoint(A,polylist[i].polyPoints,polylist[i].n,&exitPoint,True);
               break;
            case CAP:
               if (!PointInterior(B,polylist[i].polyPoints,polylist[i].n))
                  {
                  /* Point A is interior and point B is exterior */
                  pathEnd = toENTRY;
                  /* Find the last exit point */
                  if (opt)
                     {
                     NearPoint(B,polylist[i].polyPoints,polylist[i].n,&entryPoint,False);
                     }
                  else
                     {
                     IntersectPolygon(A,B,polylist[i].polyPoints,polylist[i].n,&I1,&I2,&nodeI1,&nodeI2);
                     entryPoint = I1;
                     }
                  }
               break;
            }
         }
      else
         {
         // Turn CAP in BAP
         if(polylist[i].type==CAP)
            {
            InvertPolygon(polylist[i].polyPoints,polylist[i].n);
            polylist[i].type=BAP;
            polylist[i].info = polylist[i].info | INVERTED;
      
            }
         }
      }

   // are we ending in a polygon?
   for (i=0;polylist[i].n != 0;++i)
      {
      if (polylist[i].type!=CAP && PointInterior(B,polylist[i].polyPoints,polylist[i].n))
         {
         switch (polylist[i].type)
            {
            case TAP:
               pathEnd = toB;
               DeletePolygon(polylist,i);
               --i;
               break;
            case NAP:
               pathEnd = toENTRYtoB;
               if (opt)
                  NearPoint(B,polylist[i].polyPoints,polylist[i].n,&entryPoint,True);
               else
                  DeletePolygon(polylist,i);
               break;
            case BAP:
               // If pathEnd == toENTRY we must be trying to get out of a CAP
               if(pathEnd != toENTRY)
                  {
                  pathEnd = toENTRY;
                  NearPoint(B,polylist[i].polyPoints,polylist[i].n,&entryPoint,True);
                  }
               break;
            }
         }
      }

   // Change saved CAPs into BAPs
   for (i=0;polylist[i].n != 0;++i)
      {
      if(polylist[i].type==CAP)
         {
         polylist[i].type=BAP;
         }
      }

   pathType = (pathStart << 2) + pathEnd;
   switch (pathType)
      {
      case 0:
         // fromA ---> toB
         AvoidPath(A,B,polylist,opt,path);
         break;
      case 1:
         // fromA ---> toENTRYtoB
         if (opt)
            {
            AvoidPath(A,entryPoint,polylist,opt,path);
            EndPath(B,path);
            }
         else
            {
            AvoidPath(A,B,polylist,opt,path);
            }
         break;
      case 2:
         // fromA ---> toENTRY
         AvoidPath(A,entryPoint,polylist,opt,path);
         break;
      case 4:
         // fromAtoEXIT ---> toB
         if (opt)
            {
            AvoidPath(exitPoint,B,polylist,opt,path);
            StartPath(A,path);
            }
         else
            {
            AvoidPath(A,B,polylist,opt,path);
            }
         break;
      case 5:
         // fromAtoEXIT ---> toENTRYtoB
         if (opt)
            {
            AvoidPath(exitPoint,entryPoint,polylist,opt,path);
            StartPath(A,path);
            EndPath(B,path);
            }
         else
            {
            AvoidPath(A,B,polylist,opt,path);
            }
         break;
      case 6:
         // fromAtoEXIT ---> toENTRY
         if (opt)
            {
            AvoidPath(exitPoint,entryPoint,polylist,opt,path);
            StartPath(A,path);
            }
         else
            {
            AvoidPath(A,entryPoint,polylist,opt,path);
            }
         break;
      case 8:
         // fromBAPtoEXIT ---> toB
         AvoidPath(exitPoint,B,polylist,opt,path);
         if (opt) StartPath(A,path);
         break;
      case 9:
         // fromBAPtoEXIT ---> toENTRYtoB
         if (opt)
            {
            AvoidPath(exitPoint,entryPoint,polylist,opt,path);
            StartPath(A,path);
            EndPath(B,path);
            }
         else
            {
            AvoidPath(exitPoint,B,polylist,opt,path);
            }
         break;
      case 10:
         // fromBAPtoEXIT ---> toENTRY
         AvoidPath(exitPoint,entryPoint,polylist,opt,path);
         if (opt) StartPath(A,path);
         break;
      } 

    // Re invert previously inverted polygons
   for (i=0;polylist[i].n != 0;++i)
     {
     if(polylist[i].info & INVERTED)
        {
         InvertPolygon(polylist[i].polyPoints,polylist[i].n);
        }
     }
   // Allocate memory for the return path, put the points
   // into the allocation and return the ID
   return CopyPath(path);
}

// Removes a polygon from a polygon list
void 
DeletePolygon(SOL_Polygon* polylist,int i)
{
   for (;polylist[i].n != 0;++i)
      {
      polylist[i] = polylist[i+1];
      }
}

// Add a polygon to a polygon list
void
AddPolygon(MemID polyListID,SCI_Point* points,int size,uchar type)
{
	SOL_Polygon* polyList = (SOL_Polygon*) memMgr->GetAddr(polyListID);

	// Assumes the polylist is large enough to add another entry
   for (int i=0;polyList[i].n != 0;++i)
      {}

	polyList[i].polyPoints = points;
	polyList[i].n = size;
	polyList[i].type = type;
	polyList[i++].info = 0; 

	polyList[i].polyPoints = NULL;
	polyList[i].n = 0;
	polyList[i].type = 0;
	polyList[i].info = 0; 

}

// Adds a point to the begining of a given path
void 
StartPath(SCI_Point& A,SCI_Point* path)
{
   SCI_Point p1 = A;
   int i = 0;

   while (path[i].x != ENDOFPATH)
   {
      SCI_Point p2 = path[i];
      path[i++] = p1;
      p1 = p2;
   }
   path[i++] = p1;
   path[i].x = ENDOFPATH;
}

// Adds a point to the end of a given path
void
EndPath(SCI_Point& B,SCI_Point* path)
{
   int i = 0;
   do
      ++i;
   while (path[i].x != ENDOFPATH);
   path[i++] = B;
   path[i].x = ENDOFPATH;
}

MemID
CopyPath(SCI_Point* path)
{
   int   i = 0;
   MemID pathID;
   do
      {
      }
   while  (path[i++].x != ENDOFPATH);
   // This must look like an SOL array 
	pathID.Get(MemPointList,(i+1)*sizeof(SCI_Point));
	SCI_Point* points = (SCI_Point*) *pathID;
   // element size
   points[0].x = 2;
   // max elements
   points[0].y = 2*i;
   memcpy(&points[1],path,4*i);
   return(pathID);
}

void
InvertPolygon(SCI_Point* points,int n)
{
   int i,j;
   for (i = 0,j = n-1;i<j;)
      {
      SCI_Point p = points[i];
      points[i++] = points[j];
      points[j--] = p;
      }
}

// Calculate the point on the given polygon that is nearest
// to a given point. If edge is true then disregard all points
// on the plane edge.
int 
NearPoint(SCI_Point& P,SCI_Point* polygon,int n,SCI_Point* R,Bool Edge)
{
   int         i;
   int         d=INT_MAX,dot1,dot2,dot3;
   SCI_Point   A,B,tmp1,tmp2,tmp3,tmp4;
   Bool        offScreen;

   for (i=0;i<n;++i)
      {
      A = polygon[i];
      if (i==n-1)
         B = polygon[0];
      else
         B = polygon[i+1];
      if 
         (
            Edge                        
         &&
            (
            (A.x == B.x) && ((A.x == 0) || 
            (A.x == planePoly[1].x))    ||
            (A.y == B.y) && ((A.y == 0) ||
            (A.y == planePoly[2].y))
            )
         )

         {
         // do nothing, the line is on the screen edge
         }
      else
         {
         // If P is closest to this line save the closest on this line to P

         tmp1 = B - A;
         tmp2 = A - P;
         tmp3 = B - P;

         if (((tmp1*tmp2) <= 0) && ((tmp1*tmp3) >= 0))
            /* P-R is normal to A-B 
          
                          A
            ***************************
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
              P----------*R
                         *
                         *
                         * 
            ***************************
                         B
            */

            {
            // Distance P-R is:
            // ABS{[(B-A)^(0,0,1)]*(A-P)/|B-A|}

            /* Make N = tmp4 normal to A-B */
            tmp4.x = tmp1.y;
            tmp4.y = -tmp1.x;
            dot1 = tmp2*tmp4;
            dot2 = dot1/DistanceEstimate(A,B,&offScreen);
            if (dot2 <0) dot2 = -dot2;
            if (dot2 < d)
               {
               d = dot2;
               
               // R = P + [(N*(A-P))/(N*N)]N
               // round to force point exterior to polygon
               dot2 = tmp4*tmp4;
               dot3 = dot1*(int)tmp4.x;
               R->x = P.x + (int) 
                  (
                  (
                  dot3 + sign(dot3)*(dot2 - 1)
                  )
                  /dot2
                  );
               dot1 = dot1*(int)tmp4.y;
               R->y = P.y + (int)
                  (
                  (
                  dot1 + sign(dot1)*(dot2 - 1)
                  )
                  /dot2
                  );
               }
            }
         else
            {
            /* P-R is not normal to A-B, for example 
          
                 P*
                   *
                    *
                     *
                      *
                       *
                      R * A
            ***************************
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         *
                         * 
                         *
                         *
                         * 
            ***************************
                         B
            */

            dot1 = DistanceEstimate(A,P,&offScreen);
            dot2 = DistanceEstimate(B,P,&offScreen);
            if (dot1 < d)
               {
               d = dot1;
               *R = A;
               }
            if (dot2 < d)
               {
               d = dot2;
               *R = B;
               }
            }
         }
      }
   return(d);
}

Bool
TestClockwise(SCI_Point* Points,int n)
{
   int   i;
   int   angle = 0;
   int   angleStart,angleIn,angleOut,delta;
   angleStart = ATan(Points[0].x,Points[0].y,Points[1].x,Points[1].y);
   angleIn = angleStart;
   for (i=1;i<n-1;++i)
      {
      angleOut = ATan(Points[i].x,Points[i].y,Points[i+1].x,Points[i+1].y);
      delta = angleOut - angleIn;
      if (delta > 180) delta -= 360;
      if (delta < -180) delta += 360;
      angle += delta;
      angleIn = angleOut;
      }
   // next nine lines only needed for initial testing
   angleOut = ATan(Points[i].x,Points[i].y,Points[0].x,Points[0].y);
   delta = angleOut - angleIn;
   if (delta > 180) delta -= 360;
   if (delta < -180) delta += 360;
   angle += delta;
   delta = angleStart - angleOut;
   if (delta > 180) delta -= 360;
   if (delta < -180) delta += 360;
   angle += delta;
   // at this point if previous nine lines
   // are compiled then angle = +-360
   return(angle);
}

int
TestColinear(SCI_Point* P,int* n)
{
   int  i,j;
   int  retValue = 0;
   int  nodes;
   SCI_Point P1,P2,P3,tmp1,tmp2;

   nodes = *n;
   P1 = P[0];                                            
   P2 = P[1];
   P3 = P[2];
   for (i=3;i<(nodes+2);++i)
      {
      tmp1 = P2 - P1;
      tmp2 = P3 - P2;
      if ((tmp1^tmp2) == 0)
         {
         // count of adjacent colinear line segments encountered
         ++retValue;
         // eliminate the extra node
         --*n;
         for (j=i;j<(nodes+1);++j)
            {
            P[j-2] = P[j-1];
            }
         --nodes;
         // retry this node
         --i;
         P2 = P3;
         }
      else
         {
         P1 = P2;
         P2 = P3;
         }
      P3 = P[i % nodes];
      }
   return (retValue);
}



//                  PATH AROUND ALL OBSTACLE POLYGONS
//
//
// NAME---AvoidPath---
//    polygons must be closed and not self intersecting (JORDAN ploygon) 
//    and defined in a clockwise direction.
// input:
//   line segment A-B & polygonList
//   optimization level and pointer to resultant path
// output:
//   path
//
//
//   EXAMPLE:
//        AVOIDPATH(A,B,polyList,opt,path) ;polyList = pathM,pathN (any order)
//
//        returns path:
//         NO OPTIMIZATION: opt = False
//            A,I1,N4,N3,N2,I2,I3,M5,M4,M3,M2,I4,B
//         WITH OPTIMIZATION: opt = True
//            A,N4,M5,M4,B
//        
//                  
//
//
//                                              M7 ********* M8
//                                                *        *
//               N0     N1                       *         *
//               ********                       *          * M0
//              *        *                     *            *
//             *          *                   *              *
//            *            *                 *                *
//           *              *               *                  *
//       N5 *                *          M6 *                    *
//          *I1               * I2        I3*                 I4 * M1
//  A-------*------------------*-------------*------------------*----------B
//          *                   *N2           *                * M2
//          *                  *               *               *
//          *                 *                 *              *
//       N4 ****************** N3                *             *
//                                                *            * M3
//                                                 *          *
//                                                  *        *
//                                                   *      *
//                                                 M5 ****** M4
//
//
//
//  - DESIGNATES SUBTRACTION OF 2 VECTORS      (returns a vector)
//  + DESIGNATES ADDITION OF 2 VECTORS         (returns a vector)
//  * DESIGNATES DOT PRODUCT OF 2 VECTORS      (returns a scalar)
//  ^ DESIGNATES CROSS PRODUCT OF 2 VECTORS    (returns a vector)
//  | V | DESIGNATES SIZE OF VECTOR V          (returns a scalar)
//  V(i) DESIGNATES COORDINATE i of VECTOR V   (returns a scalar)
//
//
//   polyNode:
//      next           :next polyNode
//      prev           :previous polyNode
//      I1             :(x,y) value of nearest intersection with A
//      I2             :(x,y) value of nearest intersection with B
//      d              :1 normal direction, -1 reverse direction
//      I1             :entry node
//      I2             :exit node
//      poly           :pointer to polygon
//
//
//   polyList:
//      polygon0       :pointer to possible polygon obstacle
//      polygon1       :pointer to possible polygon obstacle
//      polygon2       :pointer to possible polygon obstacle
//        .
//        .
//        .
//      polygonN       :pointer to possible polygon obstacle
//        0            :indicates end of polyList
//
//

void
AvoidPath(SCI_Point& A,SCI_Point& B,SOL_Polygon* polylist,int opt,SCI_Point* path)
{
   int         i = -1,j = -1;
   SCI_Point   startPath;
   PolyNode*   nodePath;
   PolyNode*   polygonNodePath;
	SOL_Polygon dupPolyList[MAXPOLYGONS];
   int         totalNodes = 0;
   int         polyDirections,firstPolyDirection,bestPolyDirection;
   int         polyDistance,bestPolyDistance;

   nodePath = 0;
   // duplicate polylist so that we can eliminate polygons while
   // creating unoptimized path yet still retain original polylist
   // for optimization pass.

   startPath = A;

   //   HOOK  
   // this is a good place to get rid of unwanted polygons ie ctp,cnp etc.
   do
      {
      ++i; ++j;
      if (polylist[i].type >= MTP) 
         --j;
      else
          dupPolyList[j] = polylist[i];
      }
   while (polylist[i].n != 0);

   if (j > MAXPOLYGONS) 
		msgMgr->Fatal(SrcLoc,Msg_TooManyPolygons);

   while (polygonNodePath = AvoidPolygons(startPath,B,dupPolyList))
      {
      ++totalNodes;
      // add first node in polygonNodePath to nodePath
      // note that nodePath points to last node until path complete.
      if (nodePath)
         {
         nodePath->next = polygonNodePath;
         polygonNodePath->prev = nodePath;
         }
      // free any unused nodes
		FreeNodePath(polygonNodePath->next);
      polygonNodePath->next = 0;
      nodePath = polygonNodePath;
      startPath = polygonNodePath->I2;


      // Now reduce list of polygons to only those which can interfere
      // with the path starting from the exit of the first polygon and
      // going to the point B. We only generate a path around the first
      // encountered polygon and then recurse to generate a path around
      // the next etc. until there are no more polygons in the way.
      
      ReducePolygonList(dupPolyList,polygonNodePath);
      }

   // set nodePath to point to the first node
   if (nodePath)
      {
      while (nodePath->prev != 0)
         {
         nodePath = nodePath->prev;
         }
      }

   // Now chain together the paths around the polygons
   // and optimize out any unnecessary line segments.
   if ((opt>1) && (totalNodes > 1))
      {
      // Optimize as much as possible to the level 2**n.
      // Set up current path directions and try to improve
      // on the total distance from A to B.
      if(totalNodes > MAXOPTIMIZEDNODES) totalNodes = MAXOPTIMIZEDNODES;
      polyDirections = GetPolyDirections(nodePath) % (1<<totalNodes);
      bestPolyDirection = polyDirections;
      firstPolyDirection = polyDirections;
      OptimizePath(A,B,path,nodePath,polylist,opt);
      bestPolyDistance = GetPolyDistance(path);
      while (firstPolyDirection != (polyDirections = (polyDirections + 1) % (1<<totalNodes)))
         {
         SetPolyDirections(nodePath,polyDirections,totalNodes);
         OptimizePath(A,B,path,nodePath,polylist,opt);
         polyDistance = GetPolyDistance(path);
         if (bestPolyDistance > polyDistance)
            {
            bestPolyDirection = polyDirections;
            bestPolyDistance = polyDistance;
            }
         }
      SetPolyDirections(nodePath,bestPolyDirection,totalNodes);
      }
   OptimizePath(A,B,path,nodePath,polylist,opt);
   FreeNodePath(nodePath);

   return;
}

void
FreeNodePath(PolyNode* nodePath)
{
   PolyNode*   node;
   PolyNode*   tmp;
   for (node=nodePath;node != 0;)
      {
      tmp = node->next;
      delete node;
      node=tmp;
      }
}

PolyNode*
AvoidPolygons(SCI_Point& A,SCI_Point& B,SOL_Polygon* polylist)
{
   int         n1,n2,d,i;
   SCI_Point   I1,I2;
   PolyNode*   node;
   PolyNode*   newNode;
   PolyNode*   nodePath;

   nodePath = 0;
   if (A != B)
      {
      for (i=0;polylist[i].n != 0;++i)
         {
         if (d = PolygonPath(A,B,polylist[i].polyPoints,polylist[i].n,&I1,&I2,&n1,&n2))
            {
            // this polygon is an obstruction, create path node

            newNode = New PolyNode(I1,I2,d,n1,n2,polylist[i].polyPoints,polylist[i].n);
            // add path node to list
            if (!nodePath)
               /* newNode first node in path */
               {
               nodePath = newNode;
               }
            else
               // insert path node so that the list is ordered from A to B
               {
               for (node = nodePath;True;node = node->next)
                  {
                  if (
                     (
                     (node->I1.x != A.x) 
                     && 
                     (abs(newNode->I1.x - A.x) < abs(node->I1.x - A.x))
                     )
                     ||
                     (
                     (node->I1.y != A.y) 
                     && 
                     (abs(newNode->I1.y - A.y) < abs(node->I1.y - A.y))
                     )
                     )
                        /* insert newNode before node in path */
                        {
                        newNode->prev = node->prev;
                        newNode->next = node;
                        node->prev = newNode;
                        if (newNode->prev == 0)
                           nodePath = newNode;
                        else
                           (newNode->prev)->next = newNode;
                        break;
                        }
                  if (node->next == 0) break;
                  }
               if (newNode->next == 0)
                  // insert newNode at end of list
                  {
                  node->next = newNode;
                  newNode->prev = node;
                  }
               }
            }
         }
      }
   return(nodePath);
}

void
ReducePolygonList(SOL_Polygon* polylist,PolyNode* nodePath)
{        
   PolyNode*   temp;
   int         deleted;
   SCI_Point   I1,I2;
   SCI_Point   tmp1,tmp2;

   /* Eliminate any overlaped polygons.
      For example polygon2 can be eliminated in the following case:


                       ********************
                       *                  *
                       *                  *
                       *     polygon2     *
                        *                 *
                         *               *
                          *             *
                *********  *           *  **********
                *        *  *         *  *         *
              I1*         *  *I3   I4*  *          *I2
     A----------*----------*--*-----*--*-----------*---------B
                *           *  *   *  *            *
                *            *  * *  *             *
                *             *  *  *              *
                *              *   *               *
                *               * *                *
                *                *                 *
                *                                  *
                *            polygon1              *
                *                                  *
                ************************************    
   */
   // First polygon in polylist is checked againest the others to
   // see if any polygons in the list can be eliminated.
   deleted = 0;
   I2 = nodePath->I2;
   I1 = nodePath->I1;
   nodePath = nodePath->next;
   while (nodePath)
      {
      temp = nodePath->next;
      tmp1 = I2 - I1;
      tmp2 = I2 - nodePath->I2;

      if ((tmp1*tmp2) <= 0)      
         {
         // remove nodePath->i polygon from polylist
         RemoveNode(polylist,nodePath->i - deleted);
         ++deleted;
         }
      nodePath = temp;
      }
   }

void
RemoveNode(SOL_Polygon* polylist,int i)
{
   do
      {
      polylist[i] = polylist[i+1];
      ++i;
      }
   while (polylist[i].n != 0);
}

int 
GetPolyDirections(PolyNode* nodePath)
{
   int        polyDirections = 0;
   int        d,i;
   PolyNode*  node;
   for (node=nodePath,i=0;(node != 0) && (i<=MAXOPTIMIZEDNODES);++i)
      {
      d = 1;
      if(node->d<0) d =0;
      polyDirections += (d<<i);
      node = node->next;
      }
   return(polyDirections);
   }

int
GetPolyDistance(SCI_Point* path)
{
   int   distance = 0,offScreen = 0;
   int   i;
   for (i=0;path[i+1].x != ENDOFPATH;++i)
      {
      distance += DistanceEstimate(path[i],path[i+1],&offScreen);
      }
   if (offScreen) return(INT_MAX);
   return(distance);
}

void
SetPolyDirections(PolyNode* nodePath,int polyDirections,int totalNodes)
{
   PolyNode*   node;
   int         i,d;
   for (node=nodePath,i=0;(node != 0) && (i<totalNodes);++i)
      {
      d = (polyDirections>>i) & 1;
      if (!d) d = -1;
      if (node->d != d)
         {
         if (node->d == 1)
            {
            node->d = -1;
            node->n1 = (node->n1 - 1 + node->n) % node->n;
            node->n2 = (node->n2 + 1 + node->n) % node->n;
            }
         else
            {
            node->d = 1;
            node->n1 = (node->n1 + 1 + node->n) % node->n;
            node->n2 = (node->n2 - 1 + node->n) % node->n;
            }
         }
      node = node->next;
      }
}

/*              OPTIMIZE PATH AROUND ALL OBSTACLE POLYGONS
**
**
** NAME---OptimizePath
** input:
**   line segment A-B, path nodes created by AvoidPath, polygonList, opt
** output:
**   path
**
*/

void
OptimizePath(SCI_Point& A,
             SCI_Point& B,
             SCI_Point* path,
             PolyNode* nodePath,
				 SOL_Polygon* polylist,
             int opt)
   {
   int         i,j,k,x;
   int         M,P0,PN,PG;
   PolyNode*   node;
   SCI_Point   I1,I2;
   int         nodeI1,nodeI2;


   // Chain path nodes together to make one large path
   // first point is A
   path[0].x = A.x;
   path[0].y = A.y;

   i = 1;
   for (node = nodePath;node != 0;)
      {
      path[i++] = node->I1;
      j = (node->n1 - node->d + node->n) % node->n;

      do
         {
         j = (j + node->d + node->n) % node->n;
         path[i].x = (node->poly)[j].x;
         path[i++].y = (node->poly)[j].y;
         }
      while (j != node->n2);

      path[i++] = node->I2;
      node = node->next;
      }

   // last point is B
   path[i++] = B;
   path[i].x = ENDOFPATH;
   if (i >= MAXPOLYPATH)
		msgMgr->Fatal(SrcLoc,Msg_TooManyPointsInPath);

   // start optimization of path
   if (!opt || (i < 3))
      {
      return;
      }
   else
      {
      // get rid of any adjacent dupicate points
      for (j=0;path[j].x != ENDOFPATH;++j)
         {
         if (path[j] == path[j+1])
            {
               for (k=j;path[k].x != ENDOFPATH;++k)
                  {
                  path[k] = path[k+1];
                  }
               --j;
               --i;
            }
         }

      M = i;
      P0 = 0;
      for (PG = 0,PN = M-1;P0 < M-2;PG = 0 )
         {
         x = False;
         for (;polylist[PG].n != 0;++PG)
            {
            if (IntersectPolygon(path[P0],
                                 path[PN],
                                 polylist[PG].polyPoints,
                                 polylist[PG].n,
                                 &I1,
                                 &I2,
                                 &nodeI1,
                                 &nodeI2)
               )
               {
               x = True;
               break;
               }
            }
         if (x)
            {
            if (PN > P0+2) 
               PN -= 1;
            else
               {
               P0 += 1;
               PN = M-1;
               }
            }
         else
            {
            // eliminate nodes P0+1 through PN-1
            for (i=P0+1,j=PN,k=M-PN+1;k!=0;--k)
               {
               path[i++] = path[j++];
               }
            M = M-PN+P0+1;
            PN = M-1;
            P0 += 1;
            }
         }
      }
}

int
PointInterior(SCI_Point& M,SCI_Point* polygon,int n)
{
   SCI_Point   N3,P1,P2,P3,P4;
   SCI_Point   tmp1,tmp2;
   int         nextnode,interior = 0;

   P1 = polygon[0];
   P2 = polygon[1];
   nextnode = 3;
   P3 = N3 = polygon[2];
   if (n>3)
      P4 = polygon[3];
   else
      P4 = polygon[0];
   while (True)
      {
      // If M is interior to some line segment of
      // the polygon consider it as interior.
      if (
            (
               (
               (P1.y<=M.y)        &&
               (M.y<=P2.y) 
               )
            ||
               (
               (P2.y<=M.y)        &&
               (M.y<=P1.y)
               )
            )
         &&
            (
               (
               (P1.x<=M.x)        &&
               (M.x<=P2.x) 
               )
            ||
               (
               (P2.x<=M.x)        &&
               (M.x<=P1.x)
               )
            )
         )

         {
         tmp1 = M - P1;
         tmp2 = M - P2;
         if ((tmp1^tmp2) == 0) return(True);
         }

      // The alogorithm counts the number of intersections with the
      // given polygon going from M in the negative x direction.
      // If this number is odd M must be interior to the polygon.

      // If M is above or below P1-P2 then there can't be 
      // an intersection of the ray from M in the negative x
      // direction with the line segment P1-P2.
      if (
            (
            (P1.y<M.y)        &&
            (M.y<P2.y) 
            )
         ||
            (
            (P2.y<M.y)        &&
            (M.y<P1.y)
            )
         )

         {
         // If P1-P2 or P2-P1 is chosen so that the y delta is 
         // positive and this vector is crossed with the vector
         // P1-M then the z component is positive if M lies on
         // the negative side of P1-P2 and negative if M lies
         // on the positive side of P1-P2.
         tmp1 = P2 - P1;
         if (tmp1.y < 0 ) tmp1 = -tmp1;
         tmp2 = M - P1;
         if ((tmp1^tmp2) > 0) ++interior;
         }
      else
         {
         // If the ray in the negative x direction from M passes
         // through the point P2 then one must look at the next
         // segment of the polygon which is not parallel with the
         // x axis in order to determine if the is an intersection
         // at P2. Note that consecutive line segment are not allowed
         // to be parallel in a given polygon.

         if (
            (P2.y == M.y) &&
            (M.x < P2.x)
            )
            {
            /* types of intersections checked here:

               *           *       *    *             *            *
                *           *     *      *             *          *
                 *           *   *        *             *        *
                  *           * *          *             *      *
            <------*-----------*------------*****---------******----------M
                   *                             *
                   *                              *
                   *                               *           
                   *                                *
                   *                                 *

                  intersection?
                  yes          no             yes           no          
            */



            if (P3.y != P2.y)
               {
               if ((int)(P2.y-P1.y)*(int)(P3.y-P2.y) > 0) ++interior;
               }
            else
               {
               if ((int)(P2.y-P1.y)*(int)(P4.y-P3.y) > 0) ++interior;
               }
            }
         }
      P1 = P2;
      P2 = P3;
      P3 = P4;

      // See if there are any more line segments
      // in the polygon to be tested.
      if (P3 == N3) break;

      P4 = polygon[(++ nextnode) % n];
      }

   if (interior & 1)
      return(True);
   else
      return(False);
}

/* POLYPATH.C   PATH AROUND AN OBSTACLE PLOYGON
**
** Algorithm by Larry Scott
**
** NAME---POLYPATH---
**    POLYGON must be closed and not self intersecting (JORDAN ploygon) 
** input:
**   line segment A-B & POLYGON P1,P2,...PN
** output:
**   return       0 polygon not an obstacle, 1 path of nodes returned  
**   nodeI1       first node 
**   nodeI2       last node
**   direction    1 increasing order, -1 decreasing order
**   I1           intersecting point closest to A
**   I2           intersecting point closest to B
**
**
**               EXAMPLE:
**                    PolygonPath(A,B,path) returns (1,N4,N2,-1,x1,y1,x2,y2)
**
**
**
**                 N0     N1
**                 ********
**                *        *
**               *          *
**              *            *
**             *              *
**         N5 *                *
**            *(x1,y1)=I1       * (x2,y2)=I2
**  A---------*------------------*---------------B
**            *                   *N2
**            *                  *
**            *                 *
**         N4 ****************** N3
**
*/

int
PolygonPath(SCI_Point& A,
            SCI_Point& B,
            SCI_Point* points,
            int n,
            SCI_Point* I1,
            SCI_Point* I2,
            int* nodeI1,
            int* nodeI2)
{
   int   distA,distB;
   int   offScreenA=False,offScreenB=False;

   if (!IntersectPolygon(A,B,points,n,I1,I2,nodeI1,nodeI2)) return 0;
   if (*nodeI1 == *nodeI2) return 0;
   //  nodeI1 = P[0]; first node
   //  nodeI2 = P[n - i]; last node
   distA = DistanceSquared(1,*nodeI1 + 1,*nodeI2,points,n,*I1,*I2,&offScreenA); 
   distB = DistanceSquared(-1,*nodeI1,*nodeI2 + 1,points,n,*I1,*I2,&offScreenB);
   if (offScreenA && (!offScreenB))
      {
      *nodeI2 = (*nodeI2+1) % n;
      return(-1);
      }
   if ((!offScreenA) && offScreenB)
      {
      *nodeI1 = (*nodeI1+1) % n;
      return(1);
      }
   if (distA < distB)
      {
      *nodeI1 = (*nodeI1+1) % n;
      return(1);
      }
   else
      {
      *nodeI2 = (*nodeI2+1) % n;
      return(-1);
      }
}

int
DistanceSquared(int direction,
                int nodeF,
                int nodeL,
                SCI_Point* points,
                int n,
                SCI_Point& I1,
                SCI_Point& I2,
                Bool* offScreen)
{
   int         distance = 0;
   int         nodeM;
   SCI_Point   P1,P2;

   nodeM = nodeF = nodeF % n;
   nodeL = nodeL % n;
   P1 = points[nodeM];
   for (;nodeM != nodeL;)
      {
      // adding n to mod to force nodeM positive
      nodeM = (nodeM + direction + n) % n;
      P2 = points[nodeM];
   // DistanceEstimate is more accurate than the sum 
   // of the squares of the distances for deciding 
   // which way around a polygon is shorter.

      distance += DistanceEstimate(P2,P1,offScreen);
      P1 = P2;
      }
   if (distance>0)
      {
      distance += DistanceEstimate(I1,points[nodeF],offScreen);
      distance += DistanceEstimate(points[nodeL],I2,offScreen);
      }
   else
      {
      distance += DistanceEstimate(I1,points[nodeF],offScreen);
      distance += DistanceEstimate(points[nodeL],I2,offScreen);
      }
   return(distance);
}

int
DistanceEstimate(SCI_Point& P2,SCI_Point& P1,Bool* offScreen)
{
   int   deltaX,deltaY,temp;
   // if line lies on screen edge offScreen set to True
   if (LineOnPlaneEdge(P1,P2))  *offScreen = True;
   // Estimate distance by the formula:
   // If max(deltaX,deltaY) <= (10*min(deltaX,deltaY))/6)
   //    then use (13*max(deltaX,deltaY))/10
   //    else use max(deltaX,deltaY)
   // This formula guarantees the max error is less than
   // 11% and the average error is about 5%.

   deltaX = abs(P2.x - P1.x);
   deltaY = abs(P2.y - P1.y);
   if (deltaX > deltaY)
      {
      temp = deltaX;
      deltaX = deltaY;
      deltaY = temp;
      }
   if (((deltaX*10)/6) >= deltaY)
      return(((13*deltaY)/10));
   else
      return(deltaY);
}

int
LineOnPlaneEdge(SCI_Point& P1,SCI_Point& P2)
   {  
   if 
      (
         (P1.x == P2.x)              &&
         (P1.y != P2.y)              &&
         (
            (P1.x == 0)               ||
            (P1.x == planePoly[1].x) 
         )
      )

      return(1);

   if 
      (
         (P1.y == P2.y)              &&
         (P1.x != P2.x)              &&
         (
            (P1.y == 0)               ||
            (P1.y == planePoly[2].y) 
         )
      )

      return(1);

   return(0);
}

/* INTERSECT LINE SEGMENT AND A POLYGON
**
** Algorithm by Larry Scott
**
** NAME---IntersectPolygon
**      POLYGON must be closed and not self intersecting (JORDAN polygon) 
** input:
**      line segment A-B & POLYGON P1,P2,...PN
** output: 
**      return    number ofintersections
**      I1        intersection closest to A
**      I2        intersection closest to B
**      nodeI1    start node of line intersecting at I1
**      nodeI2    start node of line intersecting at I2
*/

int
IntersectPolygon(SCI_Point& A,
                 SCI_Point& B,
                 SCI_Point* points,
                 int n,
                 SCI_Point* INT1,
                 SCI_Point* INT2,
                 int* nodeI1,
                 int* nodeI2)
{
   int         distA = INT_MAX,distB = INT_MAX,d;
   SCI_Point   N1,N2,P1,P2,P3,W,F1,tmp1,tmp2,tmp3,tmp4,I1,I2;
   SCI_Point   intersection;
   int         i,node,nextnode,f1,t1,t2;                             
   N1 = points[0];                                            
   P1 = N1;                                              
   N2 = points[1];
   P2 = points[1];
   P3 = points[2];
   i = 0; node = 0; // first three nodes already taken
   f1 = IntersectSegments(A,B,P1,P2,&intersection);
   t1 = f1;
   F1 = intersection;
   nextnode = 2;

start:
   t2 = IntersectSegments(A,B,P2,P3,&W);
   if ((t1 == NOINTERSECT) || (t1 == INTERSECT+INTERSECTC))
      {
getnextnode:
      t1 = t2;
      P1 = P2;
      P2 = P3;
      intersection = W;
      ++ node;
      if ((n-1) <= nextnode)
         if (P3.x == N2.x && P3.y == N2.y)
            {
            *INT1 = I1;
            *INT2 = I2;
            return(i);
            }
         else  
            if (P3.x == N1.x && P3.y == N1.y)
               {
               P3 = N2;
               t2 = f1;
               W = F1;
               }
            else 
               {
               P3 = N1;
               }
      else
         P3 = points[++nextnode];
      goto start;
      }
   else
      {
      if (t1 == COLINEAR)
         {
         /* If P1-P2 is colinear with A-B and A-B contains P2, then
            one of the following cases must apply. This is because 
            all polygon must be defined in a mathematical counter
            clockwise direction.
            For example:
            
               P1      P2                           There is an 
         A-----*********----------------------B     intersection at P2
                        *
                         *
               OUTSIDE    *     INSIDE
                           *
                            *
                             *                  
                             P3

               

                             *
                            *
               INSIDE      *
                          *   OUTSIDE
                         *
               P1     P2*                           There is no 
         A-----*********----------------------B     intersection at P2
                         
                          
                                
                            

             
         IF A=P2 OR B=P2


               P1      P2,A                         There is an 
               *********----------------------B     intersection at P2
                        *
                         *
               OUTSIDE    *     INSIDE
                           *
                            *
                             *             
                             P3

               
               P1      P2,A                         There is no 
         B-----*********                            intersection at P2
                        *
                         *
               OUTSIDE    *     INSIDE
                           *
                            *
                             *                    
                             P3



               P1      P2,B                         There is an 
               *********----------------------A     intersection at P2
                        *
                         *
               OUTSIDE    *     INSIDE
                           *
                            *
                             *                            
                             P3

               
               
                             P3
                             *
                            *
               INSIDE      *   OUTSIDE
                          *
                         *
               P1   P2,B*                           There is no 
         A-----*********                            intersection at P2
                         
         */
         // See if A-B contains P2.
         tmp1 = P2 - A;
         tmp2 = P2 - B;
         if ((tmp1*tmp1) <= 2)
            // A = P2
            // If (P2-P1)*(B-A) > 0 then intersection at A
            // otherwise not an intersection
            {
            tmp3 = P2 - P1;
            tmp4 = B - A;
            if ((tmp3*tmp4) > 0)
               {
               intersection = P2;
               goto foundIntersection;
               }
            else
               {
               goto getnextnode;
               }
            }
         if ((tmp2*tmp2) <= 2)
            // B = P2
            // If (P2-P1)*(B-A) < 0 then intersection at B
            // otherwise not an intersection
            {
            tmp3 = P2 - P1;
            tmp4 = B - A;
            if ((tmp3*tmp4) < 0)
               {
               intersection = P2;
               goto foundIntersection;
               }
            else
               {
               goto getnextnode;
               }
            }
         // If (P2-A)*(P2-B) < 0 then A-B contains P2.
         if ((tmp1*tmp2) < 0)
            {   
            // If (P2-P1)^(P3-P2) < 0 then P2 is an intersection.
            tmp1 = P2 - P1;
            tmp2 = P3 - P1;
            if ((tmp1^tmp2) < 0)
               {
               intersection = P2;
               goto foundIntersection;
               }
            }
         goto getnextnode;
         }

      if (t1 == INTERSECT+INTERSECTD)
         {
         if (t2 == COLINEAR)
            {
            /* If intersection at P2, A == P2 and P2-P3 is colinear,
               Then if (P3-P2)*(B-A) < 0 and (P2-P1)^(P3-P2) < 0 then 
               P2 is an intersection.

               If intersection at P2, A != P2 and P2-P3 is colinear,
               then if (P2-P1)^(P3-P2) < 0 then P2 is an intersection.

               Otherwise D is not an intersection.
               This is because all polygons must be defined in a counter
               clockwise direction.
               For example:


                          A,P2     P3             There is an no
            B---------------*******               intersection at P2
                           *                      
                          *             
                OUTSIDE  *   INSIDE    
                        *               
                     P1*           


                     P1*
                        *   OUTSIDE
                         *
                 INSIDE   *
                           *P2     P3             There is an 
            B---------------*******               intersection at P2
                            A                     

                P1*
                   *
                    *   OUTSIDE
              INSIDE *
                      *P2     P3                  There is no
                       *******-------------B      intersection at P2
                       A                          


                     A,P2     P3                  There is no
                       *******-------------B      intersection at P2
                      *                           
                     *
            OUTSIDE *   INSIDE
                   *
                P1*



                P1*
                   *    OUTSIDE
                    *
             INSIDE  *
                      *P2    P3                   There is an
            A----------*******-------------B      intersection at P2
                                                  

                      P2     P3
            A----------*******-------------B      There is no
                      *                           intersection at P2
                     *
            OUTSIDE *   INSIDE
                   *
                P1*

                  *
                   *
                    *  OUTSIDE
              INSIDE *
                      *P2    P3                   There is an
            B----------*******-------------B      intersection at P2
                                                  


                      P2     P3
            B----------*******-------------B      There is no
                      *                           intersection at P2
                     *
            OUTSIDE *   INSIDE
                   *
                P1*

            */
            // See if P2 == A
            tmp1 = P2 - A;
            d = tmp1*tmp1;
            tmp1 = P2 - P1;
            tmp2 = P3 - P2;
            if (d <= CLOSETOLINE)
               {
               // if (P3-P2)*(B-A) > 0 no intersection
               tmp3 = B - A;
               if ((tmp2*tmp3) > 0) goto getnextnode;
               }
            // If (P2-P1)^(P3-P2) < 0 then P2 is an intersection.
            if ((tmp1^tmp2) < 0)
               goto foundIntersection;
            else
               goto getnextnode;
            }


         /* If intersection at P2 then check crossing node or not.
            For example:
                                  *P3
                               *    
                            *   
                         *  
                   P2 *     
         A----------*--------------------      Not crossing node
                   *                           
                  *
                 *                     
                *
             P1*

                       P2       
             A----------*--------------------  Crossing node    
                       * *                         
                      *   *            
                     *     *     
                    *       *
                 P1*         * P3
         */
         tmp1 = B - A;
         tmp2 = P3 - A;
         tmp3 = P1 - A;
         if (((tmp1^tmp2) * (tmp1^tmp3)) > 0)
            {
            // not crossing node
            tmp4 = P2 - A;
            if ((tmp4*tmp4) <= 2)
               {
               /* P2 = A */
               /*
                          P2       
                          A*--------------------      There is an
                          * *                         intersection at P2
                         *   *      INSIDE
                        *     *     
                       *       *
                    P1*         * P3

                          P2       
                          A*--------------------      There is no
                          * *                         intersection at P2
                         *   *     OUTSIDE
                        *     *     
                       *       *
                    P3*         * P1
               */
               if (NodeTest(tmp1,P1,P2,P3))
                  {
                  goto foundIntersection;
                  }
               else
                  {
                  goto getnextnode;
                  }
               }
            tmp4 = P2 -B;
            if ((tmp4*tmp4) <= 2)
               {
               // P2 = B
               /*
                          P2       
                 A--------B*                          There is an
                          * *                         intersection at P2
                         *   *      INSIDE
                        *     *     
                       *       *
                    P1*         * P3

                          P2       
                A---------B*                          There is no
                          * *                         intersection at P2
                         *   *     OUTSIDE
                        *     *     
                       *       *
                    P3*         * P1

               */
               tmp1 = A - B;
               if (NodeTest(tmp1,P1,P2,P3))
                  {
                  goto foundIntersection;
                  }
               else
                  {
                  goto getnextnode;
                  }
               }
            /*
                       P2       
             A----------*--------------------      There is an
                       * *                         intersection at P2
                      *   *      INSIDE
                     *     *     
                    *       *
                 P1*         * P3

                       P2       
             A----------*--------------------      There is no
                       * *                         intersection at P2
                      *   *     OUTSIDE
                     *     *     
                    *       *
                 P3*         * P1
            */
            tmp1 = A - P2;
            if (NodeTest(tmp1,P1,P2,P3))
               {
               goto foundIntersection;
               }
            else
               {
               goto getnextnode;
               }
            }
         else
            {
            /* Crossing node using known inside determine if there
               is an intersection.
                                      *P3
                                   *    
                INSIDE          *   
                             *  
                       P2 *     
             A----------*--------------------      There is an
                       *                           intersection at P2
                      *
                     *                     
                    *
                 P1*

                       P3*        
                        *   
                       *     OUTSIDE
            INSIDE    *                           There is no       
                   P2*A--------------B            intersection at P2
                     *  
                     *                           
                     *
                   P1*                

                             P3*        
                              *   
                             *     OUTSIDE
                  INSIDE    *                     There is an       
              B----------P2*A                     intersection at P2
                           *  
                           *                
                           *
                         P1*               

                       P3*        
                        *   
                       *     OUTSIDE
            INSIDE    *                           There is no       
                   P2*B--------------A            intersection at P2
                     *  
                     *                           
                     *
                   P1*                
                             P3*        
                              *   
                             *     OUTSIDE
                  INSIDE    *                     There is an       
              A----------P2*B                     intersection at P2
                           *  
                           *                
                           *
                         P1*               

            */

            tmp4 = P2 - A;
            if ((tmp4*tmp4) <= 2)
               {
               // P2 = A
               // If (P2-P1)^(B-A) > 0 then there is an intersection at P2.
               tmp2 = P2 - P1;
               if ((tmp2^tmp1) > 0)
                  {
                  goto foundIntersection;
                  }
               else
                  {
                  goto getnextnode;
                  }
               }
            tmp4 = P2 - B;
            if ((tmp4*tmp4) <= 2)
               {
               // P2 = B
               // If (P2-P1)^(B-A) < 0 then there is an intersection at P2.
               tmp2 = P2 - P1;
               if ((tmp2^tmp1) < 0)
                  {
                  goto foundIntersection;
                  }
               else
                  {
                  goto getnextnode;
                  }
               }
            }
         }


      /* If intersection is at A then see if leaving or
         entering polygon.
         For example:
                        *P2
                       *      
                      *           
                     *           
          INSIDE    *   OUTSIDE
                   *     
                  *   
                 *A-------------------      There is no
                *                           intersection at P2
               *
              *                     
             *
          P1*         *P2
                     *
                    *
          INSIDE   *    OUTSIDE
                  *      
      -----------*A                         There is an
                *                           intersection at P2
               *    
              *           
             *        
          P1*              

      */
      if (t1 == INTERSECT+INTERSECTA)
         {
         tmp1 = P2 - P1;
         tmp2 = B - A;
         if ((tmp1^tmp2)<=0) goto getnextnode;
         }


      /* If intersection is at B then see if leaving or
         entering polygon.
         For example:
                        *P2
                       *      
                      *           
                     *           
          INSIDE    *   OUTSIDE
                   *     
                  *   
                 *B-------------------      There is no
                *                           intersection at P2
               *
              *                     
             *
          P1*         *P2
                     *
                    *
          INSIDE   *    OUTSIDE
                  *      
      -----------*B                         There is an
                *                           intersection at P2
               *    
              *           
             *        
          P1*              

      */
      if (t1 == INTERSECT+INTERSECTB)
         {
         tmp1 = P2 - P1;
         tmp2 = B - A;
         if ((tmp1^tmp2)>0) goto getnextnode;
         }

foundIntersection:
      // intersection, see if closer to A or B than other intersections
      tmp1 = A - intersection;
      if ((tmp1*tmp1) < distA)
         {
         distA = tmp1*tmp1;
         I1 = intersection;
         *nodeI1 = node ;
         }
      tmp1 = B - intersection;
      if ((tmp1*tmp1) < distB)
         {
         distB = tmp1*tmp1;
         I2 = intersection;
         *nodeI2 = node ;
         }
      // count intersections
      ++ i;
      goto getnextnode;
      }
}

int
NodeTest(SCI_Point& V,SCI_Point& P1,SCI_Point& P2,SCI_Point& P3)
{
   SCI_Point tmp1,tmp2;

   /*

            P2              V
             *------------------->      There is no
            * *                         intersection at P2
           *   *     OUTSIDE
          *     *     
         *       *
      P3*         * P1


    V       P2       
   <---------*                          There is an
            * *                         intersection at P2
           *   *      INSIDE
          *     *     
         *       *
      P1*         * P3

   */ 
   tmp1 = P3 - P2;   
   tmp2 = P2 - P1;   
   if ((tmp1^tmp2) > 0)
      {
      if ((tmp1^V) > 0)
         {
         return (True);
         }
      else
         {
         if ((V^tmp2) < 0)
            {
            return (True);
            }
         else
            {
            return (False);
            }
         }
      }
   else
      {
      if ((tmp1^V) > 0)
         {
         if ((V^tmp2) < 0)
            {
            return (True);
            }
         else
            {
            return (False);
            }
         }
      else
         {
         return (False);
         }
      }
}

/* INTERSECT TWO LINE SEGMENTS
**
** Algorithm by Larry Scott
**
** NAME---IntersectSegments
**      POLYGON must be closed and not self intersecting (JORDAN polygon) 
** input: 2 line segments A-B & C-D
**        address of where to place intersection point
**        It's assumed that A-B is the motion and C-D
**        is part of a polygon.
** output: return    NOINTERSECT               no intersection
**                   COLINEAR                  segments are colinear
**                   INTERSECT + INTERSECTA    intersection exactly at the point A
**                   INTERSECT + INTERSECTB    intersection exactly at the point B
**                   INTERSECT + INTERSECTC    intersection exactly at the point C
**                   INTERSECT + INTERSECTD    intersection exactly at the point D
**                   INTERSECT + INTERSECTI    intersection internal to segments
**                   intersection           intersection point
*/ 

int
IntersectSegments(SCI_Point& A,
                  SCI_Point& B,
                  SCI_Point& C,
                  SCI_Point& D,
                  SCI_Point* intersection)
{
   int         x1,x2;
   int         dot1,dot2,dot3,d;
   SCI_Point   U,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7;

   tmp1 = B - A;
   tmp2 = C - A;
   tmp3 = D - A;

   x1 = tmp1^tmp2;
   x2 = tmp1^tmp3;

   // check for colinear
   if (x1 == 0 && x2 == 0)
      // COLINEAR LINE SEGMENTS
      {
      return(COLINEAR);
      }

   // check for intersection at C
   if (x1 == 0)
      // C lies on the line through A-B find out
      // if it is in the line segment A-B.
      if (A.x == B.x)
         {
         if (
               (
               (A.y <= C.y)    &&
               (C.y <= B.y)
               )
               ||
               (
               (B.y <= C.y)    &&
               (C.y <= A.y)
               )
            )
            {
            *intersection = C;
            return(INTERSECT+INTERSECTC);
            }
         }
      else
         {
         if (
               (
               (A.x <= C.x)    &&
               (C.x <= B.x)
               )
               ||
               (
               (B.x <= C.x)    &&
               (C.x <= A.x)
               )
            )
            {
            *intersection = C;
            return(INTERSECT+INTERSECTC);
            }
         }

   // check for intersection at D
   if (x2 == 0)
      // D lies on the line through A-B find out
      // if it is in the line segment A-B.
      if (A.x == B.x)
         {
         if (
               (
               (A.y <= D.y)    &&
               (D.y <= B.y)
               )
               ||
               (
               (B.y <= D.y)    &&
               (D.y <= A.y)
               )
            )
            {
            *intersection = D;
            return(INTERSECT+INTERSECTD);
            }
         }
      else
         {
         if (
               (
               (A.x <= D.x)    &&
               (D.x <= B.x)
               )
               ||
               (
               (B.x <= D.x)    &&
               (D.x <= A.x)
               )
            )
            {
            *intersection =  D; 
            return(INTERSECT+INTERSECTD);
            }
         }

   // check for intersection at A
   tmp4 = C - D;
   tmp5 = B - C;
   tmp6 = B - D;
   dot3 = tmp4*tmp4;
   if (dot3==0)
		msgMgr->Fatal(SrcLoc,Msg_GetPathInternalError);
   if ( ((d = (tmp4*tmp2)) >= 0) && ((tmp4*tmp3) <= 0) )
      {
      tmp7.x = ((d*(int)tmp4.x + (int) (sign(tmp4.x)*dot3/2))/dot3);
      tmp7.y = ((d*(int)tmp4.y + (int) (sign(tmp4.y)*dot3/2))/dot3);
      tmp7 -= tmp2;
      d = tmp7*tmp7;
      }
   else
      {
      if ((dot1 = (tmp2*tmp2)) <= (dot2 = (tmp3*tmp3)))
         d = dot1; 
      else
         d = dot2; 
      }
   if (d <= CLOSETOLINE)
      {
      *intersection =  A; 
      return(INTERSECT+INTERSECTA);
      }

   // check for intersection at B
   if ( ((d = (tmp4*tmp6)) >= 0) && ((tmp4*tmp5) <= 0) )
      {
      tmp7.x = ((d*(int)tmp4.x + (int) (sign(tmp4.x)*dot3/2))/dot3);
      tmp7.y = ((d*(int)tmp4.y + (int) (sign(tmp4.y)*dot3/2))/dot3);
      tmp7 -= tmp6;
      d = tmp7*tmp7;
      }
   else
      {
      if ((dot1 = (tmp5*tmp5)) <= (dot2 = (tmp6*tmp6)))
         d = dot1; 
      else
         d = dot2; 
      }
   if (d <= CLOSETOLINE) 
      {
      *intersection =  B; 
      return(INTERSECT+INTERSECTB);
      }

   // check that C,D on opposite side of line through A-B
   if (x1*x2 >= 0)
      {
      return(NOINTERSECT);
      }

   // check that A,B on opposite side of line through C-D
   x1 = tmp2^tmp4;
   x2 = tmp4^tmp5;
   if (x1*x2 >= 0)
      {
      return(NOINTERSECT);
      }

   // Intersection interior to both line
   // segments calculate intersection point.

   // U is normal to C-D pointing toward the outside of the polygon
   U.x = -tmp4.y;
   U.y = tmp4.x;
   // U dot (A-C)
   dot1 = U*tmp2;
   // U dot (A-B)
   dot2 = U*tmp1;
   dot3 = dot2;
   if (dot1 < 0) 
      dot1 = - dot1;
   if (dot2 < 0) 
      dot2 = - dot2;

   if (dot3>0)
      {
      // round toward outside of polygon
      tmp6.x = (((int) tmp1.x*dot1 + (int) sign(tmp1.x)*(dot2-1))/dot2);
      tmp6.y = (((int) tmp1.y*dot1 + (int) sign(tmp1.y)*(dot2-1))/dot2);
      *intersection = A + tmp6;
      }
   if (dot3<0)
      {
      // truncate so that intersection is outside polygon
      tmp6.x = ((((int) tmp1.x)*dot1)/dot2) ;
      tmp6.y = ((((int) tmp1.y)*dot1)/dot2) ;
      *intersection = A + tmp6;
      }
   if (dot3==0)
      {
      *intersection = C;
      }  
   return(INTERSECT+INTERSECTI);
}

MemID 
MergePolygons(SCI_Point* polygon,int size,SOL_Polygon* polylist)
   {
   SCI_Point   P[MAXPOLYPATH];
   int         i,j,n;

   if (size >= MAXPOLYPATH)
      {
		msgMgr->Fatal(SrcLoc,Msg_MergedPolygonTooLarge);
      }

   // copy polygon to merge into P
   for (i=0;i < size;++i)
      P[i] = polygon[i];
   P[i].x = ENDOFPATH;
   P[i].y = ENDOFPATH;
   n = i;

   // merge given polygon with each polygon in the list
   // A merged polygon in the list will be marked as
   // MTP, MNP, MBP or MCP. The resultant polygon will be
   // marked BAP.

   for (i=0;polylist[i].n != 0;++i)
      {
      MergePolygon(P,n,polylist[i]);
      for (n=0;P[n].x != ENDOFPATH;++n) {};
      }

   // Get rid of duplicate points and colinear lines
   for (i=0;P[i].x != ENDOFPATH;++i) 
      {
         if(P[i] == P[i+1]) {
            for (j=i+1;P[j].x != ENDOFPATH;++j) 
               P[j] = P[j+1];
            --i;
         }
      }

   TestColinear(P,&i);
   P[i].x = ENDOFPATH;
   P[i].y = ENDOFPATH;

   // return the new polygon
	MemID newPolyID;
	newPolyID.Get(MemPointList,i*sizeof(SCI_Point) + sizeof(Array));
	SCIWord* intArray = (SCIWord*) memMgr->GetAddr(newPolyID);
   intArray[0] = 2;
   intArray[1] = 2*i;

	SCI_Point* newPoly = (SCI_Point*) ((char*)intArray + sizeof(Array));

   for (j=0;j < i;++j) 
      newPoly[j] = P[j];
   return newPolyID;
}

void
MergePolygon(SCI_Point* P,int n,SOL_Polygon& polygon)
   {

   #define MAXPATCHES 8

   int         i,j,k,m; 
   Bool        addNode; 
   int         t1,P_i,Q_i,P_j,Q_j; 
   int         angleIn,angleOut,delta,angle; 
   SCI_Point   *Q;
   SCI_Point   P_U,Q_U,Result[MAXPOLYPATH];
   SCI_Point   tmp1,tmp2;
   PolyPatch   Patch[MAXPATCHES],newPatch;
   int         patches = 0,p;
   
   // HOOK
   // make sure that merged polygons are marked as mtp,mnp etc.
   Q = polygon.polyPoints;
   m = polygon.n;
   for (P_i=0;P_i < n;++P_i)
      {
      for (Q_i=0;Q_i < m;++Q_i)
         {
         t1 = IntersectSegments(P[P_i],P[(P_i+1+n) % n],Q[Q_i],Q[(Q_i+1+m) % m],&P_U);
         if ((t1 != NOINTERSECT) && (t1 != INTERSECT+INTERSECTD))
            {
            /* Is this an exit from P intersection?
                  For example:
                                *P[i+1]
                               *      
                              *           
                             *           
                   INSIDE   *   OUTSIDE
                           *     
                          *   
            Q[j]---------*---------------->Q[j+1]   This is an exiting
                        *                           intersection
                       *
                      *                     
                     *
                P[i]*

                                *P[i+1]
                               *      
                              *           
                             *           
                  INSIDE    *   OUTSIDE
                           *     
                          *   
            Q[j+1]<------*-----------------Q[j]     This is an entering
                        *                           intersection
                       *
                      *                     
                     *
                P[i]*


            */
            // test for exit or entry
            tmp1 = P[(P_i+1+n) % n] - P[P_i];
            tmp2 = Q[(Q_i+1+m) % m] - Q[Q_i];
            if ((tmp1^tmp2) < 0)
               {
               // now test for clock wise or counter clockwise
               angle = 0;
               angleIn = ATan(Q[Q_i].x,Q[Q_i].y,Q[(Q_i+1+m) % m].x,Q[(Q_i+1+m) % m].y);
               for (Q_j=Q_i+1;Q_j <= Q_i + m;++Q_j)
                  {
                  angleOut = ATan(Q[(Q_j+m) % m].x,Q[(Q_j+m) % m].y,Q[(Q_j+1+m) % m].x,Q[(Q_j+1+m) % m].y);
                  delta = angleOut - angleIn;
                  if (delta > 180) delta -= 360;
                  if (delta < -180) delta += 360;
                  angle += delta;
                  angleIn = angleOut;
                  for (P_j = P_i;P_j <= P_i + n ;++P_j)
                     {
                     t1 = IntersectSegments(P[(P_j + n) % n],P[(P_j+1+n) % n],Q[(Q_j+m) % m],Q[(Q_j+1+m) % m],&Q_U);
                     if ((t1 != NOINTERSECT) && (t1 != INTERSECT+INTERSECTD))
                        {
                        // test for exit or entry
                        tmp1 = P[(P_j+1+n) % n] - P[(P_j + n) % n];
                        tmp2 = Q[(Q_j+1+m) % m] - Q[(Q_j + m) % m];
                        if ((tmp1^tmp2) > 0)
                           {
                           break;
                           }
                        else
                           {
                           t1 = NOINTERSECT;
                           }
                        }
                     else
                        {
                        t1 = NOINTERSECT;
                        }
                     }
                  if (t1 != NOINTERSECT) break;
                  }
               // we need to find an intersection
               if (t1 != NOINTERSECT)
                  {
                  // If loop to add is screen clockwise add to P patches
                  if (angle > 0)
                     {
                     if (patches >= MAXPATCHES)
		                  msgMgr->Fatal(SrcLoc,Msg_PolygonTooManyPatches);
                     // add loop to patches
                     if(patches == 0)
                        {
                        Patch[patches].P_i = P_i;
                        Patch[patches].Q_i = Q_i;
                        Patch[patches].P_U = P_U;
                        Patch[patches].P_j = (P_j + n) % n;
                        Patch[patches].Q_j = (Q_j + m) % m;
                        Patch[patches].Q_U = Q_U;
                        Patch[patches++].deleteIt = False; 
                        }
                     else
                        {
                        newPatch.P_i = P_i;
                        newPatch.Q_i = Q_i;
                        newPatch.P_U = P_U;
                        newPatch.P_j = (P_j + n) % n;
                        newPatch.Q_j = (Q_j + m) % m;
                        newPatch.Q_U = Q_U;
                        for(p = 0;p < patches; p ++)
                           if (Dominates(Patch[p],newPatch,P)) break;
                        if(p == patches)
                           {
                           Patch[patches].P_i = P_i;
                           Patch[patches].Q_i = Q_i;
                           Patch[patches].P_U = P_U;
                           Patch[patches].P_j = (P_j + n) % n;
                           Patch[patches].Q_j = (Q_j + m) % m;
                           Patch[patches].Q_U = Q_U;  
                           Patch[patches++].deleteIt = False; 
                           for(p = 0;p < patches-1 ; p ++)
                              if (Dominates(newPatch,Patch[p],P)) Patch[p].deleteIt = True;
                           }
                        }

                     }
                  }    
               }
            }
         }
      }
   if (patches)
      {
      // merge the polygons

      // mark as merged polygon
      polygon.type = polygon.type + MTP; 
      j = 0;

      for (P_i=0;P_i < n;++P_i)
         {
         // if P_i is not in a patch add node
         for (i=0,addNode=True;(i<patches) && (addNode == True);++i)
            if(Patch[i].deleteIt == False)
               if (PatchNode(Patch[i],P_i,P))
                  addNode = False;
         if (addNode) Result[j++] = P[P_i];
         // if at a patch, add the patch
         for (i=0;i<patches;++i)
            {
            if(Patch[i].deleteIt == False)
               {
               if (P_i == Patch[i].P_i)
                  {
                  if (j >= MAXPOLYPATH)
		               msgMgr->Fatal(SrcLoc,Msg_PolygonTooManyPoints);
                  if (Patch[i].P_U != P[P_i])
                     Result[j++] = Patch[i].P_U;
                  for 
                     (
                     k=(Patch[i].Q_i+1+m) % m;
                     k != Patch[i].Q_j;
                     k = (k+1+m) % m,++j
                     ) 
                     Result[j] = Q[(k+m) % m];
                  Result[j++] = Q[Patch[i].Q_j];
                  if (Patch[i].Q_U != Q[Patch[i].Q_j])                                           
                     Result[j++] = Patch[i].Q_U;
                  }
               }
            }
         }
      for (i=0;i<j;++i) P[i] = Result[i];
      if (P[i-1] == P[0]) i--;
      P[i].x = ENDOFPATH;
      P[i].y = ENDOFPATH;
      }
}

Bool
PatchNode(PolyPatch& Patch,int theNode,SCI_Point* thePoly)
{
   Bool  offScreen;

   if (Patch.P_i < Patch.P_j)
      if ((Patch.P_i < theNode) && (theNode <= Patch.P_j))
         return (True);
   if (Patch.P_j < Patch.P_i)
      if ((Patch.P_i < theNode) || (theNode <= Patch.P_j))
         return (True);
   if (Patch.P_i == Patch.P_j)
      if (
         DistanceEstimate(Patch.P_U,thePoly[Patch.P_i],&offScreen) > 
         DistanceEstimate(Patch.Q_U,thePoly[Patch.P_i],&offScreen)
         )
         return (True);
   return (False);
}

Bool
Dominates(PolyPatch& A,PolyPatch& B,SCI_Point* poly)
{

   int   Api,Bpi,Apj,Bpj;
   Bool  offScreen = False;

	Api = A.P_i;
   Apj = A.P_j;
   Bpi = B.P_i;
   Bpj = B.P_j;
   if((A.P_U == B.P_U) && (A.Q_U == B.Q_U))
      // The same patch!
      return(True);

   if(Api != Apj)
      {
      if(
         ((Api<Apj) && ((Api < Bpi) && (Bpi < Apj))) ||
         ((Api>Apj) && ((Api < Bpi) || (Bpi < Apj))) 
        )
         /*
              patch A
         *-----------------------------*
         |                             |
         |    patch B                  |
         |  *----------...             |
         |  |                          |
         |  |                          |
         |  |       ********           |
         |  |      *        *          |
         |  *-----*          *         |
         |       *            *--------*
         |      *              *
         |     *                *
         |     *                 * 
         *-----*                  *
               *                   *  
               *                  *
               *                 *
               ******************   

         */
         return(True);


      if(
         ((Api<Apj) && ((Api < Bpj) && (Bpj < Apj))) ||
         ((Api>Apj) && ((Api < Bpj) || (Bpj < Apj))) 
        )
         /*
              patch A
         *-----------------------------*
         |                             |
         |    patch B                  |
         |  *----------*               |
         |             |               |
         |             |               |
         |          ********           |
         |         *        *          |
         |        *          *         |
         |       *            *--------*
         |      *              *
         |     *                *
         |     *                 * 
         *-----*                  *
               *                   *  
               *                  *
               *                 *
               ******************   

         */
         return(True);
      }
   if(Bpi != Bpj)
      {
      if(
         ((Bpi<Bpj) && ((Bpi < Api) && (Api < Bpj))) ||
         ((Bpi>Bpj) && ((Bpi < Api) || (Api < Bpj))) 
       )
         /*
              patch B
         *-----------------------------*
         |                             |
         |    patch A                  |
         |  *----------...             |
         |  |                          |
         |  |                          |
         |  |       ********           |
         |  |      *        *          |
         |  *-----*          *         |
         |       *            *--------*
         |      *              *
         |     *                *
         |     *                 * 
         *-----*                  *
               *                   *  
               *                  *
               *                 *
               ******************   

         */
         return(False);
      if(
     
         ((Bpi<Bpj) && ((Bpi < Apj) && (Apj < Bpj))) ||
         ((Bpi>Bpj) && ((Bpi < Apj) || (Apj < Bpj))) 
       )
         /*
              patch B
         *-----------------------------*
         |                             |
         |    patch A                  |
         |  *----------*               |
         |             |               |
         |             |               |
         |          ********           |
         |         *        *          |
         |        *          *         |
         |       *            *--------*
         |      *              *
         |     *                *
         |     *                 * 
         *-----*                  *
               *                   *  
               *                  *
               *                 *
               ******************   

         */
         return(False);
      }
   if(Api != Apj)
      {
      if(Bpi == Bpj)
         {
         if(Api == Bpi)
            if (
               DistanceEstimate(poly[A.P_i],A.P_U,&offScreen) 
               <
               DistanceEstimate(poly[A.P_i],B.P_U,&offScreen)
               )
               /*
                 patch A
               *----------------*       
               |                |      
               |  ************************
               |  *                      *
               |  *                      *-------*
               |  *                      *       |
               |  *                      *       |
               |  *                      *---*p  |
               |  *                      *   |a  |
               |  *                      *   |t  |
               |  *                      *   |c  |
               |  *                      *   |h  |
               |  *                      *   |B  |
               |  *                      *---*   |
               |  *                      *       |
               |  *                      *       |
               |  *                     *        |
               |  *                    *         |
               |  *********************          |
               |                                 |
               *---------------------------------*  
               */
               return(True);
            else
               /*
                 patch A
               *----------------*       
               |                |      
               |  ************************
               |  *                      *
               |  *                      *              
               |  *                      *     
               |  *                      *     
               |  *                      *---*p
               |  *                      *   |a
               |  *                      *   |t
               |  *                      *   |c
               |  *                      *   |h
               |  *                      *   |B
               |  *                      *---*          
               |  *                      *              
               |  *                      *-------*
               |  *                      *       |
               |  *                      *       |
               |  *                     *        |
               |  *                    *         |
               |  *********************          |
               |                                 |
               *---------------------------------*  
               */
               return(False);
         if(Apj == Bpi)
            if (
               DistanceEstimate(poly[A.P_j],A.Q_U,&offScreen)
               >
               DistanceEstimate(poly[A.P_j],B.P_U,&offScreen)
               )
               /*
                 patch A
               *----------------*       
               |                |      
               |    patch B     |      
               |    *-----*     |      
               |    |     |     |      
               |    |     |     |      
               |  ************************
               |  *                      *
               |  *                      *
               |  *                      *
               |  *                      *---*
               |  *                      *   |
               |  *                      *   |
               |  *                      *   |
               |  *                      *   |
               |  *                     *    |
               |  *                    *     |
               |  *********************      |
               |                             |
               *-----------------------------*  
               */
               return(True);
            else
               /*
                 patch A
               *---------*       
               |         |      
               |         |   patch B   
               |         |   *-----*   
               |         |   |     |   
               |         |   |     |   
               |  ************************
               |  *                      *
               |  *                      *
               |  *                      *
               |  *                      *---*
               |  *                      *   |
               |  *                      *   |
               |  *                      *   |
               |  *                      *   |
               |  *                     *    |
               |  *                    *     |
               |  *********************      |
               |                             |
               *-----------------------------*  
               */
               return(False);
         }
      if(Api != Bpi) 
         /*
                                        
              patch A                            patch B                   
         *-------------*                    *-------------*                
         |             |                    |             |                
         |             |                    |             |                
         |          ********                |          ********            
         |         *        *               |         *        *           
         |        *          *              |        *          *          
         |       *            *--------*    |       *            *--------*
         |      *              *       |p   |      *              *       |p
         |     *                *      |a   |     *                *      |a
         |     *                 *     |t   |     *                 *     |t
         *-----*                  *    |c   *-----*                  *    |c
               *                   *   |h         *                   *   |h
               *                  *    |B         *                  *    |A
               *                 *-----*          *                 *-----*
               ******************                 ******************   

         */
         return(False);
      if (
         DistanceEstimate(poly[A.P_i],A.P_U,&offScreen)
         <
         DistanceEstimate(poly[A.P_i],B.P_U,&offScreen)
         )
         /*
              patch A
         *-----------------------------*
         |                             |
         |    patch B                  |
         |  *----------*               |
         |  |          |               |
         |  |          |               |
         |  |       ********           |
         |  |      *        *          |
         |  |     *          *         |
         |  |    *            *--------*
         |  |   *              *
         |  |  *                *
         |  *--*                 * 
         *-----*                  *
               *                   *  
               *                  *
               *                 *
               ******************   

         */
         return(True);
      else
         /*
              patch B
         *-----------------------------*
         |                             |
         |    patch A                  |
         |  *----------*               |
         |  |          |               |
         |  |          |               |
         |  |       ********           |
         |  |      *        *          |
         |  |     *          *         |
         |  |    *            *--------*
         |  |   *              *
         |  |  *                *
         |  *--*                 * 
         *-----*                  *
               *                   *  
               *                  *
               *                 *
               ******************   

         */
         return(False);
      }
   if((Api == Apj) && (Api != Bpi))
      {
      // if A wraps around it dominates B.
      if (
         DistanceEstimate(poly[A.P_i],A.P_U,&offScreen)
         >
         DistanceEstimate(poly[A.P_i],A.Q_U,&offScreen)
         )
         /*
              patch A
         *--------------------------------*
         |                                |
         |          ********              |
         |         *        *             |
         |        *          *--------*   |
         |       *            *       |p  |
         |      *              *      |a  |
         |     *                *     |t  |
         |     *                 *    |c  |
         *-----*                  *   |h  |
               *                   *  |B  |
         *-----*                  *---*   |
         |     *                 *        |
         |     ******************         |
         |                                |
         *--------------------------------*

         */
         return(True);
      }

   // if either patches wrap around then their can not be a dominance.
   if (
      DistanceEstimate(poly[A.P_i],A.P_U,&offScreen)
      >
      DistanceEstimate(poly[A.P_i],A.Q_U,&offScreen)
      )
      /*
           patch A
      *-----------------------------*
      |                             |
      |          ********           |
      |         *        *          |
      |        *          *         |
      |       *            *        |
      |      *              *       |
      |     *                *      |
      |     *                 *     |
      *-----*                  *    |
            *                   *   |
      *-----*                  *    |
      |     *                 *     |
      |     ******************      |
      |                             |
      *-----------------------------*

      */
      return(False);
   if (
      DistanceEstimate(poly[B.P_i],B.P_U,&offScreen)
      >
      DistanceEstimate(poly[B.P_i],B.Q_U,&offScreen)
      )
      /*
           patch B
      *-----------------------------*
      |                             |
      |          ********           |
      |         *        *          |
      |        *          *         |
      |       *            *        |
      |      *              *       |
      |     *                *      |
      |     *                 *     |
      *-----*                  *    |
            *                   *   |
      *-----*                  *    |
      |     *                 *     |
      |     ******************      |
      |                             |
      *-----------------------------*

      */
      return(False);

   if (
      DistanceEstimate(poly[B.P_i],B.P_U,&offScreen) 
      >
      DistanceEstimate(poly[B.P_i],A.P_U,&offScreen)
      )
      if (
         DistanceEstimate(poly[B.P_i],A.Q_U,&offScreen)
         > 
         DistanceEstimate(poly[B.P_i],B.P_U,&offScreen)
         )
         /*
                                         
                        *************         
                        *            *        
           *------------*             *       
         p |            *              *      
         a |            *               *     
         t |            *                *    
         c |   patchB   *                 *   
         h |  *---------*                  *  
         A |  |         *                   * 
           |  *---------*                  *  
           *------------*                 *   
                        ******************    
                                          

         */
         return(True);
   /*
                                                                
                  ********                          ********         
                  *       *                         *       *        
     *------------*        *           *------------*        *       
   p |            *         *          |            *         *      
   a |            *          *         |            *          *     
   t |            *           *        *------------*           *    
   c |   patchB   *            *                    *            *   
   h |  *---------*             *      *------------*             *  
   A |  |         *              *     |            *              * 
     |  *---------*             *      |            *             *  
     *------------*            *       *------------*            *   
                  *************                     *************    
                                                                 

   */
   return(False);
}

void KTestPoly(argList)
{	
   // Input
   // Arg1 = polygon

   // Output
   // pm.acc = 1 for clockwise;
   //          -1 for counterclockwise;
   //          directions are screen directions

   ObjectID polygonID = (ObjectID) arg(1);

	ObjectID arrayID = polygonID.GetProperty(s_points);
	MemID pointsID = arrayID.GetProperty(s_data);
   SCI_Point* points = (SCI_Point*) *pointsID;

	int size = polygonID.GetProperty(s_size);

   if (TestClockwise(points,size) < 0)
      pm.acc = 1;
   else
      pm.acc = -1;
}
