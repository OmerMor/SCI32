#include <bios.h>
#include <conio.h>
#include <dos.h>
#include <string.h>

#include "sol.hpp"
#include "kbdmgrd.hpp"
#include "kbd.hpp"
#include "event.hpp"

KbdDOS KbdMgrISR::keyboard;		//  Instance of static keyboard

KbdMgrISR::KbdMgrISR()
{
	// get and save the old keyboard interrupt vector
	prevISR = _dos_getvect(Kbd::Interrupt);

	// make sure no keys come in while we're setting up
	_disable();

	// set the new interrupt
	_dos_setvect(Kbd::Interrupt, ISR);

	// copy the BIOS keyboard buffer into our buffer
	Kbd::Key key;
	while (key = keyboard.Get()) {
		Kbd::Scan scan = Kbd::Scan(key >> 8);
		if (key & 0xff)
			key &= 0xff;
		Post(key, scan);
	}

	// we're ready -- allow keys again
	_enable();
}

KbdMgrISR::~KbdMgrISR()
{
	//	unhook ourselves

	_dos_setvect(Kbd::Interrupt, prevISR);
}

void __interrupt	__far
KbdMgrISR::ISR()
{
	/*	This routine intercepts the standard int9 keyboard interrupt and checks
		for keys which IBM does not see fit to let us know about.  If we get one,
		we translate it and stuff it in our own keyboard buffer.  Otherwise, we
		pass it to BIOS for translation, then take it out of BIOS' buffer and put
		it in ours.
	*/
	
	KbdMgrISR* mgr = (KbdMgrISR*) kbdMgr;

	_enable();

	// get the current modifiers
	Kbd::Mod shiftStatus = keyboard.Modifiers();

	// read the scan code & handle it if it's a key we want.
	// otherwise, give it to BIOS.
	Kbd::Scan inScanCode = (Kbd::Scan) inp(Kbd::DataPort);

	if (inScanCode == Kbd::CenterScan)
		// see if the numeric keypad '5' key has been shifted or NumLock is on
		// if not, we'll take it.
		if (!(shiftStatus & Kbd::AllShifts) &&
			 !(keyboard.LockStatus() & Kbd::NumLock)) {
			mgr->Post(Kbd::Center, inScanCode);
			keyboard.Reset();
			return;
		}

	if (inScanCode == Kbd::PauseScan) {
		keyboard.Reset();
		return;
	}

	// check to see if the pause (Ctrl-NumLock) was pressed, and handle it
	//	ourselves
	if (inScanCode == Kbd::NumLockScan)
		if (shiftStatus & Kbd::Ctrl) {
			mgr->Post(Kbd::Pause, inScanCode);
			keyboard.Reset();
			return;
		}
	
	// pass the key along to BIOS
	mgr->prevISR();
	
	// if it's not an extended scan key (which signals that next scan code is for
	// special key) and it's got its keyup bit set, save as "keyUp" key;
	// otherwise, read it out of BIOS's buffer
	if (inScanCode != Kbd::ExtendedScan)
		if (inScanCode & Kbd::KeyUpBit) {
			inScanCode &= ~Kbd::KeyUpBit;
			Kbd::Key key;
			if (key = mgr->keyDowns.Find(inScanCode))
				mgr->Post(key | KeyUpFlag, 0);
		}

	Kbd::Key	key;
	if (key = keyboard.Get()) {
		Kbd::Scan scan = Kbd::Scan(key >> 8);
		if (key & 0xff)
			key &= 0xff;
		mgr->Post(key, scan);
	}
}

void
KbdMgrISR::Post(Kbd::Key key, Kbd::Scan scan)
{
	//	put a key in our own keyboard buffer and save keydowns

	// Since we are now linked in, why not just post directly to the event queue?
	// TM - 7/8/93

	buffer.Add(key);

	if (!(key & KeyUpFlag))
		keyDowns.Add(key, scan);
}

Bool
KbdMgrISR::Get(SOL_Event* event)
{
	// return a key event if one is available

	if (!(event->message = buffer.Get())) {
		event->type = SOL_Event::Null;
		return False;
	}
	
	// there is a key -- return it
	if (event->message & KeyUpFlag) {
		event->message = event->message & ~KeyUpFlag;
		event->type = SOL_Event::KeyUp;
	} else
		event->type = SOL_Event::KeyDown;

	return True;
}

///////////////////////////////////////////////////////////////////////////////

KbdMgrISR::Buffer::Buffer()
{
	head = tail = 0;
}

void
KbdMgrISR::Buffer::Add(Kbd::Key key)
{
	//	add a key to the buffer

	_disable();
	int newHead = head + 1;
	if (newHead >= Size)
		newHead = 0;

	if (newHead != tail) {
		keys[head] = key;
		head = newHead;
	}
	_enable();
}
	
Kbd::Key
KbdMgrISR::Buffer::Get()
{
	_disable();
	if (tail == head) {
		_enable();
		return 0;
	}
		
	Kbd::Key key = keys[tail];

	if (++tail == Size)
		tail = 0;
	
	_enable();
	return key;
}

///////////////////////////////////////////////////////////////////////////////

KbdMgrISR::KeyDowns::KeyDowns()
{
	memset(entries, 0, sizeof(entries));
}

void
KbdMgrISR::KeyDowns::Add(Kbd::Key key, Kbd::Scan scan)
{
	//	add this keydown if it's not already there

	for (int i = 0; i < Size; i++)
		if (!entries[i].scanCode || entries[i].biosCode	== key) {
			entries[i].biosCode = key;
			entries[i].scanCode = scan;
			break;
		}
}

Kbd::Key
KbdMgrISR::KeyDowns::Find(Kbd::Scan scan)
{
	//	look for the keydown matching this scan code.  if found, clear
	//	the scan code and return the key

	for (int i = 0; i < Size; i++)
		if (entries[i].scanCode == scan) {
			entries[i].scanCode = 0;
			return entries[i].biosCode;
		}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////

Bool
KbdMgrBIOS::Get(SOL_Event* event)
{
	// return a key event if one is available

	Kbd::Key key;
	key = keyboard.Get();
	if (key & 0xff)
		key &= 0xff;

	if (!key) {
		event->MakeNull();
		return False;
	} else {
		// there is a key -- return it
		event->type = SOL_Event::KeyDown;
		event->message = key;
		return True;
	}
}
