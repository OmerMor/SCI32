//	sol.hpp	5/25/93
//		standard header for all Sierra On-Line C++ programs

#ifndef SOL_HPP
#define SOL_HPP

//	for size_t
#include <stddef.h>

//	establish compiler-dependent types and other information
#include "compiler.hpp"

//	abbreviations
typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned int		uint;
typedef unsigned long	ulong;

//	data types for interfacing to the SCI language, in which all values
//	are 16 bits
typedef Int16		SCIWord;
typedef UInt16		SCIUWord;

#if !defined(_TV_VERSION) && !defined(__TCOLLECT_H)
const Bool	True = 1;
const Bool	False = 0;
#endif

template<class T, class S>
inline S Max(S	a, T b)
{
	return a > b ? a : b;
}

template<class T, class S>
inline S Min(S	a, T b)
{
	return a < b ? a : b;
}

//	calculate the number of elements in an array whose size is known
#define NEls(array)	(sizeof (array) / sizeof *(array))

//	checks for success, and aborts if fails
void*				operator new(size_t size, char*, int);
#define New		new(__FILE__,  __LINE__)

//	Each implementation should define its own AssertFail().
//	Normally, it will simply call the msgMgr, but we give it its own function
//	so anyone can use assert() without having to drag MSG.HPP around.
int AssertFail(char* file, int line, char* expression);

#ifdef DEBUG
#define assert(expr)  (expr) || AssertFail(__FILE__, __LINE__, #expr)
#else
#define assert(expr)
#endif

#endif
