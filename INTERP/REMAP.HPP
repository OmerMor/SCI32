
#ifndef REMAP_HPP
#define REMAP_HPP

#ifndef SAVEABLE_HPP
#include "saveable.hpp"
#endif

#ifndef RGB_HPP
#include "rgb.hpp"
#endif

extern int	REMAP_START_COLOR;
extern int	REMAP_END_COLOR;
extern int	START_COLOR;
extern int	END_COLOR;

const int REMAP_COLOR_RANGE = 9;

enum RemapType {
	REMAP_OFF,
	REMAP_BY_RANGE,
	REMAP_BY_PERCENT,
	REMAP_TO_GRAY,
	REMAP_TO_PERCENT_GRAY,
	REMAP_SET_GLOBAL_NOMATCH_RANGE
};


class Remap : public Saveable
{
	public:
	static uchar		remapColor[REMAP_COLOR_RANGE][256];
	static uint			remapDepth[REMAP_COLOR_RANGE];
	static uchar		remapOn[256];
	static uint			remapCount;
	static uint			remapDepthOn;
	static Bool			updateFlag;
	static int			forbiddenStart;
	static int			forbiddenCount;

	Remap();
	virtual ~Remap();

   void	RemapOff(int color = 0);
	void	RemapByRange(int color, int start, int end, int delta, int depth = 0);
	void	RemapByPercent(int color, int percent, int depth = 0);
	void	RemapToGray(int color, int gray, int depth = 0);
	void	RemapToPercentGray(int color, int gray, int percent, int depth = 0);
	Bool	RemapAllTables(Bool palChanged);     	
   void  SaveRestoreBefore(SaveGame*, Bool save);
	void	SaveRestore(SaveGame*, Bool save);
	void	DontMapToRange(int start, int count);

	protected:
	class SingleRemap  {
		public:
		SingleRemap(uchar* remapTable);
		~SingleRemap() {}
		Bool	Update(Bool palChanged);
		Bool	ByRange();
		Bool	ByPercent();
		Bool	ToGray();
		Bool	ToPercentGray();
      int   Match(Rgb24*, int, const Rgb24&, int, int&);
		RemapType	Type() const		{return type;}

		int			percent;
		int			oldPercent;
		int			gray;
		int			oldGray;
      int         start, end, delta;
		RemapType	type;

		protected:
		Bool			Apply(uchar*);

#ifdef MACINTOSH
		Rgb24			targetColor[256];
		Rgb24			colorArray[256];
		uchar			colorChanged[256];
		int			dist[256];
#else
		Rgb24			targetColor[236];
		Rgb24			colorArray[236];
		uchar			colorChanged[236];
		int			dist[236];
#endif
		uchar*		remap;
	};

	SingleRemap*	singleRemap[REMAP_COLOR_RANGE];

   Bool	InRange(int color)	{return (REMAP_START_COLOR <= color && color <= REMAP_END_COLOR);}
};


#endif
