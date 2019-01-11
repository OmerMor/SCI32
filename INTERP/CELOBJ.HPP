#ifndef CELOBJ_HPP
#define CELOBJ_HPP

#ifndef MEMID_HPP
#include "memid.hpp"
#endif

#ifndef POINT_HPP
#include "point.hpp"
#endif

#ifndef PALETTE_HPP
#include "palette.hpp"
#endif

#ifndef RATIO_HPP
#include "ratio.hpp"
#endif


class SOL_Rect;
class ScreenItem;
class Buffer;

_Packed struct CelBase
{
	UInt16	xDim;
	UInt16	yDim;
	Int16		xHot;						//  0
	Int16		yHot;						//  0
	uchar 	skip;
	uchar 	compressType;			//  Uncompressed
	UInt16	dataFlags;
	Int32		dataByteCount;			//  0
	Int32		controlByteCount;		//  0
	Int32		paletteOffset;			//  Use later (0)
	Int32		controlOffset;			//  0
	Int32		colorOffset;			//  sizeof(CelHeader)
	Int32		rowTableOffset;		//  0
};

_Packed struct CelHeader : public CelBase
{
	UInt16   xRes;
	UInt16	yRes;
};

_Packed struct ViewHeader
{
	UInt16	viewHeaderSize;
	uchar 	loopCount;
	uchar 	stripView;
	uchar 	splitView;
	uchar 	resolution;
	UInt16 	celCount;
	UInt32	paletteOffset;
	uchar 	loopHeaderSize;
	uchar 	celHeaderSize;
	UInt16	resX;
	UInt16	resY;
	uchar	version;
	uchar	futureExpansion;
};
FakeTemplateID(ViewHeaderID, ViewHeader, MemViewHeader)

_Packed struct LoopHeader
{
	char 		altLoop;
	uchar 	flags;
	uchar 	numCels;
	char		contLoop;
	char		startCel;
	char		endCel;
	uchar 	repeatCOunt;
	uchar 	stepSize;
	Int32		paletteOffset;
	Int32		celOffset;
};

_Packed struct PicHeader
{
	UInt16	picHeaderSize;
	uchar		celCount;
	uchar		splitFlag;
	UInt16	celHeaderSize;
	UInt32	paletteOffset;
	UInt16	resX;
	UInt16	resY;
};

_Packed struct CelHeaderPic : public CelBase
{
	Int16		priority;
	Int16		xpos;
	Int16		ypos;
};

_Packed struct CelHeaderView : public CelBase
{
	Int32    linkTableOffset;
	Int16    linkTableCount;
};

const CELHEADERSIZE = sizeof(CelHeader);
const uchar REPEATC = 0x80;
const uchar REPSKIP = 0x40;

class Bitmap
{
	public:

	enum {tView, tPic, tMem, tColor};

	Bitmap(const int v, const int l, const int c)	{view = v; loop = l; cel = c; type = tView;}
	Bitmap(const int p, const int c)						{view = p; loop = 0; cel = c; type = tPic;}
	Bitmap(const MemID id)									{view = loop = cel = 0; type = tMem; memid = id; id.Lock();}
	Bitmap(const uchar c)									{view = loop = cel = 0; type = tColor; color = c;}
	Bitmap() 													{view = loop = cel = 0; type = tMem;}

	int	Type() const			{return type;}
	Bool	IsView() const			{return type == tView;}
	Bool	IsPic() const			{return type == tPic;}
	Bool	IsMemCel() const		{return type == tMem;}
	Bool	IsColor() const		{return type == tColor;}
	int	View() const			{return view;}
	int	Loop() const			{return loop;}
	int	Cel() const				{return cel;}
	int	Pic() const				{return view;}		//  Not an error
	int	Color() const			{return color;}
	MemID	Handle() const			{return memid;}
	Bool	operator==(const Bitmap&) const;
	Bool	operator!=(const Bitmap&) const;

	void	SetCel(int c)			{cel = c;}
	void	SetLoop(int l)			{loop = l;}
	void	SetView(int v)			{view = v; type = tView;}
	void	SetPic(int p)			{view = p; loop = 0; type = tPic;}
	void	SetHandle(MemID id)	{type = tMem; memid = id; id.Lock();}
	void	SetType(int theType)	{type = theType;}
	void	SetMemID(const MemID& id)	{memid = id; id.Lock(); }

	private:
	int	type;
	long	view;
	int	loop;
	int	cel;
	MemID memid;
	uchar	color;
};

