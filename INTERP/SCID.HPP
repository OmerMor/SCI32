// scid.hpp

#ifndef SCID_HPP
#define SCID_HPP

#ifndef SCII_HPP
#include "scii.hpp"
#endif

class ErrorMgr;

struct SCIDOS : SCIIBM {
	~SCIDOS();

	void					InitAudio();
	void					InitExceptions();
	void					InitInterrupts();
	void					InitPalette();
	void					InitSound();

	ConfigMgr* 	  	   MakeConfigMgr(int argc, char* argv[]);
	CritErrHandler* 	MakeCritErrHandler();
	EventMgr*			MakeEventMgr();
	GraphicsMgr*		MakeGraphicsMgr();
	MemoryMgr*			MakeMemoryMgr();
	SOL_Mouse*			MakeMouse();
	MsgMgr*			 	MakeMsgMgr();
	TimeMgr*			 	MakeTimeMgr();
	ErrorMgr*       	MakeErrorMgr();
};
void		EndVESA();

#endif

