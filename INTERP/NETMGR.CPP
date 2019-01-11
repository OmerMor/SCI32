//	netmgr.cpp

#include <dos.h>
#include <string.h>

#include "sol.hpp"

#include "config.hpp"
#include "debug.hpp"
#include "netmgr.hpp"
#include "intrpt.hpp"
#include "kernel.hpp"
#include "pmachine.hpp"
#include "selector.hpp"
#include "time.hpp"
#include "msg.hpp"
// #include "graphmgr.hpp"

NetMsgMgr*	netMsgMgr;

NetMsgMgr::NetMsgMgr()
{
	head = tail = 0;
}

NetMsgMgr::~NetMsgMgr()
{
}

void
NetMsgMgr::Bump(int& ptr) const
{
	//	move pointer to next slot

	if (++ptr == Size)
		ptr = 0;
}

void
NetMsgMgr::Flush()
{
	// flush all events specified by mask from buffer

	Net_Msg	msg;

	while (Get(&msg));
}

Bool
NetMsgMgr::Get(Net_Msg* msg)
{
   if (IsAvail()) {
//      msgMgr->MonoStr("NetMsgMgr::Get Msg Is Avail\n");
      *msg = entries[head];
      Bump(head);
      return 1;
   } else {
      msg->len = 0;
      msg->data = 0;
   }
   return 0;
}

void
NetMsgMgr::Handle(Net_Msg *)
{
}

Bool
NetMsgMgr::IsAvail(void) const
{
	// return but don't remove
   return head != tail;
}

void
NetMsgMgr::Post(Net_Msg* msg)
{
//   msgMgr->MonoStr("NetMsgMgr::Get Msg Post\n");
	entries[tail] = *msg;

	Bump(tail);
	if (tail == head) {		// throw away oldest
		Bump(head);
      msgMgr->Alert("Net Message Buffer overflow.\n");
   }
}

Bool
NetMsgMgr::IsFull()
{
   int saveTail = tail;
   Bump(tail);
   int ret = (tail == head);
   tail = saveTail;
   return ret;
}

void
NetMsgMgr::SaveRestoreAfter(SaveGame*, Bool save)
{
	if (!save)
		Flush();
}

