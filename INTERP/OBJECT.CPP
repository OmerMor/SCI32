//	object.cpp

#include <stdio.h>
#include <string.h>

#include "sol.hpp"

#include "kernel.hpp"
#include "memmgr.hpp"
#include "msg.hpp"
#include "object.hpp"
#include "pmachine.hpp"
#include "resource.hpp"
#include "script.hpp"
#include "sendstac.hpp"
#include "textid.hpp"
#include "vocab.hpp"

SOL_List	objects;

size_t indexedPropertyOffsets[MaxIndexedProperty];

////////////////////////////////////////////////////////////////////////////

Object::Object(MemID id, MemID source, size_t offset, size_t sz)
{
	//	create an Object from the Object found in an .HEP file

	Object* obj = (Object*) &source[offset];

	//	initialize the fixed properties
	size 		= obj->size;
	propDict	= obj->propDict;
	methDict	= obj->methDict;
	classNum	= obj->classNum;
	script	= obj->script;
	super		= obj->super;
	info		= obj->info;
	
	//	copy the rest of the data from the end of the object to the end of us
	memcpy(this + 1, obj + 1, sz - sizeof(*obj));

	this->id = id;

	AddToObjects();
}

Object::Object(MemID id, MemID clonee) :
	id(id), size(0), classNum(0), info(0)
{
	//	clone an object

	ObjectID obj = (ObjectID) clonee;
	obj.AssureValid();

	size 		= obj->size;
	propDict	= obj->propDict;
	methDict	= obj->methDict;
	classNum	= obj->classNum;
	script	= obj->script;
	super		= obj->super;
	info		= obj->info;

	//	copy the variable length portion
	memcpy(this + 1, (Object*) *clonee + 1,
		obj->size * sizeof(Property) - sizeof(Object));

	//	if we're copying from a class, set our super to the class and turn off
	//	our class bit
	if (obj->info & CLASSBIT) {
		super = obj->id;
		info &= ~CLASSBIT;
	}

	//	mark the object as cloned
	info |= CLONEBIT;

	//	increment script's reference count
	((ScriptID) script)->nRefs++;

	//	increment object's dictionaries' reference counts
	++((PropDictID) propDict)->nRefs;
	++((MethDictID) methDict)->nRefs;

	AddToObjects();
}

Object::~Object()
{
	DeleteFromObjects();

	id = 0;
	if (!--((PropDictID) propDict)->nRefs)
		((PropDictID) propDict).Free();
	if (!--((MethDictID) methDict)->nRefs)
		((MethDictID) methDict).Free();

	((ScriptID) script)->nRefs--;
}

void
Object::AddToObjects()
{
	//	add to objects list

#ifdef DEBUG
	SOL_NodeID node;
	node.Get();
	objects.AddToFront(node, id);
#endif
}

void
Object::DeleteFromObjects()
{
	//	remove from objects list

#ifdef DEBUG
	objects.DeleteKey(id).Free();
#endif
}

const char*
Object::Name()
{
	//	return object's name, or "" if object doesn't have a name property

	const Property* nameProp;
	if (!(nameProp = GetPropAddr(s_name)))
		return "";

	const char* name = *(TextID) *nameProp;
	if (!name)
		return "";
		
	return name;
}

Bool
Object::RespondsTo(Selector selector)
{
	//	return whether 'selector' is a property or method of us or our
	//	superclasses

	//	is 'selector' a property?
	if (GetPropAddr(selector))
		return True;

	//	search the method dictionary hierarchy
	for (Object* obj = this; obj; obj = (int) obj->super ? *(ObjectID) obj->super : 0)
		for (int i = 0; i < ((MethDictID) obj->methDict)->nEntries; i++)
			if (((MethDictID) obj->methDict)[i].selector == selector)
				return True;

	return False;
}

////////////////////////////////////////////////////////////////////////////

void
ObjectID::Get(MemID id, size_t ofs, size_t size)
{
	//	get memory to read an .HEP object into

	ID<Object, MemObject>::Get(size);
	new(handle) Object(*this, id, ofs, size);
}

void
ObjectID::Get(ObjectID clonee)
{
	//	clone an Object

	ID<Object, MemObject>::Get(clonee->size * sizeof(Property));
	new(handle) Object(*this, clonee);
}

void
ObjectID::Free()
{
	//	this is necessary because of a bug in WATCOM 9.5 (see MEMID.HPP)
	if (handle)
		(**this)->~Object();
	ID<Object, MemObject>::Free();
}

ObjectID
ObjectID::Clone()
{
	//	return pointer to copy of an object or class

	//	get memory and copy into it
	ObjectID newObject;
	newObject.Get(*this);

	return newObject;
}

////////////////////////////////////////////////////////////////////////////

const char*
GetSelectorName(Selector selector, char* str)
{
	if (!GetVocabStr(SELECTOR_VOCAB, selector, str))
		sprintf(str, "%x", selector);
	return str;
}

void
LoadPropOffsets()
{
	// Load the offsets to indexed object properties from a file.

	SCIWord* op = (SCIWord*) *resMgr->Get(MemResVocab, PROPOFS_VOCAB);

	// Read and store each offset.
	for (int i = 0 ; i < MaxIndexedProperty ; ++i)
		indexedPropertyOffsets[i] = *op++;
}

///////////////////////////////////////////////////////////////////////////

ObjectMgr objectMgr;

void
ObjectMgr::SaveRestoreDo(Saveable::Func func, SaveGame* saveGame, Bool save)
{
	objects.SaveRestoreDo(func, saveGame, save);
	Saveable::SaveRestoreDo(func, saveGame, save);
}

///////////////////////////////////////////////////////////////////////////

void
KIsObject(argList)
{
	pm.acc = (Acc) ((ObjectID) arg(1)).IsObject();
}

void
KRespondsTo(argList)
{
	pm.acc = (Acc) ((ObjectID) arg(1)).RespondsTo(arg(2));
}

void
KClone(argList)
{
	// Get a clone of the object.

	ObjectID theSource = (ObjectID) arg(1);

	ObjectID theClone = ((ObjectID) arg(1)).Clone();

	// Set any properties
	int numArgs;
	for (numArgs = argCount - 1, args += 2;
		  numArgs > 0;
		  numArgs -= 2, args += 2)
		theClone.SetProperty(arg(0), arg(1));

	pm.acc = (Acc) theClone;
}

void
KDisposeClone(argList)
{
	//	if the object was cloned, free it

	ObjectID obj = arg(1);
	obj.AssureValid();
	if ((obj->Info() & (CLONEBIT | NODISPOSE)) == CLONEBIT)
		obj.Free();
}
