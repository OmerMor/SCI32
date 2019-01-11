//***************************************************************************
//                                                                          *
//   Copyright (c)1995,M.D.O.   Programmeur:Mathieu___  CrÇation:??/??/??   *
//                                                                          *
//   Portage :  [X]:PC  [_]:MAC  [_]:CDI  [_]:3DO  [_]:SATURN  [_]:PSX      *
//              [_]:SpÇcifique                                              *
//                                                                          *
//   Type de source :  [_]:Interne   [_]:Externe   [X]:Grand public         *
//                                                                          *
//   Programme rattachÇ : [X]:Librairies  [_]:Loader                        *
//			  [_]:Autre : ______________                                       *
//                                                                          *
//***************************************************************************

#ifndef VMD_HPP
#define VMD_HPP

//#define	SLOOP_BASE	12		// UnitÇ de taille du cache
//#define	LOOP_BASE	4096	// en octet
#define	SLOOP_BASE	13		// UnitÇ de taille du cache
#define	LOOP_BASE	8192	// en octet

#define	VMD_DIFF			0x0001         // Codage par DiffÇrence
#define	VMD_RL			0x0003			// Run-Length           
#define	VMD_Y				0x0020			// Doublage Y           
#define	VMD_4				0x0040			// Quadruplage X 4plans 
#define	VMD_PACK			0x0080			// ComprÇssÇ            
#define	VMD_MSG			0x0100			// Message              
#define	VMD_PAL			0x0200			// Changement de palette
#define	VMD_VESA			0x0400			// Plus de 320x200      
#define	VMD_PACK0		0x0800			// Compression Çtendue  
#define	VMD_SOUND		0x1000			// VMD sonore           
#define	VMD_NOPAL2		0x2000			// Flag interne         
#define	VMD_MINILIST	0x4000			// Miniliste            


//       +--------------------------------------------------------+
//       +                                                        +
//       +       Retour de ViewIMD                                +
//       +                                                        +
//       +--------------------------------------------------------+
#define	VMD_NULL		0x010000L		// Rien Ö faire
#define	VMD_MOVE		0x020000L		// dÇplacement non linÇaire
#define	VMD_TIMEOUT	0x040000L		// Time-Out son
#define	VMD_BULLE	0x080000L		// Bulle
#define	VMD_ERROR	0x800000L		// N/I
#define	VMD_BEHIND	0x008000L		// Falling behind

//       +--------------------------------------------------------+
//       +                                                        +
//       +       Retour de ViewDelayIMD                           +
//       +                                                        +
//       +--------------------------------------------------------+
#define	VMD_WAITING	0x1234			// En attente synchro
#define	VMD_END		0x4321			// Fin du VMD

//       +--------------------------------------------------------+
//       +                                                        +
//       +       Flags de type de sons                            +
//       +                                                        +
//       +--------------------------------------------------------+
#define	VMD_SOUND_DECAL1		1		// PrÇsent toujours
#define	VMD_SOUND_SOUNDFIRST	2		// PrioritÇ Son
#define	VMD_SOUND16BITS		4		// Audio 16bits
#define	VMD_DECAL1				8		// DÇpart son image N¯ 1

//    	+--------------------------------------------------------+
//    	+ VMD_NOWAIT          0x100		Champ utilisÇ systäme   +
//    	+--------------------------------------------------------+
#define	CURRENT_SOUND	1			// Buffer son courant
#define	START_SOUND		2			// Buffer son dÇpart
#define	SILENT_SOUND	3			// Buffer son vide

//       +--------------------------------------------------------+
//       +                                                        +
//       +       Flags de type de block                           +
//       +                                                        +
//       +--------------------------------------------------------+
#define	SND_BLOCK		1			// Bloc Sonore 
#define	GRAPH_BLOCK		2			// Bloc Video
#define	DATA_BLOCK		3			// Bloc Data
#define	PREVIEW_BLOCK	4			// Bloc Preview
#define	BULLE_BLOCK		5			// Bloc Bulle

