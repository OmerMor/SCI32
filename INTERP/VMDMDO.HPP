//****************************************************************************
//*                                                                          *
//*   Copyright (c)1995,M.D.O.   Programmeur:Mathieu___  Cr‚ation:??/??/??   *
//*                                                                          *
//*   Portage :  [X]:PC  [_]:MAC  [_]:CDI  [_]:3DO  [_]:SATURN  [_]:PSX      *
//*              [_]:Sp‚cifique                                              *
//*                                                                          *
//*   Type de source :  [_]:Interne   [_]:Externe   [X]:Grand public         *
//*                                                                          *
//*   Programme rattach‚ : [X]:Librairies  [_]:Loader                        *
//*			  [_]:Autre : ______________                                     *
//*                                                                          *
//****************************************************************************



//       +--------------------------------------------------------+
//       +                                                        +
//       +       D‚finitions des types de machines                +
//       +                                                        +
//       +--------------------------------------------------------+


#ifdef  WINDOWS						/* Windows */
    #include <WINDOWS.H>
    #define	_NOASM_
#endif
#include <STDLIB.H>
#include "VMDIBM.HPP"



//       +--------------------------------------------------------+
//       +                                                        +
//       +       D‚finition des valeurs systŠmes                  +
//       +                                                        +
//       +--------------------------------------------------------+

#define FALSE 	0
#define TRUE 	1
#define UNKNOW  2

#ifndef NULL
  #define NULL 	0L
#endif

// Flags de GXlseek
#define SEEK_CUR        1
#define SEEK_END        2
#define SEEK_SET        0

// D‚finitions des langues
#define LG_FR		0
#define LG_DE		1
#define LG_GB		2
#define LG_SP		3
#define LG_IT		4
#define LG_US		5
#define LG_NL		6
#define LG_CO		7
#define LG_HE		8
#define LG_PO		9
#define LG_JP		10


#define	CENTER	0x8000				/* Pour le MovRot/Size     */
#define	NO_CHANGE	-1234			/* Pour le MoveRot/Size    */
#define	RAZ_MEM	0x80000000L			/* Pour le GetMem          */


//-------------------------------------------------------------------------*/
//      Flags de GXinitSystem/GXcloseSystem                                */
//-------------------------------------------------------------------------*/

#define RELEASE_ALL	0x3478		/* GXcloseSystem ss v‚rif  */
#define ASK_ALL         1			/* Verifie tout            */
#define INIT_EXTEND     2			/* Init automatique        */
#define NO_CHECK        4			/* Pas de v‚rif            */

//-------------------------------------------------------------------------*/
//      Flags de GXloadSound                                               */
//-------------------------------------------------------------------------*/

#define	DMA		1	        /* Son charg‚ en mode DMA          */
#define	HI		2	        /* Son charg‚ en m‚moire ‚tendue   */
#define	AUTOLOAD	4               /* Son qui se charge progressivemnt*/


//-------------------------------------------------------------------------*/
//      Flags de gestion de m‚moire ‚tendue                                */
//-------------------------------------------------------------------------*/

#define	HI_DISK	1			/* M‚moire ‚tendue virtuelle       */
#define	HI_RAM	2			/* M‚moire ‚tendue physique        */

//-------------------------------------------------------------------------*/
//      Flags de GRinitBitMap                                              */
//-------------------------------------------------------------------------*/

#define B_REAL   1			/* BitMap M‚moire Video            */
#define B_MASK   2			/* BitMap Transparent              */
#define B_NALLOC 32			/* N'alloue pas la ram             */
#define B_HBM	4			/* AccŠde … la m‚moire video       */


//-------------------------------------------------------------------------*/
//      Flags de GRpictMove                                                */
//-------------------------------------------------------------------------*/

#define M_SOLID 	0		/* Move Solide                     */
#define M_TRANSP	3		/* Move Transparent                */
#define	M_VSYM		0x40		/* Sym‚trie Verticale              */
#define	M_HSYM		0x80		/* Sym‚trie Horizontale            */

//-------------------------------------------------------------------------*/
//      D‚finitions pour le tableau SysValue[]                             */
//-------------------------------------------------------------------------*/

