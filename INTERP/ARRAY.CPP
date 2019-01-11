//	array.cpp

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "sol.hpp"

#include	"array.hpp"
#include	"kernel.hpp"
#include	"math.hpp"
#include	"msg.hpp"
#include "object.hpp"
#include "pmachine.hpp"
#include "selector.hpp"

void
KArray(kArgs args)
{
	uint		length;
	ObjectID	objID;

	switch (arg(1)) {
		case ARRAYNEW:
			// arg(2) = array size
			// arg(3) = array type
			pm.acc = (Acc) ArrayNew(arg(2), arg(3));
			break;

		case ARRAYSIZE:
			// arg(2) = array datablock
			ArrayVerify(arg(2), ARRAYSIZE);
			pm.acc = (Acc) ((ArrayID) arg(2))->size;
			break;

		case ARRAYAT:
			// arg(2) = array datablock
			// arg(3) = position in array
			ArrayVerify(arg(2), ARRAYAT);
			if (arg(3) > ((ArrayID) arg(2))->size - 1) {
#ifdef DEBUG
				msgMgr->Alert("Attemping to read element %d in array of size %d", arg(3) + 1, ((ArrayID) arg(2))->size - 1);
#else
				msgMgr->Alert(SrcLoc,Msg_InvalidArrayElement);
#endif

				pm.acc = 0;
			}
			else
				pm.acc = (Acc) ArrayAt(((ArrayID) arg(2)).Data(), ((ArrayID) arg(2))->elementSize, arg(3));
			break;

		case ARRAYATPUT:
			// arg(2) = array datablock
			// arg(3) = position in array
			// arg(4) = new value for position
			ArrayVerify(arg(2), ARRAYATPUT);
			CheckArrayResizeBounds("Array Resize", arg(3), (argCount - 3));
			ArrayResize((ArrayID) arg(2), arg(3) + argCount - 3);
			ArrayAtPut(((ArrayID) arg(2)).Data(), ((ArrayID) arg(2))->elementSize, arg(3), argCount - 3, (char*) &arg(4));
			pm.acc = arg(2);
			break;

		case ARRAYFREE:
			// arg(2) = array datablock
			ArrayVerify(arg(2), ARRAYFREE);
			((ArrayID) arg(2)).Free();
			break;

		case ARRAYFILL:
			// arg(2) = array datablock
			// arg(3) = position in array to start fill
			// arg(4) = length to fill
			// arg(5) = value to fill with
			ArrayVerify(arg(2), ARRAYFILL);
			length = ArrayOpLen(((ArrayID) arg(2))->size, arg(3), arg(4));
	      CheckArrayResizeBounds("Array Resize", arg(3), length);
			ArrayResize((ArrayID) arg(2), arg(3) + length);
			ArrayFill(((ArrayID) arg(2)).Data(), ((ArrayID) arg(2))->elementSize, arg(3), length, arg(5));
			pm.acc = arg(2);
			break;

		case ARRAYCPY:
			// arg(2) = destination array datablock
			// arg(3) = position in destination array to start copy
			// arg(4) = source array datablock
			// arg(5) = position in source array to start copy
			// arg(6) = length to copy
			ArrayVerify(arg(2), ARRAYCPY);
			ArrayVerify(arg(4), ARRAYCPY);
			length = ArrayOpLen(((ArrayID) arg(4))->size, arg(5), arg(6));
			CheckArrayResizeBounds("Array Resize", arg(3), length);
			ArrayResize((ArrayID) arg(2), arg(3) + length);
			ArrayCopy(((ArrayID) arg(2)).Data(), ((ArrayID) arg(2))->elementSize, arg(3), ((ArrayID) arg(4)).Data(), arg(5), length);
			pm.acc = arg(2);
			break;

		case ARRAYCMP:
			break;

		case ARRAYDUP:
			// arg(2) = source data
			ArrayVerify(arg(2), ARRAYDUP);
			pm.acc = (Acc) ArrayDup(arg(2));
			break;

		case ARRAYGETDATA:
			// arg(2) = SCI object or handle to array block
			ArrayVerify(arg(2), ARRAYGETDATA);
			objID = (ObjectID) arg(2);
			if (objID.IsObject())
				pm.acc = objID.GetProperty(s_data);
			else
				pm.acc = arg(2);
			break;
	}
}


MemID
ArrayNew(uint n, int type)
/*
	Create a new array block of 'n' size and an element size determined
	by type 'type'.

	Returns: a MemID pointing to the new array.
*/
{
	ArrayID		dataID;
	uint			elSize;
	uint			hdrSize;

	switch (type) {
		case INTARRAY:
			elSize = 2;
			hdrSize = sizeof(Array);
			break;
		case BYTEARRAY:
			elSize = 1;
			hdrSize = sizeof(Array);
			break;
		case IDARRAY:
			elSize = 2;
			hdrSize = sizeof(Array);
			break;
		case STRARRAY:
			elSize = 1;
			hdrSize = 0;
			break;
		default:
			msgMgr->Fatal(SrcLoc, Msg_BadArrayType, type);
			break;
		}
	dataID.Get(elSize * n + hdrSize);
	if (type != STRARRAY) {
		dataID->size = n;
		dataID->elementSize = elSize;
	}

	return dataID;
}


