//	message.cpp

#include <stdio.h>
#include <string.h>

#include "sol.hpp"

#include "array.hpp"
#include "config.hpp"
#include "kernel.hpp"
#include "mono.hpp"
#include	"message.hpp"
#include "msg.hpp"
#include "pmachine.hpp"
#include "resource.hpp"
#include "savegame.hpp"
#include "string.hpp"
#include "textid.hpp"

static ResNum	curModule;
static uint		curNoun;
static uint		curVerb;
static uint		curCond;
static uint		curSeq;

////////////////////////////////////////////////////////////////////////////

/*	A MsgStack keeps track of where we are in a sequence of messages, which
	may contain references, and references to references, etc.  When we come
	to the end of a sequence, we pop the stack to determine if we were 
	in a referenced sequence and carry on from there
*/

struct MsgEntry {
	uchar	noun;
	uchar	verb;
	uchar	cond;
	uchar	seq;
};

struct MsgStack {
	void		Init(ResNum module, uchar noun, uchar verb, uchar cond, uchar seq);
	void 		Push(uchar noun, uchar verb, uchar cond, uchar seq);
	Bool 		Pop();
	MsgEntry Top();
	void		IncSeq();

	ResNum	module;

private:
	enum { Max = 10 };

	MsgEntry	entries[Max];
	int		sp;
};

typedef ID<MsgStack, MemMsgStack>	MsgStackID;

static MsgStack stack;

///////////////////////////////////////////////////////////////////////////////

/*	A MsgStackStack lets SCI code save the current state of a message iteration
	so it can display a different set of messages, then return to the previous
	state.
*/

struct MsgStackStack {
	MsgStackStack();

	void		Push(MsgStack&);
	MsgStack	Pop();

private:
	enum { Max = 5 };

	MsgStackID	stack[Max];
	int			sp;
};

static MsgStackStack stackStack;

///////////////////////////////////////////////////////////////////////////////

enum {
	MSG_GET,
	MSG_NEXT,
	MSG_SIZE,
	MSG_SIZENEXT,
	MSG_GETREFNOUN,
	MSG_GETREFVERB,
	MSG_GETREFCASE,
	MSG_PUSH,
	MSG_POP,
	MSG_GETKEY
};

static void 	Get(ResNum module, uchar noun, uchar verb, uchar cond, uchar seq,
						TextID buffer);
static Bool 	GetRefValues(ResNum module, uchar noun, uchar verb, uchar cond,
						uchar seq, uchar* refNoun, uchar* refVerb, uchar* refCond);
static void 	GetNext(TextID buffer);
static void		GetSize(ResNum module, uchar noun, uchar verb, uchar cond,
						uchar seq);
static MemID	Find(MsgStack& stack, size_t* textOffset = 0,
						size_t* indexOffset = 0, Bool deep = True);
static void 	MassageText(char* dst, char* src);

#ifdef DEBUG
static MemID	FindTranslation(size_t* textOffset);
static void  	ShowTranslation();
#endif

////////////////////////////////////////////////////////////////////////////

