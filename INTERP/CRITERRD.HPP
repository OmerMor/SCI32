//	criterr.hpp

#ifndef CRITERRD_HPP
#define CRITERRD_HPP

#ifndef CRITERR_HPP
#include "criterr.hpp"
#endif

class CritErrHandlerDOS : public CritErrHandler {
public:
	CritErrHandlerDOS();
	~CritErrHandlerDOS();

protected:
	static int __far Handler(unsigned deverr, unsigned errcode, unsigned far*);
};

#endif
