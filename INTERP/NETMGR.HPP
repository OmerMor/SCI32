// netmgr.hpp

#ifndef NETMGR_HPP
#define NETMGR_HPP

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

struct Net_Msg {
   ulong  data;
   uint   len;
};

/////////////////////////////////////////////////////////////////////////////

struct NetMsgMgr : Saveable {
	NetMsgMgr();
	virtual ~NetMsgMgr();

	virtual void		Flush();
	virtual Bool		Get(Net_Msg* msg);
	virtual void		Handle(Net_Msg* msg);
	virtual Bool 		IsAvail() const;
   virtual Bool      IsFull();
	virtual void		Post(Net_Msg* msg);

	void	SaveRestoreAfter(SaveGame*, Bool save);

protected:

	void	Bump(int& ptr) const;

	enum { Size = 32 };

	Net_Msg	   entries[Size];
	int			head;
	int			tail;
};

extern NetMsgMgr*	netMsgMgr;

#endif

