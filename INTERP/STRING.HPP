//	string.hpp

#ifndef STRING_HPP
#define STRING_HPP

#include <stdarg.h>

#ifndef KERNEL_HPP
#include "kernel.hpp"
#endif

#ifndef OBJECT_HPP
#include "object.hpp"
#endif

#ifndef TEXTID_HPP
#include "textid.hpp"
#endif

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

inline TextID
StrGetData(MemID id)
{
//	following code fails to compile with CodeWarrior 1.1.1.2.
//	ObjectID obj(id);
//	return (TextID) (obj.IsObject() ? obj.GetProperty(s_data) : obj);
//	replaced code with the following...

	ObjectID	obj = (ObjectID) (id);
	if (obj.IsObject())
		return obj.GetProperty(s_data);
	else
		return (TextID) id;
}

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