const maxCOV = 100;
const maxScaledLine = 1024;

enum {NoRemapInCel, RemapInCel};

class CelObj
{
	public:
	//  Constructors, etc
	CelObj(const CelObj&);
	virtual	~CelObj()  {}

	Bool	operator==(const CelObj&) const;
	Bool	operator!=(const CelObj& src) const	{return !operator==(src);}
	int	Width() const			{return width;}
	int	Height() const			{return height;}
	int	XOrg() const			{return xorg;}
	int	YOrg() const			{return yorg;}
	int	ResX() const			{return resx;}
	int	ResY() const			{return resy;}
	int	Type() const			{return bitmap.Type();}
	uchar	Skip() const			{return skip;}
	int	RemapState() const	{return remapState;}
	Bool	Mirrored() const		{return mirrored;}

	void		SubmitPalette();
	virtual	CelObj*	Duplicate() const = 0;
	virtual	void		Draw(const Buffer&, const ScreenItem&, const SOL_Rect&, Bool horzFlip) = 0;
	virtual	void		Draw(const Buffer&, const SOL_Rect& drawRect, SOL_Point, Bool horzFlip) = 0;
	virtual	uchar		ReadPix(int x, int y, Bool mirrFlag);
	SOL_Rect	ReadLineSpan(int y);
	SCIWord GetFirstLink();
	SCIWord GetNextLink(int type);
	SCIWord GetLinkPoint(int type,int which);

	protected:
	CelObj(int v, int l, int c) : bitmap(v,l,c) {mirrored = False; compressType = 1000; hasSkip = True;}
	CelObj(int p, int c) : bitmap(p, c)			  {mirrored = False; compressType = 1000; hasSkip = True;}
	CelObj(MemID m) : bitmap(m)					  {mirrored = False; compressType = 0; hasSkip = True;}
	CelObj(uchar c) : bitmap(c)					  {}

	int	SearchCache(Bitmap, int* index) const;
	void	ScaleTables(const Ratio& ratioX, const Ratio& ratioY);
	void	DoTable(int* table, const Ratio& scale, int size);
	void	Draw(const Buffer&,const ScreenItem&,const SOL_Rect&);
	void	DrawTo(const Buffer&, const SOL_Rect& drawRect, SOL_Point,
					 const Ratio ratioX = Ratio(1), const Ratio ratioY = Ratio(1));
	void	PutCopyInCache(int index);
	virtual	uchar*	ResPointer() = 0;

	//  CelObj1  -  No remap on, but remap in data
	void  DrawHzFlip(const Buffer& dest, const SOL_Rect& toBuffer, const SOL_Point ul);
	void	DrawNoFlip(const Buffer& dest, const SOL_Rect& toBuffer, const SOL_Point ul);
	void	DrawUncompNoFlip(const Buffer& dest, const SOL_Rect& toBuffer, const SOL_Point ul);
	void	DrawUncompHzFlip(const Buffer& dest, const SOL_Rect& toBuffer, const SOL_Point ul);
	void	ScaleDraw(const Buffer&, const Ratio& ratioX, const Ratio& ratioY, const SOL_Rect& drawRect, const SOL_Point& pt);
	void	ScaleDrawUncomp(const Buffer& dest, const Ratio& ratioX, const Ratio& ratioY, const SOL_Rect& drawRect, const SOL_Point& pt);

	//  CelObj2  -  Remap on, remap in data
	void  DrawHzFlipMap(const Buffer& dest, const SOL_Rect& toBuffer, const SOL_Point ul);
	void	DrawNoFlipMap(const Buffer& dest, const SOL_Rect& toBuffer, const SOL_Point ul);
	void	DrawUncompNoFlipMap(const Buffer& dest, const SOL_Rect& toBuffer, const SOL_Point ul);
	void	DrawUncompHzFlipMap(const Buffer& dest, const SOL_Rect& toBuffer, const SOL_Point ul);
	void	ScaleDrawMap(const Buffer&, const Ratio& ratioX, const Ratio& ratioY, const SOL_Rect& drawRect, const SOL_Point& pt);
	void	ScaleDrawUncompMap(const Buffer&, const Ratio& ratioX, const Ratio& ratioY, const SOL_Rect& drawRect, const SOL_Point& pt);

