/****************************************************************************
*
*						 	  PM/Lite Library
*
*					Copyright (C) 1994 SciTech Software.
*							All rights reserved.
*
* Filename:		$RCSfile: pmode.h $
* Version:		$Revision: 1.2 $
*
* Language:		ANSI C
* Environment:	Real mode and 16/32 bit Protected Mode under MSDOS
*
* Description:	Header file for the DOS extender independant protected
*				mode programming library. This library will need to be
*				included in all programs that use SciTech Software's
*				products that are to be compiled in protected mode.
*
*				This library provides pre-built selectors for the BIOS
*				data area and VGA frame buffer, and methods for allocating
*				your own selectors for physical memory. It also returns
*				appropriate selectors for accessing memory allocated in
*				the low DOS 1Mb memory area, and routines for accessing
*				memory through a selector and offset. By using selectors
*				for accessing memory outside of the applications linear
*				address space, your code will be fully DPMI compliant and
*				will run under Windows 3.1 and OS/2 2.x DOS boxes.
*
*				Works with the following:
*
*					Real Mode DOS (large memory model)
*
*               16 bit Extenders:
*
*					Windows 3.1 DPMI
*					Borland DPMI16
*
*               32 bit Extenders:
*
*					Windows 3.1 DPMI (Win32s)
*					Phar Lap TNT DOS Extender
*					Symantec DOSX
*					FlashTek X-32/X-32VM
*					Borland DPMI32
*					Rational DOS/4GW
*                   DJGPP go32 for GNU C++          * NO SELECTORS YET!! *
*
*				Currently supports the following compilers:
*
*					Borland C++ 3.1
*					Borland C++ 4.0, 16 bit
*					Borland C++ 4.0, 32 bit
*                   Microsoft Visual C++ 1.5, 16 bit
*                   Microsoft Visual C++ 1.5, 32 bit
*					Symantec C++ 6.1, 16 bit
*					Symantec C++ 6.1, 32 bit
*					Watcom C++ 10.0, 16 bit
*                   Watcom C++ 10.0, 32 bit
*					Metaware High C++ 3.21, 32 bit
*					DJGPP port of GNU C++, 32 bit	* NO SELECTORS YET!! *
*
* $Id: pmode.h 1.2 1995/03/28 16:16:45 kjb release $
*
****************************************************************************/

#ifndef	__PMODE_H
#define	__PMODE_H

#ifndef	__DOS_H
#include <dos.h>
#endif

/*--------------------------- Macros and Typedefs -------------------------*/

/* You will need to define one of the following before you compile this
 * library for it to work correctly with the DOS extender that you are
 * using. If none is specified, it is assumed you are compiling for DOS
 * real mode.
 *
 *		REALMODE	- Dos real mode
 *		WINDPMI16	- Windows 3.1 16 bit DPMI
 *      DPMI16      - Borland's DPMI16 DOS Power Pack Extender
 *		WINDPMI32	- Windows 3.1 32 bit DPMI
 *		TNT			- Phar Lap TNT DOS Extender
 *		DOSX		- Symantec C++ DOSX
 *		X32VM		- Flashtek X-32/X-32VM
 *      DPMI32      - Borland's DPMI32 DOS Power Pack Extender
 *		DOS4GW		- Rational DOS/4GW and DOS/4GW Professional
 *		DJGPP		- DJGPP port of GNU C++
 *
 * One of the following will be defined automatically for you when in
 * protected mode (REALMODE will be defined otherwise):
 *
 *		PM286	- 286 protected mode
 *		PM386	- 386 protected mode
 */

#if defined(TNT) || defined(DOSX) || defined(X32VM) || defined(DPMI32)		\
	|| defined(DOS4GW) || defined(DJGPP) || defined(WINDPMI32)				\
	|| defined(__386__) || defined(__FLAT__) || defined(__NT__)
#define	PM386
#elif defined(DPMI16) || defined(WINDPMI16)
#define	PM286
#else
#define	REALMODE
#endif

#pragma pack(1)

/* Provide definitions for the real mode register structures passed to
 * the PM_int86() and PM_int86x() routines.
 */

#if defined(REALMODE) || defined(PM286)
typedef union REGS RMREGS;
typedef struct SREGS RMSREGS;
typedef union REGS PMREGS;
typedef struct SREGS PMSREGS;
#else
struct _RMWORDREGS {
	unsigned short ax, bx, cx, dx, si, di, cflag;
	};

struct _RMBYTEREGS {
	unsigned char   al, ah, bl, bh, cl, ch, dl, dh;
	};

typedef union {
	struct  _RMWORDREGS x;
	struct  _RMBYTEREGS h;
	} RMREGS;

typedef struct {
	unsigned short  es;
	unsigned short  cs;
	unsigned short  ss;
	unsigned short	ds;
	} RMSREGS;

