#ifndef KBDMGRW_HPP
#define KBDMGRW_HPP

#ifndef SCIWIN_HPP
#include "sciwin.hpp"
#endif

#ifndef KBDMGR_HPP
#include "kbdmgr.hpp"
#endif

#ifndef KBDW_HPP
#include "kbdw.hpp"
#endif



class KbdMgrWin : public KbdMgr
{
 public:
	Bool	Get(SOL_Event*)				{return True;}
	Kbd::Mod	Modifiers()					{return keyboard.Modifiers();}
	Bool		HandleKey(unsigned, WORD, LONG);

 private:
	KbdWin keyboard;
	void		KeyEvent(WORD);
};


#endif
