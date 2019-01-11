//	opcodes.cpp

#define WATCOM95a

#include "sol.hpp"

#include "audio.hpp"
#include "debug.hpp"
#include "event.hpp"
#include "kernel.hpp"
#include "msg.hpp"
#include "object.hpp"
#include "opcodes.hpp"
#include "pmachine.hpp"
#include "script.hpp"

extern "C" void JumpToOpcode(uchar);

extern "C" void Leave();
#pragma aux Leave = "leave"

extern "C" void PopEBX();
#pragma aux PopEBX = "pop ebx"

extern "C" void AddESP4();
#pragma aux AddESP4 = "add esp, 4"

extern "C" void AddESP8();
#pragma aux AddESP8 = "add esp, 8"

extern "C" void PopEDI_ESI_EBX();
#pragma aux PopEDI_ESI_EBX = "pop edi", "pop esi", "pop ebx"

extern "C" void PopESI_EBX();
#pragma aux PopESI_EBX = "pop esi", "pop ebx"

extern "C" void AddESP4_PopESI_EBX();
#pragma aux AddESP4_PopESI_EBX = "add esp, 4", "pop esi", "pop ebx"

extern "C" void AddESP4_PopEDI_ESI_EBX();
#pragma aux AddESP4_PopEDI_ESI_EBX = \
											"add esp, 4", "pop edi", "pop esi", "pop ebx"

extern "C" void AddESP4_PopEBX();
#pragma aux AddESP4_PopEBX = "add esp, 4", "pop ebx"

extern "C" void AddESP8_PopEBX();
#pragma aux AddESP8_PopEBX = "add esp, 8", "pop ebx"

extern "C" void AddESPC();
#pragma aux AddESPC = "add esp, 0Ch"

extern "C" void AddESP10();
#pragma aux AddESP10 = "add esp, 10h"

extern "C" int GetEBX();
#pragma aux GetEBX = "mov eax, ebx"

extern "C" int GetESI();
#pragma aux GetESI = "mov eax, esi"

extern "C" int GetEDI();
#pragma aux GetEDI = "mov eax, edi"

extern "C" int GetESP();
#pragma aux GetESP = "mov eax, esp"

void
Checker()
{
	static uchar lastOpcode;
	static uchar thisOpcode;
	static int lastESP;
	static int thisESP;
	static Bool first = True;
	static int thisEBX;
	static int lastEBX;
	static int thisESI;
	static int lastESI;
	static int thisEDI;
	static int lastEDI;

	thisESP = GetESP();
	thisEBX = GetEBX();
	thisESI = GetESI();
	thisEDI = GetEDI();

	thisOpcode = pm.ip.PeekChar();
	
	if (!first) {
		if (lastESP != thisESP && lastOpcode != 0x4A && lastOpcode != 0x56 &&
			 lastOpcode != 0x57 && lastOpcode != 0x48 && lastOpcode != 0x54 &&
			 lastOpcode != 0x55 &&
			 (lastOpcode < 0x40 || lastOpcode > 0x47)) {
			if (lastESP != thisESP)
				msgMgr->Fatal("ESP changed: %X\n", lastOpcode);
			if (lastEBX != thisEBX)
				msgMgr->Fatal("EBX changed: %X\n", lastOpcode);
			if (lastESI != thisESI)
				msgMgr->Fatal("ESI changed: %X\n", lastOpcode);
			if (lastEDI != thisEDI)
				msgMgr->Fatal("EDI changed: %X\n", lastOpcode);
		}
	} else
		first = False;

	lastESP = thisESP;
	lastEBX = thisEBX;
	lastEDI = thisEDI;
	lastESI = thisESI;
	lastOpcode = thisOpcode;
}

inline void
NextOpcode()
{
	//Checker();
#ifdef DEBUG
	if (debuggerIsOn)
		Debug();
#endif
	JumpToOpcode(pm.ip.GetUChar());
}

static void
BadOp()
{
	//	get back to opcode and abort

	msgMgr->Fatal(SrcLoc, Msg_BadOpcode, *--pm.ip);
}

static void
OP_add()
{
	//	add stack top to accumulator

	pm.acc += pm.stack.Pop();
	NextOpcode();
}

static void
OP_and()
{
	//	and accumulator with stack top

	pm.acc &= pm.stack.Pop();
	NextOpcode();
}

static void
OP_bnot()
{
	//	reverse bits in accumulator

	pm.acc ^= 0xFFFF;
	NextOpcode();
}

