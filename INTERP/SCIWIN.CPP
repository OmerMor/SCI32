/* SCIWIN.C -- Windows version of SCI */

#define INCLUDE_MMSYSTEM_H
#include "sciwin.hpp"
#include "sol.hpp"
#ifdef WIN32S
#include "winclass.hpp"
#include "wingfunc.hpp"
#endif
#include "scidlg.hpp"
#include <stdlib.h>
#include <stdio.h>
//#include "sol.hpp"
#include "event.hpp"
#include "audio.hpp"
#include "graphmw.hpp"
#include "kernel.hpp"
#include "scidll.hpp"
#include "mousew.hpp"
#include "moview.hpp"
#include "sci.hpp"
#include "kbdmgrw.hpp"
#include "pmachine.hpp"
#include "sound.hpp"
#include "soundwin.hpp"
#include "timew.hpp"

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
void                    GetScreenInfo (void);
void                    ReadCommandLine (LPSTR);
void                    MBox (char *, char *);
void                    StartTimer (void);
void                    StopTimer (void);
void                    CheckConfig (void);
void SciDisplay (char *);

void	closeWaveDev(void);
void	AudioUpdate(void);
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
int             nHORZRES, nVERTRES, nCOLORS, nBITSPIXEL, nPLANES, nRASTERCAPS;
int		forcePPC = 0;
int		asyncCheckFreq = 10;
MSG myMsg;
BOOL	  SHOWMEM   = FALSE;
BOOL    LOCKMAIN   = FALSE;
BOOL    LOCKAUDIO  = FALSE;
BOOL    GAMEPAUSED = FALSE;
BOOL    REQUIREDAC       = FALSE;       /* require user to have a DAC */
BOOL    CONFIRMDAC       = FALSE;       /* confirm from user if no DAC */
BOOL    CONFIRMMIDI  = FALSE;   /* confirm from user if no MIDI */
BOOL    CONFIRMREDBOOK = FALSE; /* confirm from user if no Redbook */
BOOL    USEAUDIO = TRUE;
BOOL    USEMIDI = TRUE;
BOOL    DOPPCTEST = TRUE;
BOOL    HIRES = TRUE;
BOOL	  TITLEBAR = TRUE;
BOOL	  ALLOWRESIZE = TRUE;
BOOL	  TRUECOLOR = FALSE;
BOOL	  TRAPCLOSE = TRUE;
#ifdef WIN32S
Bool	  WING = TRUE;
#endif

/* Windows sound devices */
#define PCSPEAKER       0
#define BASESYNTH       1
#define HIGHSYNTH       2
#define BASEADLIB       3
extern  int     MidiType;

static char		szIniFile[50] = "resource.win";
static char		szTitle[30] = "Sierra On-Line";
static char		szInfo[] = "Windows";
static char		szAppName [] = "SciWin" ;
char				szWinPatchDir[50] = "";
int     gameWidth = 320;
int     gameHeight = 200;
int     gameBorder = 0;
int     borderPixels = 5;
int     maximizedBorder = 0;
int     clientWidth, clientHeight;
int     forceWidth = 0;
int     forceHeight = 0;
int     ratioX, ratioY;
int     reserveK = 0;
int     sciMinK = 1500;
int     sciMaxK = 2000;
int     vmapWidth = 640;
int     vmapHeight = 480;
int	  DACSize = 10240;
#ifndef COLORCUR
int	  cursorViewAdd = 0;
#else
int	  maxCursorSize = 600;
#endif
Bool    isMaximized = False;
Bool    minMaxFlag = False;
extern	int	writeCount;


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

#ifndef COLORCUR
		hAltCur = LoadCursor(NULL, IDC_WAIT);
#else
		hAltCur = LoadCursor(NULL, IDC_ARROW);
