//	script.cpp	6/2/93

#include "sol.hpp"

#include "kernel.hpp"
#include "msg.hpp"
#include "object.hpp"
#include "pmachine.hpp"
#include "resource.hpp"
#include "savegame.hpp"
#include "script.hpp"
#include "string.hpp"
#include "textid.hpp"
#include "vocab.hpp"

static ScriptID	FindScript(unsigned n);
static void			TossScript(ScriptID script, Bool checkClones);

static SOL_List	scripts;
static unsigned	curScript;
static ClassTblID	classTbl;
static int			nClasses;

class Entry;

class Entrys {
	//	contains a list of entries for the variables, the strings and the
	//	objects in the script, primarily for fixup purposes
public:
	enum { MaxEntries = 200 };

	Entrys();
	~Entrys();
	MemID		Add(Entry*);
	Entry*	Find(unsigned offset);
	void		Fixup(unsigned offset);
	ObjectID	GetFirstObject();
	ObjectID	GetNextObject();
	Entry*	GetFirst();
	Entry* 	GetNext();

private:
	unsigned	nEntries;
	unsigned	maxEntries;
	Entry**	entries;
	unsigned	index;
	unsigned	objectIndex;
};

//////////////////////////////////////////////////////////////////////////////

struct Entry
{
	Entry(size_t ofs, size_t size, MemType memType,
			ScriptEntry::Type type = ScriptEntry::None) :
		offset(ofs), size(size), type(type), memType(memType)
	{
	}

	Entry(MemID base, size_t& ofs, size_t size, MemType memType,
			ScriptEntry::Type type = ScriptEntry::None, MemAttrs a = MOVEABLE) :
		offset(ofs), size(size), type(type), memType(memType)
	{
		id.Get(memType, base, ofs, size, a);
		ofs += size;
	}

	Bool
	Contains(size_t ofs)
	{
		//	return whether this id contains this offset

		return ofs >= offset && ofs < offset + size;
	}

	virtual MemID
	Fixup(Entrys* entries, unsigned fixupOffset)
	{
		//	there's an offset at this location:  what's the corresponding id?

		SCIUWord* fixee = (SCIUWord*) &id[fixupOffset - offset];
		Entry* id = entries->Find(*fixee);
		*fixee = id->id;
		return id->id;
	}

	MemID					id;
	size_t				offset;
	size_t				size;
	ScriptEntry::Type	type;
	MemType				memType;
};

struct ObjectIDEntry : Entry
{
	ObjectIDEntry(MemID base, size_t& ofs) :
		Entry(ofs, ((Object*) &base[ofs])->Size() * sizeof(Property),
			MemObject, ScriptEntry::ObjectEntry)
	{
		((ObjectID*) &id)->Get(base, ofs, size);
		ofs += size;
	}
};

struct StringIDEntry : Entry
{
	StringIDEntry(MemID base, size_t& ofs) :
		Entry(base, ofs, strlen(&base[ofs]) + 1, MemText,ScriptEntry::StringEntry)
	{
	}
};

struct VariablesIDEntry : Entry
{
	VariablesIDEntry(MemID base, size_t& ofs, unsigned nEntries) :
		Entry(base, ofs, nEntries * sizeof(Variable), MemVariables)
	{
	}
};

struct CodeIDEntry : Entry
{
	CodeIDEntry(MemID base, size_t& ofs, size_t size) :
		Entry(base, ofs, size, MemCode, ScriptEntry::None, TRANSITORY),
		nFixups(0)
	{
	}

	MemID
	Fixup(Entrys* entries, unsigned fixupOffset)
	{
		MemID id = Entry::Fixup(entries, fixupOffset);
		AddFixup(id, fixupOffset - offset);
		return id;
	}
	
	void
	AddFixup(MemID id, size_t offset)
	{
		if (!nFixups) {
			fixups.Get(MemCodeFixups, sizeof(FixupEntry));
			nFixups++;
		} else
			fixups.Realloc(++nFixups * sizeof(FixupEntry));
			
		FixupEntry* f = (FixupEntry*) *fixups;
		f[nFixups - 1].id = id;
		f[nFixups - 1].offset = offset;
	}
	
	struct FixupEntry {
		MemID		id;
		size_t	offset;
	};
	
	MemID	fixups;
	int	nFixups;
};

struct DictIDEntry : Entry
{
	DictIDEntry(MemID base, size_t& ofs, size_t size) :
		Entry(ofs, size, MemDictionary)
	{
		id.Get(MemDictionary, size);
		*(unsigned*) *id = 0;		//	zero reference count
		memcpy(&id[sizeof(unsigned)], &base[ofs], size - sizeof(unsigned));
		ofs += size - sizeof(unsigned);
	}
};