static void
OP_bt_Char()
{
	//	branch if acc is nonzero

	if (pm.acc)
		pm.ip += pm.ip.GetChar();
	else
		pm.ip++;
	NextOpcode();
}

static void
OP_bt_Word()
{
	//	branch if acc is nonzero

	if (pm.acc)
		pm.ip += pm.ip.GetWord();
	else
		pm.ip += 2;
	NextOpcode();
}

static void
OP_bnt_Char()
{
	//	branch if acc is zero

	if (!pm.acc)
		pm.ip += pm.ip.GetChar();
	else
		pm.ip++;
	NextOpcode();
}

static void
OP_bnt_Word()
{
	//	branch if acc is zero

	if (!pm.acc)
		pm.ip += pm.ip.GetWord();
	else
		pm.ip += 2;
	NextOpcode();
}

inline void
OP_call(int offset)
{
	//	call a procedure in the current script

	unsigned nArgChars = pm.ip.GetUWord();

	pm.ip.Push();

	pm.Call(pm.ip + offset, nArgChars);

	pm.ip.Pop();

	Leave();
	NextOpcode();
}

static void
OP_call_Char()
{
	OP_call(pm.ip.GetChar());
}

static void
OP_call_Word()
{
	OP_call(pm.ip.GetWord());
}

inline void
OP_callb(int entry)
{
	//	call a procedure in the base script

	pm.Dispatch(0, entry, pm.ip.GetUWord());

	PopEBX();
	NextOpcode();
}

static void
OP_callb_Char()
{
	OP_callb(pm.ip.GetUChar());
}

static void
OP_callb_Word()
{
	OP_callb(pm.ip.GetUWord());
}

static void
OP_calle_Char()
{
	//	call a procedure in another script

	unsigned script	= pm.ip.GetUChar();
	unsigned entry		= pm.ip.GetUChar();
	pm.Dispatch(script, entry, pm.ip.GetUWord());
	
#ifdef DEBUG
	AddESP4_PopEBX();
#else
	AddESP8();
#endif
	NextOpcode();
}

static void
OP_calle_Word()
{
	//	call a procedure in another script

	unsigned script= pm.ip.GetUWord();
	unsigned entry	= pm.ip.GetUWord();

	pm.Dispatch(script, entry, pm.ip.GetUWord());

#if defined(WATCOM95a) && defined(DEBUG)
	AddESP4_PopEBX();
#else
	AddESP8();
#endif
	NextOpcode();
}

inline void

OP_callk(int entry)
{
	//	call a kernel routine

	AsyncEventCheck();

	if (entry > kernelMax)
		msgMgr->Fatal(SrcLoc, Msg_BadKernel, entry);
		
	//	point at beginning of arguments
	PMStack::Ptr args = pm.stack.ptr -
		(pm.ip.GetUWord() + pm.restArgs) / sizeof(PMStack::Entry);

	//	adjust # args by pm.restArgs
	*args += pm.restArgs / sizeof(PMStack::Entry);
	pm.restArgs = 0;

#ifdef DEBUG
	RegisterKernelCall(entry);
#endif
	kernelCalls[entry].func(args);

	//	pop args off pm.stack
	pm.stack.ptr = args - 1;
	
#ifdef WATCOM95a
	#ifdef DEBUG
		PopESI_EBX();
	#else
		AddESP4_PopESI_EBX();
	#endif
#else
	AddESP4_PopEBX();
#endif

	NextOpcode();
}

static void
OP_callk_Char()
{
	OP_callk(pm.ip.GetUChar());
}

static void
OP_callk_Word()
{
	OP_callk(pm.ip.GetUWord());
}

inline void
OP_class(int classNum)
{
	//	load the accumulator with pointer to a class

	pm.acc = (Acc) GetClass(classNum);
	
	AddESP8();
	NextOpcode();
}

static void
OP_class_Char()
{
	OP_class(pm.ip.GetUChar());
}

static void
OP_class_Word()
{
	OP_class(pm.ip.GetUWord());
}

static void
OP_div()
{
	//	divide stack top by accumulator

	if (!pm.acc)
		msgMgr->Fatal(SrcLoc, Msg_ZeroDivide);
	pm.acc = (Acc) (pm.stack.Pop() / pm.acc);

	AddESP4();
	NextOpcode();
}