#define SCREEN_SIZEX   0		/*  Taille de l'‚cran en X         */
#define SCREEN_SIZEY   1		/*                       Y         */
#define SCREEN_MODE    2		/*  Mode graphique actif           */
#define MOUSE_DELTAX   3		/*  Diviseur vitesse Souris X      */
#define MOUSE_DELTAY   4		/*                          Y      */
#define SCREEN_COLORS  5		/*  Nombre de couleurs du mode graf*/
#define SYSTEM_SPEED   6		/*  PC rapide/lent                 */
#define SPEED_VALUE    7		/*  Valeur de la vitesse           */
#define SCREEN_RSIZEX  8		/*  Taille physique de l'‚cran X   */
#define SCREEN_RSIZEY  9		/*                             Y   */
#define SCREEN_STARTX 10		/*  Coordonn‚es d‚but affichage X  */
#define SCREEN_STARTY 11		/*                              Y  */
#define VGA_PRESENT   17		/*  Carte VGA pr‚sente             */
#define MOUSE_PRESENT 21		/*  Souris pr‚sente                */
#define MODE_256      22		/*  Mode 256couleurs initialis‚	   */
#define PROCESSOR     23		/*  Type de processeur             */
#define DEBUG_MODE    25		/*  Mode DEBUG                     */
#define VESA_SIZEX	26		/*  Taille en X mode VESA          */
#define VESA_SIZEY	27		/*            Y                    */
#define VESA_COLORS	28		/*  Nombre couleurs mode VESA      */
#define RAM2          29		/*  Prochain GetMem en GetMem2     */
#define BLINDE        30		/*  Teste d‚bordement Move         */
#define BREAK         31		/*  Mode BREAK                     */
#define CD_SPEED	33		/*  D‚bit en Ko/Sec pour simul     */
#define CD_DEBIT	34		/*  D‚bit en Ko/Sec du CD          */
#define MPC_MODE	35		/*  Vitesse MPC (1,2,3)            */
#define FROM_WINDOWS	36		/*  Lancement depuis Windows       */


//-------------------------------------------------------------------------*/
//      D‚finitions pour certaines variables                               */
//-------------------------------------------------------------------------*/

#define CHK_MEM         0x4000	/*  Pas de System_Err(-6)          */
#define CHK_ALL         0x0000        /*  V‚rification de tout           */
#define CHK_6           0x0800        /*  Pas de System_Err(-306)        */

#define VGA_NEED	0x40
#define VESA_NEED	0x80

//-------------------------------------------------------------------------*/
//      Définitions pour les fonctions DEBUG                               */
//-------------------------------------------------------------------------*/

  #define	SCREEN	0x0800         /* DEBUG sur écran                  */
  #define	DISK	0x1000         /* DEBUG sur fichier                */
  #define	BEEP	0x2000         /* Beep lors d'un WriteDebug        */
  #define	WAIT	0x4000         /* Attente d'une touche lors d'un WriteDebug */

//       +--------------------------------------------------------+
//       +                                                        +
//       +       D‚finitions des structures                       +
//       +                                                        +
//       +--------------------------------------------------------+

// D‚finition structure Polices
typedef _Packed struct font
{
	BYTE*		Start;
	BYTE 		SizeX;
	BYTE 		SizeY;
	UBYTE 	First;
	UBYTE 	Last;
	WORD 		SizeChar;
	UBYTE 	WidthMax;
   UBYTE		NotUsed;
	UBYTE*	WidthTable;
}  FONT;


// D‚finition structure STK
typedef _Packed struct global
{
	BYTE    GlobalsName[13];
#ifdef	_CDI_
	BYTE	GlobalsType;
#endif
	LONG    GlobalsSize;
	LONG    GlobalsOffset;
#ifndef	_CDI_
	BYTE	GlobalsType;
#endif
}  	GLOBAL;


// D‚finition structure BITMAP
typedef _Packed struct bitmap
{
	WORD    SizeX;
	WORD    SizeY;
#ifndef	_CDI_
	BYTE    BothGetMem;
	BYTE    Depth;
#endif
	WORD    Video;
#ifdef	_CDI_
	WORD	Plan;
#endif
	UBYTE   *Start;
#ifndef	_CDI_
	UWORD   Offset;
	UWORD	Plan0;
	WORD	RSizeX;
#endif
#ifdef	WINDOWS
	HBITMAP	hbm;
#endif
}	COK_BITMAP;


