//	kernel.cpp

#include "sol.hpp"

#include "kernel.hpp"

void
KDummy(kArgs)
{
}

void	KCelLink(argList);

#define Kernel(name)				{ K##name##, #name, 0 }
#define KernelSub(name, sub)	{ K##name##, #name, sub }
#define KSubFunc(name) 			{ #name, 0 }

#define Dummy			Kernel(Dummy)

/* define the KList sub functions */
KernelSubFunc kListSubFuncs[] = {
	KSubFunc(LNew),
	KSubFunc(LDispose),
	KSubFunc(LNewNode),
	KSubFunc(LFirstNode),
	KSubFunc(LLastNode),
	KSubFunc(LEmpty),
	KSubFunc(LNextNode),
	KSubFunc(LPrevNode),
	KSubFunc(LNodeValue),
	KSubFunc(LAddAfter),
	KSubFunc(LAddToFront),
	KSubFunc(LAddToEnd),
	KSubFunc(LAddBefore),
	KSubFunc(LMoveToFront),
	KSubFunc(LMoveToEnd),
	KSubFunc(LFindKey),
	KSubFunc(LDeleteKey),
	KSubFunc(LAt),
	KSubFunc(LIndexOf),
	KSubFunc(LEachElementDo),
	KSubFunc(LFirstTrue),
	KSubFunc(LAllTrue),
	KSubFunc(LSort),
	0
};

