//	msgd.hpp

#ifndef MSGD_HPP
#define MSGD_HPP

#ifndef MSG_HPP
#include "msg.hpp"
#endif

struct MsgMgrDOS : MsgMgr {
	~MsgMgrDOS();
	void	MonoOutput(char* fmt);
	void	MonoPosition(int row, int col);
};

#endif

