//	msg.hpp

#ifndef MSG_HPP
#define MSG_HPP

#include <stdarg.h>

#ifndef DOS_HPP
#include	"dos.hpp"
#endif

#ifndef SAVEABLE_HPP
#include	"saveable.hpp"
#endif

// HOOK
// Commented out messages are not found in any *.?pp file.
// These message numbers may be reused

// Messages 1-24 are pre-loaded into buffer for use during critical errors
const Msg_CriticalErr				= 1;
const Msg_Cancel						= 2;
const Msg_Quit							= 3;

//	messages 4-16 are for each kind of critical error
const Msg_DiskErrors					= 4;

const Msg_LastPreload				= 24;

const Msg_Sorry						= 25;
//const Msg_Oops							= 26;
//const Msg_NoDebug						= 27;
const Msg_NoAudioDrv					= 28;
const Msg_NoAudio						= 29;
//const Msg_CantFind					= 30;
const Msg_NoMusic						= 31;
const Msg_MaxServers					= 32;
const Msg_UnknownLanguage			= 33;
const Msg_Message						= 34;
const Msg_Bresen						= 35;
const Msg_DisposedScript			= 36;
const Msg_BadMsgVersion				= 37;
const Msg_MsgStackOverflow			= 38;
const Msg_MsgStackStackOverflow	= 39;
const Msg_MsgStackStackUnderflow	= 40;
const Msg_InvalidProperty			= 41;
const Msg_VerStampMismatch			= 42;
const Msg_BadDispatch				= 43;
const Msg_BadOpcode					= 44;
const Msg_BadKernel					= 45;
const Msg_LoadClass					= 46;
const Msg_NotObject					= 47;
const Msg_BadSelector				= 48;
const Msg_ZeroDivide					= 49;
const Msg_StackBlown					= 50;
const Msg_ZeroModulo					= 51;
const Msg_LeftClone					= 52;
const Msg_NoSoundDrv					= 53;
const Msg_CantFindSI				   = 54;
//const Msg_ScaleExceedMax			= 55;
//const Msg_InvalidView				= 56;
//const Msg_LoopTooLarge				= 57;
//const Msg_CelTooLarge				= 58;
const Msg_BadCacheIndex				= 59;
const Msg_CelTooLargeForPic		= 60;
const Msg_VariableAddress			= 61;
const Msg_FixupError					= 62;
const Msg_PalMemMoved				= 63;
const Msg_WindowTextTooBig			= 64;
const Msg_WindowTitleTooBig		= 65;
const Msg_WindowSizeTooSmall		= 66;
const Msg_BadArrayType				= 67;
const Msg_ArrayTooBig				= 68;
const Msg_RectListNoRoom			= 69;
const Msg_NoInitMemory				= 70;
const Msg_NoHandles					= 71;
//const Msg_BadHandleSignature		= 72;
//const Msg_MemError					= 73;
const Msg_MemInitError				= 74;
//const Msg_NoMemory					= 75;
const Msg_InvalidHandle				= 76;
//const Msg_PackCantLocate			= 77;
//const Msg_PackError					= 78;
//const Msg_MemChecksumInvalid		= 79;
//const Msg_MemFreeNotZero			= 80;
//const Msg_NoMonoMem					= 81;
//const Msg_ViewDoesntContainLoop	= 82;
//const Msg_ViewDoesntContainLoopCel=83;
const Msg_KStrStripTSNArgs			= 84;
const Msg_KStrStripTSNShowArg		= 85;
const Msg_KStrStripTSNUnk			= 86;
const Msg_DonePainting				= 87;
const Msg_CursorUnHide				= 88;
const Msg_StackUnderflow			= 89;
//const Msg_ConfigFileName			= 90;
//const Msg_InvalidFlag				= 91;
//const Msg_ConfigFileOpen			= 92;
const Msg_NoWindowTitle				= 93;
const Msg_TooManyScreenItems		= 94;
const Msg_InvalidScreenItemIndex	= 95;
const Msg_TooManyPlanes				= 96;
const Msg_InvalidPlaneIndex		= 97;
const Msg_InvalidResType			= 98;
const Msg_ResLoadErr					= 99;
const Msg_ResFreeErr					= 100;
//const Msg_ResVolOpenErr				= 101;
//const Msg_ResMapOpenErr				= 102;
//const Msg_CompressedVol				= 103;
const Msg_InvalidResFileHandle	= 104;
const Msg_WrongResType				= 105;
const Msg_NoMouseDriver				= 106;
const Msg_PlaneNotFound				= 107;
const Msg_InvalidPlane				= 108;
const Msg_PrevPlaneNotFound		= 109;
//const Msg_AddScreenItem				= 110;
const Msg_ScreenItemNotExist		= 111;
const Msg_CantFindPlaneID			= 112;
const Msg_InvalidMemInfoCall		= 113;
const Msg_NotAPolygon            = 114;
const Msg_TooManyPolygons		   = 115;
const Msg_TooManyPointsInPath	   = 116;
const Msg_GetPathInternalError   = 117;
const Msg_MergedPolygonTooLarge  = 118;
const Msg_PolygonTooManyPatches  = 119;
const Msg_PolygonTooManyPoints   = 120;
const Msg_SavingHandleWithClient	= 121;
const Msg_SavingLockedHandle		= 122;
const Msg_InvalidRestoreData   	= 123;
//const Msg_InternalErrorOnRestore	= 124;
const Msg_InvalidList				= 125;
const Msg_NoList						= 126;
//const Msg_DOSRepaint					= 127;
const Msg_ClassNumTooLarge			= 128;
const Msg_ShowStyleSpeed			= 129;
//const Msg_ShowStyleInProgress		= 130;
const Msg_InternalErrorOnLastLine= 131;
const Msg_ScrollAlreadyInProgress= 132;
const Msg_ScrollInXAndY				= 133;
const Msg_InvalidArrayElement 	= 134;
const Msg_DivideByZero				= 135;
const Msg_InvalidVideoMode			= 136;
const Msg_MovieError					= 137;
//const Msg_RemapError					= 138;
const Msg_GlobalPointer				= 139;
const Msg_CompressSource			= 140;
const Msg_CompressDestination		= 141;
const Msg_ResourceData				= 142;
const Msg_AVIOpen = 143;
const Msg_AVIPut = 144;
const Msg_AVISetPalette = 145;
const Msg_AVIPlay = 146;

