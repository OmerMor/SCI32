//	kbd.hpp

#ifndef KBD_HPP
#define KBD_HPP

struct Kbd {
	//	the definition of an IBM-PC keyboard

	//	Key type and values
	typedef ushort	Key;
	enum {
		Home			= 0x4700,
		Up				= 0x4800,
		PgUp			= 0x4900,
		Left			= 0x4b00,
		Center		= 0x4c00,
		Right			= 0x4d00,
		End			= 0x4f00,
		Down			= 0x5000,
		PgDn			= 0x5100,
		Insert		= 0x5200,
		Del			= 0x5300,
		Pause			= 0x7000,

		CtrlLeft		= 0x7300,
		CtrlRight	= 0x7400,
		CtrlEnd		= 0x7500,
		CtrlPgDn		= 0x7600,
		CtrlHome		= 0x7700,
		CtrlPgup		= 0x8400,

		ShiftUp		= 0x38,
		ShiftPgUp	= 0x39,
		ShiftRight	= 0x36,
		ShiftPgDn	= 0x33,
		ShiftDown	= 0x32,
		ShiftEnd		= 0x31,
		ShiftLeft	= 0x34,
		ShiftHome	= 0x37,
		ShiftCenter	= 0x35,

		F1				= 0x3b00,
		F2				= 0x3c00,
		F3				= 0x3d00,
		F4				= 0x3e00,
		F5				= 0x3f00,
		F6				= 0x4000,
		F7				= 0x4100,
		F8				= 0x4200,
		F9				= 0x4300,
		F10			= 0x4400,

		Esc			= 27,
		Enter			= 0x0d,
		Tab		 	= 0x09,
		LineFeed		= 0x0a,
		BackSpace	= 0x08,
		Space			= 0x20
	};

	//	Modifiers type and values
	typedef ushort Mod;
	enum {
		Shift			= 3,	// either shift key
		Ctrl			= 4,
		Alt			= 8,
		AllShifts	= 0xF
	};
	
	//	lock status
	typedef ushort Lock;
	enum {
		ScrollLock	= 0x10,
		NumLock		= 0x20,
		CapsLock		= 0x40,
		InsertMode	= 0x80
	};

	//	Scan type and values
	typedef uchar Scan;
	enum {
		ExtendedScan	= 0xE0,
		CenterScan		= 76,		//'5' key on numeric keypad
		PauseScan		= 225,
		NumLockScan		= 69
	};

	//	various hardware data
	enum {
		Interrupt	= 9,
		KeyUpBit		= 0x80,
		DataPort		= 0x60,
		CtrlPort		= 0x61
	};

	virtual Key		Get() = 0;
	virtual Mod		Modifiers() = 0;
	virtual Lock	LockStatus() = 0;
	virtual void	Reset() = 0;
};

#define CtrlKey(c) ((c) - 'A' + 1)


#endif
