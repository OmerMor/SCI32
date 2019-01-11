//	kernel.hpp
//		defines standard parameter macros and declares kernel routines

#ifndef KERNEL_HPP
#define KERNEL_HPP

#ifndef PMSTACK_HPP
#include	"pmstack.hpp"
#endif

typedef PMStack::Ptr	kArgs;

#define arg(n) 	args[n]
#define kArgc		arg(0)
#define argList	kArgs args
#define argCount	arg(0)

struct KernelSubFunc {
	char *name;
	int breakOnCall;
};

struct KernelCall {
	void	(*func)(argList);
	char *name;

	KernelSubFunc **subFuncs;
};

void	KLoad(argList);
void	KUnload(argList);
void	KScriptID(argList);
void	KDisposeScript(argList);
void	KLock(argList);
void	KResCheck(argList);
void	KPurge(argList);
void	KMarkMemory(argList);
void	KPreloadResource(argList);
void	KNewRoom(argList);
void  KLoadChunk(argList);
void	KSetLanguage(argList);

//	Object management.
void	KClone(argList);
void	KDisposeClone(argList);
void	KRespondsTo(argList);
void  KFindSelector(argList);
void  KFindClass(argList);

// Animated objects & views.
void	KSetNowSeen(argList);
void	KNumLoops(argList);
void	KNumCels(argList);
void	KCelWide(argList);
void	KCelHigh(argList);
void	KGetHighPlanePri(argList);
void	KGetHighItemPri(argList);
void	KShakeScreen(argList);
void	KIsOnMe(argList);
void	KShowMovie(argList);
void	KSetVideoMode(argList);
void	KAddScreenItem(argList);
void	KAddLine(argList);
void	KAddPolygon(argList);
void	KDeleteScreenItem(argList);
void	KDeleteLine(argList);
void	KDeletePolygon(argList);
void	KUpdateScreenItem(argList);
void	KUpdateLine(argList);
void	KUpdatePolygon(argList);
void	KBitmap(argList);
void	KFrameOut(argList);
void	KAddPlane(argList);
void	KDeletePlane(argList);
void	KUpdatePlane(argList);
void	KAddPicAt(argList);
void	KRepaintPlane(argList);
void	KSetShowStyle(argList);
void	KShowStylePercent(argList);
void	KSetScroll(argList);
void	KMovePlaneItems(argList);
void	KAddMagnify(argList);
void	KDeleteMagnify(argList);
void	KIsHiRes(argList);
void	KObjectIntersect(argList);
void  KBaseLineSpan(argList);

// Graphics.
void	KInvertRect(argList);
#ifdef DEBUG
#ifndef WINDOWS
void	KSaveScreen(argList);
#endif
#endif
void	KPriority(argList);
void	KRobot(argList);
void	KCreateTextBitmap(argList);

// Text functions.
void 	KText(argList);
void	KFont(argList);

void	KMessage(argList);
void	KSetQuitStr(argList);

// Scroll Window Calls
void	KScrollWindow(argList);

// Events.
void	KGetEvent(argList);
void	KGlobalToLocal(argList);
void	KLocalToGlobal(argList);
void	KMapKeyToDir(argList);

// Mouse functions.
void	KHaveMouse(argList);
void	KSetCursor(argList);
void 	KVibrateMouse(argList);

// Savegame functions.
void 	KSave(argList);
void	KRestartGame(argList);
void	KGameIsRestarting(argList);

// Sound functions.
void	KDoSound(argList);
void	KDoAudio(argList);
void	KDoSync(argList);

// List functions.
void	KList(argList);

// Math functions.
void	KRandom(argList);
void	KAbs(argList);
void	KSqrt(argList);
void	KGetAngle(argList);
void	KGetDistance(argList);
void	KATan(argList);
void	KSinMult(argList);
void	KCosMult(argList);
void	KSinDiv(argList);
void	KCosDiv(argList);
void	KMulDiv(argList);

// Miscellaneous functions.
void	KGetTime(argList);
void	KPlatform(argList);
void	KCD(argList);
#ifdef WINDOWS
void	KWinHelp(argList);
void	KMessageBox(argList);
void	KGetSierraProfileString(argList);
void	KGetSierraProfileInt(argList);
#endif

// Motion functions.
void	KBaseSetter(argList);
void	KDirLoop(argList);
void	KCantBeHere(argList);
void	KInitBresen(argList);
void	KDoBresen(argList);
void	KSetJump(argList);
void	KAvoidPath(argList);
void	KInPolygon(argList);
void	KMergePoly(argList);

// Debug functions.
#ifdef DEBUG
void	KSetDebug(argList);
void	KInspectObject(argList);
void	KProfiler(argList);
void	KRecord(argList);
void	KPlayBack(argList);
void	KMonoOut(argList);
void	KSetFatalStr(argList);
void	KIntegrityChecking(argList);
void	KCheckIntegrity(argList);
void	KTestPoly(argList);
#endif


//	Memory functions
void	KMemoryInfo(argList);

// OS functions.
void	KFileIO(argList);

// System reporting.
void	KDeviceInfo(argList);

// Palette functions.
void	KPalette(argList);
void	KPalVary(argList);
void	KPalCycle(argList);

// Array & String functions.
void	KArray(argList);
void	KString(argList);
void	KStrStripTSN(argList);
void	KStrTrnTSN(argList);
void	KStrTrnExcludeTSN(argList);

// Color remaping functions
void	KRemapColors(argList);
void  KSetPalStyleRange(argList);

#ifndef NETWORK
void	KEditText(argList);
void	KInputText(argList);
#else
void 	KEdit(argList);
#endif

void	KGetConfig(argList);
void	KCelRect(argList);

void	KTable(argList);
void	KNetwork(argList);
void	KSID(argList);
void	KBaseLineSpan(argList);
void	KCelInfo(argList);
void	KMorphOn(argList);
void	KPlayVMD(argList);
void	KSetHotRectangles(argList);

extern KernelCall	kernelCalls[];
extern int			kernelMax;

#endif
