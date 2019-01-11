// soundwin.cpp
// Provides interface linkage to the Windows sound functions which
// live in a DLL.  NOTE: Function InitWinSound must be called before
// any of the sound functions may be called.

#ifndef WIN32S
#include <i86.h>
#endif
#include "sciwin.hpp"

#include <dos.h>
#include <stdarg.h>

#include	"sol.hpp"

#include "object.hpp"
#include "midi.hpp"
#include "sound.hpp"
#include "soundwin.hpp"
#include "timew.hpp"
#include "winthunk.hpp"

#ifndef WIN32S
WORD		mySeg;
#else
DWORD			Args[5];
PVOID			TransList[5];
#endif

int	i;

void InitWinSound(void)
{
#ifndef WIN32S
	// Get current segment
	mySeg = FP_SEG((int far *)&i);
#endif
}

#pragma warning 579  9         //   Shut off cast warning for FP_OFF
int DoSound (int sndFunc, ...)
{
int	retVal;
Sound	*SNodePtr;
#ifndef WIN32S
WORD	w1, w2, w3;
DWORD pOff1, pOff2, pOff3, pOff4;
SNodeData far	*SNodeDataPtr;
#else
SNodeData	*SNodeDataPtr;
#endif

	if (!USEMIDI) return 0;

	// WARNING: cannot imbed va_arg() in function call since PASCAL
	// calling sequence will evaluate them in reverse order.
	va_list	args;
	va_start(args, sndFunc);

#ifdef WIN32S
	// First argument to DLL is the sound function;
	Args[0] = (DWORD)sndFunc;
#endif

	_disable();
	((TimeMgrWin*)timeMgr)->DisableTimer();

	switch (sndFunc) {
		case STerminate:
			// (nothing)
#ifndef WIN32S
			retVal = InvokeIndirectFunction(hDoSoundA, sndFunc,
											DLL_SOUND_A);
#else
			retVal = (* pfnUTProc)(Args, (DWORD)DLL_DOSOUND, NULL);
#endif
			break;

		case SProcess:
		case SSetReverb:
		case SMasterVol:
		case SSoundOn:
			// WORD
#ifndef WIN32S
			w1 = va_arg(args,WORD);
			retVal = InvokeIndirectFunction(hDoSoundB, sndFunc,
											w1,
											DLL_SOUND_B);
#else
			Args[1] = (DWORD)va_arg(args,WORD);
			retVal = (* pfnUTProc)(Args, (DWORD)DLL_DOSOUND, NULL);
#endif
			break;

		case SInit:
			// ptr
#ifndef WIN32S
			pOff1 = va_arg(args, DWORD);
			retVal = InvokeIndirectFunction(hDoSoundC, sndFunc,
											pOff1, mySeg,
											DLL_SOUND_C);
#else
			SNodePtr = (Sound *)va_arg(args, DWORD);
			Args[1] = (DWORD)SNodePtr;
			TransList[0] = &Args[1];
			TransList[1] = 0;
			retVal = (* pfnUTProc)(Args, (DWORD)DLL_DOSOUND, TransList);
#endif
			break;

		case SEnd:
		case SRestore:
			// ptr
#ifndef WIN32S
			SNodePtr = (Sound *)va_arg(args, DWORD);
			SNodeDataPtr = (SNodeData far *)SNodePtr->sDataPtr;
			retVal = InvokeIndirectFunction(hDoSoundC, sndFunc,
											(DWORD)FP_OFF((void far *)SNodeDataPtr),
											(WORD)FP_SEG(SNodeDataPtr),
											DLL_SOUND_C);
#else
			SNodePtr = (Sound *)va_arg(args, DWORD);
			SNodeDataPtr = (SNodeData *)SNodePtr->sDataPtr;
			SNodeDataPtr->sMidiPtr16 = SNodeDataPtr->sMidiPtr32;
			Args[1] = (DWORD)SNodeDataPtr;
			TransList[0] = &Args[1];
			TransList[1] = 0;
			retVal = (* pfnUTProc)(Args, (DWORD)DLL_DOSOUND, TransList);
#endif
			break;

		case SPlay:
		case SPause:
		case SHold:
		case SChangeVol:
		case SChangePri:
			// ptr, WORD
			// SPause can have 0 pointer
#ifndef WIN32S
			SNodePtr = (Sound *)va_arg(args, DWORD);
			if (SNodePtr)
				SNodeDataPtr = (SNodeData far *)SNodePtr->sDataPtr;
			else
				SNodeDataPtr = (SNodeData far *)0;
			w1 = va_arg(args,WORD);
			retVal = InvokeIndirectFunction(hDoSoundD, sndFunc,
											(DWORD)FP_OFF((void far *)SNodeDataPtr),
											(WORD)FP_SEG(SNodeDataPtr),
											w1,
											DLL_SOUND_D);
#else
			SNodePtr = (Sound *)va_arg(args, DWORD);
			if (SNodePtr) {
				SNodeDataPtr = (SNodeData *)SNodePtr->sDataPtr;
				SNodeDataPtr->sMidiPtr16 = SNodeDataPtr->sMidiPtr32;
				Args[1] = (DWORD)SNodeDataPtr;
				Args[2] = (DWORD)va_arg(args,WORD);
				TransList[0] = &Args[1];
				TransList[1] = 0;
				retVal = (* pfnUTProc)(Args, (DWORD)DLL_DOSOUND, TransList);
			} else {
				Args[1] = (DWORD)0;
				Args[2] = (DWORD)va_arg(args,WORD);
				retVal = (* pfnUTProc)(Args, (DWORD)DLL_DOSOUND, NULL);
			}
//			Args[2] = (DWORD)va_arg(args,WORD);
//			retVal = (* pfnUTProc)(Args, (DWORD)DLL_DOSOUND, TransList);
#endif
			break;

		case SPChange:
		case SPBend:
			// ptr, WORD, WORD
#ifndef WIN32S
			SNodePtr = (Sound *)va_arg(args, DWORD);
			SNodeDataPtr = (SNodeData far *)SNodePtr->sDataPtr;
			w1 = va_arg(args,WORD);
			w2 = va_arg(args,WORD);
			retVal = InvokeIndirectFunction(hDoSoundE, sndFunc,
											(DWORD)FP_OFF((void far *)SNodeDataPtr),
											(WORD)FP_SEG(SNodeDataPtr),
											w1,
											w2,
											DLL_SOUND_E);
#else
			SNodePtr = (Sound *)va_arg(args, DWORD);
			SNodeDataPtr = (SNodeData *)SNodePtr->sDataPtr;
			SNodeDataPtr->sMidiPtr16 = SNodeDataPtr->sMidiPtr32;
			Args[1] = (DWORD)SNodeDataPtr;
			Args[2] = (DWORD)va_arg(args,WORD);
			Args[3] = (DWORD)va_arg(args,WORD);
			TransList[0] = &Args[1];
			TransList[1] = 0;
			retVal = (* pfnUTProc)(Args, (DWORD)DLL_DOSOUND, TransList);
#endif
			break;

		case SNoteOff:
		case SNoteOn:
		case SController:
		case SFade:
			// ptr, WORD, WORD, WORD
#ifndef WIN32S
			SNodePtr = (Sound *)va_arg(args, DWORD);
			SNodeDataPtr = (SNodeData far *)SNodePtr->sDataPtr;
			w1 = va_arg(args,WORD);
			w2 = va_arg(args,WORD);
			w3 = va_arg(args,WORD);
			retVal = InvokeIndirectFunction(hDoSoundF, sndFunc,
											(DWORD)FP_OFF((void far *)SNodeDataPtr),
											(WORD)FP_SEG(SNodeDataPtr),
											w1,
											w2,
											w3,
											DLL_SOUND_F);
#else
			SNodePtr = (Sound *)va_arg(args, DWORD);
			SNodeDataPtr = (SNodeData *)SNodePtr->sDataPtr;
			SNodeDataPtr->sMidiPtr16 = SNodeDataPtr->sMidiPtr32;
			Args[1] = (DWORD)SNodeDataPtr;
			Args[2] = (DWORD)va_arg(args,WORD);
			Args[3] = (DWORD)va_arg(args,WORD);
			Args[4] = (DWORD)va_arg(args,WORD);
			TransList[0] = &Args[1];
			TransList[1] = 0;
			retVal = (* pfnUTProc)(Args, (DWORD)DLL_DOSOUND, TransList);
#endif
			break;

		case SPatchReq:
			// ptr, ptr, ptr, ptr
#ifndef WIN32S
			pOff1 = va_arg(args, DWORD);
			pOff2 = va_arg(args, DWORD);
			pOff3 = va_arg(args, DWORD);
			pOff4 = va_arg(args, DWORD);

			retVal = InvokeIndirectFunction(hDoSoundG, sndFunc,
											pOff1, mySeg,
											pOff2, mySeg,
											pOff3, mySeg,
											pOff4, mySeg,
											DLL_SOUND_G);
#else
			Args[1] = (DWORD)va_arg(args,DWORD);
			Args[2] = (DWORD)va_arg(args,DWORD);
			Args[3] = (DWORD)va_arg(args,DWORD);
			Args[4] = (DWORD)va_arg(args,DWORD);
			TransList[0] = &Args[1];
			TransList[1] = &Args[2];
			TransList[2] = &Args[3];
			TransList[3] = &Args[4];
			TransList[4] = 0;
			retVal = (* pfnUTProc)(Args, (DWORD)DLL_DOSOUND, TransList);
#endif
			break;

		default:
			retVal = 0;
			break;
	}
	((TimeMgrWin*)timeMgr)->EnableTimer();
	_enable();
	return retVal;
}
#pragma warning 579  4         //   Turn on cast warning