//       +--------------------------------------------------------+
//       +                                                        +
//       +       Paramêtres de OpenIMD                            +
//       +                                                        +
//       +--------------------------------------------------------+
#define	VMD_SOUNDFIRST	0x0004			// PrioritÇ son
#define	VMD_PICTFIRST	0x0008			// PrioritÇ image
#define	VMD_NOSOUND		0x0010			// Force sans son
#define	VMD_SKIPPLAY	0x0020			// Ne joue pas le son
#define	VMD_CACHE		0x0040			// VMD en cache
#define	VMD_NOPAL		0x0080			// Pas d'affect. palette
#define	VMD_NOWAIT		0x0100			// Retour immÇdiat ViewIMD
#define	VMD_PRELOAD		0x0200			// Mode PreLoad
#define	VMD_NOTRACK		0x0400			// Force hors Tracker
#define	VMD_FORCESOUND	0x0800			// Force lancement son

//    	+--------------------------------------------------------+
//    	+ RÇservÇ         0x1000   		Champ utilisÇ systäme   +
//    	+--------------------------------------------------------+
#define	VMD_AUTOPLAY				0x2000						// Joue tout seul
#define	VMD_DATA						0x4000						// Utilise les datas
#define	VMD_TIME(a)	(VMD_NOWAIT+0x8000+((LONG)a<<16L))	// Force vitesse

//    	+------------------------------------------------------------+
//    	+ RÇservÇ         0x00xx8000L    Bits Systäme correspondants +
//    	+------------------------------------------------------------+
#define	VMD_LOOPCACHE	(0x01000000L+VMD_CACHE)	// VMD en cache Loop
#define	VMD_ALLOCPTR	0x02000000L					// Allocation IMD_ptr


//       +--------------------------------------------------------+
//       +                                                        +
//       +       Flags interne des structures Cache               +
//       +                                                        +
//       +--------------------------------------------------------+
#define	CACHE_FLUSH			128		// A flusher lors du Close
#define	CACHE_LOCKED		64			// Non flushable
#define	CACHE_LOCK			4			// En remplissage
#define	CACHE_VDUPLICATE	2			// Ouvert plusieurs fois
#define	CACHE_USING			1			// En utilisation

//       +--------------------------------------------------------+
//       +                                                        +
//       +       Paramêtres globaux                               +
//       +                                                        +
//       +--------------------------------------------------------+
#define	VMD_8BITS	0x0001			// Force Son 8bits
#define	VMD_11K		0x0002			// Force Son 11khz

//       +--------------------------------------------------------+
//       +                                                        +
//       +       Tracker sur interruption ?                       +
//       +                                                        +
//       +--------------------------------------------------------+

#ifdef	WINDOWS
	#undef	INTERRUPT
#else
	#define	INTERRUPT
#endif

//       +--------------------------------------------------------+
//       +                                                        +
//       +       Structures des blocs divers                      +
//       +                                                        +
//       +--------------------------------------------------------+
typedef  _Packed struct
{
	WORD	X1,Y1,X2,Y2;
	UWORD	Mode;
}	PICTBLOC;


typedef	_Packed struct
{
	UBYTE	Mode;
	UBYTE	Null;
}	SOUNDBLOC;

typedef	_Packed struct
{
	WORD	Bulle;
	BYTE	Name_DAT[8];
}	BULLEBLOC;

typedef	_Packed struct
{
// Had to change the name "New" generate a compiler error
//	UBYTE	New;
	UBYTE	More;
	UBYTE	Null2;
	LONG	Size;
}	DATABLOC;

//       +--------------------------------------------------------+
//       +                                                        +
//       +       Structures de l'imd                              +
//       +                                                        +
//       +--------------------------------------------------------+
typedef	_Packed union
{
	PICTBLOC		Pict;
	SOUNDBLOC	Sound;
	DATABLOC		Data;
	BULLEBLOC	Bulle;
}	BLOCMORE;

typedef	_Packed struct
{
	UBYTE		Type;	// Type de blocs suivants
						// 0 = Fin
						// Bloc Son [Type]
						// Bloc Image [Type]
						// Bloc Data [Type]
	UBYTE		Id;
	LONG		Size;
	BLOCMORE More;
}	BLOCINFO;

typedef	_Packed struct
{
	UBYTE		Error;
	UBYTE		Message;
	LONG		Offset;
}	PICTINFO;