struct _PMDWORDREGS {
	unsigned int eax,ebx,ecx,edx,esi,edi,cflag;
	};

struct _PMWORDREGS {
	unsigned short 	ax,ax_hi;
	unsigned short 	bx,bx_hi;
	unsigned short 	cx,cx_hi;
	unsigned short 	dx,dx_hi;
	unsigned short 	si,si_hi;
	unsigned short 	di,di_hi;
	unsigned short	cflag,cflag_hi;
	};

struct _PMBYTEREGS {
	unsigned char	al, ah;	unsigned short:16;
	unsigned char	bl, bh;	unsigned short:16;
	unsigned char	cl, ch; unsigned short:16;
	unsigned char	dl, dh; unsigned short:16;
	};

typedef union {
	struct  _PMDWORDREGS e;
	struct  _PMWORDREGS  x;
	struct  _PMBYTEREGS  h;
	} PMREGS;

typedef struct {
	unsigned short  es;
	unsigned short  cs;
	unsigned short  ss;
	unsigned short	ds;
	unsigned short	fs;
	unsigned short	gs;
	} PMSREGS;
#endif

#ifdef DJGPP
#define	_cdecl		/* GCC doesn't know about _cdecl modifiers			*/
#endif

/* For the Metaware High C/C++ compiler, there is no _cdecl calling
 * convention. The conventions can be changed, but it is a complicated
 * process involving #pragmas, and all externally referenced functions
 * will use stack based calling conventions. We also need to change the
 * global aliasing conventions to use underscores for external function
 * and variables names, so that our assembler routines will link
 * correctly (except of course the main function - man what a PAIN!).
 */

#ifdef	__HIGHC__
#define	_cdecl
#pragma Global_aliasing_convention("_%r")
extern main();
#pragma Alias(main,"main")
#endif

#ifndef	__MSDOS__
#define	__MSDOS__
#endif

/* Define a macro for creating physical base addresses from segment:offset */

#define MK_PHYS(s,o)  (((unsigned long)(s) << 4) + (unsigned long)(o))

/* Define the different types of modes supported. This is a global variable
 * that can be used to determine the type at runtime which will contain
 * one of these values.
 */

typedef enum {
	PM_realMode,
	PM_286,
	PM_386,
	} PM_mode_enum;

extern int _PM_modeType;

/* Define a macro for changing the default stack size. For some compilers
 * and extenders this is done at link time, for others you need to change
 * a global variable (Borland C++, X-32 with any compiler). For compilers
 * that do it on the command line, this expands to empty.
 */

#if		defined(__TURBOC__)
#define	DECLARE_STACK(s16,s32)	\
extern unsigned _stklen = s16;
#elif	defined(X32VM)
#define	DECLARE_STACK(s16,s32)	\
unsigned int _x32_stack_size = s32;
#else
#define DECLARE_STACK(s16,s32)
#endif

/*--------------------------- Function Prototypes -------------------------*/

