// network.cpp

#define FLAT386 1

#include "sol.hpp"

#include "integ.hpp"

#include "array.hpp"
#include "event.hpp"
#include "innexec.h"
#include "kernel.hpp"
#include "msg.hpp"
#include "pmachine.hpp"
#include "network.hpp"
#include "netmgr.hpp"
#include "string.hpp"
#include "textid.hpp"
#include "time.hpp"

#include <malloc.h>
#include <string.h>

#define errorEvt  0x0800
#define NAK_DELAY 300

//** FIX ME
#define COMM_ERR  0xFF
//** 

ulong		*netTicks;
ulong		SIDTable;
uint		maxSID;

// ulong NetMemGet(unsigned short size);
// unsigned char *NetMemPtr(ulong memID);
// void NetMemFree(ulong memID);
ulong inn_mem_get( ushort size );
ubyte *inn_mem_ptr( ulong mem_handle );
void inn_mem_free( ulong mem_handle );
void INNStop(void);
void NetDoit();
void ErrorEvent (uint errType, uint errNum); 
uint INNSend(argList, int size, int isBigEndian = 0);

void
KNetwork(argList)
{
#ifdef NETWORK
   uint netErr;
   uint len;

	if (!IPCisOpen()) {
      msgMgr->MonoStr("Opening communications");
      if (!netMsgMgr) netMsgMgr = New NetMsgMgr;

      if ( INN_PreInit("innwci.drv", 16000) ) {
			msgMgr->Fatal("INNWCI.DRV error");
			return;
		}

		INN_IPCOpen();
//		if ( ipcBase == 0L ) {
//			msgMgr->Fatal("INN Executive not loaded");
//		}

		onexit(INNStop);

		pm.acc = 1;

   	INN_SetMemFunc( inn_mem_get, inn_mem_ptr, inn_mem_free );
		netTicks = INN_SetNakDelay(NAK_DELAY);
	}
	switch (arg(1)) {
		case NET_INFO:
			pm.acc = INN_NetInfo();

         break;

		case NET_CONNECT:
			
			if (INN_NetInfo() & 0xff00) {
				pm.acc = 1;
				break;
				}

         // *** Parameter may have to be type casted.
         if ( netErr = INN_NetConnect(*(TextID) arg(2))) 
            ErrorEvent(COMM_ERR, netErr);

			pm.acc = (netErr == ERR_NONE);

         break;

		case NET_RECONNECT:
			
			if (INN_NetInfo() & 0xff00) {
				pm.acc = TRUE;
				break;
				}
         if (netErr = INN_NetReConnect(*(TextID) arg(2)) )
            ErrorEvent(COMM_ERR, netErr);

			pm.acc = (netErr == ERR_NONE);

         break;

		case NET_SEND:
         INNSend (args+2,argCount-1);
         break;

		// send a message converted to big-endian
		case NET_SENDBE:
         INNSend (args+2,argCount-1, 1);
         break;

		case NET_DOIT:
         NetDoit();			

         break;

      case NET_DISCONNECT:
         INN_NetDisconnect();

         break;

		case NET_PUTMEMO:
         if (arg(2)) {
            unsigned char *test = (unsigned char *)((ArrayID) arg(2)).Data();
            INN_PutMemo((unsigned char *)((ArrayID) arg(2)).Data(),arg(3));
         } else INN_PutMemo(NULL,0);

         break;

		case NET_GETMEMO:
         {
            unsigned char *mp;
            len = INN_GetMemo(&mp);
			   if (len == 0)
				   pm.acc = NULL;
			   else {
               ArrayID theArray = (ArrayID) arg(2);             
               ArrayResize (theArray, len);
				   memcpy(theArray.Data(), mp, len);
               pm.acc = len;
		      }
         }

         break;

		case NET_PARENT:
         {
            char *sp=INN_Parent();
            if (sp == NULL)
               pm.acc = NULL;
            else {
				   len = strlen(sp) + 1;
               StrResize ((TextID) arg(2),len);
				   memcpy(*(TextID)arg(2), sp, len);
               pm.acc= (len - 1);
			   }
			   break;
         }

		case NET_CHAIN:
			INN_Chain((arg(2) == NULL)? NULL : *(TextID)arg(2));
			
			break;

		case NET_OUTDONE:
			pm.acc = INN_Outdone();
			
			break;

		case NET_FLUSH:
			INN_Flush();
			
         break;

		case NET_GETBAUD:
			pm.acc = INN_GetBaud();
			
			break;

      case NET_GETMSG:
         {
            Net_Msg msg;
            if (netMsgMgr->IsAvail()) {
               netMsgMgr->Get(&msg);
               if ((pm.acc = msg.len) != 0) {
                  ArrayResize ((ArrayID) arg(2),msg.len);
						ArrayID theArray = (ArrayID) arg(2);
                  memcpy(theArray.Data(),(void *)msg.data,msg.len);
                  inn_mem_free((ulong) msg.data);
               }
            } else pm.acc = 0;
         }
         break;
	}
#else
	if ( argCount >= 0 )
		pm.acc = 0;
#endif
}