void SetMIDIType (int type)
{
	if (!USEMIDI) return;
	_disable();
#ifndef WIN32S
	InvokeIndirectFunction(hSetMIDIType, type, DLL_SET_TYPE);
#else
	Args[0] = (DWORD)type;
	(* pfnUTProc)(Args, (DWORD)DLL_SET_MIDI_TYPE, NULL);
#endif
	_enable();
}

void OpenSnd (void)
{
	if (!USEMIDI) return;
	_disable();
	((TimeMgrWin*)timeMgr)->DisableTimer();
#ifndef WIN32S
	InvokeIndirectFunction(hOpenSnd, DLL_OPEN_SND);
#else
	(* pfnUTProc)(NULL, (DWORD)DLL_OPEN_SOUND, NULL);
#endif
	((TimeMgrWin*)timeMgr)->EnableTimer();
	_enable();
}

void CloseSnd (void)
{
	if (!USEMIDI) return;
	_disable();
	((TimeMgrWin*)timeMgr)->DisableTimer();
#ifndef WIN32S
	InvokeIndirectFunction(hCloseSnd, DLL_CLOSE_SND);
#else
	(* pfnUTProc)(NULL, (DWORD)DLL_CLOSE_SOUND, NULL);
#endif
	((TimeMgrWin*)timeMgr)->EnableTimer();
	_enable();
}