#endif
		hCurCur = hAltCur;
		SetCursor(hCurCur);

		if (!RegisterClass (&wndclass))
			return FALSE ;

	} else {
		MessageBox (GetFocus(), "Cannot run two copies of game!",
						"Sierra", MB_OK);
		return msg.wParam ;                                     /* return to Windows */
	}


	ReadCommandLine (lpszCmdLine);  /* get command line arguments */

	GetProfileInfo();

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

	if (windWidth > nHORZRES)
		posX = 0;
	else
		posX = (nHORZRES - windWidth) / 2;

	if (windHeight > nVERTRES)
		posY = 0;
	else
		posY = (nVERTRES - windHeight) / 2;

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


	if (nHORZRES > windWidth) {
		howShow = SW_SHOWNORMAL;
      isMaximized = False;
	} else {
		howShow = SW_SHOWMAXIMIZED;
		gameBorder = 0;
      isMaximized = True;
	}

	ShowWindow (hMyWnd, howShow) ;
	UpdateWindow (hMyWnd) ;

	hMyDC = GetDC (hMyWnd);                 /* leave open for duration of execution */

	CheckConfig();

	scimain (argc, argv);                   /* call sci.c to start intrpreter */

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
	if (!*timeToCheck)
		return;
	*timeToCheck = 0;

	AudioServer();
	if (!USEAUDIO)
		if (writeCount)
			AudioUpdate();
	while (PeekMessage (&myMsg, NULL, 0, 0, PM_REMOVE))
		{
		if (myMsg.message == WM_QUIT) {
			MBox("WM_QUIT","");
			exit (1);
		}
		TranslateMessage (&myMsg) ;
		DispatchMessage (&myMsg) ;
		}
		while (GAMEPAUSED)
			{
			GetMessage (&myMsg, NULL, 0, 0);
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
			strcpy(szIniFile, argv[i]);
		}
	}
}

