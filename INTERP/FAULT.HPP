//	fault.hpp

#ifndef FAULT_HPP
#define FAULT_HPP

extern "C" {
	void __interrupt __far	PanicDiv0();
	void __interrupt __far	PanicGP();
	void							InitPageFaultHandler();
};
#pragma aux (PushArgs) PanicGP;
#pragma aux (PushArgs) PanicDiv0;
#pragma aux (PushArgs) InitPageFaultHandler;

#endif