static void
OP_dup()
{
	//	put another copy of the stack top on the stack

	int val = pm.stack.Peek();
	pm.stack.Push(val);

	NextOpcode();
}

static void
OP_jmp_Char()
{
	//	unconditional branch

	pm.ip += pm.ip.GetChar();
	NextOpcode();
}

static void
OP_jmp_Word()
{
	//	unconditional branch

	pm.ip += pm.ip.GetWord();
	NextOpcode();
}

static void
OP_lea(int, int)
{
	//	load the address of a variable into the accumulator

	msgMgr->Fatal(SrcLoc, Msg_VariableAddress);
}

static void
OP_lea_Char()
{
	int varType = pm.ip.GetUChar();
	OP_lea(varType, pm.ip.GetUChar());
}

static void
OP_lea_Word()
{
	int varType = pm.ip.GetUWord();
	OP_lea(varType, pm.ip.GetUWord());
}

inline void
OP_link(int nChars)
{
	//	create space for temporary variables

	pm.stack.temps = pm.stack.ptr + 1;
	pm.stack.ptr += nChars;
	pm.stack.CheckOverflow();

	AddESP4();
	NextOpcode();
}

static void
OP_link_Char()
{
	OP_link(pm.ip.GetUChar());
}

static void
OP_link_Word()
{
	OP_link(pm.ip.GetUWord());
}

static void
OP_loadi_Char()
{
	pm.acc = pm.ip.GetChar();
	NextOpcode();
}

static void
OP_loadi_Word()
{
	pm.acc = pm.ip.GetWord();
	NextOpcode();
}

static void
OP_lofsa()
{
	//	load a MemID into the accumulator

	pm.acc = pm.ip.GetWord();
	NextOpcode();
}

static void
OP_lofss()
{
	//	load a MemID onto the stack

	pm.stack.Push(pm.ip.GetWord());
	NextOpcode();
}

static void
OP_mod()
{
	//	do the modulo of the stack top by the accumulator

	if (!pm.acc)
		msgMgr->Fatal(SrcLoc, Msg_ZeroModulo);

	//	insure modulo positive
	if (pm.acc < 0)
		pm.acc = (Acc) -pm.acc;

	int val = pm.stack.Pop() % pm.acc;

	//	make 0 <= modulo < (modulo - 1)
	if (val < 0)
		pm.acc += (Acc) val;
	else
		pm.acc = (Acc) val;

	AddESP4();
	NextOpcode();
}

static void
OP_mul()
{
	//	multiply accumulator by stack top

	pm.acc *= pm.stack.Pop();
	NextOpcode();
}

static void
OP_neg()
{
	//	negate the accumulator

	pm.acc = (Acc) -pm.acc;
	NextOpcode();
}

static void
OP_not()
{
	//	do a logical not on accumulator

	pm.acc = !pm.acc;
	NextOpcode();
}

static void
OP_or()
{
	//	or accumulator with stack top

	pm.acc |= pm.stack.Pop();
	NextOpcode();
}

static void
OP_pprev()
{
	//	push the value of acc before the last comparison

	pm.stack.Push(pm.prevAcc);
	NextOpcode();
}

static void
OP_push()
{
	pm.stack.Push(pm.acc);
	NextOpcode();
}

static void
OP_push0()
{
	pm.stack.Push(0);
	NextOpcode();
}

static void
OP_push1()
{
	pm.stack.Push(1);
	NextOpcode();
}

static void
OP_push2()
{
	pm.stack.Push(2);
	NextOpcode();
}

static void
OP_pushi_Char()
{
	pm.stack.Push(pm.ip.GetChar());
	NextOpcode();
}

static void
OP_pushi_Word()
{
	pm.stack.Push(pm.ip.GetWord());
	NextOpcode();
}

static void
OP_pushSelf()
{
	pm.stack.Push(pm.object);
	NextOpcode();
}

static void
OP_rest()
{
	//	add the 'rest' of the current stack frame to the parameters
	//	which are already there

	unsigned nParms  			= *pm.stack.parms;
	unsigned firstAddedParm = pm.ip.GetUChar();
	int nParmsToAdd			= nParms - firstAddedParm + 1;

	if (nParmsToAdd > 0) {
		pm.restArgs += nParmsToAdd * sizeof(PMStack::Entry);
		pm.stack.PushMany(&pm.stack.parms[firstAddedParm], nParmsToAdd);
	}
	
	PopEDI_ESI_EBX();
	NextOpcode();
}