// D‚finition Structure sons
typedef _Packed struct	COK_sound        		
{
//	BYTE *Start;
	BYTE far *Start;
	LONG Size;
	WORD Init;
	WORD Compt;
	WORD Freq;
	WORD  Bits;
	WORD  h;
	LONG CurrRead;
#ifdef	_CDI_
	WORD Fact;
	WORD Mode;
#endif
}	SOUND;


// Structure Coordonn‚es
typedef _Packed struct prect
{
	WORD	x1,x2,y1,y2;
}	COK_PRECT;


// Structure Image
typedef _Packed struct pict
{
	COK_BITMAP 	*Bmp;
	COK_PRECT   *Rxy;
} 	PICT;

// Structure Palette
typedef _Packed struct palette
{
	UBYTE	(*VGA)[3];
} 	PALETTE;


// Structure RVB
typedef	_Packed struct	rvb		
{
	UBYTE	R,V,B;
}	RVB;



//       +--------------------------------------------------------+
//       +                                                        +
//       +     D‚finitions variables globales                     +
//       +                                                        +
//       +--------------------------------------------------------+


#ifdef	WINDOWS
extern  BYTE  	volatile KeyMap[128];
extern  UWORD	far MIDIdelay;
extern  BOOL    far MelodyInProcess;
extern  UWORD	far CompressWidth;
#else
extern  BYTE  	volatile KeyMap[128];
extern  UWORD	MIDIdelay;
extern  BOOL    MelodyInProcess;
extern  UWORD	CompressWidth;
#endif

extern  BOOL    volatile SoundInProcess;
extern  BOOL	MonoGDR;
extern  UBYTE	GDRtest;
extern  LONG	HiSize;
extern  WORD	Hi_Count;
extern  char	LaunchName[];
extern  char	CallName[];
extern  char	*RunParams[];
extern  SOUND   *NullSound;
extern  WORD    AutoMemCheck;
extern  WORD     GlobalsHandle[];
extern  GLOBAL  *Globals[];
extern  WORD	GlobalsNb[];

extern  WORD     GraphConfig;
extern  WORD     MouseConfig;
extern  WORD     SizeRamConfig;
extern  WORD     SizeHiRamConfig;
extern  WORD     SizeHardDiskConfig;
extern  WORD     CdromConfig;
extern  WORD     SoundCardConfig;
extern  UWORD	LanguageConfig;
extern  char	*ProgName;
extern  WORD 	NbFileOpen;
extern  char	*BatchName;

extern  WORD    VIDEO_MODE;

extern  WORD    GetMem_ID[2];
extern  WORD    CurrentLanguage;
extern  BOOL    MousePresent;
extern  LONG    RamStay;
extern  ULONG   SizeRamAdd;
extern  LONG    FileLoadRam;
extern  void    *FileLoadPtr;
extern  WORD    BitMap_ID;
extern  BYTE    Video_R[];
extern  BYTE    Video_V[];
extern  BYTE    Video_B[];
extern  COK_BITMAP  *RealScreen;

extern  WORD     XMouse,YMouse,BMouse;
extern  PALETTE *WorkPal;
extern  BOOL    StickPresent;
extern  BOOL    ExternalLaunch;
extern  UWORD	SoundMode;
extern  UWORD	SoundConfig;
extern  WORD	Fill_X1,Fill_Y1,Fill_X2,Fill_Y2;

extern  WORD	far SINTAB[360];
extern  WORD	far COSTAB[360];

extern  WORD	RotClipX1,RotClipX2,RotClipY1,RotClipY2;
extern  BOOL	UseRotClip;

extern  BOOL	SoundTrack;

extern  BOOL	FontCached;

extern  WORD	UserDefined_1,UserDefined_2,UserDefined_3,UserDefined_4;


//       +--------------------------------------------------------+
//       +                                                        +
//       +       D‚finitions des fonctions                        +
//       +                                                        +
//       +--------------------------------------------------------+

extern  void    GXinitSystem(WORD Mode);
extern  void    GXcloseSystem(UWORD);

//       +--------------------------------------------------------+
//       +                                                        +
//       +       Gestions Inputs                                  +
//       +                                                        +
//       +--------------------------------------------------------+

