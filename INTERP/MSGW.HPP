//	msgw.hpp

#ifndef MSGW_HPP
#define MSGW_HPP

#ifndef MSG_HPP
#include "msg.hpp"
#endif

class MsgMgrWin : public MsgMgr {
public:
	~MsgMgrWin();
	void	MonoOutput(char* fmt);
	void	MonoPosition(int row, int col);
};

#endif

