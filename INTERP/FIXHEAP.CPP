//	fixheap.cpp		Mark Wilden		4/23/92
//		heap for fixed size entries

#include "sol.hpp"
#include "fixheap.hpp"

void*		FixedHeap::addresses[MaxFixHeaps];
MemID		FixedHeap::memids[MaxFixHeaps];
Bool		FixedHeap::firstTime = True;

FixedHeap::FixedHeap(ushort nodeSize, ushort nodesPerBlock) :
	nodeSize(nodeSize), nodesPerBlock(nodesPerBlock), head(0)
{
	if (firstTime)  {
		firstTime = False;
		for (int i = 0; i < MaxFixHeaps; i++)
			addresses[i] = NULL;
	}
}

void*
FixedHeap::Alloc()
{
	for (Block* b = head; b; b = b->next) {
		void* v;
		if (v = b->Alloc())
			return v;
	}

	b = new Block(nodeSize, nodesPerBlock, head);
	if (!b)
		return 0;
	if (!b->IsValid())
		return 0;
	return b->Alloc();
}

void
FixedHeap::Free(void* p)
{
	for (Block* b = head; b && !b->Free(p); b = b->next)
		;
	if (!b)
		msgMgr->Fatal(SrcLoc, "Attempt to free unallocated block: %p", p);
}

FixedHeap::~FixedHeap()
{
	Block* next;
	for (Block* b = head; b; b = next) {
		next = b->next;
		delete b;
	}
}

/////////////////////////////////////////////////////////////////////////////

void*
FixedHeap::Block::operator new(size_t size)
{
	for (int i = 0; i < MaxFixHeaps; i++)  {
		if (!addresses[i])
			break;
	}
	if (i == MaxFixHeaps)
		msgMgr->Fatal(SrcLoc, "Reached limit of %d fixed heaps!", MaxFixHeaps);

	memids[i].Get(MemFixedHeap, sizeof(size), LOCKED);
	addresses[i] = *memids[i];
	return addresses[i];
}

void
FixedHeap::Block::operator delete(void* p)
{
	for (int i = 0; i < MaxFixHeaps; i++)  {
		if (p == addresses[i])  {
			addresses[i] = NULL;
			memids[i].Free();
			return;
		}
	}
	msgMgr->Fatal(SrcLoc, "Address not found!");
}

FixedHeap::Block::Block(ushort nodeSize, ushort nodesPerBlock, Block*& nextBlock) :
	base(0), avail(0), next(nextBlock)
{
	nextBlock = this;
	size = nodeSize * nodesPerBlock;

	if (!base)
		base = New char[size];

	avail = base;

	char* p = base;
	for (int i = 0; i < nodesPerBlock - 1; i++) {
		*(char**) p = p + nodeSize;
		p += nodeSize;
	}
	*(char**) p = 0;
}

FixedHeap::Block::~Block()
{
}

Bool
FixedHeap::Block::IsValid()
{
	return base != 0;
}

void*
FixedHeap::Block::Alloc()
{
	void* p = avail;
	if (p)
		avail = *(char**) avail;
	return p;
}

Bool
FixedHeap::Block::Free(void* p)
{
	if (p < base || p >= base + size)
		return False;

	*(char**) p = avail;
	avail = (char*) p;

	return True;
}

#if defined(MAIN)
#include "dlist.hpp"

void
main()
{
//	DList list;
	FixedHeap fh(16, 4);

#if 0
	int i = 0;
	void* node;
	while ((node = fh.alloc()) && list.add(node))
		printf("%d allocations\r", ++i);
#endif
}
#endif