	//  CelObj3  -  No remap on, remap in data, write priority
	void	DrawHzFlip(const Buffer& dest, const Buffer& destpri, SOL_Rect, SOL_Point ul, uchar pri);
	void	DrawNoFlip(const Buffer& dest, const Buffer& destpri, SOL_Rect, SOL_Point ul, uchar pri);
	void	DrawUncompNoFlip(const Buffer& dest, const Buffer& destpri, SOL_Rect, SOL_Point ul, uchar pri);
	void	DrawUncompHzFlip(const Buffer& dest, const Buffer& destpri, SOL_Rect, SOL_Point ul, uchar pri);
	void	ScaleDraw(const Buffer& dest, const Buffer& destpri, const Ratio& ratioX, const Ratio& ratioY, const SOL_Rect& drawRect, const SOL_Point& pt, uchar pri);
	void	ScaleDrawUncomp(const Buffer& dest, const Buffer& destpri, const Ratio& ratioX, const Ratio& ratioY, const SOL_Rect& drawRect, const SOL_Point& pt, uchar pri);

	//  CelObj4  -  Remap on, remap in data, write priority
	void	DrawHzFlipMap(const Buffer& dest, const Buffer& destpri, const SOL_Rect&, const SOL_Point ul, uchar pri);
	void	DrawNoFlipMap(const Buffer& dest, const Buffer& destpri, const SOL_Rect&, const SOL_Point ul, uchar pri);
	void	DrawUncompNoFlipMap(const Buffer& dest, const Buffer& destpri, const SOL_Rect&, const SOL_Point ul, uchar pri);
	void	DrawUncompHzFlipMap(const Buffer& dest, const Buffer& destpri, const SOL_Rect&, const SOL_Point ul, uchar pri);
	void	ScaleDrawMap(const Buffer& dest, const Buffer& destpri, const Ratio& ratioX, const Ratio& ratioY, const SOL_Rect& drawRect, const SOL_Point& pt, uchar pri);
	void	ScaleDrawUncompMap(const Buffer& dest, const Buffer& destpri, const Ratio& ratioX, const Ratio& ratioY, const SOL_Rect& drawRect, const SOL_Point& pt, uchar pri);

	//  CelObjA  -  No remap in data
	void  DrawHzFlipNoMD(const Buffer& dest, const SOL_Rect& toBuffer, const SOL_Point ul);
	void	DrawNoFlipNoMD(const Buffer& dest, const SOL_Rect& toBuffer, const SOL_Point ul);
	void	DrawUncompNoFlipNoMD(const Buffer& dest, const SOL_Rect& toBuffer, const SOL_Point ul);
	void	DrawUncompHzFlipNoMD(const Buffer& dest, const SOL_Rect& toBuffer, const SOL_Point ul);
	void	ScaleDrawNoMD(const Buffer&, const Ratio& ratioX, const Ratio& ratioY, const SOL_Rect& drawRect, const SOL_Point& pt);
	void	ScaleDrawUncompNoMD(const Buffer& dest, const Ratio& ratioX, const Ratio& ratioY, const SOL_Rect& drawRect, const SOL_Point& pt);

	void	DrawUncompNoFlipNoMDNoSkip(const Buffer& dest, const SOL_Rect& toBuffer, const SOL_Point ul);
	void	DrawUncompHzFlipNoMDNoSkip(const Buffer& dest, const SOL_Rect& toBuffer, const SOL_Point ul);

	//  CelObjB  -  No remap in data, write priority
	void	DrawHzFlipNoMD(const Buffer& dest, const Buffer& destpri, SOL_Rect, SOL_Point ul, uchar pri);
	void	DrawNoFlipNoMD(const Buffer& dest, const Buffer& destpri, SOL_Rect, SOL_Point ul, uchar pri);
	void	DrawUncompNoFlipNoMD(const Buffer& dest, const Buffer& destpri, SOL_Rect, SOL_Point ul, uchar pri);
	void	DrawUncompHzFlipNoMD(const Buffer& dest, const Buffer& destpri, SOL_Rect, SOL_Point ul, uchar pri);
	void	ScaleDrawNoMD(const Buffer& dest, const Buffer& destpri, const Ratio& ratioX, const Ratio& ratioY, const SOL_Rect& drawRect, const SOL_Point& pt, uchar pri);
	void	ScaleDrawUncompNoMD(const Buffer& dest, const Buffer& destpri, const Ratio& ratioX, const Ratio& ratioY, const SOL_Rect& drawRect, const SOL_Point& pt, uchar pri);


