#ifndef KBDMGRD_HPP
#define KBDMGRD_HPP

#ifndef KBDMGR_HPP
#include "kbdmgr.hpp"
#endif

#ifndef KBDD_HPP
#include "kbdd.hpp"
#endif


class KbdMgrISR : public KbdMgr
{
	//	a manager that uses an ISR to handle special keys (we ship with this)
public:
	KbdMgrISR();
	~KbdMgrISR();

	Bool	Get(SOL_Event*);
	Kbd::Mod	Modifiers()					{return keyboard.Modifiers();}

protected:
	virtual void	Post(Kbd::Key, Kbd::Scan scanCode);

	enum { KeyUpFlag = 0x8000 };

	class Buffer {
	 public:
		Buffer();
		void		Add(Kbd::Key);
		Kbd::Key	Get();
	 private:
		enum { Size = 20 };
		Kbd::Key	keys[Size];
		int		head;
		int		tail;
	} buffer;

	static void __interrupt __far	ISR();
	void (__interrupt __far *prevISR)();

	struct KeyDowns {
		KeyDowns();
		void		Add(Kbd::Key, Kbd::Scan);
		Kbd::Key	Find(Kbd::Scan scan);
	 private:
		enum { Size = 22 };
		struct {
			Kbd::Scan	scanCode;
			Kbd::Key		biosCode;
		} entries[Size];
	} keyDowns;

	static KbdDOS keyboard;
};

class KbdMgrBIOS : public KbdMgr
{
	// This is used for debugging because the Watcom debugger doesn't work
	// when we hook keyboard interrupts.  It will not generate keyUp events
	//	or other special keys like unshifted numeric keypad 5

public:
	Bool	Get(SOL_Event*);
	Kbd::Mod	Modifiers()					{return keyboard.Modifiers();}

private:
	KbdDOS keyboard;
};


#endif
