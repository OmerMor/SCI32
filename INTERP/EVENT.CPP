//	event.cpp

#include <dos.h>
#include <string.h>

#include "sol.hpp"

#include "config.hpp"
#include "debug.hpp"
#include "event.hpp"
#include "graphmgr.hpp"
#include "intrpt.hpp"
#include "kbdmgr.hpp"
#include "kernel.hpp"
#include "mouse.hpp"
#include "pmachine.hpp"
#include "savegame.hpp"
#include "selector.hpp"
#include "time.hpp"

EventMgr*	eventMgr;

EventMgr::EventMgr()
{
	head = tail = 0;
}

EventMgr::~EventMgr()
{
}

void
EventMgr::Bump(int& ptr) const
{
	//	move pointer to next slot

	if (++ptr == Size)
		ptr = 0;
}

void
EventMgr::Flush(SOL_Event::Type mask)
{
	// flush all events specified by mask from buffer

	SOL_Event	event;

	while (Get(&event, mask))
		;
		
	if (mask & (SOL_Event::KeyDown | SOL_Event::KeyUp))
		FlushKeyboard();
}

void
EventMgr::FlushKeyboard()
{
}

Bool
EventMgr::Get(SOL_Event* event, SOL_Event::Type mask)
{
	// return next event to user

	Bool ret = False;
	for (int found = head; found != tail; Bump(found)) {
		if (entries[found].type & mask) {
			ret = True;
			break;
		}
	}

	if (ret) {		// give it to him and blank out queue entry
		*event = entries[found];
		entries[found].type = SOL_Event::Null;
		if (found == head)
         Bump(head);
	} else
		event->MakeNull();	// use his storage

	return ret;
}

void
EventMgr::Handle(SOL_Event*)
{
}

Bool
EventMgr::IsAvail(SOL_Event* event, SOL_Event::Type mask) const
{
	// return but don't remove

	Bool	ret = False;

	for (int found = head; found != tail; Bump(found)) {
		if (entries[found].type & mask) {
			ret = True;
			break;
		}
	}

	// a null event pointer says just return result
	if (event) {
		if (ret)
			*event = entries[found];
		else
			event->MakeNull();
	}

	return ret;
}

void
EventMgr::Post(SOL_Event* event)
{
	// add event tail; bump head if == tail

	event->when = timeMgr->GetTickCount();

	entries[tail] = *event;

	Bump(tail);
	if (tail == head)		// throw away oldest
		Bump(head);
}

void
EventMgr::PostCommand(int command)
{
	// post a command event
	
	SOL_Event event;
	
	event.type		= SOL_Event::Command;
	event.message	= command;
	
	Post(&event);
}

Bool
EventMgr::StillDown() const
{
	// look for any mouse ups

	return !IsAvail(0, SOL_Event::MouseUp);
}

SOL_Event
EventMgr::Wait(SOL_Event::Type mask)
{
	//	wait for an event

	SOL_Event event;
	while (!Get(&event, mask))
		;
	return event;
}

void
EventMgr::SaveRestoreAfter(SaveGame*, Bool save)
{
	if (!save)
		Flush(SOL_Event::All);
}

/////////////////////////////////////////////////////////////////////////////

EventObjectID&
EventObjectID::operator=(const SOL_Event& evt)
{
	SetIndexedProperty(evTyp, evt.type);
	SetIndexedProperty(evMod, evt.modifiers);
	SetIndexedProperty(evMsg, (Property) evt.message);
	SetIndexedProperty(evX, evt.where.x);
	SetIndexedProperty(evY, evt.where.y);
   if(mouse && mouse->IsSwift()) {
		SetIndexedProperty(evZ, evt.zaxis);
		SetIndexedProperty(evPitch, evt.pitch);
		SetIndexedProperty(evRoll, evt.roll);
		SetIndexedProperty(evYaw, evt.yaw);
   }
   return *this;
}

/////////////////////////////////////////////////////////////////////////////

SOL_Event::SOL_Event()
{
   zaxis = 0;
   pitch = 0;
   roll = 0;
	yaw = 0;
}

SOL_Event::SOL_Event(const SOL_Event& e)
{
	*this = e;
}

SOL_Event::SOL_Event(EventObjectID obj) :
	type			(obj.GetIndexedProperty(evTyp)),
	modifiers	(obj.GetIndexedProperty(evMod)),
	message		(obj.GetIndexedProperty(evMsg)),
	where			(obj.GetIndexedProperty(evX), obj.GetIndexedProperty(evY)),
	zaxis       (obj.GetIndexedProperty(evZ)),
	pitch       (obj.GetIndexedProperty(evPitch)),
	roll        (obj.GetIndexedProperty(evRoll)),
	yaw         (obj.GetIndexedProperty(evYaw))
{
}

SOL_Event&
SOL_Event::operator =(const SOL_Event& e)
{
	/*	this method is necessary because the one that WATCOM 9.5 generates
		does not work inside an interrupt function (or one it calls), since
		ES is not set
	*/
	type			= e.type;
	message		= e.message;
	modifiers	= e.modifiers;
	when			= e.when;
	where			= e.where;
   zaxis       = e.zaxis;
   pitch       = e.pitch;
   roll        = e.roll;
   yaw         = e.yaw;
	
	return *this;
}

void
SOL_Event::MakeNull()
{
	//	give him current stuff

	type = 0;
	if (mouse) {
		mouse->GlobalPosSci(&where);
		mouse->GetSwiftInfo(
						&zaxis,
						&pitch,
						&roll,
						&yaw
      );
	} else
		where = SOL_Point(0, 0);

	modifiers = eventMgr->GetModifiers();
}

SOL_Event*
SOL_Event::MapKeyToDir()
{
	kbdMgr->MapToDir(this);
	return this;
}

void
KGetEvent(argList)
{
	// fill in an event object

	SOL_Event			event;
	SOL_Event::Type	type;

	type = arg(1);
	if (type & SOL_Event::Leave)
		pm.acc = (Acc) eventMgr->IsAvail(&event, type);
	else
		pm.acc = (Acc) eventMgr->Get(&event, type);

#ifdef RDEBUG
	Recorder(evt);
#endif

	*(EventObjectID*) &arg(2) = event;
}

void
KMapKeyToDir(argList)
{
	EventObjectID	sciEvent = (ObjectID) arg(1);
	SOL_Event		event(sciEvent);

	event.MapKeyToDir();
	sciEvent = event;
	pm.acc = (Acc) sciEvent;
}