static void
OP_ret()
{
}

static void
OP_self()
{
	//	send to the current object

	pm.Messager(pm.object);
	NextOpcode();
}

static void
OP_selfID()
{
	//	load accumulator with current object

	pm.acc = (Acc) pm.object;
	NextOpcode();
}

static void
OP_send()
{
	//	send messages to the object in the accumulator

	ObjectID saveObject = pm.object;

	pm.object = (ObjectID) pm.acc;
	pm.Messager(pm.object);

	pm.object = saveObject;
	
	AddESP4();
	NextOpcode();
}

static void
OP_shl()
{
	//	shift left stack top by accumulator

	pm.acc = Acc(pm.stack.Pop() << pm.acc);
	NextOpcode();
}

static void
OP_shr()
{
	//	shift right stack top by accumulator
	//	this is done unsigned because (supposedly) that's what everyone
	//	expects

	pm.acc = Acc((SCIUWord) pm.stack.Pop() >> pm.acc);
	NextOpcode();
}

static void
OP_sub()
{
	//	subtract accumulator from stack top

	pm.acc = Acc(pm.stack.Pop() - pm.acc);
	NextOpcode();
}

inline void
OP_super(unsigned classNum)
{
	//	send to a class

	pm.Messager(GetClass(classNum));
	
	AddESP4_PopEBX();
	NextOpcode();
}

static void
OP_super_Char()
{
	OP_super(pm.ip.GetUChar());
}

static void
OP_super_Word()
{
	OP_super(pm.ip.GetUWord());
}

static void
OP_toss()
{
	//	throw away the stack top

	pm.stack.ptr--;
	NextOpcode();
}

static void
OP_xor()
{
	//	xor accumulator with stack top

	pm.acc ^= pm.stack.Pop();
	NextOpcode();
}

////////////////////////////////////////////////////////////////////////////

#define DefineCmp(funcName, op)	\
static void						\
OP_##funcName##()						\
{											\
	pm.prevAcc = pm.acc;				\
	pm.acc = pm.stack.Pop() op pm.acc;	\
	NextOpcode();								\
}

DefineCmp(eq, ==)
DefineCmp(ne, !=)
DefineCmp(gt, >)
DefineCmp(ge, >=)
DefineCmp(lt, <)
DefineCmp(le, <=)

#define DefineUCmp(funcName, op)				\
static void									\
OP_##funcName##()									\
{														\
	pm.prevAcc = pm.acc;							\
	pm.acc = (SCIUWord) pm.stack.Pop() op (SCIUWord) pm.acc;		\
	NextOpcode();								\
}

DefineUCmp(ugt, >)
DefineUCmp(uge, >=)
DefineUCmp(ult, <)
DefineUCmp(ule, <=)

////////////////////////////////////////////////////////////////////////////

//	create a function for char and short-sized operands
#define DefineAllSizes(macro, procName, vars, op, fix) \
	static void procName##_Char()	{ macro##(vars, Char, op) fix##(); NextOpcode(); } \
	static void procName##_Word() { macro##(vars, Word, op) fix##(); NextOpcode(); }

//	create a function for each sized operands on all types of variables
#define DefineAllVars(macro, procName, op, fix, lfix) \
	DefineAllSizes##(macro, OP_##procName##t, pm.stack.temps, op, fix) \
	DefineAllSizes##(macro, OP_##procName##p, pm.stack.parms, op, fix) \
	DefineAllSizes##(macro, OP_##procName##g, pm.globalPtr, op, fix) \
	DefineAllSizes##(macro, OP_##procName##l, pm.locals, op, lfix)

//	read the index into some variables and get that variable
#define ReadVar(vars, size)			vars##[pm.ip.GetU##size##()]

//	read the index into some variables and get that variable, indexed
#define ReadVarIndexed(vars, size)	vars##[pm.ip.GetU##size##() + pm.acc]

////////////////////////////////////////////////////////////////////////////

//	define functions to load the accumulator
#define DefineLoadAcc(vars, size, op) \
	pm.acc = ReadVar(vars, size);
#ifdef DEBUG
DefineAllVars(DefineLoadAcc, la, "", AddESP4, AddESP8)
#else
DefineAllVars(DefineLoadAcc, la, "", PopEBX, AddESP8)
#endif

//	define functions to load the accumulator from indexed variable
#define DefineLoadAccIndexed(vars, size, op) \
	pm.acc = ReadVarIndexed(vars, size);
