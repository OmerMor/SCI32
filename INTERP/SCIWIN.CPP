/* SCIWIN.C -- Windows version of SCI */

#define INCLUDE_MMSYSTEM_H
#include "sciwin.hpp"
#include "sol.hpp"
#ifdef WIN32S
#include "winclass.hpp"
#include "wingfunc.hpp"
#endif
#include "scidlg.hpp"
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include "event.hpp"
#include "audio.hpp"
#include "config.hpp"
#include "graphmw.hpp"
#include "kernel.hpp"
#include "memmgrw.hpp"
#include "mousew.hpp"
#include "moview.hpp"
#include "neww.hpp"
#include "sci.hpp"
#include "kbdmgrw.hpp"
#include "pmachine.hpp"
#include "sound.hpp"
#include "soundwin.hpp"
#include "timew.hpp"
#include "mmsystem.h"
#include "winthunk.hpp"

int scimain (int argc, PSTR argv[]);

#ifndef WIN32S
long                    FAR PASCAL WndProc (HWND, unsigned, WORD, LONG) ;
#else
LRESULT CALLBACK _export WndProc( HWND hWnd, UINT iMessage,WPARAM wParam, LPARAM lParam );
#endif
int                     PASCAL WinMain (HANDLE, HANDLE, LPSTR, int);
BOOL                    FAR PASCAL _export testDlgProc (HWND, WORD, WORD, LONG);
BOOL                    doDialog(int);
void                    GetProfileInfo (void);
BOOL                    GetTFOption (LPSTR, BOOL);
int                     GetIntOption (LPSTR, int);
int                     GetToken (LPSTR, int, int);
void                    GetScreenInfo (void);
void                    ReadCommandLine (LPSTR);
void                    MBox (char *, char *);
Bool                    MBox2 (char *, char *);
void                    StartTimer (void);
void                    StopTimer (void);
void                    CheckConfig (void);

void	closeWaveDev(void);
void	AudioUpdate(void);
void	PollAudBuffs(void);
unsigned	GetDACDefaultSize(void);

#ifdef WIN32S
HINSTANCE	Main::hInstance = 0;
HINSTANCE	Main::hPrevInstance = 0;
int			Main::nCmdShow = 0;
LPSTR			Main::lpszCmdLine = 0;
#endif

PSTR				argv [10];
char            commandLine[128];
int             argc;
int             posX, posY;
int             windWidth, windHeight;
HWND            hMyWnd;
DWORD           myStyle;
HINSTANCE		MyInstance;
HCURSOR			hMyCur = 0;
HCURSOR			hAltCur, hCurCur;
HCURSOR			hNullCur;
HWND            hDisp;
HWND            hWndList;
HDC             hPaintDC = (HDC)NULL;
HDC             hMyDC;
HMENU           hMenu;
HPALETTE        hPal, hOldPal;
POINT           myPoint;
int             howShow;
WORD            MapHeight, MapWidth;
DWORD           chan;
DWORD           winFlags;
char            xnum [10];
WORD            extraLines;
WORD            timerId;
char            mbuff[80];
WORD            nCheck;
char            modSciwin[] = "sciwin";
int	nHORZRES, nVERTRES, nColors, nBITSPIXEL, nPLANES, nRASTERCAPS;
int		forcePPC = 0;
int		asyncCheckFreq = 10;
int		monoDisp = 0;
MSG myMsg;
BOOL	  SHOWMEM   = FALSE;
BOOL	  SHOWNEWS   = FALSE;
BOOL    LOCKMAIN   = FALSE;
BOOL    LOCKAUDIO  = FALSE;
BOOL    GAMEPAUSED = FALSE;
BOOL    REQUIREDAC       = FALSE;       /* require user to have a DAC */
BOOL    CONFIRMDAC   = TRUE;       /* confirm from user if no DAC */
BOOL    CONFIRMMIDI  = FALSE;   /* confirm from user if no MIDI */
BOOL    CONFIRMREDBOOK = FALSE; /* confirm from user if no Redbook */
BOOL    USEAUDIO = TRUE;
BOOL    USEMIDI = TRUE;
BOOL    DOPPCTEST = TRUE;
BOOL	  TITLEBAR = TRUE;
BOOL	  TITLEBARFIXED = FALSE;
BOOL	  ALLOWRESIZE = TRUE;
BOOL	  ALLOWTRUECOLOR = TRUE;
BOOL	  TRUECOLOR = FALSE;
BOOL	  TRAPCLOSE = TRUE;
BOOL	  COLORCURSOR = FALSE;
BOOL	  MAXIMIZED = FALSE;
BOOL	  haveFocus = TRUE;
BOOL	  firstPause = TRUE;
BOOL	  waitVMDStop = FALSE;
BOOL	  waitClose = FALSE;
BOOL	  ES688WIN = FALSE;
Bool	  WING = FALSE;
BOOL	  bWin95;
#ifdef WIN32S
int     wingMinK = 3000;
#endif

/* Windows sound devices */
#define PCSPEAKER       0
#define BASESYNTH       1
#define HIGHSYNTH       2
#define BASEADLIB       3
extern  int     MidiType;

char		szIniFile[_MAX_PATH];
char		szInfo[] = "Windows";
static char		szDefaultIniFile[] = "RESOURCE.WIN";
static char		szTitle[80] = "Sierra On-Line";
static char		szAppName [] = "SciWin" ;
char				szWinPatchDir[50] = "";
int     gameWidth = SCIRESX;
int     gameHeight = SCIRESY;
int     gameBorder = 0;
int     borderPixels = 5;
int     maximizedBorder = 0;
int     clientWidth, clientHeight;
int     forceWidth = 0;
int     forceHeight = 0;
int     ratioX, ratioY;
int     reserveK = 0;
int     sciMinK = 1500;
int     sciMaxK = 8000;
int     vmapWidth;
int     vmapHeight;
int	  DACSize;
int	  cursorViewAdd = 0;
int	  maxCursorSize = 0;
DWORD   midiBuffSize[NUMMIDIBUFFS];
ulong		asyncCheckPeriod = 0;

