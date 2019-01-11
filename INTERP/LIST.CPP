// list.cpp

#include "sol.hpp"

#include	"kernel.hpp"
#include	"list.hpp"
#include	"msg.hpp"
#include	"pmachine.hpp"
#include	"savegame.hpp"

SOL_NodeID
SOL_List::At(int i)
{
	for (SOL_NodeID node = First(); (int) node && i; i--, node = node->Next())
		;

	return node;
}

SOL_NodeID
SOL_List::AddToEnd(SOL_NodeID element, SOL_NodeKey key)
{
	if (IsEmpty()) {
		First() = element;
		element->Prev() = 0;
	} else {
		Last()->Next() = element;
		element->Prev() = Last();
	}

	Last() = element;
	element->Next() = 0;

	element->Key() = key;
	
	return element;
}

SOL_NodeID
SOL_List::AddAfter(SOL_NodeID node, SOL_NodeID element, SOL_NodeKey key)
{
	if (node->IsLast()) {
		element->Next() = 0;
		Last() = element;
	} else {
		element->Next() = node->Next();
		((SOL_NodeID) node->Next())->Prev() = element;
	}

	element->Prev() = node;
	node->Next() = element;

	element->Key() = key;

	return element;
}

SOL_NodeID
SOL_List::AddBefore(SOL_NodeID node, SOL_NodeID element, SOL_NodeKey key)
{
	if (node->IsFirst()) {
		element->Prev() = 0;
		First() = element;
	} else {
		element->Prev() = node->Prev();
		((SOL_NodeID) node->Prev())->Next() = element;
	}

	element->Next() = node;
	node->Prev() = element;

	element->Key() = key;

	return element;
}

SOL_NodeID
SOL_List::AddToFront(SOL_NodeID element, SOL_NodeKey key)
{
	if (IsEmpty()) {
		Last() = element;
		element->Next() = 0;
	} else {
		First()->Prev() = element;
		element->Next() = First();
	}

	First() = element;
	element->Prev() = 0;

	element->Key() = key;

	return element;
}

Bool
SOL_List::Delete(SOL_NodeID node)
{
	//	preserve iteration state
	for (int i = 1; i <= nextPtr; i++) {
		//	see if we're deleting a next node
		if (node == nexts[i])
			nexts[i] = nexts[i]->Next();
	}

	if (node->IsFirst())
		First() = node->Next();
	else
		((SOL_NodeID) node->Prev())->Next() = node->Next();

	if (node->IsLast())
		Last() = node->Prev();
	else
		((SOL_NodeID) node->Next())->Prev() = node->Prev();

	return !IsEmpty();
}

SOL_NodeID
SOL_List::DeleteKey(SOL_NodeKey key)
{
	SOL_NodeID node;

	if (int(node = FindKey(key)))
		Delete(node);

	return node;
}

void
SOL_List::Dump(char* msg)
{
	int n = 0;
	for (SOL_NodeID node = First(); (int) node; n++, node = node->Next())
		if (!node.IsValid())
			msgMgr->Dump("%s: %X not valid handle\n", (SOL_Handle) node);
		else {
			SOL_Node* np = *node;
			msgMgr->Dump("%s: %u: %X: next %X prev %X key %X val %X\n",
				msg, n, node, np->Next(), np->Prev(), np->Key(),((KNode*) np)->val);
		}
}

SOL_NodeID
SOL_List::FindKey(SOL_NodeKey key)
{
	for (SOL_NodeID node = First();
		  (int) node && node->Key() != key;
		  node = node->Next())
	    ;

	return node;
}

void
SOL_List::SaveRestore(SaveGame* saveGame, Bool save)
{
	saveGame->SaveRestore(&head, sizeof head, save);
	saveGame->SaveRestore(&tail, sizeof tail, save);
	saveGame->SaveRestore(nexts, sizeof nexts, save);
	saveGame->SaveRestore(&nextPtr, sizeof nextPtr, save);
}

///////////////////////////////////////////////////////////////////////////////

void
KNewList(kArgs)
{
	//	create a new list and return its address

	SOL_ListID list;
	list.Get();
	pm.acc = (Acc) list;
}

void
KNewNode(argList)
{
	KNodeID node;
	node.Get();
	node->val = arg(1);
	pm.acc = (Acc) node;
}