void GetProfileInfo (void)
{
char            buff[80];

	if (!GetPrivateProfileString ((LPSTR)szInfo, "title", NULL, szTitle,
											sizeof(szTitle), (LPCSTR)szIniFile))
		MBox ("Windows title not found","");

	GetPrivateProfileString ((LPSTR)szInfo, "winPatchDir", NULL, szWinPatchDir,
											sizeof(szWinPatchDir), (LPCSTR)szIniFile);

	GetPrivateProfileString ((LPSTR)szInfo, "synthtype", NULL, buff,
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
	movieType		= GetPrivateProfileInt ((LPSTR)szInfo, "movieplayer", 0, (LPSTR)szIniFile);
	borderPixels	= GetPrivateProfileInt ((LPSTR)szInfo, "borderpixels", 5, (LPSTR)szIniFile);
	gameBorder = borderPixels;
	reserveK			= GetPrivateProfileInt ((LPSTR)szInfo, "reservek", 0, (LPSTR)szIniFile);
	sciMinK			= GetPrivateProfileInt ((LPSTR)szInfo, "scimink", 1500, (LPSTR)szIniFile);
	sciMaxK			= GetPrivateProfileInt ((LPSTR)szInfo, "scimaxk", 2000, (LPSTR)szIniFile);
#ifndef COLORCUR
	cursorViewAdd	= GetPrivateProfileInt ((LPSTR)szInfo, "cursorviewadd", 0, (LPSTR)szIniFile);
#else
	maxCursorSize	= GetPrivateProfileInt ((LPSTR)szInfo, "maxCursorSize", 600, (LPSTR)szIniFile);
	maxCursorSize = (((maxCursorSize + 3) >> 2) << 2);
#endif
	forcePPC			= GetPrivateProfileInt ((LPSTR)szInfo, "forceppc", 0, (LPSTR)szIniFile);
	DACSize			= GetPrivateProfileInt ((LPSTR)szInfo, "DACSize", GetDACDefaultSize(), (LPSTR)szIniFile);
   DACSize = ((DACSize + 1023) / 1024) * 1024;
	asyncCheckFreq	= GetPrivateProfileInt ((LPSTR)szInfo, "asynccheckfreq", 10, (LPSTR)szIniFile);

	CONFIRMDAC		= GetTFOption("confirmdac",FALSE);
	REQUIREDAC		= GetTFOption("requiredac",FALSE);
	CONFIRMMIDI		= GetTFOption("confirmmidi",FALSE);
	USEMIDI			= GetTFOption("usemidi",TRUE);
	USEAUDIO			= GetTFOption("useaudio",TRUE);
	CONFIRMREDBOOK	= GetTFOption("confirmredbook",FALSE);
	TITLEBAR			= GetTFOption("titlebar",TRUE);
	LOCKMAIN			= GetTFOption("lockmain",FALSE);
	LOCKAUDIO		= GetTFOption("lockaudio",FALSE);
	SHOWMEM			= GetTFOption("showmem",FALSE);
	HIRES				= GetTFOption("hires",TRUE);
	DOPPCTEST		= GetTFOption("doppctest",TRUE);
	ALLOWRESIZE		= GetTFOption("allowresize",TRUE);
	TRAPCLOSE		= GetTFOption("trapclose",TRUE);
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
	nPLANES = GetDeviceCaps (hMyDC, PLANES);
	nCOLORS = 1 << (nPLANES * nBITSPIXEL);
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
	else
		maximizedBorder = 0;

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

	clientWidth = gameWidth + (2 * gameBorder);
	clientHeight = gameHeight + (2 * gameBorder);
}

void
CheckConfig (void)
{
PCMWAVEFORMAT   pwf;
HMIDIOUT                        hMidiOut;
//MCI_OPEN_PARMS        mciOpenParms;
//MCI_GENERIC_PARMS     mciGenericParms;
WORD                            wError;
BOOL                            dlgResult;

	if (!(nRASTERCAPS & RC_PALETTE))
		TRUECOLOR = TRUE;
	else
		TRUECOLOR = FALSE;
	if (nCOLORS < 256)
		{
		GetPrivateProfileString ("boot", "display.drv", "none", mbuff, 40,
									"system.ini");
		MessageBox (NULL,
			"Your current display driver is not supported by this game."
			"  We require a driver which can display 256 colors or more."
			"  You can install a different driver by selecting the appropriate"
			" options from Windows Setup.",
				mbuff, MB_OK | MB_ICONSTOP);
		exit(1);
		}

	pwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pwf.wf.nChannels  = 1;
	pwf.wf.nSamplesPerSec = 11025;
	pwf.wf.nAvgBytesPerSec = 11025;
	pwf.wf.nBlockAlign = 1;
	pwf.wBitsPerSample = (WORD)8;

	wError = waveOutOpen (NULL,
								WAVE_MAPPER,
								(LPWAVEFORMAT)&pwf.wf,
								(DWORD)0,
								(DWORD)0,
								(DWORD)WAVE_FORMAT_QUERY);
	if (wError) {
		USEAUDIO = FALSE;
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
	}

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

#ifdef WIN32S
	if (WING) {
		if (!WingInit(640,480)) {
			WING = FALSE;
		}
	}
	if (SHOWMEM) {
		if (WING)
			MBox("Using WinG","");
		else
			MBox("Not using WinG","");
	}
#endif
}


void
MBox (char* text, char* title)
{
#ifdef COLORCUR
	ShowCursor(1);
#endif
	OemToAnsiBuff((LPCSTR)text, (LPSTR)text, lstrlen((LPCSTR)text));
	MessageBox (hMyWnd, (LPSTR)text, (LPSTR)title, MB_OK | MB_TASKMODAL);
#ifdef COLORCUR
	ShowCursor(0);
#endif
}


//#ifdef DEBUG
void SciDisplay (char* szFormat)
/**********************************************************************
	Display debug messages in a scroll box.
***********************************************************************/
{
static int nIndex = 0;

	if (!hWndList) {
		hWndList = CreateWindow ("Listbox", NULL,
						WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL
						 | WS_THICKFRAME | WS_CAPTION | DS_NOIDLEMSG,
						490, 240, 140, 200,
//	before WIN32S	hMyWnd, 2,
//						GetWindowWord (hMyWnd, GWW_HINSTANCE), NULL);
						hMyWnd, NULL,
						MyInstance, NULL);
	}
	if (nIndex > 49)
		SendMessage (hWndList, LB_DELETESTRING, nIndex - 50, (DWORD) NULL);
	nIndex = (int)SendMessage (hWndList, LB_ADDSTRING, NULL, (DWORD)((LPSTR) szFormat));
	SendMessage (hWndList, LB_SETCURSEL, nIndex, (DWORD) NULL);
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

	switch (iMessage)
		{
		case MM_WOM_DONE:
			AudioUpdate();
			break;

		case WM_CHAR:                                   /* standard character entered */
		case WM_KEYDOWN:                                /* for non-standard characters */
		case WM_SYSKEYDOWN:                     /* ALT key combinations */
		case WM_SYSCHAR:                                /* ALT character entered */
			if (pm.game)
				if (!((KbdMgrWin*)kbdMgr)->HandleKey(iMessage, wParam, lParam))
					return DefWindowProc (hWnd, iMessage, wParam, lParam) ;
			break;

		case WM_LBUTTONDOWN:                    /* mouse button events */
		case WM_LBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			if (pm.game) {
				mPoint.x = LOWORD(lParam);
				mPoint.y = HIWORD(lParam);
				((MouseWin*)mouse)->WinMouseButton(iMessage, mPoint);
			}
			break;

		case WM_MOUSEMOVE:                      /* mouse movement event */
			if (pm.game) {
				mPoint.x = LOWORD(lParam);
				mPoint.y = HIWORD(lParam);
				mouse->SetPos(mPoint);
			}
			break;

#ifdef WIN32S
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
			if (pm.game) {
					if (audioInstalled) {
						audArgs.count = 1;
						audArgs.func = STOP;
						KDoAudio((kArgs) &audArgs);
						closeWaveDev();
//						CallDAC(DAC_TERMINATE,0);
					}
				PauseSnd ((ObjectID)0, TRUE);
				SuspendSounds(TRUE);
				CloseSnd ();
				GAMEPAUSED = TRUE;
				((TimeMgrWin*)timeMgr)->DisableTimer();
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
					gameBorder = maximizedBorder;
               isMaximized = True;
					break;

				case SC_RESTORE:
//					MBox("rest","");
					if (!minMaxFlag) {
						gameBorder = borderPixels;
	               isMaximized = False;
					} else
						minMaxFlag = False;
					break;

				case SC_CLOSE:
					if (TRAPCLOSE) {
						// set global to allow the game to do any saves and then
						// do its own terminate.
						pm.Global(g_quit) = 2;
						return 0L;
					} else
						break;
				}
			return DefWindowProc (hWnd, iMessage, wParam, lParam) ;

		case WM_PAINT:                                  /* re-paint client area */
			hPaintDC = BeginPaint (hWnd, &ps);
			if (pm.game) {
//				if (isMaximized)
//					MBox("paint Max","");
//				else
//					MBox("paint not Max","");
				// check if dimensions changed
				GetClientRect(hWnd, &cliRect);
				cliRect.right -= (2 * gameBorder);
				cliRect.bottom -= (2 * gameBorder);
//				if (!isMaximized && ((cliRect.right != gameWidth) || (cliRect.bottom != gameHeight))) {
				if (ALLOWRESIZE &&
				   ((cliRect.right != gameWidth) || (cliRect.bottom != gameHeight))) {
					// re-size with proper aspect ratio
					if (cliRect.right != gameWidth)
						gameWidth = cliRect.right;
					else
						gameWidth = (ratioX * cliRect.bottom) / ratioY;
					gameWidth = Min(Max(gameWidth,LOWRESX),vmapWidth);
					if (isMaximized) {
//						sprintf(mbuff,"Max width:%d",gameWidth);
//						MBox(mbuff,"");
						gameWidth = vmapWidth;
					}
					gameHeight = (ratioY * gameWidth) / ratioX;
					// compute new client width, height
					cliRect.right = gameWidth + (2 * gameBorder);
					cliRect.bottom = gameHeight + (2 * gameBorder);
					if (!isMaximized) {
						AdjustWindowRect(&cliRect, myStyle, FALSE);
					}
					if (!isMaximized) {
						SetWindowPos(hMyWnd, (HWND)NULL,
											0, 0,
											cliRect.right - cliRect.left,
											cliRect.bottom - cliRect.top,
											SWP_NOZORDER | SWP_NOMOVE); 
					}
					graphMgr->NewScreenDim(gameWidth, gameHeight);
					graphMgr->FrameOut();
				} else {
					// nothing changed; just do a re-paint
						graphMgr->Repaint(SOL_Rect(0, 0, gameWidth-1, gameHeight-1));
				}
			}
			EndPaint (hWnd, &ps);
			hPaintDC = (HDC) NULL;
			break;

		case WM_QUERYENDSESSION:
			sprintf(mbuff, "Please quit %s before terminating Windows", szTitle);
			MBox(mbuff,"");
//			DestroyWindow(hMyWnd);
			return 0L;

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