Bool    isMaximized = False;
Bool    minMaxFlag = False;
extern	int		writeCount;

extern	int 			vmdStatus;
extern	BOOL			vmdShowFullFrame;
extern	BOOL			vmdKillFocus;

#pragma warning 579  9         //   Shut off cast warning for lpfnWndProc

int PASCAL
#ifndef WIN32S
WinMain (HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
#else
WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow )
#endif
/***********************************************************************
	Standard Windows application main procedure.
************************************************************************/
{
#ifndef WIN32S
HWND        hWnd ;
#endif
MSG         msg ;
WNDCLASS    wndclass ;
extern BOOL haveMouse;
RECT            windRect;

#ifdef WIN32S
	 Main::hInstance = hInstance;
	 Main::hPrevInstance = hPrevInstance;
	 Main::nCmdShow = nCmdShow;
	 Main::lpszCmdLine = lpszCmdLine;
#endif

	howShow = nCmdShow;             //to eliminate W3 warning
	msg.wParam = 0;                 //ditto

#ifndef WIN32S
	if (!hPrevInstance) {
		MyInstance = hInstance;
		wndclass.style				= CS_HREDRAW | CS_VREDRAW ;
		wndclass.lpfnWndProc		= (WNDPROC)WndProc ;
		wndclass.cbClsExtra		= 0 ;
		wndclass.cbWndExtra		= 0 ;
		wndclass.hInstance		= hInstance ;
		wndclass.hIcon				= LoadIcon (hInstance, "myicon") ;
#else
	if ( ! Main::hPrevInstance ) {
		MyInstance = hInstance;
		wndclass.style				= CS_HREDRAW | CS_VREDRAW ;
		wndclass.lpfnWndProc		= ::WndProc ;
		wndclass.cbClsExtra		= 0 ;
		  // Reserve extra bytes for each instance of the window;
		  // we will use these bytes to store a pointer to the C++
		  // (MainWindow) object corresponding to the window.
		  // the size of a 'this' pointer depends on the memory model.
		  wndclass.cbWndExtra    = sizeof( MainWindow * );
		  wndclass.hInstance     = Main::hInstance;
		wndclass.hIcon				= LoadIcon (Main::hInstance, "myicon") ;
#endif
		wndclass.hCursor			= NULL;
		wndclass.hbrBackground  = GetStockObject (BLACK_BRUSH) ;
		wndclass.lpszMenuName   = NULL ;
		wndclass.lpszClassName  = szAppName ;

		hAltCur = LoadCursor(NULL, IDC_ARROW);
		hNullCur = LoadCursor(MyInstance,"nullcur");
		hCurCur = hAltCur;
		SetCursor(hCurCur);

		if (!RegisterClass (&wndclass))
			return FALSE ;

	} else {
		MessageBox (GetFocus(), "Cannot run two copies of game!",
						"Sierra", MB_OK);
		return msg.wParam ;                                     /* return to Windows */
	}

	bWin95 = ((HIBYTE(LOWORD(GetVersion()))) >= 95);

	ReadCommandLine (lpszCmdLine);  /* get command line arguments */

	GetProfileInfo();

	if (COLORCURSOR) {
		hCurCur = hNullCur;
		SetCursor(hCurCur);
	}

	GetScreenInfo();

	windRect.top	 = 0;
	windRect.left	 = 0;
	windRect.bottom = clientHeight;
	windRect.right	 = clientWidth;

	if (TITLEBAR)
		myStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
	else
		myStyle = WS_POPUP | WS_CLIPCHILDREN;

	AdjustWindowRect (&windRect, myStyle, FALSE);
	windWidth  = windRect.right - windRect.left;
	windHeight = windRect.bottom - windRect.top;

	posX = 0;
	posY = 0;
	if (nHORZRES > windWidth)
		posX = (nHORZRES - windWidth) / 2;
	if (nVERTRES > windHeight)
		posY = (nVERTRES - windHeight) / 2;

	if ((nHORZRES <= windWidth) || MAXIMIZED) {
		howShow = SW_SHOWMAXIMIZED;
		gameBorder = maximizedBorder;
      isMaximized = True;
	} else {
		howShow = SW_SHOWNORMAL;
		isMaximized = False;
	}

	// remove title bar if not enough room
	if (TITLEBAR & !TITLEBARFIXED)
		if ((windHeight - (2 * borderPixels)) > nVERTRES) {
			TITLEBAR = FALSE;
			myStyle = WS_POPUP | WS_CLIPCHILDREN;
		}

#ifndef WIN32S
	hWnd = CreateWindow (szAppName, szTitle,
								myStyle,
								posX, posY,
								windWidth, windHeight,
								NULL, NULL, hInstance, NULL) ;
	hMyWnd = hWnd;
#else
	MainWindow MainWnd;
	if (!hMyWnd)
			return FALSE ;
#endif

	SetCapture(hMyWnd);

	ShowWindow (hMyWnd, howShow) ;
	UpdateWindow (hMyWnd) ;

	hMyDC = GetDC (hMyWnd);                 /* leave open for duration of execution */

	CheckConfig();

	InitThunk();

	GetWindowsMemory();

	scimain(argc, argv);                   /* call sci.c to start interpreter */

	exit (1);
//	return msg.wParam ;                     /* return to Windows */
}

#ifdef WIN32S
// Do not create unless previously registered.
MainWindow::MainWindow( void )
	 {
		  // Pass 'this' pointer in lpParam of CreateWindow().
		  hWnd = CreateWindow( szAppName,
				szTitle,
				myStyle,
				posX, posY,
				windWidth, windHeight,
				NULL,
				NULL,
				Main::hInstance,
				(LPSTR) this );
	hMyWnd = hWnd;                                          /* for public use */
	 }
#endif




void AsyncEventCheck ()
/**********************************************************************
	Normal Windows message processing loop made a function since it is
	called from more than one place within the interpreter.  Use PeekMessage
	so that we can return when message queue exhausted.
***********************************************************************/
{
ulong		currTime;
static	ulong		nextTime = 0;

	if (waitVMDStop) {
		if (vmdStatus == statusNotOpen) {
			waitVMDStop = FALSE;
			if (waitClose) {
				pm.Global(g_quit) = 2;
				waitClose = FALSE;
			} else {
				GAMEPAUSED = TRUE;
				firstPause = TRUE;
			}
		} else
			return;
	}

	while (GAMEPAUSED) {
		if (firstPause) {
			firstPause = FALSE;
			((TimeMgrWin*)timeMgr)->DisableTimer();
		}
		GetMessage (&myMsg, NULL, 0, 0);
		if (myMsg.message == WM_QUIT)
			exit (1);
		TranslateMessage (&myMsg) ;
		DispatchMessage (&myMsg) ;
	}

	PollAudBuffs();

	if (ES688WIN) {
		// The ES688WIN.DRV driver version 4.13.02 on the Compaq presario
		// (and other machines) crashes while playing DAC.
		// The following code fixes the problem, altough I don't know why.
		// TM - 7/31/95
		currTime = timeGetTime();
		if (currTime < nextTime)
			return;
		nextTime = currTime + asyncCheckPeriod;
	} else {
		if (!*timeToCheck)
			return;
		*timeToCheck = 0;
	}

	AudioServer();
	if (!USEAUDIO)
		if (writeCount)
			AudioUpdate();

	while (PeekMessage (&myMsg, NULL, 0, 0, PM_REMOVE))
		{
		if (myMsg.message == WM_QUIT)
			exit (1);
		TranslateMessage (&myMsg) ;
		DispatchMessage (&myMsg) ;
		}
}


void
ReadCommandLine (LPSTR lpszCmdLine)
/*************************************************************************
	Parse command line arguments
**************************************************************************/
{
char    *outChar;
LPSTR   inChar;

	argc = 1;
	inChar = lpszCmdLine;
	outChar = (char *)commandLine;
	while (*inChar)                                         /* quit on null */
		{
		while (*inChar == ' ')                  /* skip over spaces */
			inChar++;
		argv [argc++] = outChar;                /* next argument pointer */
		while ((*inChar != ' ') && *inChar)
			*outChar++ = *inChar++;         /* move argument, stop on space or null */
		*outChar++ = NULL;                              /* terminate string */
		}

	//find the config file name
	for (int i = 1; i < argc; ++i) {
		if (*argv[i] != '-' && *argv[i] != '/') {
			_fullpath(szIniFile, argv[i], sizeof szIniFile);
			strupr(szIniFile);
		}
	}

	if (!ConfigMgr::FindConfigFile(szIniFile, szDefaultIniFile)) {
		char buf[100];
		sprintf(buf, "Can't open configuration file %s.\nPlease run Setup.", szIniFile);
		MBox(buf, "Error");
		exit(1);
	}
}

void GetProfileInfo (void)
{
char            buff[80];
unsigned			 aSize, dSize;

	if (!GetPrivateProfileString ((LPSTR)szInfo, "title", "", szTitle,
											sizeof(szTitle), (LPCSTR)szIniFile))
		MBox ("Windows title not found","");

	GetPrivateProfileString ((LPSTR)szInfo, "winPatchDir", "", szWinPatchDir,
											sizeof(szWinPatchDir), (LPCSTR)szIniFile);

	GetPrivateProfileString ((LPSTR)szInfo, "synthtype", "", buff,
											sizeof(buff), (LPSTR)szIniFile);
	if ((buff[0] == 'P') || (buff[0] == 'p'))
		MidiType = PCSPEAKER;
	if ((buff[0] == 'B') || (buff[0] == 'b'))
		MidiType = BASESYNTH;
	if ((buff[0] == 'H') || (buff[0] == 'h'))
		MidiType = HIGHSYNTH;
	if ((buff[0] == 'A') || (buff[0] == 'a'))
		MidiType = BASEADLIB;

	forceWidth		= GetPrivateProfileInt ((LPSTR)szInfo, "wwidth", 0, (LPSTR)szIniFile);
	forceHeight		= GetPrivateProfileInt ((LPSTR)szInfo, "wheight", 0, (LPSTR)szIniFile);
	borderPixels	= GetPrivateProfileInt ((LPSTR)szInfo, "borderpixels", 5, (LPSTR)szIniFile);
	gameBorder = borderPixels;
	reserveK			= GetPrivateProfileInt ((LPSTR)szInfo, "reservek", 600, (LPSTR)szIniFile);
	sciMinK			= GetPrivateProfileInt ((LPSTR)szInfo, "scimink", 1500, (LPSTR)szIniFile);
	sciMaxK			= GetPrivateProfileInt ((LPSTR)szInfo, "scimaxk", 8000, (LPSTR)szIniFile);
#ifdef WIN32S
	wingMinK			= GetPrivateProfileInt ((LPSTR)szInfo, "wingmink", 3000, (LPSTR)szIniFile);
#endif
	cursorViewAdd	= GetPrivateProfileInt ((LPSTR)szInfo, "cursorviewadd", 0, (LPSTR)szIniFile);
	maxCursorSize	= GetPrivateProfileInt ((LPSTR)szInfo, "maxCursorSize", 4, (LPSTR)szIniFile);
	maxCursorSize = (((maxCursorSize + 3) >> 2) << 2);
	forcePPC			= GetPrivateProfileInt ((LPSTR)szInfo, "forceppc", 0, (LPSTR)szIniFile);
	dSize 			= (unsigned)GetIntOption ("DACSize", GetDACDefaultSize());
	aSize				= GetIntOption ("audioSize", INT_HBUFFSIZE*2);
	AdjustAudBuffSizes(&aSize,&dSize);
	DACSize			= (int)dSize;
	vmapWidth		= GetToken("hires",0,640);
	vmapHeight		= GetToken("hires",1,480);
	vmapWidth=640;
	vmapHeight=480;

	asyncCheckFreq	= GetPrivateProfileInt ((LPSTR)szInfo, "asynccheckfreq", 10, (LPSTR)szIniFile);
	asyncCheckPeriod = 1000 / asyncCheckFreq;
	if (asyncCheckPeriod < 1)
		asyncCheckPeriod = 1;
	monoDisp			= GetPrivateProfileInt ((LPSTR)szInfo, "monodisp", 0, (LPSTR)szIniFile);
	reserveSize		= GetPrivateProfileInt ((LPSTR)szInfo, "reserveSize", 36000, (LPSTR)szIniFile);
	cacheSize1		= GetPrivateProfileInt ((LPSTR)szInfo, "cacheSize1", 400, (LPSTR)szIniFile);
	cacheSize2		= GetPrivateProfileInt ((LPSTR)szInfo, "cacheSize2", 500, (LPSTR)szIniFile);
	cacheSize3		= GetPrivateProfileInt ((LPSTR)szInfo, "cacheSize3", 50, (LPSTR)szIniFile);
	entrySize1		= GetPrivateProfileInt ((LPSTR)szInfo, "entrySize1", 32, (LPSTR)szIniFile);
	entrySize2		= GetPrivateProfileInt ((LPSTR)szInfo, "entrySize2", 512, (LPSTR)szIniFile);
	entrySize3		= GetPrivateProfileInt ((LPSTR)szInfo, "entrySize3", 1200, (LPSTR)szIniFile);

	midiBuffSize[0] = (DWORD)GetPrivateProfileInt ((LPSTR)szInfo, "midiBuffSize0", 100,		(LPSTR)szIniFile);
	midiBuffSize[1] = (DWORD)GetPrivateProfileInt ((LPSTR)szInfo, "midiBuffSize1", 100,		(LPSTR)szIniFile);
	midiBuffSize[2] = (DWORD)GetPrivateProfileInt ((LPSTR)szInfo, "midiBuffSize2", 1000,	(LPSTR)szIniFile);
	midiBuffSize[3] = (DWORD)GetPrivateProfileInt ((LPSTR)szInfo, "midiBuffSize3", 1000,	(LPSTR)szIniFile);
	midiBuffSize[4] = (DWORD)GetPrivateProfileInt ((LPSTR)szInfo, "midiBuffSize4", 10000,	(LPSTR)szIniFile);
	midiBuffSize[5] = (DWORD)GetPrivateProfileInt ((LPSTR)szInfo, "midiBuffSize5", 65000,	(LPSTR)szIniFile);
	midiBuffSize[6] = (DWORD)GetPrivateProfileInt ((LPSTR)szInfo, "midiBuffSize6", 0,		(LPSTR)szIniFile);
	midiBuffSize[7] = (DWORD)GetPrivateProfileInt ((LPSTR)szInfo, "midiBuffSize7", 0,		(LPSTR)szIniFile);
	midiBuffSize[8] = (DWORD)GetPrivateProfileInt ((LPSTR)szInfo, "midiBuffSize8", 0,		(LPSTR)szIniFile);
	midiBuffSize[9] = (DWORD)GetPrivateProfileInt ((LPSTR)szInfo, "midiBuffSize9", 0,		(LPSTR)szIniFile);

	CONFIRMDAC		= GetTFOption("confirmdac",TRUE);
	REQUIREDAC		= GetTFOption("requiredac",FALSE);
	CONFIRMMIDI		= GetTFOption("confirmmidi",FALSE);
	USEMIDI			= GetTFOption("usemidi",TRUE);
	USEAUDIO			= GetTFOption("useaudio",TRUE);
	CONFIRMREDBOOK	= GetTFOption("confirmredbook",FALSE);
	TITLEBAR			= GetTFOption("titlebar",TRUE);
	TITLEBARFIXED	= GetTFOption("titlebarfixed",FALSE);
	LOCKMAIN			= GetTFOption("lockmain",FALSE);
	LOCKAUDIO		= GetTFOption("lockaudio",FALSE);
	SHOWMEM			= GetTFOption("showmem",FALSE);
	SHOWNEWS			= GetTFOption("shownews",FALSE);
	DOPPCTEST		= GetTFOption("doppctest",TRUE);
	ALLOWRESIZE		= GetTFOption("allowresize",TRUE);
	ALLOWTRUECOLOR	= GetTFOption("allowtruecolor",TRUE);
	TRAPCLOSE		= GetTFOption("trapclose",TRUE);
	COLORCURSOR		= GetTFOption("colorcursor",FALSE);
	MAXIMIZED		= GetTFOption("maximized",FALSE);
#ifdef WIN32S
	WING				= GetTFOption("wing",TRUE);
#endif

	if (forcePPC)
		DOPPCTEST = TRUE;
}

BOOL	GetTFOption(LPSTR optionName, BOOL defaultVal)
{
char	TFBuff[10];

	GetPrivateProfileString ((LPSTR)szInfo, optionName, "x", TFBuff,
									sizeof(TFBuff), (LPSTR)szIniFile);
	if ((TFBuff[0] == 'T') || (TFBuff[0] == 't'))
		return TRUE;
	if ((TFBuff[0] == 'F') || (TFBuff[0] == 'f'))
		return FALSE;
	return defaultVal;
}

int	GetIntOption(LPSTR optionName, int defaultVal)
{
char	str[50];
int	num;

	GetPrivateProfileString ((LPSTR)szInfo, optionName, "q", str,
									sizeof(str), (LPSTR)szIniFile);
	if (str[0] == 'q')
		return defaultVal;

	if (strchr(str,'X') || strchr(str,'x'))
		sscanf(str, "%x", &num);
	else {
		sscanf(str, "%d", &num);
		if (strchr(str,'K') || strchr(str,'k'))
			num *= 1024;
		else if (strchr(str,'M') || strchr(str,'m'))
			num *= 1024 * 1024;
	}
	return num;
}

int	GetToken(LPSTR optionName, int tokenNum, int defaultVal)
{
char	str[50];
char	*tmp;
char	*strPtr;
int	i;

	GetPrivateProfileString ((LPSTR)szInfo, optionName, "q", str,
									sizeof(str), (LPSTR)szIniFile);
	if (str[0] == 'q')
		return defaultVal;
	strPtr = str;
	for (i = 0; i < tokenNum + 1; i++) {
		tmp = strtok(strPtr, " =\t;,");
		strPtr = 0;
	}
	if (tmp) {
		strcpy(str,tmp);
		return atoi(str);
	} else
		return defaultVal;
}

void
GetScreenInfo(void)
{
int nASPECTX, nASPECTY;
int nCYCAPTION, nCXFRAME, nCYFRAME;
int maxClientX, maxClientY;

	hMyDC = CreateIC ("Display", NULL, NULL, NULL);
	nRASTERCAPS = GetDeviceCaps (hMyDC, RASTERCAPS);
	nHORZRES = GetDeviceCaps (hMyDC, HORZRES);
	nVERTRES = GetDeviceCaps (hMyDC, VERTRES);
	nASPECTX = GetDeviceCaps (hMyDC, ASPECTX);
	nASPECTY = GetDeviceCaps (hMyDC, ASPECTY);
	nBITSPIXEL = GetDeviceCaps (hMyDC, BITSPIXEL);
	if (nBITSPIXEL > 24)		//some drivers have reported 32 bits
		nBITSPIXEL = 24;
	nPLANES = GetDeviceCaps (hMyDC, PLANES);
	nColors = 1 << (nPLANES * nBITSPIXEL);
	DeleteDC (hMyDC);

	nCYCAPTION = GetSystemMetrics(SM_CYCAPTION);
	nCXFRAME   = GetSystemMetrics(SM_CXFRAME);
	nCYFRAME   = GetSystemMetrics(SM_CYFRAME);

	if (nASPECTX == nASPECTY) {
		ratioX = 4;
		ratioY = 3;
	} else {
		ratioX = 4 * nASPECTY;
		ratioY = 3 * nASPECTX;
	}

	maxClientX = nHORZRES - (2 * nCXFRAME) - (2 * gameBorder);
	maxClientY = nVERTRES - (2 * nCYFRAME) - (2 * gameBorder) - nCYCAPTION;

	if (maxClientX > ((ratioX * maxClientY) / ratioY)) {
		gameWidth  = (ratioX * maxClientY) / ratioY;
		gameHeight = maxClientY;
	} else {
		gameWidth  = maxClientX;
		gameHeight = (ratioY * maxClientX) / ratioX;
	}

	if (nHORZRES > vmapWidth)
		maximizedBorder = (nHORZRES - vmapWidth) / 2;
	else {
		if (bWin95)
			maximizedBorder = 0;
		else
			// Windows 3.1 does not display row 0, column 0 in maximized mode
			maximizedBorder = 1;
	}

	if (gameWidth > vmapWidth) {
		gameWidth = vmapWidth;
		gameHeight = (ratioY * gameWidth) / ratioX;
	}

	// check if input overrides
	if (forceWidth) {
		gameWidth = forceWidth;
		gameHeight = (ratioY * gameWidth) / ratioX;
	}
	if (forceHeight)
		gameHeight = forceHeight;

	//game cannot be larger than vmap
	gameWidth = Min(gameWidth,vmapWidth);
	gameHeight = Min(gameHeight,vmapHeight);

	clientWidth = gameWidth + (2 * gameBorder);
	clientHeight = gameHeight + (2 * gameBorder);
}

void
CheckConfig (void)
{
PCMWAVEFORMAT   pwf;
HWAVEOUT			hWave;
HMIDIOUT       hMidiOut;
WORD                            wError;
BOOL                            dlgResult;
char	szErrorText[150];

	if (SHOWMEM) {
		sprintf(mbuff,"&CheckConfig:%08x",&CheckConfig);
		MBox(mbuff,"");
	}

	if (!(nRASTERCAPS & RC_PALETTE))
		TRUECOLOR = TRUE;
	else
		TRUECOLOR = FALSE;

	if (nColors < 256)
		{
		GetPrivateProfileString ("boot", "display.drv", "none", mbuff, 40,
									"system.ini");
		MessageBox (NULL,
			"Your current display driver is not supported by this game."
			"  We require a driver which can display at least 256 colors."
			"  You can install a different driver by selecting the appropriate"
			" options from Windows Setup.",
				mbuff, MB_OK | MB_ICONSTOP);
		exit(1);
		}

	if ((nColors > 256) && !ALLOWTRUECOLOR)
		{
		GetPrivateProfileString ("boot", "display.drv", "none", mbuff, 40,
									"system.ini");
		MessageBox (NULL,
			"Your current display driver is not supported by this game."
			"  We require a driver which can display exactly 256 colors."
			"  You can install a different driver by selecting the appropriate"
			" options from Windows Setup.",
				mbuff, MB_OK | MB_ICONSTOP);
		exit(1);
		}

	// see if we are using that !@#$% es688win.drv audio driver.
	GetPrivateProfileString ("drivers", "wave", "none", mbuff, 40, "system.ini");
	if (lstrcmpi(mbuff,"es688win.drv"))
		ES688WIN = FALSE;
	else
		ES688WIN = TRUE;

	pwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pwf.wf.nChannels  = 1;
	pwf.wf.nSamplesPerSec = 11025;
	pwf.wf.nAvgBytesPerSec = 11025;
	pwf.wf.nBlockAlign = 1;
	pwf.wBitsPerSample = (WORD)8;

	wError = waveOutOpen ((LPHWAVEOUT)&hWave,
									WAVE_MAPPER,
									(LPWAVEFORMAT)&pwf.wf,
									(DWORD)hMyWnd,
									(DWORD)0,
									CALLBACK_WINDOW);
	if (wError) {
		USEAUDIO = FALSE;
		if (REQUIREDAC || CONFIRMDAC)
			if (!waveOutGetErrorText(wError, (LPSTR)szErrorText, sizeof(szErrorText)))
				MessageBox (NULL, szErrorText, "waveOutOpen", MB_OK);

		if (REQUIREDAC) {
			doDialog(DLG_REQUIREDAC);
			exit(1);
		}
		if (CONFIRMDAC) {
			dlgResult = doDialog(DLG_CONFIRMDAC);
			if (nCheck)
				WritePrivateProfileString ((LPSTR)szInfo, "confirmdac", NULL,
													(LPSTR)szIniFile);
			if (!dlgResult)
				exit(1);
		}
	} else
		waveOutClose(hWave);

/*      If there is no device connected to the MIDI Mapper, then force the MIDI
	device to be the PC speaker.  Default is base-level synth.
*/
	if (midiOutOpen((LPHMIDIOUT)&hMidiOut, -1,
						(DWORD)NULL, (DWORD)NULL, (DWORD)NULL)) {
		MidiType = PCSPEAKER;
		if (CONFIRMMIDI) {
			dlgResult = doDialog(DLG_CONFIRMMIDI);
			if (nCheck)
				WritePrivateProfileString ((LPSTR)szInfo, "confirmmidi", NULL,
													(LPSTR)szIniFile);
			if (!dlgResult)
				exit(1);
		}
	} else
		midiOutClose(hMidiOut);

#ifdef JUNK
/* Check for Redbook audio driver */
// this is causing a protection fault in mmsystem.dll.  Will investigate
// later.  TM 9/27/93

	if (CONFIRMREDBOOK) {
		mciOpenParms.wDeviceID = NULL;
		*(DWORD*)&mciOpenParms.lpstrDeviceType = (LPSTR)AllocAlias16(MCI_DEVTYPE_CD_AUDIO);
		if (mciSendCommand(NULL, MCI_OPEN,
										MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID,
										(DWORD)(LPVOID)&mciOpenParms))
			{
			dlgResult = doDialog(DLG_CONFIRMREDBK);
			if (nCheck)
				WritePrivateProfileString ((LPSTR)szInfo, "confirmredbook", NULL,
													(LPSTR)szIniFile);
			if (!dlgResult)
				exit(1);
		}
		mciSendCommand(mciOpenParms.wDeviceID, MCI_CLOSE,
											MCI_WAIT,
											(DWORD)(LPVOID)&mciGenericParms);
      FreeAlias16((DWORD)mciOpenParms.lpstrDeviceType);
	}
#endif
}


void
MBox (char* text, char* title)
{
//	if (COLORCURSOR)
//		ShowCursor(1);
	OemToAnsiBuff((LPCSTR)text, (LPSTR)text, lstrlen((LPCSTR)text));
	MessageBox (hMyWnd, (LPSTR)text, (LPSTR)title, MB_OK | MB_TASKMODAL);
//	if (COLORCURSOR)
//		ShowCursor(0);
}

Bool
MBox2 (char* text, char* title)
{
	OemToAnsiBuff((LPCSTR)text, (LPSTR)text, lstrlen((LPCSTR)text));
	return (MessageBox (hMyWnd, (LPSTR)text, (LPSTR)title, MB_YESNO | MB_TASKMODAL) == IDNO);
}


void SciDisplay (char* szFormat,int xOrig,int yOrig,int width,int height,int history)
/**********************************************************************
	Display debug messages in a scroll box.
***********************************************************************/
{
static int nIndex = 0;
static int linesStored = 50;

	if (!monoDisp) {
		if (!hWndList) {
			hWndList = CreateWindow ("Listbox", NULL,
							WS_OVERLAPPED | WS_VISIBLE | WS_BORDER | WS_VSCROLL
							 | WS_THICKFRAME | WS_CAPTION | DS_NOIDLEMSG,
							xOrig, yOrig, width, height,
							hMyWnd, NULL,
							MyInstance, NULL);
			SetFocus(hMyWnd);
			linesStored = history;
		}

		if (nIndex > linesStored-1)
			SendMessage (hWndList, LB_DELETESTRING, nIndex - linesStored,(DWORD) NULL);
		char *cp;
		while ((cp = strchr(szFormat,'\n')) != NULL)
			*cp = ' ';
		nIndex = (int)SendMessage (hWndList, LB_ADDSTRING, NULL, (DWORD)((LPSTR) szFormat));
		SendMessage (hWndList, LB_SETCURSEL, nIndex, (DWORD) NULL);
		return;
	}

	if (monoDisp > 2)
		return;
	OutputDebugString(szFormat);
	if (monoDisp == 2)
		OutputDebugString("\n");
}
//#endif

#ifdef WIN32S
LRESULT  CALLBACK _export WndProc( HWND hWnd, UINT iMessage, WPARAM wParam,
                                 LPARAM lParam )
{
    // Pointer to the (C++ object that is the) window.
	 Window *pWindow = (Window *)GetWindowLong(hWnd,0);

    // The pointer pWindow will have an invalid value if the WM_CREATE
    // message has not yet been processed (we respond to the WM_CREATE
    // message by setting the extra bytes to be a pointer to the
    // (C++) object corresponding to the Window identified
    // by hWnd).  The messages that
    // precede WM_CREATE must be processed without using pWindow so we
    // pass them to DefWindowProc.
    // How do we know in general if the pointer pWindow is invalid?
    // Simple: Windows allocates the window extra bytes using LocalAlloc
    // which zero initializes memory; thus, pWindow will have a value of
    // zero before we set the window extra bytes to the 'this' pointer.
    // Caveat emptor: the fact that LocalAlloc will zero initialize the
    // window extra bytes is not documented; therefore, it could change
    // in the future.

    if ( pWindow == 0 )
    {
        if ( iMessage == WM_CREATE )
        {
            LPCREATESTRUCT lpcs;

            lpcs = (LPCREATESTRUCT) lParam;
            pWindow = (Window *) lpcs->lpCreateParams;

            // Store a pointer to this object in the window's extra bytes;
            // this will enable us to access this object (and its member
            // functions) in WndProc where we are
            // given only a handle to identify the window.

			   SetWindowLong(hWnd, 0, (LONG)pWindow);

            // Now let the object perform whatever
            // initialization it needs for WM_CREATE in its own
            // WndProc.
      return pWindow->WndProc( iMessage, wParam, lParam );
        }
        else
            return DefWindowProc( hWnd, iMessage, wParam, lParam );
    }
    else
        return pWindow->WndProc( iMessage, wParam, lParam );
}


LRESULT MainWindow::WndProc( UINT iMessage, WPARAM wParam, LPARAM lParam )

#else				//WIN32S

long FAR PASCAL
WndProc (HWND hWnd, unsigned iMessage, WORD wParam, LONG lParam)

#endif			//WIN32S
/*************************************************************************
	Normal Windows message processing procedure.
**************************************************************************/
{
PAINTSTRUCT  ps;
SOL_Point       mPoint;
RECT                    cliRect;
struct {
	ushort  count;
	ushort  func;
} static audArgs;

	if (GAMEPAUSED) {
		if ((iMessage != WM_SETFOCUS) &&
			 (iMessage != WM_SYSCOMMAND) &&
			 (iMessage != WM_QUERYENDSESSION) &&
			 (iMessage != WM_PAINT) &&
			 (iMessage != WM_DESTROY))
			return DefWindowProc (hWnd, iMessage, wParam, lParam) ;
	}

	switch (iMessage)
		{
		case WM_CHAR:                                   /* standard character entered */
		case WM_KEYDOWN:                                /* for non-standard characters */
		case WM_SYSKEYDOWN:                     /* ALT key combinations */
		case WM_SYSKEYUP:
		case WM_SYSCHAR:                                /* ALT character entered */
			if (pm.game)
				if (!((KbdMgrWin*)kbdMgr)->HandleKey(iMessage, wParam, lParam))
					return DefWindowProc (hWnd, iMessage, wParam, lParam) ;
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			if (pm.game && haveFocus) {
				mPoint.x = LOWORD(lParam);
				mPoint.y = HIWORD(lParam);
				if (COLORCURSOR) {
					((MouseWinClr*)mouse)->WinMouseButton(iMessage, mPoint);
				} else
					((MouseWin*)mouse)->WinMouseButton(iMessage, mPoint);
			} else
				return DefWindowProc (hWnd, iMessage, wParam, lParam) ;
			break;

		case WM_MOUSEMOVE:
			if (pm.game && haveFocus) {
				mPoint.x = (short)LOWORD(lParam);
				mPoint.y = (short)HIWORD(lParam);
				mouse->SetPos(mPoint);
			} else
				return DefWindowProc (hWnd, iMessage, wParam, lParam) ;
			break;

#ifdef XWIN32S
		case WM_PALETTECHANGED:
			// This was added because when the application called WinHelp,
			// the user minimized the help screen, and the application then
			// called WinHelp quit,  the game's palette got messed up and was
			// only corrected by losing and regaining the focus.  This was
			// all under WIN32S.
			if (WING) {
				hOldPal = SelectPalette (hMyDC, hPal, 0);
				RealizePalette (hMyDC);
			}
			break;
#endif

		case WM_SETFOCUS:
			haveFocus = TRUE;
			// SetCapture here was causing a problem if an icon was dragged
			// into the game window, it would become the cursor.  Removing
			// SetCapture caused no ill effects.  TM 6/12/95	
//			SetCapture(hMyWnd);
			if (pm.game)
				{
				/* Microsoft says to realize the palette in response to
					WM_QUERYNEWPALETTE, but this doesn't work when the game is
					reduced to an icon and then brought back, so I moved it here.
				*/
				hOldPal = SelectPalette (hMyDC, hPal, 0);
				RealizePalette (hMyDC);

				((TimeMgrWin*)timeMgr)->EnableTimer();
				GAMEPAUSED = FALSE;
				OpenSnd ();
				SuspendSounds(FALSE);
				PauseSnd ((ObjectID)0, FALSE);
				}
			break;

		case WM_KILLFOCUS:
			haveFocus = FALSE;
			ReleaseCapture();
			if (pm.game) {
				if (vmdStatus != statusNotOpen) {
					waitVMDStop = TRUE;
					vmdKillFocus = TRUE;
				} else {
					GAMEPAUSED = TRUE;
					firstPause = TRUE;
					if (audioInstalled) {
						audArgs.count = 1;
						audArgs.func = STOP;
						KDoAudio((kArgs) &audArgs);
						closeWaveDev();
					}
				}
				PauseSnd ((ObjectID)0, TRUE);
				SuspendSounds(TRUE);
				CloseSnd ();
//				((TimeMgrWin*)timeMgr)->DisableTimer();
			}
			break;

		case WM_SYSCOMMAND:
			switch (wParam)
				{
				case SC_MINIMIZE:
					if (isMaximized)
						minMaxFlag = True;
					break;

				case SC_MAXIMIZE:
				case 0xF032:		//double-click on title bar
					gameBorder = maximizedBorder;
               isMaximized = True;
					break;

				case SC_RESTORE:
				case 0xF122:		//double-click on title bar
					if (!minMaxFlag) {
						gameBorder = borderPixels;
	               isMaximized = False;
					} else
						minMaxFlag = False;
					break;

				case SC_CLOSE:
					if (vmdStatus == statusPlaying) {
						((KbdMgrWin*)kbdMgr)->HandleKey(WM_CHAR, VK_ESCAPE, 65537);
						waitVMDStop = TRUE;
						waitClose = TRUE;
						return 0L;
					}
					if (TRAPCLOSE) {
						// set global to allow the game to do any saves and then
						// do its own terminate.
						pm.Global(g_quit) = 2;
						// if minimized, restore window to get close message
						if (IsIconic(hMyWnd))
							ShowWindow (hMyWnd, SW_RESTORE) ;
						return 0L;
					} else
						break;

				case SC_SCREENSAVE:
					return 0L;
				}
			return DefWindowProc (hWnd, iMessage, wParam, lParam) ;

		case MM_MCINOTIFY:
			AviNotify(wParam,lParam);
			break;

		case WM_PAINT:                                  /* re-paint client area */
			hPaintDC = BeginPaint (hWnd, &ps);
			if (pm.game) {
				if (!haveFocus) {
					graphMgr->Repaint(SOL_Rect(0, 0, gameWidth-1, gameHeight-1));
					break;
				}
				if (vmdStatus == statusPlaying)
					vmdShowFullFrame = TRUE;
				// check if dimensions changed
				GetClientRect(hWnd, &cliRect);
				cliRect.right -= (2 * gameBorder);
				cliRect.bottom -= (2 * gameBorder);
				if (ALLOWRESIZE &&
				   ((cliRect.right != gameWidth) || (cliRect.bottom != gameHeight))) {
					// re-size with proper aspect ratio
					if (cliRect.right != gameWidth)
						gameWidth = cliRect.right;
					else
						gameWidth = (ratioX * cliRect.bottom) / ratioY;
					gameWidth = Min(Max(gameWidth,SCIRESX),vmapWidth);
					if (isMaximized) {
						gameWidth = vmapWidth;
					}
					gameHeight = (ratioY * gameWidth) / ratioX;
					gameHeight = Min(gameHeight,vmapHeight);
					// compute new client width, height
					cliRect.right = gameWidth + (2 * gameBorder);
					cliRect.bottom = gameHeight + (2 * gameBorder);
					if (!isMaximized) {
						AdjustWindowRect(&cliRect, myStyle, FALSE);
						SetWindowPos(hMyWnd, (HWND)NULL,
											0, 0,
											cliRect.right - cliRect.left,
											cliRect.bottom - cliRect.top,
											SWP_NOZORDER | SWP_NOMOVE); 
					}
					graphMgr->NewScreenDim(gameWidth, gameHeight);
					graphMgr->FrameOut();
				} else {
					// don't allow any size change
					cliRect.right = gameWidth + (2 * gameBorder);
					cliRect.bottom = gameHeight + (2 * gameBorder);
					if (!isMaximized) {
						AdjustWindowRect(&cliRect, myStyle, FALSE);
						SetWindowPos(hMyWnd, (HWND)NULL,
											0, 0,
											cliRect.right - cliRect.left,
											cliRect.bottom - cliRect.top,
											SWP_NOZORDER | SWP_NOMOVE); 
					}
					// nothing changed; just do a re-paint
					graphMgr->Repaint(SOL_Rect(0, 0, gameWidth-1, gameHeight-1));
				}
			}
			EndPaint (hWnd, &ps);
			hPaintDC = (HDC) NULL;
			if (pm.game && haveFocus && COLORCURSOR)
				((MouseWinClr*)mouse)->SetPaintFlag();
			break;

		case WM_QUERYENDSESSION:
  			if (TRAPCLOSE) {
				if (MessageBox(NULL,"The game has not been saved.  Exit the game anyway?",szTitle,MB_YESNO | MB_ICONEXCLAMATION)
					== IDYES)  
					return 1L;
				else
					return 0L;
			} else
				return 0L;
//			sprintf(mbuff, "Please quit %s before terminating Windows", szTitle);
//			MBox(mbuff,"");
//			DestroyWindow(hMyWnd);
//			return 0L;
//			break;

		case WM_DESTROY:		/* terminate (ALT F4) */
/*      Do exit instead of PostQuitMessage to avoid having to ReleaseDC */
			exit (0);

		default:
			return DefWindowProc (hWnd, iMessage, wParam, lParam) ;
		}
		return 0L ;
}



BOOL doDialog(int dlgId)
{
FARPROC fpProc;
int nResult;

   fpProc = MakeProcInstance((FARPROC)testDlgProc, MyInstance);
   nResult = DialogBox(MyInstance, MAKEINTRESOURCE(dlgId), NULL, fpProc);
   FreeProcInstance(fpProc);
	if (nResult == IDCANCEL)
		return(FALSE);
	else
		return(TRUE);
}



BOOL FAR PASCAL _export testDlgProc (HWND hDlg, WORD wMsg, WORD wParam, LONG lParam) {

   switch (wMsg) {
      case WM_INITDIALOG:
			if (lParam);            //to eliminate W3 warning
			return TRUE;

      case WM_COMMAND:
	 switch (wParam) {

	    case IDOK:

	    case IDCANCEL:
					nCheck = (WORD)SendMessage(GetDlgItem(hDlg,DLG_CHECKBOX), BM_GETCHECK, 0, 0L);
	       EndDialog(hDlg, wParam);
	       break;
	 }
	 break;

      default:
			return(FALSE);
   }
   return(FALSE);
}


