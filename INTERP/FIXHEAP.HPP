//	fixheap.hpp		Mark Wilden		4/23/92
//		heap for fixed size entries

#ifndef FIXHEAP_HPP
#define FIXHEAP_HPP

#ifndef MEMID_HPP
#include "memid.hpp"
#endif

const MaxFixHeaps = 50;


class FixedHeap {
public:
	FixedHeap(ushort nodeSize, ushort nodesPerBlock);
	~FixedHeap();

	void* Alloc();
	void 	Free(void*);

	static	void*		addresses[MaxFixHeaps];
	static	MemID		memids[MaxFixHeaps];

protected:
	class Block {
	public:
		Block(ushort nodeSize, ushort nodesPerBlock, Block*& nextBlock);
		~Block();
		void* 	operator new(size_t);
		void		operator delete(void*);

		Bool		IsValid();
		void*		Alloc();
		Bool		Free(void*);

		Block*	next;

	protected:
		char*			avail;
		char*			base;
		size_t		size;
	};

	static	Bool		firstTime;

	Block*	head;
	ushort	nodeSize;
	ushort	nodesPerBlock;
};

#endif
