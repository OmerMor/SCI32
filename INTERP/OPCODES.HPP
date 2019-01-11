//	opcodes.hpp

#ifndef OPCODES_HPP
#define OPCODES_HPP

/*******************************************************************************
 *
 *						OPCODES
 *
 *
 * The opcodes for SCI are partially bit-mapped.  The char is mapped as
 *
 * 	txxx xxxb
 *
 * where
 * 	t = 0	->	Arithmetic, stack, etc. operations
 * 	t = 1	->	Load/store operations
 * 
 * 	b = 0	->	Following address/value is a int.
 * 	b = 1	->	Following address/value is a char.
 *
 *
 * The load/store operations are further bit-mapped:
 *
 * 	1boo idvv
 *
 * where
 * 	oo = 0	->	Load
 * 	oo = 1	->	Store
 * 	oo = 2	->	Increment, then load
 * 	oo = 3	->	Decrement, then load
 *
 * 	i = 0	->	Load/store from address as-is.
 * 	i = 1	->	Load/store indexed.  Index is in A.
 *
 *	d = 0	->	Load to accumulator
 *	d = 1	->	Load to stack
 *
 * 	vv = 0	->	Global
 * 	vv = 1	->	Local
 * 	vv = 2	->	Tmp
 * 	vv = 3	->	Parameter (load only -- specifies a different stack
 * 				frame than auto)
 *
 * Load operations leave the requested value in the A.  Store operations
 * store the A in memory unless they are indexed, in which case they store
 * the top value on the stack, popping the stack.
 ******************************************************************************/
 
const uchar	OP_LDST		= 0x80;		// load/store if set
const uchar	OP_BYTE		= 0x01;		// char operation if set, int otw

const uchar	OP_TYPE		= 0x60;		// mask for operation type
const uchar	OP_LOAD		= 0x00;		// load 
const uchar	OP_STORE		= 0x20;		// store 
const uchar	OP_INC		= 0x40;		// increment operation 
const uchar	OP_DEC		= 0x60;		// decrement operation 

const uchar	OP_INDEX		= 0x10;		// indexed op if set, non-indexed otw 

const uchar	OP_STACK		= 0x08;		// load to stack if set

const uchar	OP_VAR		= 0x06;		// mask for var type
const uchar	OP_GLOBAL	= 0x00;		// global var
const uchar	OP_LOCAL		= 0x02;		// local var
const uchar	OP_TMP		= 0x04;		// temporary var (on the stack)
const uchar	OP_PARM		= 0x06;		// parameter (different stack frame than
												// tmp)

// Opcodes other than load/store.
const uchar	op_bnot				= 0x00;
const uchar	op_add				= 0x02;
const uchar	op_sub				= 0x04;
const uchar	op_mul				= 0x06;
const uchar	op_div				= 0x08;
const uchar	op_mod				= 0x0A;
const uchar	op_shr				= 0x0C;
const uchar	op_shl				= 0x0E;
const uchar	op_xor				= 0x10;
const uchar	op_and				= 0x12;
const uchar	op_or					= 0x14;

const uchar	op_neg				= 0x16;
const uchar	op_not				= 0x18;

const uchar	op_eq					= 0x1A;
const uchar	op_ne					= 0x1C;
const uchar	op_gt					= 0x1E;
const uchar	op_ge					= 0x20;
const uchar	op_lt					= 0x22;
const uchar	op_le					= 0x24;

const uchar	op_ugt				= 0x26;
const uchar	op_uge				= 0x28;
const uchar	op_ult				= 0x2A;
const uchar	op_ule				= 0x2C;

const uchar	op_bt					= 0x2E;
const uchar	op_bnt				= 0x30;
const uchar	op_jmp				= 0x32;

const uchar	op_loadi				= 0x34;
const uchar	op_push				= 0x36;
const uchar	op_pushi				= 0x38;
const uchar	op_toss				= 0x3A;
const uchar	op_dup				= 0x3C;
const uchar	op_link				= 0x3E;

const uchar	op_call				= 0x40;
const uchar	op_callk				= 0x42;
const uchar	op_callk_Word		= 0x42;
const uchar	op_callk_Char		= 0x43;
const uchar	op_callb				= 0x44;
const uchar	op_calle				= 0x46;

const uchar	op_ret				= 0x48;

const uchar	op_send				= 0x4A;
//const uchar	op_sendk			= 0x4C;
//const uchar	op_sendb			= 0x4E;
const uchar	op_class				= 0x50;
//const uchar	op_objID			= 0x52;
const uchar	op_self				= 0x54;
const uchar	op_super				= 0x56;
const uchar	op_rest				= 0x58;
const uchar	op_lea				= 0x5A;
const uchar	op_selfID			= 0x5C;
//const uchar	op_superc		= 0x5E;
const uchar	op_pprev				= 0x60;

const uchar	op_pToa				= 0x62;
const uchar	op_aTop				= 0x64;
const uchar	op_pTos				= 0x66;
const uchar	op_sTop				= 0x68;
const uchar	op_ipToa				= 0x6A;
const uchar	op_dpToa				= 0x6C;
const uchar	op_ipTos				= 0x6E;
const uchar	op_dpTos				= 0x70;

const uchar	op_lofsa				= 0x72;
const uchar	op_lofss				= 0x74;

const uchar op_push0				= 0x76;
const uchar op_push1				= 0x78;
const uchar op_push2				= 0x7A;
const uchar op_pushSelf			= 0x7C;

const uchar op_fileName			= 0x7D;
const uchar op_lineNum			= 0x7E;

const UInt16 OP_LABEL			= 0x7000;

void	GetSourceLineNum();
void	GetSourceModuleFileName();

#endif