extern  WORD    ReadKey(void);
extern  WORD    WaitKey(void);
extern  void    FlushKbd(void);

extern  WORD    Joystick(void);
extern  WORD    JoystickB(void);
extern  WORD	 ReadJoystick(WORD *,WORD *);
extern  void    GXreadMouse(WORD *ptrX,WORD *ptrY,WORD *ptrB);
extern  void    GXposMouse(WORD X,WORD Y);
extern  void    WaitNoMouse(void);
extern  void	 WaitMouse(void);

//       +--------------------------------------------------------+
//       +                                                        +
//       +       Gestion Fichiers                                 +
//       +                                                        +
//       +--------------------------------------------------------+

extern  WORD	 GXopen(char *File,WORD Flags);
extern  UWORD   GXread(WORD handle,void  *ptr,UWORD Size);
extern  UWORD   _fGXread(WORD handle,void far *ptr,UWORD Size);
extern  UWORD   GXwrite(WORD handle,void  *ptr,UWORD Size);
extern  void    GXclose(WORD handle);
extern  long    GXlseek(WORD handle, long offset, WORD fromwhere);

extern  long    GXfilesize(BYTE *name);
extern  void    *GXfileLoad(BYTE *name);
extern  void    *GXfileLoad2(BYTE *name);

extern  void    GXloadCMP(BYTE *Name,COK_BITMAP *Bmp);

extern  void    DelGlobalList(void);
extern  void    AddGlobalList(BYTE *file);

extern  void    ExecModule(char *file,char *add);

//       +--------------------------------------------------------+
//       +                                                        +
//       +       Gestion m‚moire                                  +
//       +                                                        +
//       +--------------------------------------------------------+

extern  void*   GetMem(size_t Size);
extern  void	 ReleaseMem(void * memoryPtr);
extern  void 	 ReleaseVMDMem();
extern  LONG    MMmaxBlock(void);
extern  void*	 GetMem2(LONG Size);
extern  LONG    MMmaxBlock2(void);
extern  LONG    MMfreeMem(void);
extern  LONG    MMfreeMem2(void);


//       +--------------------------------------------------------+
//       +                                                        +
//       +       Gestion graphique                                +
//       +                                                        +
//       +--------------------------------------------------------+

extern  FONT    *GXloadFont(BYTE *file);
extern  void    GXfreeFont(FONT *font);

extern  COK_BITMAP  *GRinitBitMap(WORD VideoMode,WORD SizeX,WORD SizeY,WORD Status);
extern  void    GRfreeBitMap(COK_BITMAP *Bmp);

extern  void    GRsetVideoMode(WORD Mode);

extern  void    GRpictMove(COK_BITMAP *BitMap1,COK_BITMAP *BitMap2,WORD X1,WORD Y1,WORD X2,WORD Y2,WORD Xd,WORD Yd,WORD Mode);
extern  void    GRfillZone(COK_BITMAP *Bmp,WORD X1,WORD Y1,WORD X2,WORD Y2,WORD Coul);
extern  void    GRplot(WORD X,WORD Y,WORD Coul,COK_BITMAP *Bmp);
extern  void    GRdrawLine(COK_BITMAP *bmp,WORD X1,WORD Y1,WORD X2,WORD Y2,WORD C);
extern  void    DrawLineClip(COK_BITMAP *bmp,WORD X1,WORD Y1,WORD X2,WORD Y2,WORD C);
extern  void    GRdrawCircle(COK_BITMAP *bmp,WORD Xc,WORD Yc,WORD R,WORD C);
extern  void    GRdispChar(UWORD Ch,WORD X,WORD Y,FONT *Font,WORD TextMode,WORD ColT,WORD ColF,COK_BITMAP *Screen);
extern  void    ClearBitMap(COK_BITMAP *bmp);
extern  void    GRsysUnpack(BYTE *ptr,WORD SizeX,WORD SizeY,WORD X,WORD Y,WORD Mode,COK_BITMAP *bmp);
extern  void    GRinvertRect(COK_BITMAP *Bmp,WORD X1,WORD Y1,WORD X2,WORD Y2);
extern  void    StringDisplay(char *str,WORD X,WORD Y,WORD Ct,WORD Cf,WORD mode,COK_BITMAP *bmp,FONT *font);

extern  void 	  Setup360(void);

