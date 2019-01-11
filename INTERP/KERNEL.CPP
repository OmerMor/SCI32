//	kernel.cpp

#include "sol.hpp"

#include "kernel.hpp"

void
KDummy(kArgs)
{
}

#define Kernel(name)	{ K##name##, #name }

#define Dummy			Kernel(Dummy)

KernelCall kernelCalls[] = {
	//	Resource handling.
	Kernel(Load),
	Kernel(Unload),
	Kernel(ScriptID),
	Kernel(DisposeScript),
	Kernel(Lock),
	Kernel(ResCheck),
	Kernel(Purge),

	//	Object management.
	Kernel(Clone),
	Kernel(DisposeClone),
	Kernel(RespondsTo),

	// Animated objects & views.
	Kernel(SetNowSeen),							// 10
	Kernel(NumLoops),
	Kernel(NumCels),
	Kernel(CelWide),
	Kernel(CelHigh),
	Kernel(GetHighPlanePri),
	Kernel(GetHighItemPri),
	Kernel(ShakeScreen),
	Kernel(IsOnMe),
	Kernel(ShowMovie),
	Kernel(SetVideoMode),						// 20
	Kernel(AddScreenItem),
 	Kernel(DeleteScreenItem),
	Kernel(UpdateScreenItem),
	Kernel(FrameOut),
	Kernel(AddPlane),
	Kernel(DeletePlane),
	Kernel(UpdatePlane),
	Kernel(RepaintPlane),
	Kernel(SetShowStyle),
	Kernel(ShowStylePercent),					// 30
	Kernel(SetScroll),
   Kernel(AddMagnify),
   Kernel(DeleteMagnify),
	Kernel(IsHiRes),

	// Graphics.
	Kernel(Robot),
	Kernel(InvertRect),

	// Text functions.
	Kernel(TextSize),
	Kernel(Message),
	Kernel(TextColors),
	Kernel(TextFonts),							// 40
#if defined(DEBUG) && !defined(WINDOWS)
	Kernel(SaveScreen),
#else
	Dummy,	//Kernel(SaveScreen),
#endif
	Kernel(SetQuitStr),
	Kernel(EditText),
	Kernel(InputText),
	Kernel(CreateTextBitmap),
	Kernel(Priority),

	// Events.
	Kernel(GetEvent),
	Kernel(GlobalToLocal),
	Kernel(LocalToGlobal),
	Kernel(MapKeyToDir),							// 50

	// Mouse functions.
	Kernel(HaveMouse),
	Kernel(SetCursor),
	Kernel(VibrateMouse),

	// Savegame functions.
	Kernel(SaveGame),
	Kernel(RestoreGame),
	Kernel(RestartGame),
	Kernel(GameIsRestarting),
	Kernel(MakeSaveCatName),
	Kernel(MakeSaveFileName),
	Kernel(GetSaveFiles),						// 60
	Kernel(GetSaveDir),
	Kernel(CheckSaveGame),
	Kernel(CheckFreeSpace),

	// Sound functions.
	Kernel(DoSound),
	Kernel(DoAudio),
	Kernel(DoSync),

	// List functions.
	Kernel(NewList),
	Kernel(DisposeList),
	Kernel(NewNode),
	Kernel(FirstNode),							// 70
	Kernel(LastNode),
	Kernel(EmptyList),
	Kernel(NextNode),
	Kernel(PrevNode),
	Kernel(NodeValue),
	Kernel(AddAfter),
	Kernel(AddToFront),
	Kernel(AddToEnd),
	Dummy,	//Kernel(MoveToFront),
	Dummy,	//Kernel(MoveToEnd),				// 80
	Kernel(FindKey),
	Dummy,	//Kernel(FindFirst),
	Dummy,	//Kernel(FindNext),
	Dummy,	//Kernel(FindPrev),
	Kernel(DeleteKey),
	Dummy,	//Kernel(DeleteID),
	Dummy,	//Kernel(Sort),
	Kernel(ListAt),
	Kernel(ListIndexOf),
	Kernel(ListEachElementDo),					// 90
	Kernel(ListFirstTrue),
	Kernel(ListAllTrue),

	// Math functions.
	Kernel(Random),
	Kernel(Abs),
	Kernel(Sqrt),
	Kernel(GetAngle),
	Kernel(GetDistance),
	Kernel(ATan),
	Kernel(SinMult),
	Kernel(CosMult),								// 100
	Kernel(SinDiv),
	Kernel(CosDiv),

	// Miscellaneous functions.
	Kernel(GetTime),
	Kernel(Platform),

	// Motion functions.
	Kernel(BaseSetter),
	Kernel(DirLoop),
	Kernel(CantBeHere),
	Kernel(InitBresen),
	Kernel(DoBresen),
	Kernel(SetJump),								// 110
	Kernel(AvoidPath),
	Kernel(InPolygon),
	Kernel(MergePoly),

	// Debug functions.
#ifdef DEBUG
	Kernel(SetDebug),
	Kernel(InspectObject),
#else
	Dummy,	//Kernel(SetDebug),
	Dummy,	//Kernel(InspectObject),
#endif
	Kernel(MemoryInfo),
#ifdef DEBUG
	Kernel(Profiler),
	Kernel(Record),
	Kernel(PlayBack),
	Kernel(MonoOut),								// 120
	Kernel(SetFatalStr),
#else
	Dummy,	//Kernel(Profiler),
	Dummy,	//Kernel(Record),
	Dummy,	//Kernel(PlayBack),
	Dummy,	//Kernel(MonoOut),				// 120
	Dummy,	//Kernel(SetFatalStr),
#endif

	// OS functions.
	Kernel(GetCWD),
	Kernel(ValidPath),
	Kernel(FileIO),

	// System reporting.
	Dummy,	//Kernel(SysInfo),
	Kernel(DeviceInfo),

	// Palette functions.
	Kernel(Palette),
	Kernel(PalVary),
	Kernel(PalCycle),
	
	// Array & String functions.
	Kernel(Array),									// 130
	Kernel(String),

	// Color Remaping Functions
	Kernel(RemapColors),

#ifdef DEBUG
	Kernel(IntegrityChecking),
	Kernel(CheckIntegrity),
#else
	Dummy,	//Kernel(IntegrityChecking),
	Dummy,	//Kernel(CheckIntegrity),
#endif

	Kernel(ObjectIntersect),

#ifdef DEBUG
	Kernel(MarkMemory),							// 136
#else
	Dummy,	//Kernel(MarkMemory)
#endif
	
	Kernel(TextWidth),							// 137
	Kernel(PointSize),							// 138

	Kernel(AddLine),								// 139
	Kernel(DeleteLine),							// 140
	Kernel(UpdateLine),							// 141

	Kernel(AddPolygon),							// 142
	Kernel(DeletePolygon),						// 143
	Kernel(UpdatePolygon),						// 144

	Kernel(Bitmap),								// 145
	
	Kernel(ScrollWindow),                  // 146
	Kernel(SetFontRes),                    // 147
	Kernel(MovePlaneItems),                // 148
	Kernel(PreloadResource),					// 149
#ifdef INN
	Kernel(BaseLineSpan), 						// 150
#else
	Dummy,
#endif
	Kernel(NewRoom),								// 151
	Kernel(CheckCDisc),							// 152
	Kernel(GetSaveCDisc),						// 153
#ifdef DEBUG
	Kernel(TestPoly),                      // 154
#else
	Dummy,	//Kernel(TestPoly)            // 154
#endif

#ifdef WINDOWS
	Kernel(WinHelp),                        // 155
#else
	Dummy,		//Kernel(WinHelp)           // 155
#endif
	Kernel(LoadChunk),								 // 156
	Kernel(SetPalStyleRange),						 // 157
	Kernel(AddPicAt),						 			 // 158
#ifdef WINDOWS
	Kernel(MessageBox)                      // 159
#else
	Dummy 		//Kernel(MessageBox)        // 159
#endif
};

int	kernelMax = sizeof(kernelCalls) / sizeof(*kernelCalls);
