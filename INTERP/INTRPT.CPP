//	intrpt.cpp

#include <conio.h>
#include <dos.h>

#include "sol.hpp"

#include "audio.hpp"
#include "config.hpp"
#include "intrpt.hpp"
#include "msg.hpp"
#include "timed.hpp"

#define	TIMER_INT		8
#define	PRTSCR_INT		5
#define	CTRLBRK_INT		0x1b
#define	BRKEXIT_INT		0x23
#define	TIMER_COUNT		3
#define	ADJ_COUNT		11
#define	INT_STAT_REG	0x20
#define	TIMER_PORT		0x40
#define	EOI				0x20
#define	TIMER_DIVISOR	19886

Bool		interruptsSet = False;
int		timerCounter;
int		adjustCounter;

struct Server {
	Bool			inUse;		// is server active?
	int			freq;			// desired freqency of service
	int			count;		// count of ticks to next service
	ServerFunc*	function;	// pointer to server function
} servers[4];

// previous addresses of hooked interrupts
void (__interrupt __far *prevTimerInt)();
void (__interrupt __far *prevPrtscrInt)();
void (__interrupt __far *prevCtrlbrkInt)();
void (__interrupt __far *prevBrkexitInt)();

static void __interrupt	__far	TimerIntHandler();
static void __interrupt __far	NullInterrupt();
static void __interrupt __far BreakExitHandler();

void
InstallServer(ServerFunc* proc, int ticks)
{
	// install a server at the head of the timer interrupt chain.  'proc'
	// is a pointer to the server procedure, and 'ticks' is the number of 
	// ticks between service requests.
	
	Server*	cur;
	Server*	freeSlot = 0;
	
	for (cur = servers; cur < servers + NEls(servers); cur++)
		// check to see if this server is already in the list.  If so, return
		if (cur->inUse) {
			if (cur->function == proc)
				return;

		//	otherwise, save this slot
		} else if (!freeSlot)
			freeSlot = cur;
	
	if (!freeSlot)
		msgMgr->Fatal(SrcLoc, Msg_MaxServers);

	freeSlot->freq		= ticks;
	freeSlot->count	= ticks;
	freeSlot->function= proc;
	freeSlot->inUse	= True;
}

void
DisposeServer(ServerFunc* proc)
{
	// remove the timer interrupt server pointed to by 'proc' from 
	//	the server chain.

	Server*	cur;
	
	_disable();

	for (cur = servers; cur < servers + NEls(servers); cur++)
		if (cur->inUse && cur->function == proc) {
			cur->inUse = False;
			break;
		}

	_enable();
}

void
SetInterrupts()
{
	//Set misc interrupt vectors
	prevPrtscrInt = _dos_getvect(PRTSCR_INT);
#ifdef DEBUG
	if (!configMgr->Get(ConfigMgr::WVIDEO))
#endif
		_dos_setvect(PRTSCR_INT, NullInterrupt);
	
	prevCtrlbrkInt = _dos_getvect(CTRLBRK_INT);
#ifdef DEBUG
	if (!configMgr->Get(ConfigMgr::WVIDEO))
#endif
		_dos_setvect(CTRLBRK_INT, NullInterrupt);

	prevBrkexitInt = _dos_getvect(BRKEXIT_INT);
	_dos_setvect(BRKEXIT_INT, BreakExitHandler);

	//Initialize the user timer interrupt counter.
	timerCounter = TIMER_COUNT;
	adjustCounter = ADJ_COUNT;

	//Set timer interrupt vector
	prevTimerInt = _dos_getvect(TIMER_INT);
	_dos_setvect(TIMER_INT, TimerIntHandler);

	//Change the timer interrupt frequency to 60/second
	outp(TIMER_PORT+3, 0x36);		//timer 0, LSB, MSB, mode 3
	outp(TIMER_PORT, TIMER_DIVISOR & 0xff);
	outp(TIMER_PORT, TIMER_DIVISOR >> 8);

	interruptsSet = True;
}

void
ResetInterrupts()
{
	if (!interruptsSet)
		return;

	//Set the timer frequency back to normal (18.2/second)
	outp(TIMER_PORT+3, 0x36);		//timer 0, LSB, MSB, mode 3
	outp(TIMER_PORT, 0);
	outp(TIMER_PORT, 0);

	//Restore timer interrupt vector
	_dos_setvect(TIMER_INT, prevTimerInt);

	//Restore misc interrupt vectors
	_dos_setvect(BRKEXIT_INT, prevBrkexitInt);
	_dos_setvect(CTRLBRK_INT, prevCtrlbrkInt);
	_dos_setvect(PRTSCR_INT, prevPrtscrInt);
	interruptsSet = False;
}

static void __interrupt	__far
TimerIntHandler()
{
	Server*	cur;

	//Update the current time.
	sysTicks++;

	//Finish up the interrupt by either issuing the EOI or 
	//passing it on to the machine's interrupt handler, which 
	//will finish it for us.
	Bool doChain = False;
	if (--timerCounter)
	   //Nothing to do -- issue the end-of-interrupt
		outp(INT_STAT_REG,EOI);
	else {
		timerCounter = TIMER_COUNT;
		if (!--adjustCounter) {
	 	   //Time to adjust for the difference between our
	 	   //20/second interrupts and the native 18.2/second
	 	   //by not issuing this interrupt.
			adjustCounter = ADJ_COUNT;
			outp(INT_STAT_REG,EOI);
		} else {
			//Time to call the BIOS routine (which will issue EOI for us)
			doChain = True;			//flag to do at end
		}
	}
	
	//Call each server in the timer interrupt list.
	for (cur = servers; cur < servers + NEls(servers); cur++)
		if (cur->inUse)
			//Decrement this server's counter.  If it reaches zero, 
			//reload it and call the server.
			if (!--cur->count) {
				cur->count = cur->freq;
				(*(cur->function))();
			}

	if (doChain)
		_chain_intr(prevTimerInt);
}

static void __interrupt __far
NullInterrupt()
{
}

static void __interrupt __far
BreakExitHandler()
{
	exit(1);
}
