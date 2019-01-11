// pmip.hpp
// 	pmachine instruction pointer

#ifndef PMIP_HPP
#define PMIP_HPP

#ifndef MEMID_HPP
#include "memid.hpp"
#endif

class SaveGame;

struct PMIP
{
	static void	Init();

	PMIP() : ptr(0)
	{
	}
	
	PMIP(MemID id, size_t offset) : id(id), ptr(&id[offset])
	{
	}
	
	PMIP& operator --()
	{
		ptr--;
		return *this;
	}
	
	PMIP&	operator ++()
	{
		ptr++;
		return *this;
	}
	
	PMIP&	operator ++(int)
	{
		return operator++();
	}
	
	uchar& operator *() const
	{
		return (uchar&) *ptr;
	}
	
	uchar& operator[](int ofs) const
	{
		return (uchar&) ptr[ofs];
	}
	
	PMIP&	operator +=(int ofs)
	{
		ptr += ofs;
		return *this;
	}
	
	PMIP operator +(int ofs) const
	{
		return PMIP(id, ptr + ofs);
	}
	
	void Clear()
	{	
		id			= 0;
		ptr		= 0;
		stackPtr = stack;
	}
	
	uchar GetUChar()
	{
		uchar c = operator *();
		ptr++;
		return c;
	}
	
	char GetChar()
	{
		char c = operator *();
		ptr++;
		return c;
	}
	
	SCIWord GetWord()
	{
		SCIWord c = (SCIWord&) operator*();
		ptr += sizeof(SCIWord);
		return c;
	}
	
	SCIUWord	GetUWord()
	{
		SCIUWord c = (SCIUWord&) operator*();
		ptr += sizeof(SCIUWord);
		return c;
	}
	
	size_t Offset() const
	{
		return ptr - (char*) *id;
	}
	
	uchar PeekChar() const
	{
		return **this;
	}
	
	uchar PeekNextChar() const
	{
		return (*this)[1];
	}
	
	SCIUWord	PeekNextUWord() const
	{
		return (SCIUWord&) (*this)[1];
	}
	
	void Push()
	{
		*stackPtr++ = *this;
	}
	
	void Pop()
	{
		*this = *--stackPtr;
	}

	size_t	ScriptOffset() const;

	PMIP* StackPtr() const
	{
		return stackPtr;
	}
	
	void	Lock();
	void	SaveRestoreBefore(SaveGame*, Bool);
	void	SaveRestoreAfter(SaveGame*, Bool);
	void	SetOffsetFromPtr();
	void	SetPtrFromOffset();
	void	Unlock();

	static void		TossMemID(MemID id, unsigned scriptNum);
	static char*	MakeStackDspStr(char* dest);

	enum { MaxStack = 100 };

private:
//	PMIP(MemID id, char* ptr);
	PMIP(MemID id, char* ptr) : id(id), ptr(ptr)
	{
	}

	MemID		id;		//	id to code
	char*		ptr;		//	memory location of current instruction
	size_t	offset;	//	used when locking and unlocking for save/restore

	static PMIP		stack[];
	static PMIP		retPMIP;			//	points to a ret opcode
	static PMIP		tossRetPMIP;	//	points to a toss, ret sequence
	static PMIP*	stackPtr;
};

#endif