typedef	_Packed struct
{
	SOUND	Sound;
	BOOL	Zero[40];
//	UBYTE	*Buffer[40];
//	void	*Release;
	UBYTE	far *Buffer[40];
	void	far *Release;
	BYTE	CurrentBuffer;
	BYTE	Track;
	WORD	TotalBuffers;
//	UBYTE	*LastPtr;
	UBYTE	far *LastPtr;
	UWORD	LastSize;
	WORD	Level;
	UWORD	oBytes;
}	SND_I;

	
typedef	_Packed struct
{
	WORD  		h;						// 0
	WORD			Version;				// 2
	WORD			NbPict;				// 4
	WORD			X,Y;					// 6
	WORD			SizeX,SizeY;		// 10
	UWORD			Type;					// 14
	WORD			NbTrack;				// 16
	LONG			Start;				// 18
	COK_BITMAP*	Bmp;					// 22
	UBYTE			Pal[256][3];		// 26
	LONG			MaxPtr,MaxPtr2;	// 794

	WORD			Freq;					// 802
	UWORD			Bytes;				// 804
	WORD			NbBuffer;			// 806
	UWORD			SoundInfo;			// 808
	LONG			InfoOfs;				// 810

	UBYTE*		TempPal;
	PICTINFO*	PictInfo;
	BLOCINFO*	BlocInfo;
	WORD 			Pos;
	LONG			StartTime;
	LONG			StartCount;
	BOOL			ToAddTime,NotUsed;
	WORD			OpenInfo;
	BYTE			Name[16];
	LONG			FixedParams;
	WORD			RealFreq;
	WORD			TimeOut;

	SND_I			S;
}	IMD;


typedef	_Packed struct
{
	BOOL	Used,Null;
	IMD	*Imd;
//	UBYTE	*Start;
	UBYTE	far *Start;
//	UBYTE	*Ram;
	UBYTE	far *Ram;
	ULONG	Pos,Size;
	WORD	Freq;
	ULONG	FreqF,PosF;
	WORD	Loop;
	WORD	Stop,WaitPos;
	WORD	Level;
	WORD	NbBuff;
	WORD	Volume;
	WORD	GotoVolume;
	WORD	DecayTime;
	WORD	Chain;
	WORD	DecayTimeBase;
        WORD    BaseVolume;
}	TRACKI;

typedef	_Packed struct
{
	UWORD	LastSector;
	void	*LastPtr;
	WORD	NbSector;
	WORD	UsedSector;
	UWORD	NextSector;
	LONG	Cpos;
	UWORD	RealSize;
	UWORD	Sector[1];
}	CACHE_LOOP;

typedef	_Packed struct
{
	LONG	Cpos;
	BOOL	*Cached;
}	CACHE_NORMAL;

typedef	_Packed struct
{
	LONG	Size;
	BYTE	Name[9];
	BOOL	Used,Writing;
	IMD	*Imd;
	WORD	NbPict;
	WORD	Looping;
	LONG	Offset;
	LONG	Time;
	WORD	h;
	UBYTE	 *Data;
	CACHE_LOOP	*Loop;
	CACHE_NORMAL	*Normal;
}	CACHE;

//       +--------------------------------------------------------+
//       +                                                        +
//       +       Variables                                        +
//       +                                                        +
//       +--------------------------------------------------------+
extern	UWORD			IMD_break;
extern	BOOL			IMD_check;
extern	BOOL			IMD_get;
extern	WORD			IMD_msg;
extern	UBYTE*		IMD_ptr;
extern	UBYTE*		IMD_ptr2;
extern	WORD			IMD_X1,IMD_X2,IMD_Y1,IMD_Y2;
extern  	BOOL  		IMDinProcess;
extern	WORD			i_ChainChannel;
extern	SOUND*		i_ChainSound;
extern	WORD			i_CountPreload;
extern	BYTE*			i_DataLoad;
extern	LONG			i_FixedParams;
extern	IMD*			i_ImdLoading;
extern	BYTE			i_InsertFileDAT[];
extern	WORD			i_InsertBulle;
extern	WORD			i_LoopingCache;
extern	COK_BITMAP*	i_MiniBmp;
extern	WORD			i_NeedAssignC1,i_NeedAssignC2;
extern	WORD			i_SizeMax,i_SizeTotal,i_LoopFreeMem;
extern	UBYTE*		i_TempPal;
extern	UBYTE*		i_TempResample;
extern	IMD			i_Vmd;
extern  	WORD  		NextTrackLevel;
extern	BOOL			SoundTrack,LockTrack;
extern	BOOL			Track_NewPal;
extern	WORD			Track_X1,Track_Y1,Track_X2,Track_Y2;
extern	TRACKI		Tracks[4];
extern	int 			vmdDouble;
extern	void			(*X_DuringLoadCache)(void);
extern	void    		(*X_TrackLoop)(void);


