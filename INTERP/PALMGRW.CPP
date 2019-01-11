#include "sciwin.hpp"
#include "sol.hpp"
#include "graphmgr.hpp"
#include "palmgrw.hpp"
#include "shiftpal.hpp"
#include "mbox.hpp"

void ClearSystemPalette(void);

// Logical palette
_Packed struct {
		WORD					palVersion;
		WORD					palNumEntries;
		union {
			PALETTEENTRY		palPalEntry[256];
			DWORD					dwEntry[256];
			};
		} lPal;

// Bitmap header
extern _Packed struct {
		BITMAPINFOHEADER	bmih;
		WORD	colorix [MAXPALETTE];
		} bi;

#ifdef WIN32S
// System palette
PALETTEENTRY	systemPal[256];

// for eliminating duplicates of system palette entries 246-255
static union {
	PALETTEENTRY	sysWork;
	DWORD				dwWork;
};

DWORD				sysEntry[10];
DWORD				sysAlternate[10];

Bool	paletteUpdated = True;
#endif


void
InitPalette()
{
int	i;

  	lPal.palVersion		= 0x300;
  	lPal.palNumEntries	= USERPALETTE;

#ifdef WIN32S
	if (!WING) {
#endif
		// DIB palette - not identity
		// set palette to black
	  	lPal.palVersion		= 0x300;
	  	lPal.palNumEntries	= USERPALETTE;

	  	for (i = 0; i < USERPALETTE; i++) {
		  	lPal.palPalEntry[i].peRed	= (BYTE)0;
		  	lPal.palPalEntry[i].peGreen	= (BYTE)0;
		  	lPal.palPalEntry[i].peBlue	= (BYTE)0;
	  		lPal.palPalEntry[i].peFlags = (BYTE)PC_RESERVED;
		}
	
	  	hPal = CreatePalette ((LPLOGPALETTE)&lPal);
	  	hOldPal = SelectPalette (hMyDC, hPal, 0);
	  	RealizePalette (hMyDC);

		// Here, we de-select the palette and then select it again.  This magic
		// is done so that our logical palette entry 0 is really at system
		// palette entry 10 so we can accuratly display device dependent bitmaps.
		// I don't know why this works.
	  	SelectPalette (hMyDC, hOldPal, 0);
	  	DeleteObject (hPal);

	  	hPal = CreatePalette ((LPLOGPALETTE)&lPal);
	  	hOldPal = SelectPalette (hMyDC, hPal, 0);
	  	RealizePalette (hMyDC);

#ifdef WIN32S
	} else {

		ClearSystemPalette();

		// Identity palette for WinG
	  	lPal.palVersion		= 0x300;
	  	lPal.palNumEntries	= 256;

		GetSystemPaletteEntries(hMyDC, 0, 256, (LPPALETTEENTRY)lPal.palPalEntry);

		// fill active palette with black
		for (i = 10; i < 246; i++) {
			lPal.palPalEntry[i].peRed = (BYTE)0;
			lPal.palPalEntry[i].peGreen = (BYTE)0;
			lPal.palPalEntry[i].peBlue = (BYTE)0;
//			lPal.palPalEntry[i].peFlags = (BYTE)(PC_NOCOLLAPSE | PC_RESERVED);
			lPal.palPalEntry[i].peFlags = (BYTE)PC_RESERVED;
		}

		// set flags of system colors to 0
		for (i = 0; i < 10; i++)
			lPal.palPalEntry[i].peFlags = (BYTE)0;

		for (i = 246; i < 256; i++)
			lPal.palPalEntry[i].peFlags = (BYTE)0;

		// set up arrays to eliminate duplicate colors
		for (i = 0; i < 10; i++) {
			lPal.palPalEntry[i + 246].peFlags = (BYTE)PC_RESERVED;
			sysWork = lPal.palPalEntry[i + 246];

			// set comparison array
			sysEntry[i] = dwWork;
			lPal.palPalEntry[i + 246].peFlags = (BYTE)0;

			// bump one color to make alternate value different
			if (sysWork.peRed < 255)
				sysWork.peRed++;
			else if (sysWork.peGreen < 255)
				sysWork.peGreen++;
			else if (sysWork.peBlue < 255)
				sysWork.peBlue++;
			else
				sysWork.peBlue--;
			sysAlternate[i] = dwWork;
		}

		hPal = CreatePalette ((LPLOGPALETTE)&lPal);
		hOldPal = SelectPalette (hMyDC, hPal, 0);
		RealizePalette (hMyDC);

		paletteUpdated = True;

#ifdef JUNK
		// now check again to see if they are really the same
		GetSystemPaletteEntries(hMyDC, 0, 256, (LPPALETTEENTRY)systemPal);

		for (i=0;i<256;i++) {
			if ((systemPal[i].peRed != lPal.palPalEntry[i].peRed) ||
				(systemPal[i].peGreen != lPal.palPalEntry[i].peGreen) ||
				(systemPal[i].peBlue != lPal.palPalEntry[i].peBlue)) {
					sprintf(mbuff,"c:%d r:%d:%d g:%d:%d b:%d:%d",i,
								systemPal[i].peRed, lPal.palPalEntry[i].peRed,
								systemPal[i].peGreen, lPal.palPalEntry[i].peGreen,
								systemPal[i].peBlue, lPal.palPalEntry[i].peBlue
								);
					SciDisplay(mbuff);
			}
		}
#endif	//JUNK
	}
#endif
}


