
#include <io.h>
#include <sys\stat.h>
#include <fcntl.h>

#include "sol.hpp"

#include	"wingfunc.hpp"
#include "mbox.hpp"


FPWINGCREATEDC					fpWinGCreateDC = NULL;
FPWINGRECOMMENDDIBFORMAT	fpWinGRecommendDIBFormat = NULL;
FPWINGCREATEBITMAP			fpWinGCreateBitmap = NULL;
FPWINGBITBLT					fpWinGBitBlt = NULL;
FPWINGSETDIBCOLORTABLE		fpWinGSetDIBColorTable = NULL;

winGBitmapInfo	winGbi;

HMODULE		hWinG32;
HDC			hWinGDC;
HBITMAP		hWinGBitmap, oldWinGBitmap;
void far * 	pBits;
Bool			bDIBInverted = True;


BOOL WingInit (int vmapWidth, int vmapHeight)
{
char	buff[150];
int	fh;

	// See if we can find the WinG library
	if (!GetSystemDirectory(buff,150)) {
		MBox("no system dir","");
		return FALSE;
	}
	lstrcat(buff,"\\wing32.dll");
	if ((fh = open(buff,O_RDONLY)) == -1) {
		sprintf(mbuff,"file:%s not found",buff);
		MBox(mbuff,"");
		return FALSE;
	} else {
		close(fh);
	}

	// Load the WinG library and link to it
	hWinG32 = LoadLibrary(buff);
	if( hWinG32 == NULL ) {
		MBox("WinG LoadLibrary fail","");
		return FALSE;
	}

	fpWinGCreateDC = (FPWINGCREATEDC)GetProcAddress(hWinG32,"WinGCreateDC" );
	fpWinGRecommendDIBFormat = (FPWINGRECOMMENDDIBFORMAT)GetProcAddress(hWinG32,"WinGRecommendDIBFormat" );
	fpWinGCreateBitmap = (FPWINGCREATEBITMAP)GetProcAddress(hWinG32,"WinGCreateBitmap" );
	fpWinGBitBlt = (FPWINGBITBLT)GetProcAddress(hWinG32,"WinGBitBlt" );
	fpWinGSetDIBColorTable = (FPWINGSETDIBCOLORTABLE)GetProcAddress(hWinG32,"WinGSetDIBColorTable");

	if (!(fpWinGCreateDC &&
			fpWinGRecommendDIBFormat &&
			fpWinGCreateBitmap &&
			fpWinGBitBlt &&
			fpWinGSetDIBColorTable)) {
		MBox("WinG GetProcAddress fail","");
		return FALSE;
	}

	hWinGDC = (*fpWinGCreateDC)();
	if (!hWinGDC) {
		MBox("WinG CreateDC fail","");
		return FALSE;
	}

	if (!(*fpWinGRecommendDIBFormat)((BITMAPINFO far *)&winGbi)) {
		MBox("WinG recommend fail","");
		return FALSE;
	}

	if (winGbi.bmih.biHeight == -1)
		bDIBInverted = False;

	winGbi.bmih.biWidth = 	(DWORD)vmapWidth;
	winGbi.bmih.biHeight *=	(DWORD)vmapHeight;

	hWinGBitmap = (*fpWinGCreateBitmap) (hWinGDC, (BITMAPINFO far *)&winGbi, &pBits);
	if (!hWinGBitmap) {
		MBox("WinG CreateBitmap fail","");
		return FALSE;
	}

	oldWinGBitmap = (HBITMAP)SelectObject (hWinGDC, hWinGBitmap);
	return TRUE;
}


void WingTerm (void)
{
	SelectObject (hWinGDC, oldWinGBitmap);
	DeleteObject(hWinGBitmap);
	DeleteDC(hWinGDC);
	FreeLibrary( hWinG32 );
}