struct PropDictIDEntry : DictIDEntry
{
	PropDictIDEntry(MemID base, size_t& ofs, unsigned nEntries) :
		DictIDEntry(base, ofs,
			sizeof(PropDict) + nEntries * sizeof(PropDictEntry))
	{
	}
};

struct MethDictIDEntry : DictIDEntry
{
	MethDictIDEntry(MemID base, size_t& ofs) :
		DictIDEntry(base, ofs, sizeof(MethDict) +
			(SCIUWord&) base[ofs] * sizeof(MethDictEntry))
	{
	}
};

struct DispatchesIDEntry : Entry
{
	DispatchesIDEntry(MemID base, size_t& ofs) :
		Entry(base, ofs, sizeof(DispatchTbl) +
			(SCIUWord&) base[ofs] * sizeof(DispatchEntry), MemDispatchTbl)
	{
	}
};

//////////////////////////////////////////////////////////////////////////////

Entrys::Entrys() :
	nEntries(0), objectIndex(0), entries(0), maxEntries(0)
{
}

Entrys::~Entrys()
{
	for (int i = 0; i < nEntries; i++)
		delete entries[i];
	delete[] entries;
}

MemID
Entrys::Add(Entry* entry)
{
	if (nEntries + 1 >= maxEntries) {
		Entry** newEntrys = New Entry*[maxEntries += 5];
		memcpy(newEntrys, entries, nEntries * sizeof(Entry*));
		delete[] entries;
		entries = newEntrys;
	}
	entries[nEntries++] = entry;
	return entry->id;
}

Entry*
Entrys::Find(unsigned offset)
{
	for (int i = 0; i < nEntries; i++)
		if (entries[i]->offset == offset)
			return entries[i];
	msgMgr->Fatal(SrcLoc, Msg_FixupError, offset, curScript);
	return 0;
}

void
Entrys::Fixup(unsigned offset)
{
	//	replace the offset at this location with the corresponding id

	//	find the id corresponding to this offset
	for (int i = 0; i < nEntries; i++)
		if (entries[i]->Contains(offset))
			break;
	if (i == nEntries)
		msgMgr->Fatal(SrcLoc, Msg_FixupError, offset, curScript);

	entries[i]->Fixup(this, offset);
}

Entry*
Entrys::GetFirst()
{
	index = 0;
	return GetNext();
}

Entry*
Entrys::GetNext()
{
	if (index >= nEntries)
		return 0;

	return entries[index++];
}

ObjectID
Entrys::GetFirstObject()
{
	objectIndex = 0;
	return GetNextObject();
}

ObjectID
Entrys::GetNextObject()
{
	while (objectIndex < nEntries &&
			 entries[objectIndex]->type != ScriptEntry::ObjectEntry)
		objectIndex++;
	if (objectIndex >= nEntries)
		return 0;

	return ((ObjectIDEntry*) entries[objectIndex++])->id;
}

//////////////////////////////////////////////////////////////////////////////

DispatchEntry
GetDispatchEntry(unsigned scriptNum, unsigned entryNum, ScriptID* script)
{
	//	return the dispatch entry for this script/entry, which will either
	//	be the address of an object, or the offset of the code in hunk

	ScriptID s = ScriptPtr(scriptNum);
	if (script)
		*script = s;
	if (s->dispatches->nEntries <= entryNum)
		msgMgr->Fatal(SrcLoc, Msg_BadDispatch, entryNum);
	return s->dispatches[entryNum];
}