void
KFirstNode(argList)
{
	SOL_ListID list = (SOL_ListID) arg(1);

	if (!list) {
		msgMgr->Alert(SrcLoc, Msg_NoList, "FirstNode");
		pm.acc = 0;
		return;
	}

	pm.acc = list->First();
}

void
KLastNode(argList)
{
	SOL_ListID list = (SOL_ListID) arg(1);

	if (!list) {
		msgMgr->Alert(SrcLoc, Msg_NoList, "LastNode");

		pm.acc = 0;
		return;
	}

	pm.acc = list->Last();
}

void
KEmptyList(argList)
{
	SOL_ListID list = (SOL_ListID) arg(1);

	if (!list) {
		msgMgr->Alert(SrcLoc, Msg_NoList, "EmptyList");

		pm.acc = False;
		return;
	}

	pm.acc = list->IsEmpty();
}

void
KNextNode(argList)
{
	pm.acc = ((SOL_NodeID) arg(1))->Next();
}

void
KPrevNode(argList)
{
	pm.acc = ((SOL_NodeID) arg(1))->Prev();
}

void
KNodeValue(argList)
{
	pm.acc = ((KNodeID) arg(1))->val;
}

void
KAddAfter(argList)
{
	SOL_ListID list = arg(1);

	if (!list) {
		msgMgr->Alert(SrcLoc, Msg_NoList, "AddAfter");
		pm.acc = 0;
		return;
	}

	pm.acc = list->AddAfter(arg(2), arg(3), arg(4));
}

void
KAddToFront(argList)
{
	SOL_ListID list = arg(1);

	if (!list) {
		msgMgr->Alert(SrcLoc, Msg_NoList, "AddToFront");
		pm.acc = 0;
		return;
	}

	pm.acc = list->AddToFront(arg(2), arg(3));
}

void
KAddToEnd(argList)
{
	SOL_ListID list = arg(1);

	if (!list) {
		msgMgr->Alert(SrcLoc, Msg_NoList, "AddToEnd");
		pm.acc = 0;
		return;
	}

	pm.acc = list->AddToEnd(arg(2), arg(3));
}

void
KFindKey(argList)
{
	SOL_ListID list = arg(1);

	if (!list) {
		msgMgr->Alert(SrcLoc, Msg_NoList, "FindKey");

		pm.acc = 0;
		return;
	}

	pm.acc = list->FindKey(arg(2));
}

void
KDisposeList(argList)
{
	//	dispose of allocated memory associated with list

	SOL_ListID list = arg(1);

	if (!list.IsValid()) {
		msgMgr->Alert(SrcLoc, Msg_NoList, "DisposeList");
		return;
	}

	while (!list->IsEmpty()) {
		KNodeID kNode = list->First();
		list->Delete(kNode);
		kNode.Free();
	}

	list.Free();
}

void
KDeleteKey(argList)
{
	SOL_ListID list = arg(1);

	if (!list) {
		msgMgr->Alert(SrcLoc, Msg_NoList, "DeleteKey");
		pm.acc = 0;
		return;
	}

	KNodeID theNode = list->DeleteKey(arg(2));
	pm.acc = !!theNode;	// convert to 1 or 0
	if ((int) theNode)
		theNode.Free();
}

void
KListAllTrue(argList)
{
	SOL_ListID list = arg(1);

	if (!list) {
		msgMgr->Alert(SrcLoc, Msg_NoList, "AllTrue");
		pm.acc = False;
		return;
	}

	//	an empty list is "all true," apparently
	pm.acc = True;
	
	ObjectID saveObject = pm.object;

	list->PushNext();

	for (SOL_NodeID node = list->First(); (int) node; node = list->Next()) { 
		list->Next() = node->Next();
		ObjectID obj = ((KNodeID) node)->val;

		int nArgs = argCount - 2;
	
		// add 1 for selector and 1 for # args
		int nArgChars = (nArgs + 2) * sizeof(Property);
	
		//	set new current object
		pm.object = obj;
	
		//	push the selector
		pm.stack.Push(arg(2));
	
		//	push the number of arguments
		pm.stack.Push(nArgs);
	
		//	push the arguments
		pm.stack.PushMany(&arg(3), nArgs);
	
		//	call the method
		pm.QuickMessage(obj, nArgChars);
		
		if (!pm.acc) {
			pm.acc = False;
			break;
		}
	}

	//	restore current object
	pm.object = saveObject;
	
	list->PopNext();
}

