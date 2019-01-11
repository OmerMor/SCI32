//	winhelp.cpp

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


#include "sciwin.hpp"
#include "sol.hpp"

#include	"kernel.hpp"
#include "textid.hpp"

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
