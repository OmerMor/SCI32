#if !defined(SHIFTPAL_HPP)
#define SHIFTPAL_HPP

#define	PALSHIFT	10

extern	Bool	WING;

int  AddPalShift(int val);
void ShiftPaletteView (uchar* resBuffer);
void ShiftPalettePic  (uchar* resBuffer);
void ShiftPaletteCel  (uchar* resBuffer, long celOffset);

#endif