SCIWord
ArrayAt(void* data, int esize, uint at)
/*
	Find the value at position 'at' in the array.

	Returns: The value.
*/
{
	void*	dp;

	if (data == NULL)
		return 0;

	dp = ArrayData(data, at, esize);

	if (esize == 1)
		return *(uchar*) dp;
	else
		return *(SCIWord*) dp;
}


void*
ArrayAtPut(void* data, int esize, uint at, uint n, char* args)
/*
	Set the value at position 'at' in the array.

	Returns: A pointer to the array data.
*/
{
	void*	dp;
	uint	i;

	if (data != NULL) {
		dp = ArrayData(data, at, esize);

		for (i = 0 ; i < n ; ++i) {
			if (esize == 1)
				*((char*) dp + i) = ((SCIWord *)args)[i];
			else 
				((SCIWord*) dp)[i] = ((SCIWord*) args)[i];
			}
		}

	return data;
}


void*
ArrayFill(void* data, int esize, uint from, size_t length, int value)
/*
	Fill a portion of the array determined by 'from' and 'length' with
	the value 'value'.

	Returns: Pointer to the array data.
*/
{
	void*	dp;
	uint	i;

	if (data != NULL) {
		dp = ArrayData(data, from, esize);

		if (esize == 1) {
			uchar *cp = (uchar*) dp;
			for (i = 0 ; i < length ; ++i)
				*cp++ = (uchar) value;
		} else {
			SCIWord* wp = (SCIWord*) dp;
			for (i = 0 ; i < length ; ++i)
				*wp++ = value;
		}
	}

	return data;
}


void*
ArrayCopy(void* dest, int esize, uint to, void* src, uint from, size_t length)
/*
	Copy a portion of one array to another.  The section to copy is
	determined by 'from' and 'length', and is copied to position 'to' in
	the destination array.

	Returns:	Pointer to the destination array data.
*/
{
	void*	fp;
	void*	tp;

	if (dest != NULL) {
		fp = ArrayData(src, from, esize);
		tp = ArrayData(dest, to, esize);
		if (length)
			memcpy(tp, fp, length * esize);
	}

	return dest;
}


MemID
ArrayDup(MemID srcID)
/*
	Duplicates an array block.

	Returns: MemID pointing to the new block.
*/
{
	MemID	destID;

	destID.Get(MemArray, srcID.Size());
	memcpy(*destID, *srcID, srcID.Size());
	return destID;
}


SCIWord
ArrayOpLen(uint size, uint n, size_t l)
/*
	Determines the length of an array operation, as determined by the size
	of the source array and length 'l'.  If 'l' is -1, the operation should
	include the entirety of the array.  Otherwise, the length of operation
	is 'l' elements.

	Returns: The appropriate length.
*/
{
	if (l != -1)
		return l;
	else if (n > size)
		return 0;
	else 
		return size - n;
}


void*
ArrayData(void* ptr, uint at, int esize)
/*
	Finds the position 'at' in the array.

	Returns: A pointer to the position.
*/
{
	if (ptr == NULL)
		return ptr;

	return ((char*) ptr + at * esize);
}


void
CheckArrayResizeBounds(char* str, uint in1, uint in2)
/*
	Determines if an array comprised parts whose lengths are 'in1' and 'in2'
	would be too large.

	Returns: Nothing.
*/
{
   if (in1 + in2 <= USHRT_MAX) 
      return;

	msgMgr->Fatal(SrcLoc, Msg_ArrayTooBig, str, in1, in2);
}


void
ArrayResize(ArrayID srcID, uint newSize)
/*
	Resizes the array data block.

	Returns:	Nothing.
*/
{
	int esize = srcID->elementSize;

	if (!(!srcID)) {
		if (newSize > srcID->size) {
			srcID.Realloc(newSize * esize + sizeof(srcID->elementSize) + sizeof(srcID->size));
			srcID->size = newSize;
		}
	}
}

void
ArrayVerify(ArrayID data, int operation)
{
	char	oper[12];

	if (!data) {
		switch (operation) {
			case ARRAYSIZE:
				strcpy(oper, "ArraySize");
				break;

			case ARRAYAT:
				strcpy(oper, "ArrayAt");
				break;

			case ARRAYATPUT:
				strcpy(oper, "ArrayAtPut");
				break;

			case ARRAYFREE:
				strcpy(oper, "ArrayFree");
				break;

			case ARRAYFILL:
				strcpy(oper, "ArrayFill");
				break;

			case ARRAYCPY:
				strcpy(oper, "ArrayCpy");
				break;

			case ARRAYCMP:
				strcpy(oper, "ArrayCmp");
				break;

			case ARRAYDUP:
				strcpy(oper, "ArrayDup");
				break;

			case ARRAYGETDATA:
				strcpy(oper, "ArrayGetData");
				break;
		}
  		msgMgr->Fatal("Attempted %s operation on array with no data", oper);
	}
}
		

