//***************************************************************************
//                                                                          *
//   Copyright (c)1995,M.D.O.   Programmeur:Mathieu___  Cr‚ation:??/??/??   *
//                                                                          *
//   Portage :  [X]:PC  [_]:MAC  [_]:CDI  [_]:3DO  [_]:SATURN  [_]:PSX      *
//              [_]:Sp‚cifique                                              *
//                                                                          *
//   Type de source :  [_]:Interne   [_]:Externe   [X]:Grand public         *
//                                                                          *
//   Programme rattach‚ : [X]:Librairies  [_]:Loader                        *
//			  [_]:Autre : ______________                                       *
//                                                                          *
//***************************************************************************

#include <FCNTL.H>

#define	PC_SOUND	1
#define	MDO_SOUND	2
#define	SB_SOUND	4
#define	AD_SOUND	8
#define	MV_SOUND	16
#define	MIDI_SOUND	0x4000
#define	ASK_SOUND	0x8000
#define	ANA_SOUND	(MDO_SOUND|SB_SOUND|MV_SOUND)

#define O_DENYALL   0x10
#define O_DENYWRITE 0x20
#define O_DENYREAD  0x30
#define O_DENYNONE  0x40
#define O_FAST          0x8000
#define O_NoRAM         0x0200
#define O_NoCD          0x4000

#define WORD short
#define BYTE char
#ifndef WINDOWS
#define BOOL char
#endif




// this typedef gives and error in non-win32s Windows
#ifdef WIN32S
  typedef signed long     LONG;
#endif
#ifndef WINDOWS
  typedef signed long     LONG;
#endif

#define _Cdecl __cdecl
#define _CType __cdecl
typedef unsigned short  UWORD;
typedef unsigned long   ULONG;
typedef unsigned char   UBYTE;

//       +--------------------------------------------------------+
//       +                                                        +
//       +       D‚finitions internes librairies                  +
//       +                                                        +
//       +--------------------------------------------------------+

enum
{
  DR_NUL,
  DR_VGA,
  DR_EGA,
  DR_CGA,
  DR_360,
  DR_VESA,
  DR_FIN,
};

#define	L_CGA	1
#define	L_EGA	2
#define	L_VGA	4
#define	L_360	8
#define	GetGdriver(Video)    (Gdrivers[NumGdriver[Video]])

//       +--------------------------------------------------------+
//       +                                                        +
//       +     Structures sp‚cifiques PC                          +
//       +                                                        +
//       +--------------------------------------------------------+

#ifndef	WINDOWS
typedef	struct				/* Gestion HIMEM                   */
{
	LONG	Offset;
	LONG	Size;
	WORD	Handle;
	UBYTE	Mode;
	UBYTE	Null;
	LONG Seek;
	void	*ptr;
}	HI_H;

typedef	struct
	{
	HI_H	*HiList;
	WORD	Hi_Nb,Hi_Pos;
	} HI_T;

typedef _Packed struct		       		/* Gestions Canaux DMA             */
{
	UBYTE	Channel;
	UBYTE	Page;
	UBYTE	BaseAddr;
	UBYTE	BaseCount;
	UBYTE	Mask;
	UBYTE	Mode;
	UBYTE	Flop;
	UBYTE	Enable;
	UBYTE	Disable;
	UBYTE	PlayMode;
	UBYTE	RecMode;
	UBYTE	Free;
} 	DMAport;

typedef	_Packed struct				/* Gestion IRQ                     */
{
	WORD	Number;
	UBYTE	MaskEnable;
	UBYTE	MaskDisable;
	WORD	Vector;
} 	IRQport;

typedef _Packed struct				/* Gestion GDR                     */
	{
	WORD	Version;
	void far (*LGRtext)();
	void far (*LGRset)();
	void far (*LGRfillZone)(void *Bmp,WORD X1,WORD Y1,WORD X2,WORD Y2,WORD Coul);
	void far (*LGRpictMove)(void *BitMap1,void *BitMap2,WORD X1,WORD Y1,WORD X2,WORD Y2,WORD Xd,WORD Yd,WORD Mode);
	void far (*LGRdispChar)(UWORD Ch,WORD X,WORD Y,void *Font,WORD TextMode,WORD ColT,WORD ColF,void *Screen);
	void far (*LGRsysUnpack)(BYTE *ptr,WORD SizeX,WORD SizeY,WORD X,WORD Y,WORD Mode,void *bmp,WORD Best);
	void far (*LGRplot)(WORD X,WORD Y,WORD C,void *Bmp);
	void far (*LGRinvertRect)(void *Bmp,WORD X1,WORD Y1,WORD X2,WORD Y2);
	void far (*LGRassignColor)(WORD Num,BYTE Comp1,BYTE Comp2,BYTE Comp3,WORD Comp4,WORD Video);
	void far (*LDrawLine)(void *Bmp,WORD X1,WORD Y1,WORD X2,WORD Y2,WORD C);
	void far (*LPaintPoly)(WORD N,WORD points[][2],WORD C,void *Bmp);
	UWORD	pStartFill;
	UWORD	p3D_ymin;
	UWORD	p3D_ymax;
	void (*LSetAlias)(BOOL B);
	void (*LSelectGrad)(UBYTE *ptr);
	void (*LAssignFillMask)(WORD Pal[16]);
	void (*LSetColVAR)(WORD);
	UWORD	ptrLineL,ptrLineH;
	UWORD	FillSizeY;
	} GRAPHDRIVER;
