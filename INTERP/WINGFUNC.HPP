#if !defined(WINGFUNC_HPP)
#define WINGFUNC_HPP

#ifndef ___WIN386_INCLUDED__
#include "winodef.hpp"
#include <windows.h>
#endif


BOOL WingInit (int vmapWidth, int vmapHeight);
void WingTerm (void);

typedef HDC 	 (FAR PASCAL * FPWINGCREATEDC) (void);
typedef BOOL 	 (FAR PASCAL * FPWINGRECOMMENDDIBFORMAT) (BITMAPINFO FAR *pFormat);
typedef HBITMAP (FAR PASCAL * FPWINGCREATEBITMAP) (HDC	WingDC,
																BITMAPINFO	const FAR *pHeader,
																void FAR *FAR *ppBits);
typedef BOOL (FAR PASCAL * FPWINGBITBLT) (HDC	hdcDest,
													int	nXOriginDest,
													int	nYOriginDest,
													int	nWidthDest,
													int	nHeightDest,
													HDC	hdcSrc,
													int	nXOriginSrc,
													int	nYOriginSrc);
typedef UINT (FAR PASCAL * FPWINGSETDIBCOLORTABLE) (HDC	hWinGDC,
													UINT	StartIndex,
													UINT	NumberOfEntries,
													RGBQUAD far *pColors);

extern FPWINGCREATEDC				fpWinGCreateDC;
extern FPWINGRECOMMENDDIBFORMAT	fpWinGRecommendDIBFormat;
extern FPWINGCREATEBITMAP			fpWinGCreateBitmap;
extern FPWINGBITBLT					fpWinGBitBlt;
extern FPWINGSETDIBCOLORTABLE		fpWinGSetDIBColorTable;

_Packed struct winGBitmapInfo {
		BITMAPINFOHEADER	bmih;
		RGBQUAD				ColorTable[256];
		};

extern	winGBitmapInfo	winGbi;

extern HMODULE		hWinG32;
extern HDC			hWinGDC;
extern HBITMAP		hWinGBitmap, oldWinGBitmap;
extern void far * pBits;
extern Bool			bDIBInverted;

#endif