//       +--------------------------------------------------------+
//       +                                                        +
//       +       Fonctions                                        +
//       +                                                        +
//       +--------------------------------------------------------+
extern	void	OpenLTK(char *);
extern	void	CloseLTK(void);

//			+--------------------------------------------------------+
//			+                                                        +
//			+     Fonctions de traitement des VMD                    +
//			+                                                        +
//			+--------------------------------------------------------+
extern	void			CloseIMD(IMD *);
extern	IMD*			OpenIMD(BYTE *,COK_BITMAP *,LONG);
extern	ULONG			ViewIMD(IMD *,WORD);
extern	ULONG			ViewIMDs(IMD *,WORD);

extern	BOOL			AllocIMD_ptr(IMD *);
extern	BOOL			CheckIMD_ptr(LONG S1,LONG S2);
extern	void			Full360(COK_BITMAP *,COK_BITMAP *,WORD,WORD,WORD,WORD,WORD,WORD);
extern	BLOCINFO*	GetActiveTrack(IMD *Imd,WORD n,WORD mode);
extern	void			Make360(IMD *,WORD,WORD,WORD);
extern	void			MoveIMD(IMD *Imd,WORD n,COK_BITMAP *Bmp,WORD x,WORD y);
extern	void			SetIMD_XY(IMD *,WORD,WORD);
extern	LONG			ViewDelayIMD(IMD *Imd);

//			+--------------------------------------------------------+
//			+                                                        +
//			+     Fonctions de crÇation de VMD                       +
//			+                                                        +
//			+--------------------------------------------------------+
extern	BOOL			BuildDiffMap(COK_BITMAP *Bmp);
extern	BOOL			CompressSoundTrack(BLOCINFO  *Bloc,WORD h,UBYTE *ptr,WORD nbbuf,WORD bytes,WORD n,BOOL Noise);
extern	BOOL			CreateIMD(COK_BITMAP *b,char *f,WORD m,void *(*fct)(WORD,WORD,LONG));
extern	void			FinishIMD(void);
extern	BOOL			WriteIMD(WORD,WORD,WORD,WORD,BOOL Diff,BOOL Pack,BOOL MiniList,BOOL Pal,UWORD mode);

//			+--------------------------------------------------------+
//			+                                                        +
//			+     Gestion du Cache IMD                               +
//			+                                                        +
//			+--------------------------------------------------------+
extern	void			CloseCache(void);
extern	void			FlushCache(void *);
extern	WORD			GetNextPictCached(void *);
extern	void			InitCache(WORD nb,WORD sizemax,WORD totalsize,WORD loopstay);
extern	BOOL			InCache(void *);
extern	void			LoadCache(void *,WORD);
extern	void			LoadInLooping(IMD *Imd,LONG time);
extern	LONG			TimeStayLooping(IMD *Imd);

//			+--------------------------------------------------------+
//			+                                                        +
//			+     Gestion du Tracker                                 +
//			+                                                        +
//			+--------------------------------------------------------+
extern	void			CloseTrack(void);
extern	void			InitTrack(WORD Saturation);
extern	void			SetTrackVolume(WORD ch,WORD Level,WORD Time);
extern	WORD			StartSoundT(WORD ch,SOUND *Snd,WORD nbr,WORD freq);
extern	void			StopSoundT(int ch,int val);
extern	BOOL			TrackInProcess(WORD);
extern	void			TrackLoop(void);

