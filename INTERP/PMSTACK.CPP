//	pmstack.cpp

#include "sol.hpp"

#include "msg.hpp"
#include "pmstack.hpp"

void
PMStack::Init()
{
	stack.Get(MemPMStack, Size * sizeof(Entry), PMStackHandle);
}

void
PMStack::End()
{
	stack.Free();
}

void
PMStack::Clear()
{
	stack.Fill('S');
	ptr = temps = parms = stackBase = (Ptr) *stack;
}

size_t
PMStack::ParmsOffset()
{
	return (char*) parms - (char*) stackBase;
}

size_t
PMStack::PtrOffset()
{
	return (char*) ptr - (char*) stackBase;
}

size_t
PMStack::TempsOffset()
{
	return (char*) temps - (char*) stackBase;
}
