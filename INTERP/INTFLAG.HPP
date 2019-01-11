#ifndef INTFLAG_HPP
#define INTFLAG_HPP


extern "C"  {
	short	ReadInterruptAndClear();
	void	SetInterruptFlag(short);
	short	SetESToDS();
	void	SetES(short);
	void	SetStack(void*, short);
	void	RestoreStack(short);
}

#pragma aux (PushArgs) ReadInterruptAndClear;
#pragma aux (PushArgs) SetInterruptFlag;
#pragma aux (PushArgs) SetESToDS;
#pragma aux (PushArgs) SetES;
#pragma aux (PushArgs) SetStack;
#pragma aux (PushArgs) RestoreStack;

#endif

