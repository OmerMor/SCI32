#include "assert.hpp"
#include "msg.hpp"


#ifdef DEBUG

void AssertMsgFail(const char* filename, int line, const char* str)
{
	msgMgr->Fatal("Assert failed: %s(%d) - %s\n", filename, line, str);
}

#endif
