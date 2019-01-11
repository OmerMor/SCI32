#ifndef POLYGON_HPP
#define POLYGON_HPP

#ifndef POINT_HPP
#include "point.hpp"
#endif

#ifndef LIST_HPP
#include "list.hpp"
#endif

class MemID;

class	SOL_Polygon
{
public:
	SCI_Point*  polyPoints; // MemID for the points of polygon (In SOL array)
   uchar       type;       // number of nodes in polygon
	uchar	      info;		   // bit flag info	
   int         n;          // number of nodes in polygon
};

class PolyNode
{
public:
   PolyNode*   next;		// pointer to next path in list
   PolyNode* 	prev;		// pointer to previous path in list
   SCI_Point   I1;		// intersection nearest to A
   SCI_Point   I2;		// intersection nearest to B
   int         d;       // direction to travel on polygon
   int         n1;      // first node encountered when traversing polygon
   int         n2;      // last node encountered when traversing polygon
   SCI_Point*  poly;    // points to polygon
   int         n;       // number of nodes in polygon
   int         i;       // index into polylist for this polygon


   PolyNode(SCI_Point theI1,
            SCI_Point theI2,
            int theD,
            int theN1,
            int theN2,
            SCI_Point* thePoints,
            int theN)
	{
      I1 = theI1;
      I2 = theI2;
      d = theD;
      n1 = theN1;
      n2 = theN2;
      poly = thePoints;
      n = theN;
      next = 0;
      prev = 0;
   }
};

class	PolyPatch
{
public:
   int         P_i;     // Intersection node of given polygon
	int      	Q_i;		// start intersection node of patch loop
	SCI_Point   P_U;		// start intersection point of patch loop
   int         P_j;     // end intersection node of given polygon
   int         Q_j;     // end intersection node of patch loop
   SCI_Point   Q_U;     // end intersection point of patch loop
	int			deleteIt;
};

#define sign(x) ((x)>0 ? 1:(x)<0 ? -1:0)
#define ENDOFPATH    0x7777
#define NOINTERSECT  -1
#define INTERSECT    1
#define COLINEAR     0
#define INTERSECTA   2
#define INTERSECTB   4
#define INTERSECTC   8
#define INTERSECTD   16
#define INTERSECTI   32
#define MAXPOLYGONS  30 	
#define MAXPOLYPATH      100

/* Types of polygons */
#define TAP 0  // Total access polygon
#define NAP 1  // Near point access polygon
#define BAP 2  // Barred access polygon
#define CAP 3  // Contained accessable polygon
#define MBP 4  // Merge this barred access polygon
#define MTP 5 	// Merged total access polygon
#define MNP 6 	// Merged near point access polygon
#define MCP 7 	// Merged contained accessable polygon

#define MAXOPTIMIZEDNODES 3
// allows 1 x and/or y delta error
#define CLOSETOLINE 2

// Prototypes
// The three main routines
MemID GetPath(SCI_Point& A,SCI_Point& B,SOL_Polygon* polylist,int opt,SCI_Point& planeDim);
int   PointInterior(SCI_Point& M,SCI_Point* polygon,int n);
MemID MergePoly(SCI_Point* polygon,SOL_Polygon* polylist);

void			AddPolygon(MemID polyListID,SCI_Point* points,int size,uchar type);
void        AvoidPath(SCI_Point&,SCI_Point&,SOL_Polygon*,int,SCI_Point*);
PolyNode*   AvoidPolygons(SCI_Point&,SCI_Point&,SOL_Polygon*);
MemID       BuildPolygonList(SOL_ListID,int);
MemID       CopyPath(SCI_Point*);
void        DeletePolygon(SOL_Polygon*,int);
int         DistanceEstimate(SCI_Point&,SCI_Point&,Bool*);
int         DistanceSquared(int,int,int,SCI_Point*,int,SCI_Point&,SCI_Point&,Bool*);
Bool        Dominates(PolyPatch&,PolyPatch&,SCI_Point*);
void        EndPath(SCI_Point&,SCI_Point*);
void        FreeNodePath(PolyNode*);
int         GetPolyDirections(PolyNode*);
int         GetPolyDistance(SCI_Point*);
int         IntersectPolygon(SCI_Point&,SCI_Point&,SCI_Point*,int,SCI_Point*,SCI_Point*,int*,int*);
int         IntersectSegments(SCI_Point&,SCI_Point&,SCI_Point&,SCI_Point&,SCI_Point*);
void        InvertPolygon(SCI_Point*,int);
int         LineOnPlaneEdge(SCI_Point&,SCI_Point&);
MemID       MergePolygons(SCI_Point*,int,SOL_Polygon*);
void        MergePolygon(SCI_Point*,int,SOL_Polygon&);
int         NearPoint(SCI_Point&,SCI_Point*,int,SCI_Point*,Bool);
int         NodeTest(SCI_Point&,SCI_Point&,SCI_Point&,SCI_Point&);
void        OptimizePath(SCI_Point&,SCI_Point&,SCI_Point*,PolyNode*,SOL_Polygon*,int);
Bool        PatchNode(PolyPatch&,int,SCI_Point*);
int         PolygonPath(SCI_Point&,SCI_Point&,SCI_Point*,int,SCI_Point*,SCI_Point*,int*,int*);
void        ReducePolygonList(SOL_Polygon*,PolyNode*);
void        RemoveNode(SOL_Polygon*,int);
void        SetPolyDirections(PolyNode*,int,int);
void        StartPath(SCI_Point&,SCI_Point*);
Bool        TestClockwise(SCI_Point*,int);
int         TestColinear(SCI_Point*,int*);

#endif
