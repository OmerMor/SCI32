// list.hpp

#ifndef LIST_HPP
#define LIST_HPP

#ifndef MEMID_HPP
#include "memid.hpp"
#endif

#ifndef SAVEABLE_HPP
#include "saveable.hpp"
#endif

typedef SCIWord	SOL_NodeKey;

struct SOL_Node {
	MemID&			Next()		{ return next; }
	MemID&			Prev()		{ return prev; }
	SOL_NodeKey&	Key()			{ return key; }

	Bool				IsFirst()	{ return !Prev(); }
	Bool				IsLast()		{ return !Next(); }
	
protected:
	MemID			next;
	MemID			prev;
	SOL_NodeKey	key;
};

FakeTemplateID(SOL_NodeID, SOL_Node, MemListNode);

struct KNode : SOL_Node {
	SCIWord		val;
};

FakeTemplateID(KNodeID, KNode, MemListKNode);

///////////////////////////////////////////////////////////////////////////////

class SOL_List : public Saveable {
public:
	SOL_List() : nextPtr(0) {}

	SOL_NodeID&		First()		{ return head; }
	SOL_NodeID&		Last()		{ return tail; }
	SOL_NodeID&		Next()		{ return nexts[nextPtr]; }
	Bool				IsEmpty()	{ return !First(); }

	SOL_NodeID		At(int i);
	SOL_NodeID		AddAfter(SOL_NodeID, SOL_NodeID, SOL_NodeKey = 0);
	SOL_NodeID		AddBefore(SOL_NodeID, SOL_NodeID, SOL_NodeKey = 0);
	SOL_NodeID		AddToFront(SOL_NodeID, SOL_NodeKey = 0);
	SOL_NodeID		AddToEnd(SOL_NodeID, SOL_NodeKey = 0);
	void				CheckIntegrity(char* msg);
	SOL_NodeID		DeleteKey(SOL_NodeKey);
	Bool				Delete(SOL_NodeID);
	void				Dump(char* msg = "");
	SOL_NodeID		FindKey(SOL_NodeKey);
	void				MoveToFront(SOL_NodeID);
	void				MoveToEnd(SOL_NodeID);
	void				PushNext()	{ nextPtr++; }
	void				PopNext()	{ nextPtr--; }
	void				Sort(int criterion, Bool descending);

	void	SaveRestore(SaveGame*, Bool save);

protected:
	SOL_NodeID	head;
	SOL_NodeID	tail;
	SOL_NodeID	nexts[10];	//	nextNode stack for iteration safety
	int			nextPtr;
};

FakeTemplateID(SOL_ListID, SOL_List, MemList)

#endif