void
KMessage(argList)
{
	Bool	rc;

	switch (arg(1)) {
		case MSG_GET:
			Get(	(ResNum) arg(2),							// module
					arg(3),										// noun
					arg(4),										// verb
					arg(5),										// case
					arg(6),										// sequence
					TextID(argCount < 7 ? 0 : arg(7))	// buffer
				);
			break;

		case MSG_NEXT:
			GetNext(TextID(argCount < 2 ? 0 : arg(2))); //	buffer
			break;

		case MSG_SIZE:
			GetSize(	(ResNum) arg(2),	// module
						arg(3),				// noun
						arg(4),				// verb
						arg(5),				// case
						arg(6)				// sequence
					 );
			break;
		
		case MSG_GETREFNOUN:
		case MSG_GETREFVERB:
		case MSG_GETREFCASE:
			uchar	refNoun;
			uchar	refVerb;
			uchar	refCond;
			rc = GetRefValues((ResNum) arg(2),		// module
										 		arg(3),		// noun
										 		arg(4),		// verb
										 		arg(5),		// case
										 		arg(6),		// sequence
										 		&refNoun,
										 		&refVerb,
										 		&refCond
							 );
			if (!rc)
				pm.acc = -1;
			else 
				pm.acc = arg(1) == MSG_GETREFNOUN ? refNoun :
						   arg(1) == MSG_GETREFVERB ? refVerb :
						                              refCond;
			break;

		case MSG_PUSH:
			stackStack.Push(stack);
			break;
			
		case MSG_POP:
			stack = stackStack.Pop();
			break;
		
		case MSG_GETKEY:
		{
			Property* data = (Property*) ((ArrayID) arg(2)).Data();
			data[0] = curModule;
			data[1] = curNoun;
			data[2] = curVerb;
			data[3] = curCond;
			data[4] = curSeq;
			break;
		}
		
		default:
			msgMgr->Fatal(SrcLoc, Msg_Message, arg(1));
	}
}

static void 
Get(ResNum module, uchar noun, uchar verb, uchar cond, uchar seq, TextID buffer)
{
	//	sets current message stack parameters and gets message associated with
	//	them
	//	if found,pm.acc = talker number and if buffer supplied, copies the message
	//		to the buffer minus stage directions
	//	else pm.acc = 0 and if buffer supplied, copies an error message to it

	stack.Init(module, noun, verb, cond, seq);
	
	GetNext(buffer);
}

static void 
GetNext(TextID buffer)
{
	//	finds the next message in the sequence on the stack
	//	if found,pm.acc = talker number and if buffer supplied, copies the message
	//		to the buffer minus stage directions and increments current seq num
	//	else pm.acc = 0 and if buffer supplied, copies an error message to it

	//	save the stack in case of error or if this is just a query
	MsgStack saveStack = stack;

	size_t	textOffset;
	size_t	indexOffset;
	MemID id = Find(stack, &textOffset, &indexOffset);

	if (!id) {
		if ((int) buffer) {
			MsgEntry entry = saveStack.Top();
			buffer.Realloc(100);
			sprintf(*buffer, "Msg %d: %d %d %d %d not found",
									saveStack.module, entry.noun, entry.verb, entry.cond,
									entry.seq);
			buffer.Snug();
		}
		pm.acc = 0;

	} else {
		if ((int) buffer) {
			buffer.Realloc(strlen(&id[textOffset]) + 1);
			MassageText(*buffer, &id[textOffset]);
			MsgEntry entry = stack.Top();
			curModule = stack.module;
			curNoun = entry.noun;
			curVerb = entry.verb;
			curCond = entry.cond;
			curSeq = entry.seq;
			stack.IncSeq();
#ifdef DEBUG
			if (configMgr->Get(ConfigMgr::Translation))
				ShowTranslation();
#endif
		}
		pm.acc = ((IndexEntry*) &id[indexOffset])->talker;
	}
	
	//	if this is just a query, restore stack
	if (!buffer)
		stack = saveStack;
	else
		buffer.Snug();
}

static void 
GetSize(ResNum module, uchar noun, uchar verb, uchar cond, uchar seq)
{
	//	return size of buffer needed to hold the message, including trailing
	//	0, or 0 if message not found

	//	use a local stack, since no need to save info across calls
	MsgStack	stack;
	stack.Init(module, noun, verb, cond, seq);

	//	find the message
	size_t textOffset;
	MemID id = Find(stack, &textOffset);
	if (!id)
		pm.acc = 0;
	else {
		//	make a buffer to massage it into
		TextID buf;
		buf.Get(strlen(&id[textOffset]) + 1);
		MassageText(*buf, &id[textOffset]);
	
		//	return the massaged length plus one for the trailing null
		pm.acc = strlen(*buf) + 1;
	
		buf.Free();
	}
}