void
KListAt(argList)
{
	//	return the element's key at a position

	SOL_ListID list = arg(1);

	if (!list) {
		msgMgr->Alert(SrcLoc, Msg_NoList, "ListAt");

		pm.acc = 0;
		return;
	}

	KNodeID node = list->At(arg(2));
	pm.acc = (int) node ? node->val : 0;
}

void
KListEachElementDo(argList)
{
	SOL_ListID list = arg(1);

	if (!list) {
		msgMgr->Alert(SrcLoc, Msg_NoList, "ListEachElementDo");
		pm.acc = 0;
		return;
	}

	ObjectID saveObject = pm.object;

	list->PushNext();

	for (SOL_NodeID node = list->First(); (int) node; node = list->Next()) { 
		list->Next() = node->Next();
		ObjectID obj = ((KNodeID) node)->val;

		int nArgs = argCount - 2;
	
		// add 1 for selector and 1 for # args
		int nArgChars = (nArgs + 2) * sizeof(Property);
	
		//	set new current object
		pm.object = obj;
	
		//	push the selector
		pm.stack.Push(arg(2));
	
		//	push the number of arguments
		pm.stack.Push(nArgs);
	
		//	push the arguments
		pm.stack.PushMany(&arg(3), nArgs);
	
		//	call the method
		pm.QuickMessage(obj, nArgChars);
	}

	//	restore current object
	pm.object = saveObject;
	
	list->PopNext();
}

void
KListFirstTrue(argList)
{
	pm.acc = 0;

	SOL_ListID list = arg(1);

	if (!list) {
		msgMgr->Alert(SrcLoc, Msg_NoList, "FirstTrue");
		return;
	}

	ObjectID saveObject = pm.object;

	list->PushNext();
	
	for (SOL_NodeID node = list->First(); (int) node; node = list->Next()) { 
		list->Next() = node->Next();
		ObjectID obj = ((KNodeID) node)->val;

		int nArgs = argCount - 2;
	
		// add 1 for selector and 1 for # args
		int nArgChars = (nArgs + 2) * sizeof(Property);
	
		//	set new current object
		pm.object = obj;
	
		//	push the selector
		pm.stack.Push(arg(2));
	
		//	push the number of arguments
		pm.stack.Push(nArgs);
	
		//	push the arguments
		pm.stack.PushMany(&arg(3), nArgs);
	
		//	call the method
		pm.QuickMessage(obj, nArgChars);
		
		if (pm.acc) {
			pm.acc = obj;
			break;
		}
	}

	//	restore current object
	pm.object = saveObject;
	
	list->PopNext();
}

void
KListIndexOf(argList)
{
	//	return the index of an element, or -1 if not found

	SOL_ListID list = arg(1);

	if (!list) {
		msgMgr->Alert(SrcLoc, Msg_NoList, "IndexOf");
		pm.acc = 0;
		return;
	}

	int n;
	SOL_NodeID node;
	for (n = 0, node = list->First(); (int) node; n++, node = node->Next())
		if (((KNodeID) node)->val == arg(2)) {
			pm.acc = n;
			return;
		}
	pm.acc = -1;
}

/////////////////////////////////////////////////////////////////////////////

void
SOL_List::CheckIntegrity(char* msg)
{
	static SOL_NodeID nodes[1000];
	int nNodes = 0;

	for (SOL_NodeID node = First(); (int) node; node = node->Next()) {
		if (!node.IsValid())
			msgMgr->Fatal("Invalid node in list: %s", msg);
		if (nNodes >= 1000)
			msgMgr->Fatal(">= 1000 nodes in list: %s", msg);
		//	make sure this node is not already in the list
		for (int i = 0; i < nNodes; i++)
			if (nodes[i] == node) {
				msgMgr->Alert(SrcLoc, Msg_InvalidList);
				Dump("CheckListIntegrity");
				exit(1);
			}
		nodes[nNodes++] = node;
	}
}