ScriptID
LoadScript(unsigned n)
{
	//	load a script's heap and hunk resources, and break up into memory
	//	blocks

   curScript = n;
   
	Entrys entries;

	//	create the script node
	ScriptID script;
	script.Get();
	scripts.AddToFront(script, n);

	//	read the heap resource
	ScriptHeapID heap = resMgr->Get(MemResHeap, n, lock);

	//	create the variables block
	size_t ofs = offsetof(ScriptHeap, vars);
	if (heap->nVars)
		script->vars =
			entries.Add(New VariablesIDEntry(heap, ofs, heap->nVars));

	//	create the object blocks
	while ((SCIUWord&) heap[ofs]) {
		entries.Add(New ObjectIDEntry(heap, ofs));
		script->nIDs++;
		script->nRefs++;
	}

	//	skip over the 0 that terminates objects
	ofs += sizeof(SCIUWord);

	//	create the string blocks
	//	one byte of padding is added if strings end on odd address
	while (ofs < heap->fixups - 1) {
		entries.Add(New StringIDEntry(heap, ofs));
		script->nIDs++;
	}
	ofs = heap->fixups;

	//	fix up the heap
	SCIUWord* fixups = (SCIUWord*) &heap[ofs];
	unsigned	nFixups = *fixups++;
	while (nFixups--)
		entries.Fixup(*fixups++);

	heap.Unlock();

	//	read the hunk
	ScriptHunkID hunk = resMgr->Get(MemResHunk, n, lock);

	//	create the dispatch table block
	ofs = offsetof(ScriptHunk, nDispatches);
	DispatchesIDEntry dispatchesIDEntry(hunk, ofs);
	script->dispatches = dispatchesIDEntry.id;

	// create the dictionaries
	for (ObjectID obj = entries.GetFirstObject();
		  (int) obj;
		  obj = entries.GetNextObject()) {

		// the compiler puts the class number of the superclass in the -super-
		// property and the class number of the class (if it is one) in the
		//	script property
		UInt16 scriptNum = (UInt16) obj->script;
		UInt16 superNum = (UInt16) obj->super;

		if (scriptNum == (UInt16) OBJNUM)
			obj->classNum = superNum;
		else
			obj->classNum = scriptNum;

		if (superNum == (UInt16) -1)
			obj->super = 0;
		else {
			obj->super = GetClass(superNum); ObjectID i = obj->super; i = i;
			curScript = n;
		}

		if (scriptNum == (UInt16) OBJNUM)
			obj->propDict = ((ObjectID) obj->super)->propDict;
		else {
			if (scriptNum >= nClasses)
				msgMgr->Fatal(SrcLoc, Msg_ClassNumTooLarge, n);
			classTbl[scriptNum].obj = obj;
			PropDictIDEntry propDict(hunk, ofs, obj->size);
			obj->propDict = propDict.id;
		}

		((PropDictID) obj->propDict)->nRefs++;

		MethDictIDEntry methDict(hunk, ofs);
		obj->methDict = methDict.id;
		((MethDictID) obj->methDict)->nRefs++;

		obj->script = script;
	}

	//	create the code block
	script->codeOffset = ofs;
	CodeIDEntry codeIDEntry(hunk, ofs, hunk->fixups - script->codeOffset);
	script->code = codeIDEntry.id;

	//	fixup the dispatch table and code
	fixups = (SCIUWord*) &hunk[ofs];
	nFixups = *fixups++;
	for (int i = 0; i < nFixups; i++) {
		Entry* id = fixups[i] < script->codeOffset ?
			(Entry*) &dispatchesIDEntry : (Entry*) &codeIDEntry;
		id->Fixup(&entries, fixups[i]);
	}

	//	save the code fixups for when we restore the game
	script->codeFixups = codeIDEntry.fixups;
	script->nCodeFixups = codeIDEntry.nFixups;

	//	offset code references to start of code block instead of start of hunk

	//	offset dispatch table code entries
	//	if a dispatch table entry is not in the fixups, it's a code offset
	unsigned offset = dispatchesIDEntry.offset + sizeof(DispatchTbl);
	for (i = 0; i < script->dispatches->nEntries; i++) {
		DispatchEntry& fixee = script->dispatches[i];
		//	don't bother with unused slots
		if (fixee.codeOffset) {
			//	see if this offset is in the fixups
			for (int j = 0; j < nFixups; j++)
				if (offset == fixups[j])
					break;
			//	wasn't in fixups, therefore offset it
			if (j == nFixups)
				fixee.codeOffset -= script->codeOffset;
		}
		offset += sizeof(DispatchEntry);
	}
	
	//	offset method dictionaries
	for (obj = entries.GetFirstObject();
		  (int) obj;
		  obj = entries.GetNextObject()) {
		MethDictID methDict = obj->methDict;
		for (int i = 0; i < methDict->nEntries; i++)
			methDict[i].offset -= script->codeOffset;
	}
	
	//	create the ids table
	if (script->nIDs) {
		ScriptEntryID ids;
		ids.Get(script->nIDs * sizeof(ScriptEntry));
		script->ids = ids;
		ScriptEntry* idEntry = *script->ids;
		for (Entry* entry = entries.GetFirst();
			  entry;
			  entry= entries.GetNext()) {
			if (entry->type == ScriptEntry::ObjectEntry ||
				 entry->type == ScriptEntry::StringEntry) {
				idEntry->id = entry->id;
				idEntry->type = entry->type;
				idEntry++;
			}
		}
	}

	//	we're done with the hunk resource
	hunk.Unlock();

	return script;
}

void
ReloadScript(ScriptID script)
{
	//	reload a script's code back into memory after a restore

	//	get the resource with the code in it
	ScriptHunkID hunk = resMgr->Get(MemResHunk, script->Key(), lock);
	
	//	allocate memory for the code
	MemID m;
	m.Get(MemCode, hunk, script->codeOffset,
		hunk->fixups - script->codeOffset, TRANSITORY);
	script->code = m;
	
	//	use the table generated from when we originally loaded this module
	//	to fix up the code
	CodeIDEntry::FixupEntry* fixups =
		(CodeIDEntry::FixupEntry*) *script->codeFixups;
	int nFixups = script->nCodeFixups;
	char* code = (char*) *script->code;
	for (int i = 0; i < nFixups; i++) {
		*(SCIUWord*) &code[fixups->offset] = fixups->id;
		fixups++;
	}

	hunk.Unlock();
}

