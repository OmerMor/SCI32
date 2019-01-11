//	criterr.hpp

#ifndef CRITERR_HPP
#define CRITERR_HPP

class CritErrHandler {
public:
	CritErrHandler() : tripped(False) {}
	virtual ~CritErrHandler();

	//	how critical error should be handled
	enum Method {
		Abort,		//	allow the user to retry or abort program 
		Retry,		//	allow the user to retry or return failure to caller
		Fail			//	return failure to the caller without user input
	};
	
	void		Clear() 					{ tripped = False; }
	Method	SetMethod(Method m)	{ Method rc = method; method = m; return rc; }
	Bool 		Tripped()				{ return tripped; }
	
protected:
	Method			method;
	volatile Bool	tripped;

};
extern CritErrHandler * critErrHandler;

#endif
