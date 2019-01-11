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
	unsigned n = 0;
	msgMgr->Dump("%s: \n", msg);
	for (SOL_NodeID node = First(); (int) node; n++, node = node->Next())
		if (!node.IsValid())
			msgMgr->Dump("%s: %X not valid handle\n", (SOL_Handle) node);
		else {
			SOL_Node* np = *node;
			msgMgr->Dump("%u: %X: next %X prev %X key %X val %X\n",
				n, (int) node, np->Next(), np->Prev(), np->Key(),((KNode*) np)->val);
		}
	msgMgr->Dump("\n");
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


void
SOL_List::Sort(int criterion, Bool descend)
{
	//	sort a list by property or by value, by ascending or descending order

	if (criterion == -1) { // sorting by object values or integers
		// go through every node in the list
		SOL_NodeID leastNode;
		for (SOL_NodeID node = leastNode = First();
			  (int) node;
			  node = leastNode->Next()) { 
			SOL_NodeKey val1 = ((KNodeID) node)->val;
			leastNode = node;
			// go through all unsorted nodes in the list
			for (SOL_NodeID newNode = node->Next();
				  (int) newNode;
				  newNode = newNode->Next()) { 
				SOL_NodeKey val2 = ((KNodeID) newNode)->val;
				if (!descend && val2 < val1 || descend && val1 < val2) {
					leastNode = newNode;
					val1 = val2;
				}
			}
			// if the leastNode is not sorted, then put it in order
			if (leastNode != node) {
				Delete(leastNode);
				AddBefore(node, leastNode, ((KNodeID) leastNode)->val);
			}
		}

	} else { // sorting by property values
		SOL_NodeID leastNode;
		for (SOL_NodeID node = leastNode = First();
			  (int) node;
			  node = leastNode->Next()) { 
			ObjectID obj1 = (ObjectID) ((KNodeID) node)->val;
			Property val1 = obj1.GetProperty(criterion);
			leastNode = node;
			for (SOL_NodeID newNode = node->Next();
				  (int) newNode;
				  newNode = newNode->Next()) { 
				ObjectID obj2 = (ObjectID) ((KNodeID) newNode)->val;
				Property val2 = obj2.GetProperty(criterion);
				if (!descend && val2 < val1 || descend && val1 < val2) {
					leastNode = newNode;
					val1 = val2;
				}
			}

			if (leastNode != node) {
				Delete(leastNode);
				AddBefore(node, leastNode, ((KNodeID) leastNode)->val);
			}
		}
	}
}

void
SOL_List::MoveToFront (SOL_NodeID node)
{
	Delete(node);
	AddToFront(node);
}

void
SOL_List::MoveToEnd(SOL_NodeID node)
{
	Delete(node);
	AddToEnd(node);
}

///////////////////////////////////////////////////////////////////////////////

