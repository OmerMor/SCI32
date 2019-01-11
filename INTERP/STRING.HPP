//	string.hpp

#ifndef	STRING_HPP
#define	STRING_HPP

#include <stdarg.h>

#ifndef KERNEL_HPP
#include "kernel.hpp"
#endif

class TextID;

// Subfunction codes
enum {
	STRNEW,
	STRSIZE,
	STRAT,
	STRATPUT,
	STRFREE,
	STRFILL,
	STRCPY,
	STRCMP,
	STRDUP,
	STRGETDATA,
	STRLEN,
	STRFORMAT,
	STRFORMATAT,
	STRTOINT,
	STRTRIM,
	STRUPR,
	STRLWR,
	STRTRN,
	STRTRNEXCLUDE
	};

// Defines for whitespace trimming
#define	L_TRIM	4
#define	C_TRIM	2
#define	R_TRIM	1


char*		StrTrim(char* str, int trimFlags = L_TRIM | R_TRIM, char showChar = '\0');
Bool		StrTrn(char* srcID, char* srcPat, char* destPat, char* destID);
Bool		StrTrnExclude(char* srcID, char* srcPat, char* destPat, char* destID);
int		StrStrCount(char* str, char* pattern);
void		StrResize(TextID srcID, uint newSize);

char*		Reverse(char* str);
Bool		IsPrintable(char* str);
uchar		ToLower(uchar c);
va_list*	MakeVprintfArgs(char* fmt, kArgs args);

#endif
