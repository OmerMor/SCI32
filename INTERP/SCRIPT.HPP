//	script.hpp	6/2/93

#ifndef SCRIPT_HPP
#define SCRIPT_HPP

#ifndef MEMID_HPP
#include "memid.hpp"
#endif

#ifndef LIST_HPP
#include "list.hpp"
#endif

#ifndef SAVEABLE_HPP
#include "saveable.hpp"
#endif

typedef SCIWord	Property;	//	duplicated in OBJECT.HPP until I can figure out
										//	how to remove circularity
class ObjectID;

struct ScriptMgr : Saveable {
	void	SaveRestoreBefore(SaveGame*, Bool);
	void	SaveRestore(SaveGame*, Bool save);
	void	SaveRestoreAfter(SaveGame*, Bool);
	void	SaveRestoreDo(Saveable::Func, SaveGame*, Bool save);
};
extern ScriptMgr scriptMgr;

///////////////////////////////////////////////////////////////////////////

// struct of script heap resource file
_Packed struct ScriptHeap {
	UInt16	fixups;		// offset to fixup table
	UInt16	nVars;		// number of variables
	Int16		vars;			// beginning of variables
};

FakeTemplateID(ScriptHeapID, ScriptHeap, MemResHeap)

// struct of script hunk resource file
_Packed struct ScriptHunk {
	UInt16	fixups;		// offset to fixup table
	UInt16	script;		// pointer to script node in heap [obsolete]
	UInt16	hasFarText;	// does this resource have far text?
	UInt16	nDispatches;// number of dispatch table entries
	UInt16	dispatches;	// beginning of dispatch table
};

FakeTemplateID(ScriptHunkID, ScriptHunk, MemResHunk)

////////////////////////////////////////////////////////////////////////

union DispatchEntry {
	UInt16	objectID;		//	id of object (can't be a MemID, since
									//	MemID has a constructor)
	UInt16	codeOffset;		//	offset of code in script's code block
};

struct DispatchTbl {
	UInt16	nEntries;	//	number of entries
	//	followed by variable number of DispatchEntrys
};

FakeTemplateID(FakeDispatchTblID, DispatchTbl, MemDispatchTbl)
struct DispatchTblID : FakeDispatchTblID
{
	DispatchTblID() {}
	DispatchTblID(const MemID& id) : FakeDispatchTblID(id) {}
	DispatchEntry&	operator[](int s)
		{ return ((DispatchEntry*) (**this + 1))[s]; }
};

////////////////////////////////////////////////////////////////////////

struct ScriptEntry {
	enum Type {	None, ObjectEntry, StringEntry } type;
	MemID	id;
};

FakeTemplateID(FakeScriptEntryID, ScriptEntry, MemScriptEntry)
struct ScriptEntryID : FakeScriptEntryID
{
	ScriptEntryID() {}
	ScriptEntryID(const MemID& id) : FakeScriptEntryID(id) {}
	ScriptEntry& operator[](int s) { return (**this)[s]; }
};

////////////////////////////////////////////////////////////////////////

struct Variable
{
	Property	var;
};

FakeTemplateID(FakeVariableID, Variable, MemVariables)
struct VariableID : FakeVariableID
{
	VariableID() {}
	VariableID(const MemID& id) : FakeVariableID(id) {}
	Property&	operator[](int s) { return (**this)[s].var; }
};

////////////////////////////////////////////////////////////////////////

struct Script : SOL_Node
{
	Script() : nRefs(0), nIDs(0), codeOffset(0), nCodeFixups(0)
	{
	}

	unsigned			nRefs;		// # of objects that point to script
	VariableID		vars;			// id of variables
	DispatchTblID	dispatches;	//	id of dispatch table
	unsigned			nIDs;			//	# of objects and strings
	ScriptEntryID	ids;			//	id of table of objects and strings
	MemID				code;			//	id of procedure and method code
	MemID				codeFixups;	//	table of fixups for restoring savegame
	int				nCodeFixups;//	how many
	size_t			codeOffset;	//	offset of code from beginning of .SCR
};

FakeTemplateID(ScriptID, Script, MemScript)

////////////////////////////////////////////////////////////////////////

ObjectID			GetClass(unsigned);
void				LoadClassTbl();
ScriptID			LoadScript(unsigned num);
ScriptID			ScriptPtr(unsigned num);
void				DisposeAllScripts();
DispatchEntry	GetDispatchEntry(unsigned scriptNum, unsigned entryNum,
						ScriptID* script = 0);

#endif