extern  WORD    PolyClip(WORD nb,WORD points[][2],WORD x0,WORD y0,WORD xs,WORD ys);
extern  void    PaintPoly(WORD n,WORD points[][2],WORD col,COK_BITMAP *bmp);
extern  void    AssignFillMask(WORD Pal[16]);

extern  BOOL    DecompressIFF(UBYTE *tampon,COK_BITMAP *Bmp,UBYTE (*IFFpalette)[3],WORD *NbCol);

extern  void    PutBlock(UWORD Id,PICT *Pict,WORD Xd,WORD Yd,WORD mode,COK_BITMAP *bmpD);
extern  PICT    *LoadPictSmall(char *name);
extern  void    ReleasePictSmall(PICT *Pict);

extern  void    DrawLine(COK_BITMAP *bmp,WORD X1,WORD Y1,WORD X2,WORD Y2,WORD C);

extern  void    GRtextDisp(char *str,WORD X,WORD Y,WORD Ct,WORD Cf,FONT *Font,COK_BITMAP *Bmp);
extern  void    GRfillShape(COK_BITMAP *bmp,WORD X1,WORD Y1,WORD X2,WORD Y2,WORD C1,WORD C2);
extern  void    BuildMask(COK_BITMAP *Bmp);

extern  void	  MoveRot(COK_BITMAP *,COK_BITMAP *,WORD,WORD,WORD,WORD,WORD,WORD,WORD,WORD);
extern  void	  MoveSize(COK_BITMAP *,COK_BITMAP *,WORD,WORD,WORD,WORD,WORD,WORD,WORD);
extern  void	  PutSize(PICT *,WORD,COK_BITMAP *,WORD,WORD,WORD);
extern  void	  PutRot(PICT *,WORD,COK_BITMAP *,WORD,WORD,WORD,WORD);

extern  void    LoadFontCache(void);
extern  void    ReleaseFontCache(void);


//       +--------------------------------------------------------+
//       +                                                        +
//       +       Fonctions diverses                               +
//       +                                                        +
//       +--------------------------------------------------------+

extern  void    InitTimer(WORD spd);
extern  void    TimerRun(void);

extern  void    GXdelay(WORD Delay);
extern  LONG    ReadTimer(void);

#define	GXrandom(a)	random(a)
#define	GXabs(n)	abs(n)
#define	GXlabs(n)	labs(n)
#define	GXmin(a,b)	((a)>(b)?(b):(a))
#define	GXmax(a,b)	((a)>(b)?(a):(b))
#define	GXlmin(a,b)	((a)>(b)?(b):(a))
#define	GXlmax(a,b)	((a)>(b)?(a):(b))
#define	GXatoi(a)	atoi(a)
#define	GXltoa(a,b)	ltoa(a,b,10)
#define	GXitoa(a,b)	itoa(a,b,10)



//       +--------------------------------------------------------+
//       +                                                        +
//       +       Gestion sonore                                   +
//       +                                                        +
//       +--------------------------------------------------------+

  extern  void    GXnosound(void);
  extern  void    GXsound(WORD Freq);
  extern  void far *GXgetSoundMem(size_t size);
  extern  void    GXreleaseSoundMem(void far *addr);
  extern  void    GXstartSound(SOUND *sound,WORD nbr,WORD freq);
  extern  void    GXstopSound(WORD time);
  extern  SOUND   *GXloadSound(char *file,UBYTE Mode);
  extern  void    GXfreeSound(SOUND *snd);
  extern  void    WaitNoSound(void);
  extern  void    WaitSoundEnd(void);
  extern  void    GXstartMusic(SOUND **sndtbl,WORD *music,WORD time);

  extern  void	  StartMIDI(void *ptr,WORD nbr,WORD decay);
  extern  void	  StopMIDI(WORD decay);

  extern  void    SetSoundVolume(WORD a);
  extern  void    SetDigitLevel(WORD);

  extern  BOOL	  AutoLoadSound(SOUND *Snd,WORD Nb);



//       +--------------------------------------------------------+
//       +                                                        +
//       +       Gestion Hard Ecran                               +
//       +                                                        +
//       +--------------------------------------------------------+

extern  void    ShakeScreen(BOOL state);
extern  void    GRvSync(WORD Video);