//			+--------------------------------------------------------+
//			+                                                        +
//			+     Gestion des pistes DATA                            +
//			+                                                        +
//			+--------------------------------------------------------+
extern	void			InitDataIMD(IMD *Imd);
extern	BOOL			IsDataLoaded(char *);


//			+--------------------------------------------------------+
//			+                                                        +
//			+     Gestion Auto-Loading                               +
//			+                                                        +
//			+--------------------------------------------------------+
extern	void			CheckLoading(char *str);
extern	void			EndLoading(void);
extern	void			LoadingLoop(void);
extern	void			StartLoading(char *str,WORD Xd,WORD Yd,COK_BITMAP *Bmp);

//			+--------------------------------------------------------+
//			+                                                        +
//			+     Gestion Vignettes                                  +
//			+                                                        +
//			+--------------------------------------------------------+
extern	void			CloseVIG(IMD *imd);
extern	IMD*			OpenVIG(char *file,COK_BITMAP *Bmp);
extern	void			ViewVIG(IMD *imd,WORD pict);

//			+--------------------------------------------------------+
//			+                                                        +
//			+     Gestion Musique                                    +
//			+                                                        +
//			+--------------------------------------------------------+
extern	void			ChainMusic(LONG,LONG);
extern	void			CloseMusic(void);
extern	BOOL			DisableMusic(LONG);
extern	void			EnableMusic(void);
extern	LONG			GetMusicTime(LONG);
extern	void			InitMusic(char *nom,LONG SegSize,LONG *TabTh,WORD Size);
extern	void			MusicLoop(void);
extern	void			PlayMusic(LONG,LONG);
extern	void			SetMusicLoop(LONG,LONG);
extern	void			StopMusic(int);

enum {
	// various SCIVMD event defines
	SCIVMD_END = 1,
	SCIVMD_ESC_PRESSED = 2,
	SCIVMD_MOUSE_DOWN = 4,
	SCIVMD_HOT_RECTANGLE = 8,
	SCIVMD_TO_FRAME = 16,
	SCIVMD_RETURN_INTERVAL = 32,
	SCIVMD_KILLFOCUS = 64
};

enum {
	// play parameters
	SCIVMD_DOUBLE = 1,
//	SCIVMD_NO_SKIPPING = 2,
	SCIVMD_BLACK_LINES = 4,
	SCIVMD_NO_AYSNC_CHECKS = 8,
	SCIVMD_RGB = 16,
	SCIVMD_LEAVE_SCREEN_BLACK = 32,
	SCIVMD_LEAVE_LAST_FRAME = 64
};

enum avi_command_list {
      SCIVMD_OPEN = 0,
      SCIVMD_PUT,
      SCIVMD_PLAY,								 // Obsolete
      SCIVMD_STOP,
      SCIVMD_PAUSE,
      SCIVMD_RESUME,
      SCIVMD_CLOSE,
	   SCIVMD_SETPALETTE,
	   SCIVMD_GET_LENGTH,
	   SCIVMD_GET_POSITION,
	   SCIVMD_GET_STATUS,
	   SCIVMD_CUE,
	   SCIVMD_SEEK,
	   SCIVMD_FRAMES_SKIPPED,
	   SCIVMD_WAIT_EVENT,
	   SCIVMD_PUT_DOUBLE,						 // Obsolete
	   SCIVMD_CURSOR,
	   SCIVMD_START_BLOB,
	   SCIVMD_STOP_BLOBS,
	   SCIVMD_ADD_BLOB,
	   SCIVMD_DELETE_BLOB,
		SCIVMD_BLACK,
		SCIVMD_SKIP,
	   SCIVMD_RESTRICT_PALETTE
};

enum vmd_status {
	statusNotOpen,
	statusOpen,
	statusPlaying,
	statusPaused,
	statusStopped,
	statusDone,
};

const unsigned int SCIVMD_ERROR = 0xffff;

extern			int vmdBlackLines;
extern			int vmdDouble;
extern			int vmdNoAsync;
extern			int vmdSkipOn;
extern			int vmdRGB;
extern			int vmdRGBStart;
extern			int vmdRGBEnd;
extern			int vmdPaletteStart;
extern			int vmdPaletteEnd;
extern			int vmdLeaveScreenBlack;
extern			int vmdBehind;
extern			int vmdAhead;

#endif