ScriptID
ScriptPtr(unsigned n)
{
	// Return a pointer to the node for script n. Load the script if necessary.

	ScriptID sp;

	if (!(sp = FindScript(n)))
		sp = LoadScript(n);

	return sp;
}

static ScriptID
FindScript(unsigned n)
{
	// Return a pointer to the node for script n if it is in the script list,
	// or 0 if it is not in the list.

	return scripts.FindKey(n);
}

ObjectID
GetClass(unsigned n)
{
	if (n >= nClasses)
		msgMgr->Fatal(SrcLoc, Msg_ClassNumTooLarge, n);

	if (!classTbl[n].obj) {
		LoadScript(classTbl[n].scriptNum);
		if (!classTbl[n].obj)
			msgMgr->Fatal(Msg_LoadClass, n);
	}

	return classTbl[n].obj;
}

void
LoadClassTbl()
{
	MemID sciClassTblID = resMgr->Get(MemResVocab, CLASSTBL_VOCAB);
	nClasses = sciClassTblID.Size() / sizeof(SCI_ClassTblEntry);

	classTbl.Get(sizeof(ClassTbl) + nClasses * sizeof(ClassTblEntry));
	classTbl->nEntries = nClasses;
	for (int i = 0; i < nClasses; i++) {
		classTbl[i].scriptNum =
			((SCI_ClassTblEntry*) *sciClassTblID)[i].scriptNum;
		classTbl[i].obj = 0;
	}
}

void
DisposeAllScripts()
{
	// dispose of the entire script list (for restart game)

	for (ScriptID sp = scripts.First();
		  !scripts.IsEmpty();
		  sp = scripts.First())
		TossScript(sp, False);
}

static void
TossScript(ScriptID sp, Bool checkClones)
{
	unsigned n = (SCIUWord) sp->Key();

	// remove all classes belonging to script number n from the class table
	for (int i = 0 ; i < classTbl->nEntries; i++)
		if (classTbl[i].scriptNum == n)
			classTbl[i].obj = 0;

	//	dispose of the objects and strings
	for (i = 0; i < sp->nIDs; i++) {
		switch (sp->ids[i].type) {
			case ScriptEntry::ObjectEntry:
				((ObjectID) sp->ids[i].id).Free();
				break;
			case ScriptEntry::StringEntry:
				((TextID) sp->ids[i].id).Free();
				break;
			default:
				assert(!"Impossible ScriptEntry type being disposed");
		}
	}

	//	dispose of the other ids
	sp->ids.Free();
	sp->vars.Free();
	sp->dispatches.Free();
	sp->codeFixups.Free();

	if ((int) sp->code) {
		PMIP::TossMemID(sp->code, n);
		sp->code.Free();
	}

	if (checkClones && sp->nRefs)
		msgMgr->Fatal(Msg_LeftClone, n);

	scripts.Delete(sp);
	sp.Free();
}

/////////////////////////////////////////////////////////////////////////////

void
KScriptID(argList)
{
	pm.acc = GetDispatchEntry((ResNum) arg(1),
		argCount == 1 ? 0 : arg(2)).objectID;
}

void
KDisposeScript(argList)
{
	// Remove script n from the active script list and deallocate the space
	// taken by its code and variables.

	/* allow for return code calculation on disposescript
		This prevents return
				(DisposeScript self)
				(return value)
		from getting "executing in disposed script" error
		by using
				(DisposeScript self value)
	*/

	if (argCount == 2)
		pm.acc = arg(2);

	ScriptID sp;

	if (int(sp = FindScript(arg(1))))
		TossScript(sp, True);
}

///////////////////////////////////////////////////////////////////////////

ScriptMgr scriptMgr;

void
ScriptMgr::SaveRestoreBefore(SaveGame*, Bool)
{
}

void
ScriptMgr::SaveRestore(SaveGame* saveGame, Bool save)
{
	saveGame->SaveRestore(&classTbl, sizeof classTbl, save);
	saveGame->SaveRestore(nClasses, save);
}

void
ScriptMgr::SaveRestoreAfter(SaveGame*, Bool save)
{
	//	reload script code

	if (!save)
		for (ScriptID id = scripts.First(); (int) id; id = id->Next())
			ReloadScript(id);
}

void
ScriptMgr::SaveRestoreDo(Saveable::Func func, SaveGame* saveGame, Bool save)
{
	scripts.SaveRestoreDo(func, saveGame, save);

	Saveable::SaveRestoreDo(func, saveGame, save);
}
