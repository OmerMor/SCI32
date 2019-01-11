//	pmstack.hpp

#ifndef PMSTACK_HPP
#define PMSTACK_HPP

#ifndef MEMID_HPP
#include "memid.hpp"
#endif

#ifndef MSG_HPP
#include "msg.hpp"
#endif

#ifndef OBJECT_HPP
#include "object.hpp"
#endif

struct PMStack {
	enum { Size = 0x1000 };

	typedef SCIWord	Entry;
	typedef Entry*		Ptr;

	void	Init();
	void	End();

	void		Clear();
	size_t	ParmsOffset();

	Property Peek()
	{
		return *ptr;
	}
	
	size_t	PtrOffset();

	void		SaveRestore(SaveGame*, Bool save);
	size_t	TempsOffset();

	void CheckOverflow()
	{
		if (ptr + 1 >= stackBase + Size)
			msgMgr->Fatal(SrcLoc, Msg_StackBlown);
	}
	
	void CheckUnderflow()
	{
		if (ptr - 1 < stackBase)
			msgMgr->Fatal(SrcLoc, Msg_StackUnderflow);
	}
	
	Entry Pop()
	{
		return *ptr--;
	}

	void Push(Entry p)
	{
		*++ptr = p;
	}

	void PushMany(Entry* p, int n)
	{
		memcpy(ptr + 1, p, n * sizeof(Entry));
		ptr += n;
	}

	Ptr		ptr;
	Ptr		parms;
	MemID		stack;
	Ptr		stackBase;
	Ptr		temps;
};

#endif
