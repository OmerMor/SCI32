/*
 * dcs386fc.h
 *
 *	$Header:   I:/sys/sci/vcs/dcomp.hpp   1.1   29 Dec 1994 14:24:26   jasonh  $
 *
 *	$Log:   I:/sys/sci/vcs/dcomp.hpp  $
   
      Rev 1.1   29 Dec 1994 14:24:26   jasonh
   Added DOS linefeeds from Macintosh
   
      Rev 1.0   05 Nov 1993 17:04:58   kenk
   Initial revision.
 * 
 *    Rev 1.0   18 Oct 1991 12:43:18   ScottK
 * Initial revision.
 * 
 *
 */
#define SCRATCHRAM_SIZE 10496		/* Size of required buffer for scratchRAM
										This must be 10496 for standard dcsd32
										This must be ????? for fast dcsd32 */

/*** Prototypes ***/
extern "C" {
void Initcompress(char *scratchRAM);
void Compress(
		char			**src,
		char			**dst,
		unsigned long	*srcCnt,
		unsigned long	*dstCnt,
		char			*scratchRAM
		);
int Decompress(
		char			**src,
		char			**dst,
		unsigned long	*srcCnt,
		unsigned long	*dstCnt,
		char			*scratchRAM
		);

};

#pragma aux (PushArgs) Initcompress;
#pragma aux (PushArgs) Compress;
#pragma aux (PushArgs) Decompress;