#endif

//       +--------------------------------------------------------+
//       +                                                        +
//       +       Variables sp‚cifiques PC                         +
//       +                                                        +
//       +--------------------------------------------------------+

#ifndef	WINDOWS
extern	DMAport	*RefCh;
extern	IRQport	*RefIt;
extern	WORD	SoundIRQ;
extern	WORD	SoundDMA;
extern 	GRAPHDRIVER 	*Gdrivers[];
extern	BYTE 	NumGdriver[];
#endif

#define	memcpyXTD	memcpy
#define	export

#ifdef	WINDOWS
  #if !defined(__STRING_H)
    void * _Cdecl __memcpy__(void *__dest, const void *__src,size_t __n);
    void * _CType __memset__(void *__s, int __c, size_t __n);
  #endif
#endif

#define	OutDelay()	asm db 0xEB,0x00

extern	WORD	VESA_CurrPage;
extern	WORD	VESA_PageSize;
extern	BOOL	VESA_AltPage;
extern	void	(*VESA_PageFonction)(void);

#ifndef	WINDOWS
extern  BOOL		LoadGdriver(WORD Video);
extern  void 		FreeGdriver(WORD Video);
extern  void 		FreeGdrivers(void);
extern  void    	StartupFillMgr(void *Bmp);
extern  void    	SelectTrans(UWORD Color);
extern  GRAPHDRIVER 	*GetDriver(WORD Video);
extern  void	  	InitPaint();
#endif


#ifdef	_WINDOWS_
typedef	_Packed struct
{
	BITMAPINFOHEADER InfoHeader;
	RGBQUAD ColorTable[256];
}	WING_INFO;

extern	HINSTANCE	hInstance;
extern	HWND		hWndMain;
extern	HDC		hdcMDO;
extern	WNDCLASS  	WindowClass;
extern	WING_INFO	WinG_Info;
extern	BOOL		OkWING;
#ifdef	OKDCI
extern	BOOL		OkDCI;
extern	BOOL		LockDCI;
#endif

extern	void		DCIBeginAccess(WORD,WORD,WORD,WORD);
extern	void		DCIEndAccess(void);
extern	void		memsetDCI(LONG,UBYTE,UWORD);
extern	void		memcpy2DCI(LONG,void *,UWORD);
extern	void		memcpy2DCI_dbl(LONG,void *,UWORD,WORD);
extern	long		PASCAL DefaultMainWndProc(HWND,UINT,WPARAM,LPARAM);
extern	void		(*X_Repaint)(HDC,HWND);
#endif


//       +--------------------------------------------------------+
//       +                                                        +
//       +       Include _DEV_.H                                  +
//       +                                                        +
//       +--------------------------------------------------------+

typedef	_Packed struct
{
	char        ff_reserved[21];
	char        ff_attrib;
	unsigned    ff_ftime;
	unsigned    ff_fdate;
	long        ff_fsize;
	char        ff_name[128];
}	FFBLK;


extern	BOOL	FindFirst(char *,FFBLK *,int);
extern	BOOL	FindNext(FFBLK *);

extern	char	*strclip(char *,char *);

extern	void	InstallMultiH(int NbFiles,int keepH);
extern	void	CloseMultiH();
extern  	int  	GetRealH(int);

extern	char	*Path2Directory(char *);
extern	char	*Path2Name(char *);
extern	void	AppendFileList(char *file,char *clip,char **tab,int *nbf);


#ifndef	WINDOWS
  extern  WORD	NewHandle(BOOL);
  extern  void	CloseHiRam(void);
  extern  void	OpenHiRam(WORD nbh,ULONG size);
  extern  WORD	_AllocHi(HI_T *st,LONG size);
  extern  void	ReleaseHi(WORD h2);
  extern  LONG	HiMaxBlock(BOOL);
  extern  void	TransferHi(WORD h2,void *ptr,LONG size,WORD s);
  extern  void	_ReadHi(WORD h2,void *ptr,LONG size);
  extern  void	_WriteHi(WORD h2,void *ptr,LONG size);
  extern  void	ReadHi(WORD h2,void *ptr,LONG size);
  extern  void	SeekHi(WORD h2,LONG offset);
  extern  void	WriteHi(WORD h2,void *ptr,LONG size);
  extern  void	*HiLock(WORD h);
  extern  void	HiUnLock(WORD h,BOOL s);
  extern  WORD	SaveHi(void *ptr,WORD);
  extern  WORD	AllocHi(LONG size,BOOL b);
  extern  HI_H	*HiHandle(WORD h2);
#endif
