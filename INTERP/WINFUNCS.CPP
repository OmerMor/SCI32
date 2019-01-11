

#include "sciwin.hpp"
#include "sol.hpp"

#include	"kernel.hpp"
#include "pmachine.hpp"
#include "textid.hpp"



//***************************************************************************
// Windows HELP kernel call.
//
// The first parameter specifies the type of help requested and corresponds
// to the values passed to the Windows WinHelp function as follows:
//
//		HELP_CONTEXT				(val 1)
//		HELP_CONTENTS				(val 3)
//		HELP_CONTEXTPOPUP			(val 8)
//		HELP_SETWINPOS				(val 203)
//		HELP_QUIT					(val 2)
//
// The second parameter specifies the file name.
//
// Subfunctions HELP_CONTEXT and HELP_CONTEXTPOPUP require a third parameter
// which specifies the context number defined in the [MAP] section of the
// .HPJ file.
//
// Subfunction HELP_SETWINPOS requires the following additional parameters:
//		param 3 - left
//		param 4 - top
//		param 5 - width
//		param 6 - height
//		param 7 - maximize flag
//***************************************************************************

void
KWinHelp(argList)
{
LPCSTR		lpszHelpFile;
UINT			fuCommand;
DWORD			dwData;
HELPWININFO	hwi;

	lpszHelpFile = (LPCSTR)*((TextID) arg(2));
	fuCommand = (UINT)arg(1);

	switch (arg(1)) {

		case HELP_CONTEXT:
			dwData = (DWORD)arg(3);
			break;

		case HELP_CONTENTS:
			dwData = (DWORD)0;
			break;

		case HELP_CONTEXTPOPUP:
			dwData = (DWORD)arg(3);
			break;

		case HELP_SETWINPOS:
			hwi.wStructSize = sizeof(HELPWININFO);
			hwi.x =		(int)arg(3);
			hwi.y =		(int)arg(4);
			hwi.dx =		(int)arg(5);
			hwi.dy =		(int)arg(6);
			hwi.wMax =	(int)arg(7);
			// Note: Setting the following to a null string seems to have the effect
			// of changing the size of all help windows.
			hwi.rgchMember[0] = 0;
			dwData = (DWORD)((LPHELPWININFO)&hwi);
			break;

		case HELP_QUIT:
			dwData = (DWORD)0;
			break;

		default:
			fuCommand = 0;
			break;
	}

	if (fuCommand)
		WinHelp(hMyWnd, lpszHelpFile, fuCommand, dwData);
}

//***************************************************************************
// Windows MessageBox kernel call.
//
//		param 1 - message text
//		param 2 - message title
//		param 3 - messagebox style - may be any combination of the following:
//						(see Windows API for definitions)
//
//			MB_OK						0x0000
//			MB_OKCANCEL				0x0001
//			MB_ABORTRETRYIGNORE	0x0002
//			MB_YESNOCANCEL			0x0003
//			MB_YESNO					0x0004
//			MB_RETRYCANCEL			0x0005
//
//			MB_ICONHAND				0x0010
//			MB_ICONQUESTION		0x0020
//			MB_ICONEXCLAMATION	0x0030
//			MB_ICONASTERISK		0x0040
//
//			MB_ICONINFORMATION	MB_ICONASTERISK
//			MB_ICONSTOP				MB_ICONHAND
//
//			MB_DEFBUTTON1			0x0000
//			MB_DEFBUTTON2			0x0100
//			MB_DEFBUTTON3			0x0200
//  
//			MB_APPLMODAL			0x0000
//			MB_SYSTEMMODAL			0x1000
//			MB_TASKMODAL			0x2000
//														
//		Return:
//			acc will contain one of the following:
//
//			0 - not enough memory
//			1 - OK
//			2 - CANCEL
//			3 - ABORT
//			4 - RETRY
//			5 - IGNORE
//			6 - YES
//			7 - NO
//
//***************************************************************************
					
void
KMessageBox(argList)
{

	pm.acc = (Acc)MessageBox(hMyWnd,
									(LPCSTR)*((TextID)arg(1)),		// text
									(LPCSTR)*((TextID)arg(2)),		// title
									(UINT)arg(3));						// style
}
