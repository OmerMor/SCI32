//	sendstac.hpp

#ifndef SENDSTAC_HPP
#define SENDSTAC_HPP

#ifndef OBJECT_HPP
#include "object.hpp"
#endif

#ifndef SAVEABLE_HPP
#include	"saveable.hpp"
#endif

class SOL_Window;

struct SendStack : Saveable {
	~SendStack() {}

	struct Entry {
		ObjectID	object;
		Selector	selector;
	};

	virtual void	Clear() {}
	virtual void	Hide() {}
	virtual void	Pop() {}
	virtual void	PushObject(ObjectID) {}
	virtual void	PushSelector(Selector) {}
	virtual void	Show() {}
	virtual void	Toggle() {}
	virtual Entry*	Top() { return 0; }
};

////////////////////////////////////////////////////////////////////////////////
#ifdef DEBUG

struct SendStackDebug : SendStack
{
	SendStackDebug();
	~SendStackDebug();

	void		Clear();
	void		Hide();
	void		Pop();
	void		PushObject(ObjectID object);
	void		PushSelector(Selector selector);
	void		Show();
	void		Toggle();
	Entry*	Top();

	void		SaveRestoreBefore(SaveGame*, Bool);

private:
	enum { Size = 100 };

	Bool			on;
	int			lastPtr;
	int			ptr;
	Entry			stack[Size];
	SOL_Window*	window;
};

#endif

#endif

