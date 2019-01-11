#ifndef PALMGRT_HPP
#define PALMGRT_HPP

#ifndef PALMGRI_HPP
#include "palmgri.hpp"
#endif

class PaletteMgrTrue : public PaletteMgrIndex
{
	public:
	PaletteMgrTrue() : PaletteMgrIndex()  {}
	Bool	UpdateForFrame();

	protected:
	void	ApplyAll();
	void	ApplyCycles();
   void  ApplyVary();
   void	ApplyFade();
};


#endif