void
TermPalette()
{
  	SelectPalette (hMyDC, hOldPal, 0);
  	DeleteObject (hPal);
}

void
UpdatePalette(SOLPalette &current, SOLPalette &next, int &gammaValue)
{
int	i, hiIndex;
int	colorVal, hiVal;

   if (current == next)
		return;
	current = next;

	hiVal = 0;

#ifdef WIN32S
	if (!WING) {
#endif
		// Normal palette for DIB
      if (gammaValue > -1) {
			for (i = 0; i < 236; i++)  {
				Rgb24 rgb = current[i];
			 	lPal.palPalEntry[i].peRed =   (BYTE)gamma[gammaValue][rgb.red];
			 	lPal.palPalEntry[i].peGreen = (BYTE)gamma[gammaValue][rgb.green];
			 	lPal.palPalEntry[i].peBlue =  (BYTE)gamma[gammaValue][rgb.blue];
				colorVal = lPal.palPalEntry[i].peRed +
							  lPal.palPalEntry[i].peGreen +
							  lPal.palPalEntry[i].peBlue;
				if (colorVal > hiVal) {
					hiVal = colorVal;
					hiIndex = i;
				}
			}
		} else {
			for (i = 0; i < 236; i++)  {
				Rgb24 rgb = current[i];
			 	lPal.palPalEntry[i].peRed =   (BYTE)rgb.red;
			 	lPal.palPalEntry[i].peGreen = (BYTE)rgb.green;
			 	lPal.palPalEntry[i].peBlue =  (BYTE)rgb.blue;
				colorVal = lPal.palPalEntry[i].peRed +
							  lPal.palPalEntry[i].peGreen +
							  lPal.palPalEntry[i].peBlue;
				if (colorVal > hiVal) {
					hiVal = colorVal;
					hiIndex = i;
				}
			}
		}

		// Set index 255 to lightest color so that debug window is visible
		bi.colorix[255] = hiIndex;

		AnimatePalette (hPal, 0, USERPALETTE,
							(LPPALETTEENTRY) &(lPal.palPalEntry [0]));
#ifdef WIN32S
	} else {
		// Identity palette for WinG
		paletteUpdated = True;
      if (gammaValue > -1) {
			for (i = 10; i < 246; i++)  {
				Rgb24 rgb = current[i-10];
			 	lPal.palPalEntry[i].peRed =   (BYTE)gamma[gammaValue][rgb.red];
			 	lPal.palPalEntry[i].peGreen = (BYTE)gamma[gammaValue][rgb.green];
			 	lPal.palPalEntry[i].peBlue =  (BYTE)gamma[gammaValue][rgb.blue];
			}
		} else {
			for (i = 10; i < 246; i++)  {
				Rgb24 rgb = current[i-10];
			 	lPal.palPalEntry[i].peRed =   (BYTE)rgb.red;
			 	lPal.palPalEntry[i].peGreen = (BYTE)rgb.green;
			 	lPal.palPalEntry[i].peBlue =  (BYTE)rgb.blue;
			}
		}

		for (i = 10; i < 246; i++) {
			DWORD palEntry = lPal.dwEntry[i];
			for (int j = 0; j < 10; j++) {
				if (sysEntry[j] == palEntry)
					lPal.dwEntry[i] = sysAlternate[j];
			}
		}

	  	SelectPalette (hMyDC, hOldPal, 0);
	  	DeleteObject (hPal);

	  	hPal = CreatePalette ((LPLOGPALETTE)&lPal);
	  	hOldPal = SelectPalette (hMyDC, hPal, 0);
	  	RealizePalette (hMyDC);

//		AnimatePalette (hPal, 10, 236,
//							(LPPALETTEENTRY) &(lPal.palPalEntry [0]));

	}
#endif
}


/*
 *  Clear the System Palette so that we can ensure an identity palette 
 *  mapping for fast performance.
 */

void ClearSystemPalette(void)
{
  //*** A dummy palette setup
  struct
  {
    WORD Version;
    WORD NumberOfEntries;
    PALETTEENTRY aEntries[256];
  } Palette =
  {
    0x300,
    256
  };

  HPALETTE ScreenPalette = 0;
  HDC ScreenDC;
  int Counter;
  UINT nMapped = 0;
  BOOL bOK = FALSE;
  int  nOK = 0;
  
  //*** Reset everything in the system palette to black
  for(Counter = 0; Counter < 256; Counter++)
  {
    Palette.aEntries[Counter].peRed = 0;
    Palette.aEntries[Counter].peGreen = 0;
    Palette.aEntries[Counter].peBlue = 0;
    Palette.aEntries[Counter].peFlags = PC_NOCOLLAPSE;
  }

  //*** Create, select, realize, deselect, and delete the palette
  ScreenDC = GetDC(NULL);
  ScreenPalette = CreatePalette((LOGPALETTE *)&Palette);

  if (ScreenPalette)
  {
    ScreenPalette = SelectPalette(ScreenDC,ScreenPalette,FALSE);
    nMapped = RealizePalette(ScreenDC);
    ScreenPalette = SelectPalette(ScreenDC,ScreenPalette,FALSE);
    bOK = DeleteObject(ScreenPalette);
  }

  nOK = ReleaseDC(NULL, ScreenDC);

  return;
}