DefineAllVars(DefineLoadAccIndexed, lai, "", AddESP4, AddESP4)

//	define functions to push variables on the stack
#define DefineLoadStack(vars, size, op) \
	pm.stack.Push(ReadVar(vars, size));
#ifdef DEBUG
DefineAllVars(DefineLoadStack, ls, "", AddESP4, AddESP4)
#else
DefineAllVars(DefineLoadStack, ls, "", PopEBX, AddESP8)
#endif

//	define functions to push indexed variables on the stack
#define DefineLoadStackIndexed(vars, size, op) \
	pm.stack.Push(ReadVarIndexed(vars, size));
#ifdef DEBUG
DefineAllVars(DefineLoadStackIndexed, lsi, "", AddESP4, AddESPC)
#else
DefineAllVars(DefineLoadStackIndexed, lsi, "", AddESP4, AddESP8_PopEBX)
#endif

////////////////////////////////////////////////////////////////////////////

//	define functions to store the accumulator to a variable
#define DefineStoreAcc(vars, size, op) \
	ReadVar(vars, size) = pm.acc;
#ifdef DEBUG
DefineAllVars(DefineStoreAcc, sa, "", AddESP4, AddESP8)
#else
DefineAllVars(DefineStoreAcc, sa, "", PopEBX, AddESP8)
#endif

//	define functions to store the top of the stack to a variable
#define DefineStoreStack(vars, size, op) \
	ReadVar(vars, size) = pm.stack.Pop();
#ifdef DEBUG
DefineAllVars(DefineStoreStack, ss, "", AddESP4_PopEBX, AddESP8)
#else
DefineAllVars(DefineStoreStack, ss, "", AddESP4_PopEBX, AddESPC)
#endif

//	define functions to store the accumulator to an indexed variable
#define DefineStoreAccIndexed(vars, size, op) \
	pm.acc = ReadVarIndexed(vars, size) = pm.stack.Pop();
#ifdef DEBUG
DefineAllVars(DefineStoreAccIndexed, sai, "", AddESP8, AddESP10)
#else
DefineAllVars(DefineStoreAccIndexed, sai, "", AddESP8, AddESP8)
#endif

//	define functions to store the top of the stack to an indexed variable
#define DefineStoreStackIndexed(vars, size, op) \
	ReadVarIndexed(vars, size) = pm.stack.Pop();
#ifdef DEBUG
DefineAllVars(DefineStoreStackIndexed, ssi, "", AddESP4_PopEBX, AddESP8)
#else
DefineAllVars(DefineStoreStackIndexed, ssi, "", AddESP8, AddESP10)
#endif

////////////////////////////////////////////////////////////////////////////

//	define functions to load the accumulator with inced/deced variable
#define DefineIncDecToAcc(vars, size, op) \
	pm.acc = op ## ReadVar(vars, size);
#ifdef DEBUG
DefineAllVars(DefineIncDecToAcc, ia, ++, AddESP4, AddESP8)
DefineAllVars(DefineIncDecToAcc, da, --, AddESP4, AddESP8)
#else
DefineAllVars(DefineIncDecToAcc, ia, ++, PopEBX, AddESP8)
DefineAllVars(DefineIncDecToAcc, da, --, PopEBX, AddESP8)
#endif

