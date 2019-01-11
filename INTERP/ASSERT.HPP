#ifndef ASSERT_HPP
#define ASSERT_HPP

void AssertMsgFail(const char* filename, int line, const char* str);

#ifdef DEBUG
#define	assertMsg(exp, str) \
	if (exp) {}				\
	else						\
	AssertMsgFail(__FILE__, __LINE__, str)

#else
#define	assertMsg(exp, str)
#endif




#endif