	Bitmap	bitmap;
	long		celOffset; 			//  Offset from view top to the cel
	long 		paletteOffset;		//  Offset from view top to the view palette
	int		width, height;
	int		xorg, yorg;
	int		resx, resy;			// XY Resolution cel was meant to display at
	uchar		skip;					// The skip color for this cel
	Bool		hasSkip;
	int		compressType;
	int		remapState;			// See enum
	Bool		mirrored; 			// True if recorded as mirrored
	Bool		horzFlip;			// True if mirrored & userMirrored flags need
										// to flip the data. (TEMP VALUE, do not
										// use in comparisons, copying, etc.)


	//	Symantec's compiler doesn't let us just call this 'Array'
	struct CelObjArray
	{
		struct CelObjArrayItem
		{
			CelObjArrayItem() : cel(NULL), timeStamp(0) {}
			inline ~CelObjArrayItem();

			long			timeStamp;
			CelObj*		cel;
		};
		CelObjArrayItem&	operator[](int i)		{return list[i];}

		protected:
		CelObjArrayItem list[maxCOV];
	};

	static long timeStamp;
	static CelObjArray array;

	static int scaleX1[maxScaledLine];
	static int scaleY1[maxScaledLine];
	static uchar line[maxScaledLine];
	static int* scaleX;
	static int* scaleY;
};

class CelObjView : public CelObj
{
	public:
	CelObjView(int v, int l, int c);
	CelObjView(const CelObjView& src) : CelObj(src) {}
	CelObj*	Duplicate() const			{return New CelObjView(*this);}
	void		Draw(const Buffer&, const ScreenItem&, const SOL_Rect&, Bool horzFlip);
	void		Draw(const Buffer&, const SOL_Rect& drawRect, SOL_Point, Bool horzFlip);
	void		Draw(const Buffer&, const SOL_Rect& drawRect, SOL_Point, Bool horzFlip,
					  const Ratio ratioX, const Ratio ratioY);

	protected:
	Bool		AnalyzeForRemap();
	Bool		AnalyzeUncompForRemap();
	uchar*	ResPointer();
};


class CelObjMem : public CelObj
{
	public:
	CelObjMem(MemID handle);
	CelObjMem(const CelObjMem& src) : CelObj(src) {}
	CelObj*	Duplicate() const			{return New CelObjMem(*this);}
	void		Draw(const Buffer&, const ScreenItem&, const SOL_Rect&, Bool horzFlip);  
	void		Draw(const Buffer&, const SOL_Rect& drawRect, SOL_Point, Bool horzFlip);
	void		Draw(const Buffer&, const SOL_Rect& drawRect, SOL_Point, Bool horzFlip,
					  const Ratio ratioX, const Ratio ratioY);

	protected:
	uchar*	ResPointer();
};

class CelObjPic : public CelObj
{
	public:
	CelObjPic(int p, int c);
	CelObjPic(const CelObjPic&);

	CelObj*		Duplicate() const			{return New CelObjPic(*this);}
	SOL_Point	Pos() const 				{return pos;}
	int			Priority() const			{return priority;}
	int			CelCount() const			{return celCount;}

	void			Draw(const Buffer&, const ScreenItem&, const SOL_Rect&, Bool userMirror);
	void			Draw(const Buffer&, const SOL_Rect& drawRect, SOL_Point, Bool userMirror);

	protected:
	Bool			AnalyzeForSkip();
	Bool			AnalyzeUncompForSkip();
	uchar*		ResPointer();

	int			celCount;
	SOL_Point	pos;
	int			priority;
};

class CelObjColor : public CelObj
{
	public:
	CelObjColor(uchar theColor,int theWidth,int theHeight);
	CelObj*	Duplicate() const			{return New CelObjColor(*this);}
	void		Draw(const Buffer&,const ScreenItem&, const SOL_Rect&, Bool);
	void		Draw(const Buffer&,const SOL_Rect& drawRect,SOL_Point,Bool);
	void		Draw(const Buffer&,const SOL_Rect& drawRect);

	protected:
	uchar*	ResPointer();
};

inline
CelObj::CelObjArray::CelObjArrayItem::~CelObjArrayItem()
{
	delete cel;
}


inline Bool
Bitmap::operator==(const Bitmap& src) const
{
	return  (view == src.view  &&  loop == src.loop  &&  cel == src.cel  &&
				type == src.type  &&  memid == src.memid);
}

inline Bool
Bitmap::operator!=(const Bitmap& src) const
{
	return  !(view == src.view  &&  loop == src.loop  &&  cel == src.cel &&
				 type == src.type  &&  memid == src.memid);
}

#endif