void
KSID(argList)
{
#ifdef NETWORK
	switch (arg(1)) {
		case SID_INIT:
			if (SIDTable == NULL) {
				maxSID = arg(2);
				SIDTable = (ulong) ArrayNew(maxSID, INTARRAY);
			}

			break;

		case SID_ADD:
			break;

		case SID_DELETE:
			break;

		case SID_LOOKUP:
			break;

		case SID_GETLIDS:
			break;

		case SID_GETSIDS:
			break;
	}
#else
	if ( argCount >= 0 )
		pm.acc = 0;
#endif
}

void 
ErrorEvent (uint errType, uint errNum) 
{ 
   // *** Insert error msg stuff here.
   SOL_Event event;

	event.type = errorEvt;
	event.message = errType;
	event.modifiers = errNum;
	event.when = timeMgr->GetTickCount();
   eventMgr->Post(&event);
}

void 
NetDoit(void)
{
	int			netErr;
	Net_Msg		msg;
	ulong			msgID;
	uint			len;

	if (netErr = INN_NetDoit())
		ErrorEvent(COMM_ERR, netErr);

	*netTicks = timeMgr->GetTickCount();
   if (len = INN_NetRcv(&msgID)) {
      // Turn the message into an event.
		msg.data = msgID;
		msg.len  = len;
      netMsgMgr->Post(&msg);
	} 
}
//	while (!netMsgMgr->IsFull() && () {

ulong 
NetMemGet(unsigned short size)
{
	ulong	msgID;

	msgID = (ulong) ArrayNew(size, BYTEARRAY);
	return msgID;
}



unsigned char * 
NetMemPtr(ulong memID)
{
	return (unsigned char *) ((ArrayID)memID).Data();
}



void 
NetMemFree(ulong memID)
{
	((ArrayID) memID).Free();
}

void 
INNStop(void)
{
   INN_SetMemFunc(NULL, NULL, NULL);
   INN_IPCClose();
   INN_Chain("");
}

uint
INNSend(argList, int size, int isBigEndian) 
{
	uint		i, n, len, val, lastSize;
	strptr	sp;
	char     *tp, *mp, *saveP;
	SCIWord	*pp;
	ArrayID	ar;

	// Determine the length of the message.
	len = 0;
	for (i = 1, sp = *(TextID)arg(0) ; i <= (uint) size && *sp ; ++i, ++sp) {
		switch (*sp) {
			case 'b':
				++len;
				break;

			case 'w':
				len += 2;
				break;

			case 's':
				len += strlen(*(TextID)arg(i)) + 1;

				break;

			case 'S':
            len += arg(i++);
				break;

			case 'a':
            len += arg(i++);
				break;

			case '+':
				n = size - i + 1;
				len += (lastSize == 'b')? n : 2 * n;
				break;
			}
		lastSize = *sp;
		}

   if ( len == 0)
      return FALSE;

	// Allocate a ByteArray in which to put the message.

   saveP = mp = (char *) malloc(len+1);
	
   for (i = 1, sp = *(TextID)arg(0) ; i <= (uint) size && *sp ; ++i, ++sp) {
		switch (*sp) {
			case 'b':
				*mp++ = (ubyte) arg(i);
				break;

			case 'w':
				val = arg(i);

				if ( isBigEndian ) {
					*mp++ = (ubyte) (val >> 8);
					*mp++ = (ubyte) (val & 0xff);
				} else {
					*mp++ = (ubyte) (val & 0xff);
					*mp++ = (ubyte) (val >> 8);
				}

				break;

			case 's':
				tp = *(TextID) arg(i);

				while (*tp) 
					*mp++ = *tp++;

				*mp++ = (ubyte) 0;

				break;

			case 'S':
				n = arg(i++);
				tp = *(TextID) arg(i);

				while (n--)
					*mp++ = *tp++;

				break;

			case 'a':
            n = arg(i++);
            ar = (ArrayID) arg(i);
				tp = (char *)ar.Data();
				while (n--)
					*mp++ = *tp++;
				break;

   	   case '+':
				   pp = &arg(i);
				   n = size - i + 1;

				   if (lastSize == 'b') {
					   while (n--)
						   *mp++ = (ubyte) ((*pp++) & 0xff);
					} else {
					   while (n--) {
						   val = *pp++;
							if ( isBigEndian ) {
							   *mp++ = (ubyte) (val >> 8);
							   *mp++ = (ubyte) (val & 0xff);
							} else {
							   *mp++ = (ubyte) (val & 0xff);
							   *mp++ = (ubyte) (val >> 8);
							}
						}
					}
				   break;
			   } 
		   lastSize = *sp;
		}

	/* Send the message to the network layer.
	 */
	return INN_NetSend((ulong)saveP, len);

}

ulong inn_mem_get( ushort size )
{
//	p = (ubyte *) ;
// 		inn_fatal( __FILE__, __LINE__,
// 			         "Farmalloc() cannot alloc memory." );
// 	static ubyte      *p;
// 
// 	if ( (p = (ubyte *) malloc(size)) == NULL )
// 		inn_fatal( __FILE__, __LINE__,
// 			         "Malloc() cannot allocate memory." );
	return (ulong) malloc(size);
}



ubyte *inn_mem_ptr( ulong mem_handle )
{
	return (ubyte *) mem_handle;			// Normalize to prevent segment wrap
}

void inn_mem_free( ulong mem_handle )
{
	free( (ubyte *) mem_handle );
// 	free( (ubyte *) mem_handle );
}
