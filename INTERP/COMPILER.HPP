// compiler.hpp
//		compiler-specific information:  WATCOM 32-bit

#ifndef COMPILER_HPP
#define COMPILER_HPP

//	turn off WATCOM's "assignment found in boolean" warning: 'if (a = b())'
#pragma warning 391	9

// turn off WATCOM's "must look ahead ..a declaration/type or an expression"
#pragma warning 604  9
#pragma warning 595  9

// turn off WATCOM's warning about truncation when assigning ints to shorter
#pragma warning 389 9

// turn off WATCOM's warning about expression only useful for its side effects
#pragma warning 387 9

// turn off WATCOM's warning about sizeof() using compiler-generated info
#pragma warning 549 9

//	create a "type" that specifies that arguments are pushed on the stack
//	instead of being passed in registers.  Also, leave names alone.  Note
//	that this is necessary even if a function doesn't take arguments, else
//	the name will be changed.  The "*" prevents this.
#pragma aux PushArgs "*" parm caller [];

//	make an inline function called from within another inline function get
//	expanded inline
#pragma inline_recursion on;
#pragma inline_depth 255;

// Force the following functions to be inline 
//   (Assume <string.h> included before compiler.hpp)
#pragma intrinsic( memset );
#pragma intrinsic( memcpy );
#pragma intrinsic( memcmp );

// Boolean
typedef int	Bool;

//	data types for external data (files or OS data structures)
//	each contain the number of bits indicated
typedef short				Int16;
typedef unsigned short	UInt16;
typedef int					Int32;
typedef unsigned int		UInt32;

//	this should be in a WATCOM header file, but it's not, so...
extern "C" char** _argv;

//	heap checking assertion
#include <malloc.h>
#define HEAPCHK	assert(_heapchk() <= _HEAPEMPTY);

//	file functions
#define ffblk		find_t
#define ff_name	name
#define ff_attrib	attrib
#define ff_fsize	size
#define ff_ftime	wr_time

#define findfirst(path, struc, attrs)	_dos_findfirst(path, attrs, struc)
#define findnext(struc)						_dos_findnext(struc)

#define fnmerge	_makepath
#define fnsplit	_splitpath

struct  ftime   {
    unsigned    ft_tsec  : 5;   /* Two second interval */
    unsigned    ft_min   : 6;   /* Minutes */
    unsigned    ft_hour  : 5;   /* Hours */
    unsigned    ft_day   : 5;   /* Days */
    unsigned    ft_month : 4;   /* Months */
    unsigned    ft_year  : 7;   /* Year */
};

#endif
