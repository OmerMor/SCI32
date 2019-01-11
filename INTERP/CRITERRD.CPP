// criterrd.cpp

#include <dos.h>
#include <stdio.h>

#include "sol.hpp"

#include "audio.hpp"
#include "celobj.hpp"
#include "criterrd.hpp"
#include "event.hpp"
#include "errormgr.hpp"
#include "memid.hpp"
#include "msg.hpp"
#include "palette.hpp"

CritErrHandlerDOS::CritErrHandlerDOS()
{
	_harderr(Handler);
}

CritErrHandlerDOS::~CritErrHandlerDOS()
{
}

int __far
CritErrHandlerDOS::Handler(unsigned deverr, unsigned errcode, unsigned far*)
{

	// if the players aren't up yet we can't read the error file
	// nor buffer the error
	if (!errorMgr || !memMgr || activeSamples) {
		if (activeSamples) {
			_disable();
			AudioStop(ALL_SAMPLES);
		}
		msgMgr->Fatal("Critical Error:Unable to recover.\n Check your CD-ROM and restart game.\n");
	}

	// downcast, since we know what type it really is, and we need access
	// to protected members
	CritErrHandlerDOS* handler = (CritErrHandlerDOS*) critErrHandler;

	//Allow interrupts so that we can get the user's response.
	_enable();

	// just return failure to the caller and let him deal with it
	if (handler->method == Fail) {
		handler->tripped = True;
		return _HARDERR_FAIL;
	}

	//Create an error message and print the alert.
	char msg[MsgMgr::MaxStr + 1];
	msgMgr->Get(Msg_CriticalErr, msg);

	char quitOrCancel[15];
	msgMgr->Get(handler->method == Abort ? Msg_Quit : Msg_Cancel, quitOrCancel);

	// is it a disk error?
	char  errorType[30];
	if (!(deverr & 0xE000))
		// Find correct error message between messages 4 - 16
		msgMgr->Get(Msg_DiskErrors + (errcode & 0x0f), errorType);
	else
		*errorType = 0;

	MemID theText;
	theText.Get(MemText, MsgMgr::MaxStr + 1);
	sprintf((char*)*theText, msg, errorType, quitOrCancel);

	if (!eventMgr)
	{
	    errorMgr->Display("\nCritical Error:Unable to recover.\n Check to see disc is properly inserted\n");
	    _exit(255);
	}

	errorMgr->Display((char*)*theText);
	
	SOL_Event dbgEvent;
	while (True) {
		eventMgr->Get(&dbgEvent,SOL_Event::MouseDown | SOL_Event::KeyDown);
		if (dbgEvent.type == SOL_Event::KeyDown) {

			if (dbgEvent.message == Kbd::Enter){
				//Have DOS try again.
				errorMgr->Clear();
				return _HARDERR_RETRY;
			}

			if (dbgEvent.message == Kbd::Esc)
				//We'll be handling this one way or another
				if (handler->method == Abort) {
					//We can't run without this - terminate the program.
					return _HARDERR_ABORT;
				} else {
					//We can run without the IO -- return an error.
					handler->tripped = True;
					errorMgr->Clear();
					return _HARDERR_FAIL;
				}
			
		}    
		if (dbgEvent.type == SOL_Event::MouseDown) {
			//Have DOS try again.    
			errorMgr->Clear();
			return _HARDERR_RETRY;
		}
	}
}
