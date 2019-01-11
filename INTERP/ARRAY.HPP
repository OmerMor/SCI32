//	array.hpp

#ifndef	ARRAY_HPP
#define	ARRAY_HPP

#ifndef MEMID_HPP
#include "memid.hpp"
#endif

enum ArrayType {
	INTARRAY,
	IDARRAY,
	BYTEARRAY,
	STRARRAY
	};

_Packed struct Array {
	SCIUWord elementSize;
	SCIUWord size;
	};

FakeTemplateID(FakeArrayID, Array, MemArray)
struct ArrayID : FakeArrayID {
	ArrayID() {}
	ArrayID(MemID& id) : FakeArrayID(id) {}
	ArrayID(SOL_Handle h) : FakeArrayID(h) {}

	void
	Get(size_t size)
	{
		// initialize arrays to 0

		//	include defaulted args because CodeWarrior 1.1.1.2 doesn't
		//	consider enums a separate type and so can't distinguish between
		//	overloaded Get()s.
		FakeArrayID::Get(size, MOVEABLE | INITMEMORY, 0, 0, 0, IllegalHandle);
	}

	void*	Data() { return (int) *this ? **this + 1 : 0; }
};

// Subfunction codes
enum {
	ARRAYNEW,
	ARRAYSIZE,
	ARRAYAT,
	ARRAYATPUT,
	ARRAYFREE,
	ARRAYFILL,
	ARRAYCPY,
	ARRAYCMP,
	ARRAYDUP,
	ARRAYGETDATA
	};


MemID			ArrayNew(uint n, int type);
SCIWord		ArrayAt(void* data, int esize, uint n);
void*			ArrayAtPut(void* data, int esize, uint at, uint n, char* args);
void*			ArrayFill(void* data, int esize, uint from, size_t length, int value);
void*			ArrayCopy(void* data, int esize, uint to, void* src, uint from, size_t length);
MemID			ArrayDup(MemID source);
SCIWord		ArrayOpLen(uint size, uint n, size_t l);
void*			ArrayData(void* ptr, uint at, int esize);
void  		CheckArrayResizeBounds(char* str, uint in1, uint in2);
void			ArrayResize(ArrayID id, uint newSize);
void			ArrayVerify(ArrayID data, int operation);

#endif
