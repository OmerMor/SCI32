#include "sol.hpp"
#include "ratio.hpp"

Ratio::Ratio(const Ratio& src)
{
	numer = src.numer;
	denom = src.denom;
}

void
Ratio::Normalize()
{
	if (numer == denom)
	{
		numer = denom = 1;
		return;
	}

	while (!(numer & 1)  &&  !(denom & 1))
	{
		numer >>= 1;
		denom >>= 1;
	}

	while (!(numer % 3)  &&  !(denom % 3))
	{
		numer /= 3;
		denom /= 3;
	}
		
	while (!(numer % 5)  &&  !(denom % 5))
	{
		numer /= 5;
		denom /= 5;
	}
}

Bool
Ratio::operator==(const Ratio& src) const
{
	if (numer == src.numer)
		return (denom == src.denom);

	return False;
#if 0
	if (numer > src.numer)  {
		if (!src.numer  ||  !src.denom)
			return False;

		return (!(numer % src.numer)  &&  !(denom % src.denom));
	}

	if (!numer  ||  !denom)
		return False;

	return (!(src.numer % numer)  &&  !(src.denom % denom));
#endif
}

Bool
Ratio::operator!=(const Ratio& src) const
{
	return !operator==(src);
}


#ifdef DEBUG
Bool
Ratio::Assert() const
{
	if (numer < 0  || denom < 0)
		return False;
	return True;	
}
#endif




