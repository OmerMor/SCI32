#ifndef PALETTE_HPP
#define PALETTE_HPP

#ifndef RGB_HPP
#include "rgb.hpp"
#endif

// Color defines
#ifdef	MACINTOSH
	const uchar SOL_BLACK = 255;
	const uchar SOL_WHITE = 0;
#else
	const uchar SOL_BLACK = 0;
	const uchar SOL_WHITE = 255;
#endif

const uchar SOL_GREY = 7;
const uchar SOL_SKIP = 250; // Default skip color for dialogs


class HunkPalette;
const MaxColors = 256;

class SOLPalette			//  Avoid creating on stack.  Over 1K size
{
	public:
	//  Constructors, assign, destructor
	SOLPalette(const HunkPalette&);
	SOLPalette(const SOLPalette&);
	SOLPalette();
	SOLPalette& operator=(const HunkPalette&);
	SOLPalette& operator=(const SOLPalette&);

	//  Const Methods
	int				operator==(const SOLPalette&) const;
	int				operator!=(const SOLPalette& src) const {return !operator==(src);}
	const Rgb24F&	operator[](int i) const			{return colors[i];} 
	int				Compare(const SOLPalette&, uchar *diff) const;
	const int		InUse(int i) const				{return colors[i].flag;}

	//  Normal Methods
	Rgb24F&	operator[](int i)					{return colors[i];}
	uchar&	InUse(int i)						{return colors[i].flag;}
	void		Merge(const SOLPalette&);
	int		Match(const Rgb24&) const;
	int		Match(const Rgb24&, int mindist, int* finalDist, const uchar* forbidden) const;

	protected:
	Rgb24F colors[MaxColors];
};


class HunkPalette
{
	public:
	HunkPalette()									{palInHunk = NULL;}
	void		Init(void*);
	long		Version() const					{return version;}
	int		Count() const						{return count;}
	void		SetVersion(int ver);

	protected:
	friend class SOLPalette;
	_Packed struct PalHeader
	{
		char	hdSize;
		char	palName[9];
		char	palCount;
		short	reserved;
	};

	PalHeader*	palInHunk;
	int			count;
	long			version;


	//  Individual palette within mult palette

	_Packed struct CompPal
	{
		char		title[10];		//  8 chars, 0 terminated
		uchar		startOffset;
		uchar		nCycles;			//  number of cycling ranges following header
		UInt16	fe;				//  future expansion (0)
		UInt16	nColors;			//  number of "colors" defined in this palette
		uchar		def;				//  "Default" flag setting (1)
		uchar		type;				//  (0 = each RGB has flag)
										//  (1 = All RGBs share default flag)
		UInt32	valid;
	};

	public:
	CompPal*	PalAddr() const					{return (CompPal*) ((uchar*) palInHunk + (sizeof(PalHeader) + (2 * count)));}

};


#endif