//	define functions to push an inced/deced variable onto the stack
#define DefineIncDecToStack(vars, size, op) \
	pm.stack.Push(op ## ReadVar(vars, size));
#ifdef DEBUG
DefineAllVars(DefineIncDecToStack, is, ++, AddESP4, AddESP4)
DefineAllVars(DefineIncDecToStack, ds, --, AddESP4, AddESP4)
#else
DefineAllVars(DefineIncDecToStack, is, ++, PopEBX, AddESP8)
DefineAllVars(DefineIncDecToStack, ds, --, PopEBX, AddESP8)
#endif

//	define functions to load the accumulator with inced/deced indexed variable
#define DefineIncDecToAccIndexed(vars, size, op) \
	pm.acc = op ## ReadVarIndexed(vars, size);
DefineAllVars(DefineIncDecToAccIndexed, iai, ++, AddESP4, AddESP4)
DefineAllVars(DefineIncDecToAccIndexed, dai, --, AddESP4, AddESP4)

//	define functions to push an inced/deced indexed variable onto the stack
#define DefineIncDecToStackIndexed(vars, size, op) \
	pm.stack.Push(op ## ReadVarIndexed(vars, size));
#ifdef DEBUG
DefineAllVars(DefineIncDecToStackIndexed, isi, ++, AddESP4, AddESPC)
DefineAllVars(DefineIncDecToStackIndexed, dsi, --, AddESP4, AddESPC)
#else
DefineAllVars(DefineIncDecToStackIndexed, isi, ++, AddESP4, AddESP8_PopEBX)
DefineAllVars(DefineIncDecToStackIndexed, dsi, --, AddESP4, AddESP8_PopEBX)
#endif

////////////////////////////////////////////////////////////////////////////

//	create a function for char and short-sized operands for property access
#define DefineAllSizesProp(macro, getSet, procName, fix) \
	static void OP_##procName##_Char() \
		{ macro(pm.object.##getSet##PropAtOffset(pm.ip.GetUChar())) \
		  fix##(); \
		  NextOpcode(); \
		} \
	static void OP_##procName##_Word() \
		{ macro(pm.object.##getSet##PropAtOffset(pm.ip.GetUWord())) \
		  fix##(); \
		  NextOpcode(); \
		}

#define DefinePToa(operand)	pm.acc = operand##;
DefineAllSizesProp(DefinePToa, Get, pToa, AddESP4_PopESI_EBX);

#define DefinePTos(operand)	pm.stack.Push(##operand##);
#ifdef DEBUG
#ifdef WATCOM95a
DefineAllSizesProp(DefinePTos, Get, pTos, AddESP4_PopESI_EBX);
#else
DefineAllSizesProp(DefinePTos, Get, pTos, AddESP4_PopEBX);
#endif
#else
DefineAllSizesProp(DefinePTos, Get, pTos, AddESP4_PopESI_EBX);
#endif

#define DefineATop(operand)	operand = pm.acc;
DefineAllSizesProp(DefineATop, Set, aTop, PopEDI_ESI_EBX);

#define DefineSTop(operand)	operand = pm.stack.Pop();
DefineAllSizesProp(DefineSTop, Set, sTop, AddESP4_PopEDI_ESI_EBX);

#define DefineIPToa(operand)	pm.acc = ++operand##;
DefineAllSizesProp(DefineIPToa, Set, ipToa, PopEDI_ESI_EBX);

#define DefineIPTos(operand)	pm.stack.Push(++##operand##);
DefineAllSizesProp(DefineIPTos, Set, ipTos, PopEDI_ESI_EBX);

#define DefineDPToa(operand)	pm.acc = --operand##;
DefineAllSizesProp(DefineDPToa, Set, dpToa, PopEDI_ESI_EBX);

#define DefineDPTos(operand)	pm.stack.Push(++##operand##);
DefineAllSizesProp(DefineDPTos, Set, dpTos, PopEDI_ESI_EBX);

////////////////////////////////////////////////////////////////////////////

void
GetSourceLineNum()
{
	pm.curSourceLineNum = pm.ip.GetWord();
}

void
GetSourceModuleFileName()
{
	for (char* cp = pm.curSourceFile; *cp = pm.ip.GetChar(); cp++)
		;
}

static void
OP_fileName()
{
	//	process source module file name
	
	GetSourceModuleFileName();
	
	NextOpcode();
}

static void
OP_lineNum()
{
	//	process source line number
	
	GetSourceLineNum();

	NextOpcode();
}

////////////////////////////////////////////////////////////////////////////

void (*opTbl[256])() = {
	OP_bnot,				//	00
	BadOp,				//	01
	OP_add,				//	02
	BadOp,				//	03
	OP_sub,				//	04
	BadOp,				//	05
	OP_mul,				//	06
	BadOp,				//	07
	OP_div,				//	08
	BadOp,				//	09
	OP_mod,				//	0A
	BadOp,				//	0B
	OP_shr,				//	0C
	BadOp,				//	0D
	OP_shl,				//	0E
	BadOp,				//	0F
	OP_xor,				//	10
	BadOp,				//	11
	OP_and,				//	12
	BadOp,				//	13
	OP_or,				//	14
	BadOp,				//	15
	OP_neg,				//	16
	BadOp,				//	17
	OP_not,				//	18
	BadOp,				//	19
	OP_eq,				//	1A
	BadOp,				//	1B
	OP_ne,				//	1C
	BadOp,				//	1D
	OP_gt,				//	1E
	BadOp,				//	1F
	OP_ge,				//	20
	BadOp,				//	21
	OP_lt,				//	22
	BadOp,				//	23
	OP_le,				//	24
	BadOp,				//	25
	OP_ugt,				//	26
	BadOp,				//	27
	OP_uge,				//	28
	BadOp,				//	29
	OP_ult,				//	2A
	BadOp,				//	2B
	OP_ule,				//	2C
	BadOp,				//	2D
	OP_bt_Word,			//	2E
	OP_bt_Char,			//	2F
	OP_bnt_Word,		//	30
	OP_bnt_Char,		//	31
	OP_jmp_Word,		//	32
	OP_jmp_Char,		//	33
	OP_loadi_Word,		//	34
	OP_loadi_Char,		//	35
	OP_push,				//	36
	BadOp,				//	37
	OP_pushi_Word,		//	38
	OP_pushi_Char,		//	39
	OP_toss,				//	3A
	BadOp,				//	3B
	OP_dup,				//	3C
	BadOp,				//	3D
	OP_link_Word,		//	3E
	OP_link_Char,		//	3F
	OP_call_Word,		//	40
	OP_call_Char,		//	41
	OP_callk_Word,		//	42
	OP_callk_Char,		//	43
	OP_callb_Word,		//	44
	OP_callb_Char,		//	45
	OP_calle_Word,		//	46
	OP_calle_Char,		//	47
	OP_ret,				// 48		BadOp,				//	48
	BadOp,				//	49
	OP_send,				//	4A
	BadOp,				//	4B
	BadOp,				//	4C
	BadOp,				//	4D
	BadOp,				//	4E
	BadOp,				//	4F
	OP_class_Word,		//	50
	OP_class_Char,		//	51
	BadOp,				//	52
	BadOp,				//	53
	OP_self,				//	54
	OP_self,				//	55
	OP_super_Word,		//	56
	OP_super_Char,		//	57
	BadOp,				//	58
	OP_rest,				//	59
	OP_lea_Word,		//	5A
	OP_lea_Char,		//	5B
	OP_selfID,			//	5C
	BadOp,				//	5D
	BadOp,				//	5E
	BadOp,				//	5F
	OP_pprev,			//	60
	BadOp,				//	61
	OP_pToa_Word,		//	62
	OP_pToa_Char,		//	63
	OP_aTop_Word,		//	64
	OP_aTop_Char,		//	65
	OP_pTos_Word,		//	66
	OP_pTos_Char,		//	67
	OP_sTop_Word,		//	68
	OP_sTop_Char,		//	69
	OP_ipToa_Word,		//	6A
	OP_ipToa_Char,		//	6B
	OP_dpToa_Word,		//	6C
	OP_dpToa_Char,		//	6D
	OP_ipTos_Word,		//	6E
	OP_ipTos_Char,		//	6F
	OP_dpTos_Word,		//	70
	OP_dpTos_Char,		//	71
	OP_lofsa,			//	72
	BadOp,				//	73
	OP_lofss,			//	74
	BadOp,				//	75
	OP_push0,			//	76
	BadOp,				//	77
	OP_push1,			//	78
	BadOp,				//	79
	OP_push2,			//	7A
	BadOp,				//	7B
	OP_pushSelf,		//	7C
	OP_fileName,		//	7D
	OP_lineNum,			//	7E
	BadOp,				//	7F
	OP_lag_Word,		//	80
	OP_lag_Char,		//	81
	OP_lal_Word,		//	82
	OP_lal_Char,		//	83
	OP_lat_Word,		//	84
	OP_lat_Char,		//	85
	OP_lap_Word,		//	86
	OP_lap_Char,		//	87
	OP_lsg_Word,		//	88
	OP_lsg_Char,		//	89
	OP_lsl_Word,		//	8A
	OP_lsl_Char,		//	8B
	OP_lst_Word,		//	8C
	OP_lst_Char,		//	8D
	OP_lsp_Word,		//	8E
	OP_lsp_Char,		//	8F
	OP_laig_Word,		//	90
	OP_laig_Char,		//	91
	OP_lail_Word,		//	92
	OP_lail_Char,		//	93
	OP_lait_Word,		//	94
	OP_lait_Char,		//	95
	OP_laip_Word,		//	96
	OP_laip_Char,		//	97
	OP_lsig_Word,		//	98
	OP_lsig_Char,		//	99
	OP_lsil_Word,		//	9A
	OP_lsil_Char,		//	9B
	OP_lsit_Word,		//	9C
	OP_lsit_Char,		//	9D
	OP_lsip_Word,		//	9E
	OP_lsip_Char,		//	9F
	OP_sag_Word,		//	A0
	OP_sag_Char,		//	A1
	OP_sal_Word,		//	A2
	OP_sal_Char,		//	A3
	OP_sat_Word,		//	A4
	OP_sat_Char,		//	A5
	OP_sap_Word,		//	A6
	OP_sap_Char,		//	A7
	OP_ssg_Word,		//	A8
	OP_ssg_Char,		//	A9
	OP_ssl_Word,		//	AA
	OP_ssl_Char,		//	AB
	OP_sst_Word,		//	AC
	OP_sst_Char,		//	AD
	OP_ssp_Word,		//	AE
	OP_ssp_Char,		//	AF
	OP_saig_Word,		//	B0
	OP_saig_Char,		//	B1
	OP_sail_Word,		//	B2
	OP_sail_Char,		//	B3
	OP_sait_Word,		//	B4
	OP_sait_Char,		//	B5
	OP_saip_Word,		//	B6
	OP_saip_Char,		//	B7
	OP_ssig_Word,		//	B8
	OP_ssig_Char,		//	B9
	OP_ssil_Word,		//	BA
	OP_ssil_Char,		//	BB
	OP_ssit_Word,		//	BC
	OP_ssit_Char,		//	BD
	OP_ssip_Word,		//	BE
	OP_ssip_Char,		//	BF
	OP_iag_Word,		//	C0
	OP_iag_Char,		//	C1
	OP_ial_Word,		//	C2
	OP_ial_Char,		//	C3
	OP_iat_Word,		//	C4
	OP_iat_Char,		//	C5
	OP_iap_Word,		//	C6
	OP_iap_Char,		//	C7
	OP_isg_Word,		//	C8
	OP_isg_Char,		//	C9
	OP_isl_Word,		//	CA
	OP_isl_Char,		//	CB
	OP_ist_Word,		//	CC
	OP_ist_Char,		//	CD
	OP_isp_Word,		//	CE
	OP_isp_Char,		//	CF
	OP_iaig_Word,		//	D0
	OP_iaig_Char,		//	D1
	OP_iail_Word,		//	D2
	OP_iail_Char,		//	D3
	OP_iait_Word,		//	D4
	OP_iait_Char,		//	D5
	OP_iaip_Word,		//	D6
	OP_iaip_Char,		//	D7
	OP_isig_Word,		//	D8
	OP_isig_Char,		//	D9
	OP_isil_Word,		//	DA
	OP_isil_Char,		//	DB
	OP_isit_Word,		//	DC
	OP_isit_Char,		//	DD
	OP_isip_Word,		//	DE
	OP_isip_Char,		//	DF
	OP_dag_Word,		//	E0
	OP_dag_Char,		//	E1
	OP_dal_Word,		//	E2
	OP_dal_Char,		//	E3
	OP_dat_Word,		//	E4
	OP_dat_Char,		//	E5
	OP_dap_Word,		//	E6
	OP_dap_Char,		//	E7
	OP_dsg_Word,		//	E8
	OP_dsg_Char,		//	E9
	OP_dsl_Word,		//	EA
	OP_dsl_Char,		//	EB
	OP_dst_Word,		//	EC
	OP_dst_Char,		//	ED
	OP_dsp_Word,		//	EE
	OP_dsp_Char,		//	EF
	OP_daig_Word,		//	F0
	OP_daig_Char,		//	F1
	OP_dail_Word,		//	F2
	OP_dail_Char,		//	F3
	OP_dait_Word,		//	F4
	OP_dait_Char,		//	F5
	OP_daip_Word,		//	F6
	OP_daip_Char,		//	F7
	OP_dsig_Word,		//	F8
	OP_dsig_Char,		//	F9
	OP_dsil_Word,		//	FA
	OP_dsil_Char,		//	FB
	OP_dsit_Word,		//	FC
	OP_dsit_Char,		//	FD
	OP_dsip_Word,		//	FE
	OP_dsip_Char		//	FF
};

void
PMachine::ExecuteCode()
{
	NextOpcode();
}

#pragma aux JumpToOpcode parm [EDX] = \
	"mov eax, dword ptr opTbl[edx * 4]",	\
	"jmp eax"