void 
KList(argList)
{
	enum {
		ListNew,
		ListDispose,
		ListNewNode,
		ListFirstNode,
		ListLastNode,
		ListEmpty,
		ListNextNode,
		ListPrevNode,
		ListNodeValue,
		ListAddAfter,
		ListAddToFront,
		ListAddToEnd,
		ListAddBefore,
		ListMoveToFront,
		ListMoveToEnd,
		ListFindKey,
		ListDeleteKey,
		ListAt,
		ListIndexOf,
		ListEachElementDo,
		ListFirstTrue,
		ListAllTrue,
		ListSort
	};

	switch (arg(1)) {
		case ListNew: {
			SOL_ListID list;
			list.Get();

			pm.acc = (Acc) list;

			break;
		}

		case ListDispose: {
			SOL_ListID list = arg(2);

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

			break;
		}

		case ListNewNode: {
			KNodeID node;
			node.Get();
			node->val = arg(2);
			if (argCount > 2)
				node->Key() = arg(3);
			else
				node->Key() = arg(2);

			pm.acc = (Acc) node;

			break;
		}

		case ListFirstNode: {
			SOL_ListID list = (SOL_ListID) arg(2);

			if (!list) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "FirstNode");
				pm.acc = 0;
				return;
			}

			pm.acc = list->First();

			break;
		}

		case ListLastNode: {
			SOL_ListID list = (SOL_ListID) arg(2);

			if (!list) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "LastNode");

				pm.acc = 0;
				return;
			}

			pm.acc = list->Last();

			break;
		}

		case ListEmpty: {
			SOL_ListID list = (SOL_ListID) arg(2);

			if (!list) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "EmptyList");

				pm.acc = False;
				return;
			}

			pm.acc = list->IsEmpty();

			break;
		}

		case ListNextNode: {
			pm.acc = ((SOL_NodeID) arg(2))->Next();
			break;
		}

		case ListPrevNode: {
			pm.acc = ((SOL_NodeID) arg(2))->Prev();
			break;
		}

		case ListNodeValue: {
			pm.acc = ((KNodeID) arg(2))->val;
			break;
		}

		case ListAddAfter: {
			SOL_ListID list = arg(2);

			if (!list) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "AddAfter");
				pm.acc = 0;
				return;
			}

			pm.acc = list->AddAfter(arg(3), arg(4), arg(5));

			break;
		}

		case ListAddToFront: {
			SOL_ListID list = arg(2);

			if (!list) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "AddToFront");
				pm.acc = 0;
				return;
			}

			pm.acc = list->AddToFront(arg(3), arg(4));

			break;
		}

		case ListAddToEnd: {
			SOL_ListID list = arg(2);

			if (!list) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "AddToEnd");
				pm.acc = 0;
				return;
			}

			pm.acc = list->AddToEnd(arg(3), arg(4));

			break;
		}

		case ListAddBefore: {
			SOL_ListID list = arg(2);

			if ( !list ) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "AddBefore");
				pm.acc = 0;
				return;
			}

			pm.acc = list->AddBefore(arg(3), arg(4), arg(5));
			
			break;
		}

		case ListMoveToFront: {
			SOL_ListID list = arg(2);

			if ( !list ) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "MoveToFront");
				pm.acc = 0;
				return;
			}

			list->MoveToFront(arg(3));
			
			break;
		}

		case ListMoveToEnd: {
			SOL_ListID list = arg(2);

			if ( !list ) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "MoveToEnd");
				pm.acc = 0;
				return;
			}

			list->MoveToEnd(arg(3));
			
			break;
		}

		case ListFindKey: {
			SOL_ListID list = arg(2);

			if (!list) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "FindKey");

				pm.acc = 0;
				return;
			}

			pm.acc = list->FindKey(arg(3));

			break;
		}

		case ListDeleteKey: {
			SOL_ListID list = arg(2);

			if (!list) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "DeleteKey");
				pm.acc = 0;
				return;
			}

			KNodeID theNode = list->DeleteKey(arg(3));
			pm.acc = !!theNode;	// convert to 1 or 0

			if ((int) theNode)
				theNode.Free();

			break;
		}

		case ListAt: {
			SOL_ListID list = arg(2);

			if (!list) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "ListAt");

				pm.acc = 0;
				return;
			}

			KNodeID node = list->At(arg(3));
			pm.acc = (int) node ? node->val : 0;

			break;
		}

		case ListIndexOf: {
			SOL_ListID list = arg(2);
			SOL_NodeID node;
			int n;

			if (!list) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "IndexOf");
				pm.acc = 0;
				return;
			}

			for (n = 0, node = list->First(); (int) node; n++, node = node->Next()) {

				// for CodeWarrior 1.1.1.2
				//if (((KNodeID) node)->val == arg(3))
				KNodeID kNode = node;
				if (kNode->val == arg(3)) {
					pm.acc = n;
					return;
				}
			}

			pm.acc = -1;
			break;
		}

		case ListEachElementDo: {
			SOL_ListID list = arg(2);

			if (!list) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "ListEachElementDo");
				pm.acc = 0;
				return;
			}

			ObjectID saveObject = pm.object;

			list->PushNext();

			for (SOL_NodeID node = list->First(); (int) node; node = list->Next()) { 
				list->Next() = node->Next();

				// for CodeWarrior 1.1.1.2
				//ObjectID obj = ((KNodeID) node)->val;
				KNodeID kNode = node;
				ObjectID obj = kNode->val;

				int nArgs = argCount - 3;
	
				// add 1 for selector and 1 for # args
				int nArgChars = (nArgs + 2) * sizeof(Property);
	
				//	set new current object
				pm.object = obj;
	
//BEW REMOVED				//	push the selector
//BEW REMOVED				pm.stack.Push(arg(3));
	
//BEW REMOVED				//	push the number of arguments
//BEW REMOVED				pm.stack.Push(nArgs);
	
//BEW CHANGED				//	push the arguments
//BEW CHANGED				pm.stack.PushMany(&arg(4), nArgs);
	
				//	push the selector, # of Args, ptr to arg list
				pmPushMany( arg(3), nArgs, &arg(4));
	
//BEW CHANGED				//	call the method
//BEW CHANGED				pm.QuickMessage(obj, nArgChars);

				//	call the method
				// MUST be cast as short or else CPP converts them signed (Bryan Waters)
				pmQuickMessage( (unsigned short) obj, (unsigned short) nArgChars, pm.StackPtr);
			}

			//	restore current object
			pm.object = saveObject;
	
			list->PopNext();

			break;
		}

		case ListFirstTrue: {
			pm.acc = 0;

			SOL_ListID list = arg(2);

			if (!list) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "FirstTrue");
				return;
			}

			ObjectID saveObject = pm.object;

			list->PushNext();
	
			for (SOL_NodeID node = list->First(); (int) node; node = list->Next()) { 
				list->Next() = node->Next();

				// for CodeWarrior 1.1.1.2
				// ObjectID obj = ((KNodeID) node)->val;
				KNodeID kNode = node;
				ObjectID obj =  kNode->val;

				int nArgs = argCount - 3;
	
				// add 1 for selector and 1 for # args
				int nArgChars = (nArgs + 2) * sizeof(Property);
	
				//	set new current object
				pm.object = obj;
	
//BEW REMOVED				//	push the selector
//BEW REMOVED				pm.stack.Push(arg(3));
	
//BEW REMOVED				//	push the number of arguments
//BEW REMOVED				pm.stack.Push(nArgs);
	
//BEW CHANGED				//	push the arguments
//BEW CHANGED				pm.stack.PushMany(&arg(4), nArgs);
	
				//	push the selector, # of Args, ptr to arg list
				pmPushMany( arg(3), nArgs, &arg(4));
	
//BEW CHANGED				//	call the method
//BEW CHANGED				pm.QuickMessage(obj, nArgChars);

				//	call the method
				// MUST be cast as short or else CPP converts them signed (Bryan Waters)
				pmQuickMessage( (unsigned short) obj, (unsigned short) nArgChars, pm.StackPtr);
		
				if (pm.acc) {
					pm.acc = obj;
					break;
				}
			}

			//	restore current object
			pm.object = saveObject;
	
			list->PopNext();

			break;
		}

		case ListAllTrue: {
			SOL_ListID list = arg(2);

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

				// for CodeWarrior 1.1.1.2
				// ObjectID obj = ((KNodeID) node)->val;
				KNodeID kNode = node;
				ObjectID obj =  kNode->val;

				int nArgs = argCount - 3;
	
				// add 1 for selector and 1 for # args
				int nArgChars = (nArgs + 2) * sizeof(Property);
	
				//	set new current object
				pm.object = obj;
	
//BEW REMOVED				//	push the selector
//BEW REMOVED				pm.stack.Push(arg(3));
	
//BEW REMOVED				//	push the number of arguments
//BEW REMOVED				pm.stack.Push(nArgs);
	
//BEW CHANGED				//	push the arguments
//BEW CHANGED				pm.stack.PushMany(&arg(4), nArgs);
	
				//	push the selector, # of Args, ptr to arg list
				pmPushMany( arg(3), nArgs, &arg(4));
	
//BEW CHANGED				//	call the method
//BEW CHANGED				pm.QuickMessage(obj, nArgChars);

				//	call the method
				// MUST be cast as short or else CPP converts them signed (Bryan Waters)
				pmQuickMessage( (unsigned short) obj, (unsigned short) nArgChars, pm.StackPtr);
		
				if (!pm.acc) {
					pm.acc = False;
					break;
				}
			}

			//	restore current object
			pm.object = saveObject;
	
			list->PopNext();

			break;
		}

		case ListSort: {
			SOL_ListID list = arg(2);

			if (!list.IsValid()) {
				msgMgr->Alert(SrcLoc, Msg_NoList, "Sort");
				return;
			}

			list->Sort(arg(3), argCount > 3 ? arg(4) : False);
			
			break;
		}
	}
}