class MsgMgr : public Saveable {
public:
	enum {
		MaxStr			= 256,
		MaxFatalBuf		= 1000,
		MaxPreloadedBuf= 2000
	};

	MsgMgr();
	virtual void Init();
	
	void	AddFatalStr(char* str);

	//	Alerts are used to display text to a customer or programmer
	Bool	Alert(int errNum, ...);
	Bool 	Alert(char* fmt, ...);
	Bool	Alert(char, char* fileName, int lineNum, int errNum, ...);
	Bool	Alert(char, char* fileName, int lineNum, char* fmt, ...);

	//	Fatals are used to end the program--via the debugger in the debug
	//	version
	void	Fatal(int errNum, ...);
	void	Fatal(char* fmt, ...);
	void	Fatal(char, char* fileName, int lineNum, int errNum, ...);
	void	Fatal(char, char* fileName, int lineNum, char* fmt, ...);
	
	void	Dump(char* fmt, ...);

	char*	Get(int errNum, char *buf);

	void				Mono(char* fmt, ...);
	void				MonoArgs(char* fmt, va_list);
	void				MonoPause(char* fmt, ...);
	virtual void	MonoPosition(int row, int col) = 0;
	void				MonoStr(char* str);

	void	SaveRestore(SaveGame*, Bool save);

	void	SetSCIFatalStr(char* str);
	void	SetQuitStr(char* str);

protected:
	void	AddPMIP(char* msg);
	void	AddSrcLoc(char* msg, char* fileName, int lineNum);

	virtual Bool	AlertOutput(char* msg);
	virtual void 	FatalOutput(char* msg);

	char*	Make(int errNum, va_list);
	char*	Make(char* fmt, va_list);
	char*	Make(char* fileName, int lineNum, int errNum, va_list);
	char*	Make(char* fileName, int lineNum, char* fmt, va_list);

	virtual void	MonoOutput(char* str) = 0;
	
	int	OpenFile();
	char*	Read(int errNum, char* buf, int fd);

	char	buf[MaxStr + 1];
	char	fatalStrs[MaxFatalBuf + 1];
	char*	fileName;
	char	fullFileName[MaxPath + 1];
	char	preloadedMsgs[MaxPreloadedBuf + 1];
	char	quitStr[MaxStr + 1];
	char	sciFatalStr[MaxStr + 1];

} extern * msgMgr;

//	use SrcLoc as the first argument to Fatal to display the source location
//	in the error message
#define SrcLoc	'a', __FILE__, __LINE__

#endif