static Bool 
GetRefValues(ResNum module, uchar noun, uchar verb, uchar cond, uchar seq,
				 uchar* refNoun, uchar* refVerb, uchar* refCond)
{
	//	set reference values for this message, returning False if message not
	//	found.

	//	use a local stack, since no need to save info across calls
	MsgStack stack;
	stack.Init(module, noun, verb, cond, seq);

	size_t indexOffset;
	MemID id;
	if (!(id = Find(stack, 0, &indexOffset, False)))
		return False;

	IndexEntry* indexEntry = (IndexEntry*) &id[indexOffset];
	*refNoun = indexEntry->refNoun;
	*refVerb = indexEntry->refVerb;
	*refCond = indexEntry->refCond;

	return True;
}

static MemID
Find(MsgStack& stack, size_t* textOffset, size_t* indexOffset, Bool deep)
{
	//	finds a message and returns its MemID and the offsets of its text and
	//	index entry if found, else 0
	//	'deep' means whether reference chain is followed

	MemID			id = resMgr->Get(MemResMsg, stack.module);
	MsgData*		data = (MsgData*) *id;
	IndexEntry*	ip;
	
	if (data->version < MessageMajorVersion)
		msgMgr->Fatal(SrcLoc, Msg_BadMsgVersion);
	
	while (1) {
		//	search for message on top of stack
		MsgEntry entry;
		entry = stack.Top();
		int i;
		for (i = 0, ip = data->entries; i < data->nEntries; i++, ip++)
			if (ip->noun == entry.noun &&
				 ip->verb == entry.verb &&
				 ip->cond == entry.cond &&
				 ip->seq == entry.seq)
				break;
		
		//	if wasn't found, try to pop; otherwise return failure
		if (i == data->nEntries)
			if (deep && stack.Pop())
				continue;
			else
				return 0;
		
		//	if it's a reference, push it onto the stack and go back around
		if (deep && (ip->refNoun || ip->refVerb || ip->refCond)) {
			stack.IncSeq();
			stack.Push(ip->refNoun, ip->refVerb, ip->refCond, 1);
			continue;
		}

		//	otherwise, return the message
		if (textOffset)
			*textOffset = ip->offset;
		if (indexOffset)
			*indexOffset = (char*) ip - (char*) *id;
		return id;
	}
}

#ifdef DEBUG
static MemID
FindTranslation(size_t* textOffset)
{
	//	finds the translation of the current message and returns its MemID and the
	// offset of its text if found, else 0

	MemID			id = resMgr->Get(MemResMsgTranslation, curModule);
	MsgData*		data = (MsgData*) *id;
	IndexEntry*	ip;
	
	if (data->version < MessageMajorVersion)
		msgMgr->Fatal(SrcLoc, Msg_BadMsgVersion);
	
	int i;
	for (i = 0, ip = data->entries; i < data->nEntries; i++, ip++)
		if (ip->noun == curNoun &&
			 ip->verb == curVerb &&
			 ip->cond == curCond &&
			 ip->seq == curSeq)
			break;
	
	//	if wasn't found, return failure
	if (i == data->nEntries)
		return 0;

	//	otherwise, return the message
	*textOffset = ip->offset;
	return id;
}
#endif

