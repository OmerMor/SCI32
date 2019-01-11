// event.hpp

#ifndef EVENT_HPP
#define EVENT_HPP

#ifndef KBD_HPP
#include	"kbd.hpp"
#endif

#ifndef MEMID_HPP
#include	"memid.hpp"
#endif

#ifndef OBJECT_HPP
#include	"object.hpp"
#endif

#ifndef POINT_HPP
#include	"point.hpp"
#endif

#ifndef SAVEABLE_HPP
#include	"saveable.hpp"
#endif

// direction events
enum Direction {
	dirStop,
	dirN,
	dirNE,
	dirE,
	dirSE,
	dirS,
	dirSW,
	dirW,
	dirNW
};

class SOL_Event;

struct EventObjectID : ObjectID {
	EventObjectID(SOL_Handle h) : ObjectID(h) {}
	EventObjectID&	operator=(const SOL_Event&);
};

struct SOL_Event {
	typedef ushort Type;
	enum {
		Null			= 0x0000,

		MouseDown	= 0x0001,
		MouseUp		= 0x0002,
		Mouse			= MouseDown | MouseUp,

		KeyDown		= 0x0004,
		KeyUp			= 0x0008,
		Key			= KeyDown | KeyUp,

		Direction	= 0x0010,

		JoyDown		= 0x0020,
		JoyUp			= 0x0040,
		Joy			= JoyDown | JoyUp,

		Command		= 0x0080,

      MouseExt    = 0x0200,

		All			= 0x7FFF,
		Leave			= 0x8000
	};
	
	SOL_Event();
	SOL_Event(const SOL_Event&);
	SOL_Event(EventObjectID);
	
	SOL_Event&	operator=(const SOL_Event&);

	void			MakeNull();
	SOL_Event*	MapKeyToDir();

	Type			type;			// type of event
	int			message;		// variable data
	Kbd::Mod		modifiers;	// misc extra stuff
	ulong			when;			// 60 second of ticks
	SOL_Point	where;		// global mouse coords
   int         zaxis;      // next four for swift mouse
   int         pitch;
   int         roll;
   int         yaw;
};

/////////////////////////////////////////////////////////////////////////////

struct EventMgr : Saveable {
	EventMgr();
	virtual ~EventMgr();

	virtual void		Flush(SOL_Event::Type mask);
	virtual Bool		Get(SOL_Event* event, SOL_Event::Type mask=SOL_Event::All);
	virtual Kbd::Mod	GetModifiers() { return 0; };
	virtual void		Handle(SOL_Event* event);
	virtual Bool 		IsAvail(SOL_Event* rec = 0,
								SOL_Event::Type mask = SOL_Event::All) const;
	virtual void		Post(SOL_Event* event);
	virtual void		PostCommand(int command);

	virtual Bool 		StillDown() const;
	virtual SOL_Event	Wait(SOL_Event::Type mask = SOL_Event::KeyDown);

	void	SaveRestoreAfter(SaveGame*, Bool save);

protected:
	virtual void		FlushKeyboard();

	void	Bump(int& ptr) const;

	enum { Size = 16 };

	SOL_Event	entries[Size];
	int			head;
	int			tail;
};

extern EventMgr*	eventMgr;

extern void		AsyncEventCheck();

#endif
