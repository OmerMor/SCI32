// intrpt.hpp

#ifndef INTRPT_HPP
#define INTRPT_HPP

typedef void	(ServerFunc)();
#pragma aux (ServerFunc, PushArgs);

void	InstallServer(ServerFunc*, int);
void	DisposeServer(ServerFunc*);
void	SetInterrupts();
void	ResetInterrupts();

#endif