KernelCall kernelCalls[] = {
	//	Resource handling.
	Kernel(Load),									// 0
	Kernel(Unload),
	Kernel(ScriptID),
	Kernel(DisposeScript),
	Kernel(Lock),
	Kernel(ResCheck),
	Kernel(Purge),
	Kernel(SetLanguage),
	Dummy,
	Dummy,

	//	Object management.
	Kernel(Clone),									// 10
	Kernel(DisposeClone),
	Kernel(RespondsTo),
	Kernel(FindSelector),
	Kernel(FindClass),
	Dummy,
	Dummy,
	Dummy,
	Dummy,
	Dummy,

	// Animated objects & views.
	Kernel(SetNowSeen),							// 20
	Kernel(NumLoops),
	Kernel(NumCels),
	Kernel(IsOnMe),
   Kernel(AddMagnify),
   Kernel(DeleteMagnify),
	Kernel(CelRect),
	Kernel(BaseLineSpan),

	Kernel(CelWide),								// 28
	Kernel(CelHigh),
	Kernel(AddScreenItem),
 	Kernel(DeleteScreenItem),
	Kernel(UpdateScreenItem),

	Kernel(FrameOut),								// 33

	Kernel(CelInfo),								// 34

	Kernel(Bitmap),								// 35
	Kernel(CelLink),								
	Dummy,
	Dummy,
	Dummy,

	Kernel(AddPlane),								// 40
	Kernel(DeletePlane),
	Kernel(UpdatePlane),
	Kernel(RepaintPlane),

	Kernel(GetHighPlanePri),					// 44
	Kernel(GetHighItemPri),

	Kernel(SetShowStyle),						// 46
	Kernel(ShowStylePercent),					
	Kernel(SetScroll),
	Kernel(MovePlaneItems),                
	Kernel(ShakeScreen),
	Dummy,
	Dummy,
	Dummy,
	Dummy,

	Kernel(IsHiRes),								// 55
	Kernel(SetVideoMode),						
	Kernel(ShowMovie),

	Kernel(Robot),									// 58
#ifndef NETWORK
	Kernel(CreateTextBitmap),
#else
	Dummy,
#endif

	Kernel(Random),								// 60
	Kernel(Abs),
	Kernel(Sqrt),
	Kernel(GetAngle),
	Kernel(GetDistance),
	Kernel(ATan),
	Kernel(SinMult),
	Kernel(CosMult),								
	Kernel(SinDiv),
	Kernel(CosDiv),

	Kernel(Text),									// 70
	Dummy,

	Kernel(Message),								// 72
	Kernel(Font),
#ifndef NETWORK
	Kernel(EditText),
	Kernel(InputText),
#else
	Kernel(Edit),
	Dummy,
#endif
	Kernel(ScrollWindow),
	Dummy,
	Dummy,
	Dummy,

	Kernel(GetEvent),								// 80
	Kernel(GlobalToLocal),
	Kernel(LocalToGlobal),
	Kernel(MapKeyToDir),							

	Kernel(HaveMouse),							// 84
	Kernel(SetCursor),
	Kernel(VibrateMouse),
	Dummy,
	Dummy,
	Dummy,

	Kernel(List),									// 90
	Kernel(Array),
	Kernel(String),
	Kernel(FileIO),

	Kernel(BaseSetter),							// 94
	Kernel(DirLoop),
	Kernel(CantBeHere),
	Kernel(InitBresen),
	Kernel(DoBresen),
	Kernel(SetJump),								
	Kernel(AvoidPath),
	Kernel(InPolygon),
	Kernel(MergePoly),
	Kernel(ObjectIntersect),
	Dummy,

	Kernel(MemoryInfo),							// 105
	Kernel(DeviceInfo),

	Kernel(Palette),								// 107
	Kernel(PalVary),
	Kernel(PalCycle),

	Kernel(RemapColors),							// 110

	Kernel(AddLine),								// 111
	Kernel(DeleteLine),							
	Kernel(UpdateLine),							

	Kernel(AddPolygon),							// 114
	Kernel(DeletePolygon),						
	Kernel(UpdatePolygon),						

	Kernel(DoSound),								// 117
	Kernel(DoAudio),
	Kernel(DoSync),

	Kernel(Save),									// 120

	Kernel(GetTime),								// 121
	Kernel(Platform),
	Kernel(CD),

	Kernel(SetQuitStr),							// 124
	Kernel(GetConfig),

	Kernel(Table),									// 126

#ifdef WINDOWS
	Kernel(WinHelp),              			// 127
#else
	Dummy,		
#endif

#ifdef NETWORK
	Kernel(Network),								// 128
#else
	Kernel(Dummy),
#endif

#ifdef DEBUG
	Kernel(SetDebug),								// 129
	Kernel(InspectObject),
	Kernel(MonoOut),								
	Kernel(SetFatalStr),
	Kernel(IntegrityChecking),
	Kernel(CheckIntegrity),
	Kernel(MarkMemory),
#else
	Kernel(Dummy),								// 129
	Kernel(Dummy),
//	Kernel(MonoOut),								
	Kernel(Dummy),								
	Kernel(Dummy),
	Kernel(Dummy),
	Kernel(Dummy),
	Kernel(Dummy),
#endif
							
#if defined(DEBUG) && !defined(WINDOWS)
	Kernel(SaveScreen),						// 136
#else
	Dummy,
#endif

#ifdef DEBUG
	Kernel(TestPoly),                      // 137
#else
	Dummy,	//Kernel(TestPoly)            // 137
#endif

	Kernel(LoadChunk),							// 138
	Kernel(SetPalStyleRange),					// 139
	Kernel(AddPicAt),						 		// 140
#ifdef WINDOWS
	Kernel(MessageBox),                    // 141
#else
	Dummy, 		//Kernel(MessageBox)       // 141
#endif
	Kernel(NewRoom),								// 142
	Dummy,		//Kernel(PreloadResource)	// 143
	Kernel(Priority),								// 144
	Kernel(MorphOn),								// 145
	Kernel(PlayVMD),								// 146
	Kernel(SetHotRectangles),					// 147
	Kernel(MulDiv),								// 148
	
#ifdef WINDOWS
	Kernel(GetSierraProfileInt),				// 149
	Kernel(GetSierraProfileString),			// 150
	Kernel(SetWindowsOption),					// 151
	Kernel(GetWindowsOption),					// 152
	Kernel(WinDLL)									// 153
#else
	Dummy,											// 149
	Dummy, 											// 150
	Dummy,											// 151
	Dummy,											// 152
	Dummy												// 153
#endif
};

int	kernelMax = sizeof(kernelCalls) / sizeof(*kernelCalls);