#ifdef	__cplusplus
extern "C" {			/* Use "C" linkage when in C++ mode	*/
#endif

/* Routines to access data through a selector and offset. For real mode
 * and 16 bit protected mode, the offset can only be a maximum of 64k.
 */

unsigned char _cdecl PM_getByte(unsigned s, unsigned o);
unsigned short _cdecl PM_getWord(unsigned s, unsigned o);
unsigned long _cdecl PM_getLong(unsigned s, unsigned o);
void _cdecl PM_setByte(unsigned s, unsigned o, unsigned char v);
void _cdecl PM_setWord(unsigned s, unsigned o, unsigned short v);
void _cdecl PM_setLong(unsigned s, unsigned o, unsigned long v);

/* Routines for copying data between the applications data space and
 * memory accessible through a selector and offset.
 */

void _cdecl PM_memcpynf(void *dst,unsigned src_s,unsigned src_o,unsigned n);
void _cdecl PM_memcpyfn(unsigned dst_s,unsigned dst_o,void *src,unsigned n);

/* Routine to return a selector to the BIOS data area at segment 0x40 */

unsigned _cdecl PM_getBIOSSelector(void);

/* Routine to return a selector to the VGA frame buffer. The selector
 * will map to the correct portion of video memory depending on the
 * current video mode (0x3, 0x7 or graphics).
 */

unsigned _cdecl PM_getVGASelector(void);

/* Routines to return specific selectors to the VGA frame buffer memory */

unsigned _cdecl PM_getVGAColorTextSelector(void);
unsigned _cdecl PM_getVGAMonoTextSelector(void);
unsigned _cdecl PM_getVGAGraphSelector(void);

/* Routines to map/free physical memory into the current DS segment. After
 * the mapping has been allocated, it cannot be freed. Hence you should
 * only allocate the mapping once and cache the value for use by other
 * parts of your application. If this cannot be done, this function will
 * return a NULL pointer.
 *
 * This routine will also work for memory addresses below 1Mb, but the
 * memory cannot cross the 1Mb boundary.
 */

void * _cdecl PM_mapPhysicalAddr(long base,long limit);

/* Routine to load save default data segment selector value into a code
 * segment variable, and another to load the value into the DS register.
 */

void _cdecl PM_loadDS(void);
void _cdecl PM_saveDS(void);

/* Routine to get a selector:offset for accessing a low 1Mb memory block.
 * You dont need to free this pointer, but in 16 bit protected mode
 * the selector allocated will be re-used the next time this routine is
 * called. If you need a permanent selector, allocate it with
 * PM_createSelector instead.
 */

void _cdecl PM_mapRealPointer(unsigned *sel,unsigned *off,unsigned r_seg,
	unsigned r_off);

/* Routine to create an arbritray selector to physical memory. In 16 bit
 * protected mode you can pass a 32 bit limit to create a 32 bit
 * protected mode data selector.
 */

unsigned _cdecl PM_createSelector(unsigned long base,unsigned long limit);
void _cdecl PM_freeSelector(unsigned sel);

/* Routine to create 32 bit code segment alias selector from a data
 * selector. This is only useful for 16 bit protected mode code that
 * needs to be able to call 32 bit protected mode code in a USE32 segment.
 * The selector can be freed with PM_freeSelector();
 */

#ifdef	PM286
unsigned _cdecl PM_createCode32Alias(unsigned sel);
#endif

/* Routine to allocate a block of conventional memory below the 1Mb
 * limit so that it can be accessed from real mode. Ensure that you free
 * the segment when you are done with it.
 *
 * This routine returns a selector and offset to the segment that has been
 * allocated, and also returns the real mode segment and offset which can
 * be passed to real mode routines. Will return 0 if memory could not be
 * allocated.
 *
 * Please note that with some DOS extenders, memory allocated with the
 * following function cannot be freed, hence it will be allocated for the
 * life of your program. Thus if you need to call a bunch of different
 * real-mode routines in your program, allocate a single large buffer at
 * program startup that can be re-used throughout the program execution.
 */

int _cdecl PM_allocRealSeg(unsigned size,unsigned *sel,unsigned *off,
	unsigned *r_seg,unsigned *r_off);
void _cdecl PM_freeRealSeg(unsigned sel,unsigned off);

/* Routine to call a real mode assembly language procedure. Register
 * values are passed in and out in the 'regs' and 'sregs' structures. We
 * do not provide any method of copying data from the protected mode stack
 * to the real mode stack, so if you need to pass data to real mode, you will
 * need to write a real mode assembly language hook to recieve the values
 * in registers, and to pass the data through a real mode block allocated
 * with the PM_allocRealSeg() routine.
 */

void _cdecl PM_callRealMode(unsigned seg,unsigned off, RMREGS *regs,
	RMSREGS *sregs);

/* Routines to generate real mode interrupts using the same interface that
 * is used by int86() and int86x() in realmode. This routine is need to
 * call certain BIOS and DOS functions that are not supported by some
 * DOS extenders. No translation is done on any of the register values,
 * so they must be correctly set up and translated by the calling program.
 *
 * Normally the DOS extenders will allow you to use the normal int86()
 * function directly and will pass on unhandled calls to real mode to be
 * handled by the real mode handler. However calls to int86x() with real
 * mode segment values to be loaded will cause a GPF if used with the
 * standard int86x(), so you should use these routines if you know you
 * want to call a real mode handler.
 */

int _cdecl PM_int86(int intno, RMREGS *in, RMREGS *out);
int _cdecl PM_int86x(int intno, RMREGS *in, RMREGS *out,RMSREGS *sregs);

/* Routines to generate native interrupts (ie: protected mode interrupts
 * for protected mode apps) using an interface the same as that use by
 * int86() and int86x() in realmode. These routines are required because
 * many 32 bit compilers use different register structures and different
 * functions causing major portability headaches. Thus we provide our
 * own and solve it all in one fell swoop ;-)
 */

void _cdecl PM_segread(PMSREGS *sregs);
int _cdecl PM_int386(int intno, PMREGS *in, PMREGS *out);
int _cdecl PM_int386x(int intno, PMREGS *in, PMREGS *out,PMSREGS *sregs);

/* Functions to manage real mode interrupt vectors */

void _cdecl _PM_getRMvect(int intno, long *realisr);
void _cdecl _PM_setRMvect(int intno, long realisr);

#ifdef	__cplusplus
}						/* End of "C" linkage for C++	*/
#endif

#pragma pack()

#endif /* __PMODE_H */
