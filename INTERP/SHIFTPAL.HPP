#if !defined(SHIFTPAL_HPP)
#define SHIFTPAL_HPP

#define	PALSHIFT	10

#ifdef MACINTOSH
	extern	Bool	WING; //KGN defined in shiftpal.cpp
#else
	extern	Bool	WING;
#endif

int  AddPalShift(int val);
void ShiftPaletteView (uchar* resBuffer);
void ShiftPalettePic  (uchar* resBuffer);
void ShiftPaletteCel  (uchar* resBuffer, long celOffset);

#endif