extern  void    ScreenOrigin(WORD X,WORD Y);
extern  void    SplitScreen(WORD Y);
extern  void    ScreenOriginW(WORD adr);
extern  void    SetScreenSize(COK_BITMAP *);


//       +--------------------------------------------------------+
//       +                                                        +
//       +       Gestion Palette                                  +
//       +                                                        +
//       +--------------------------------------------------------+

  extern  void 	  *LoadPalette(char *file);

  extern  void    GRassignColor(WORD Num,BYTE Comp1,BYTE Comp2,BYTE Comp3,WORD Video);
  extern  void    GRassignPalette(PALETTE *pal);

  extern  void    GradiantPalette(PALETTE *pal,WORD tempo,WORD Mode);
  extern  void    BlackPalette(void);
  extern  BOOL    ExecuteGradiant(void);

  extern  void	  MoveBitmap(COK_BITMAP *,WORD,WORD,WORD,WORD);
  extern  void	  AssignColors(WORD,WORD);
  extern  void	  AssignColorsPtr(WORD,WORD,WORD,void *);


//       +--------------------------------------------------------+
//       +                                                        +
//       +       Fonctions systŠmes                               +
//       +                                                        +
//       +--------------------------------------------------------+

extern  void    CreateDebug(UWORD mode);
extern  void    WriteDebug(int mode,char *format,...);
extern  void    CloseDebug(void);
extern  void	 VMD_Shutdown();

extern  LONG	 CompactData(void *,LONG,void *,BOOL);
extern  BOOL	 CompressEvent(LONG,LONG);
extern  "C" 	 LONG DecompressData(void *,void *);

extern  void	 SetAlias(BOOL);

extern  LONG	 Mapmem(void);
extern  LONG	 Chkdsk(WORD d);

extern  WORD    GXopenOld(char *file,WORD mode);
extern  void    GXcloseOld(WORD h);
extern  WORD    GXreadOld(WORD handle,void *ptr,UWORD size);
extern  LONG    GXlseekOld(WORD handle,LONG offs,WORD mode);

extern  LONG	 BlocProcess(void *fct,WORD val,UBYTE *ptrA,LONG Size);
extern  LONG	 _fBlocProcess(void *fct,WORD val,UBYTE far *ptrA,LONG Size);

#define _Invert(a)	a=(((a>>8)&0xFF)+((a&0xFF)<<8))
#define _InvertL(a)	a=(((a>>24)&0xFF)+((a>>8)&0xFF00L)+((a<<8)&0xFF0000L)+((a&0xFF)<<24))

extern  void	  InitVESA(void);

extern  void	  *CastPointer(void *,LONG);
extern  void	  FreeCast(void *);
extern  void	  InstallAlias(void **,UWORD *);
extern  void	  FreeAlias(void **,UWORD *);

extern  BOOL	  EjectCD(BOOL);


//       +--------------------------------------------------------+
//       +                                                        +
//       +       Pointeurs fonctions                              +
//       +                                                        +
//       +--------------------------------------------------------+

extern  void   *(*X_GXfileLoad256)(BYTE *);
extern  void 	(*X_SetVideoStop)(WORD);
extern  void 	(*X_GXstopSound)(WORD);
extern  LONG 	(*X_CDOSfilesize)(char *);
extern  void   *(*X_CDOSfileLoad)(char *);
extern  void 	(*X_CDOSuninstall)(void);
extern  void 	(*X_CloseHiRam)(void);
extern  void 	(*X_AdStopSound)(WORD);


//       +--------------------------------------------------------+
//       +                                                        +
//       +       Fonctions Sp‚cifiques Windows                    +
//       +                                                        +
//       +--------------------------------------------------------+

#ifdef	WINDOWS
// Mode d'‚cran
#define	FULL_SCREEN		1
#define	BIS_SCREEN		2
#define	DOUBLE_SCREEN	4
#define	FULL_PALETTE	8

extern	void		SelectWindow(HWND);
extern	void		UnSelectWindow(void);
extern	WORD		LockRam(WORD size);
extern	void		MakeWindow(WORD sx,WORD sy,HWND hw,void (*paint)(HDC,HWND),BOOL);
extern	BOOL		SystemLoop(WORD);
extern	BOOL            LockPointer(void *);
extern	void		UnlockPointer(void *);
#endif