static void 
MassageText(char* dst, char* src)
{
	//	copies src into dst, deleting stage directions, which are phrases
	//	in caps enclosed by parens; and interpreting escape sequences of
	//	the form \xx, where x is a hex digit

	static char	hexDigits[] = "01234567890abcdef";

	char*	dp = dst;

	for (; *src; src++) {
		if (*src == '\\') {
			char* cp;
			int	val = 0;
			char	c;

			src++;

			//	check first digit
			c = ToLower(*src);
			if (!(cp = strchr(hexDigits, c))) {
				*dp++ = *src;
				continue;
			} else {
				val = val * 16 + cp - hexDigits;
				src++;
			}

			//	check second digit
			c = ToLower(*src);
			if (!(cp = strchr(hexDigits, c))) {
				src--;
				continue;
			} else
				val = val * 16 + cp - hexDigits;

			*dp++ = (uchar) val;

		} else if (*src == '(') {
			//	find end of this stage direction (if it is) and set src to it
			for (char* end = src; *end; end++)
				if (*end == ')') {
					/*	found end of stage direction:
						strip trailing whitespace, then exit	*/
					end++;
					while (*end && (*end == ' ' || *end == '\n' || *end == '\r'))
						end++;
					src = --end;
					break;

				} else if (*end >= 'a' && *end <= 'z') {
					//	it's not a stage direction
					*dp++ = *src;
					break;
				}

		} else
			*dp++ = *src;
	}
	*dp = '\0';
}

#ifdef DEBUG
static void 
ShowTranslation()
{
	MemID		msg;
	size_t	offset;
	TextID	text;

	if (msg = FindTranslation(&offset)) {
		text.Get(strlen(&msg[offset]) + 1);
		MassageText(*text, &msg[offset]);
	} else
		text.Add("Translation not found");

	text.Add('\n');
	msgMgr->MonoStr(*text);
	text.Free();
}
#endif

///////////////////////////////////////////////////////////////////////////

void 
MsgStack::Init(ResNum module, uchar noun, uchar verb, uchar cond, uchar seq)
{
	//	start a new stack

	this->module = module;
	sp = -1;
	
	Push(noun, verb, cond, seq);
}

void 
MsgStack::Push(uchar noun, uchar verb, uchar cond, uchar seq)
{
	//	push a message onto stack

	if (++sp >= Max)
		msgMgr->Fatal(SrcLoc, Msg_MsgStackOverflow, Max);
	entries[sp].noun	= noun;
	entries[sp].verb	= verb;
	entries[sp].cond	= cond;
	entries[sp].seq	= seq;
}

MsgEntry 
MsgStack::Top()
{
	//	return the message on the top of stack, but don't pop it off

	MsgEntry entry;
	entry.noun	= entries[sp].noun;
	entry.verb	= entries[sp].verb;
	entry.cond	= entries[sp].cond;
	entry.seq	= entries[sp].seq;
	return entry;
}

Bool 
MsgStack::Pop()
{
	//	throw away top of stack

	return --sp >= 0;
}

void 
MsgStack::IncSeq()
{
	//	increment the sequence number of the entry on the top of stack

	entries[sp].seq++;
}

///////////////////////////////////////////////////////////////////////////

MsgStackStack::MsgStackStack() : sp(-1)
{
}

void 
MsgStackStack::Push(MsgStack& ms)
{
	//	save current stack on stack stack

	if (++sp >= Max)
		msgMgr->Fatal(SrcLoc, Msg_MsgStackStackOverflow, Max);

	stack[sp].Get();
	**stack[sp] = ms;
}

MsgStack
MsgStackStack::Pop()
{
	//	restore previous saved stack
	
	if (sp < 0)
		msgMgr->Fatal(SrcLoc, Msg_MsgStackStackUnderflow);
		
	MsgStackID id = stack[sp--];
	MsgStack stack = **id;
	id.Free();
	return stack;
}

///////////////////////////////////////////////////////////////////////////

MessageMgr messageMgr;

void
MessageMgr::SaveRestore(SaveGame* saveGame, Bool save)
{
	saveGame->SaveRestore(&curModule, sizeof curModule, save);
	saveGame->SaveRestore(&curNoun, sizeof curNoun, save);
	saveGame->SaveRestore(&curVerb, sizeof curVerb, save);
	saveGame->SaveRestore(&curCond, sizeof curCond, save);
	saveGame->SaveRestore(&curSeq, sizeof curSeq, save);

	saveGame->SaveRestore(&stack, sizeof stack, save);

	saveGame->SaveRestore(&stackStack, sizeof stackStack, save);
}
