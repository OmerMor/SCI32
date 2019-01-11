// sci.hpp

#ifndef SCI_HPP
#define SCI_HPP

class ConfigMgr;
class CritErrHandler;
class EventMgr;
class FontMgr;
class ErrorMgr;
class GraphicsMgr;
class MemoryMgr;
class MsgMgr;
class ResourceMgr;
class SOL_Mouse;
class TimeMgr;

struct SCI {
	virtual ~SCI();
	
	virtual void   Run(int argc, char* argv[]);

protected:
	virtual void            Init(int argc, char* argv[]);

	virtual void            InitAudio() = 0;
	virtual void            InitExceptions() {}
	virtual void            InitInterrupts() = 0;
	virtual void            InitPalette() = 0;
	virtual void            InitPlane();
	virtual void            InitSound() = 0;

	virtual ConfigMgr*      MakeConfigMgr(int argc, char* argv[]) = 0;
	virtual CritErrHandler* MakeCritErrHandler() = 0;
	virtual EventMgr*       MakeEventMgr() = 0;
	virtual FontMgr*        MakeFontMgr();
	virtual ErrorMgr*       MakeErrorMgr() = 0;
	virtual GraphicsMgr*    MakeGraphicsMgr() = 0;
	virtual MemoryMgr*      MakeMemoryMgr() = 0;
	virtual SOL_Mouse*      MakeMouse() = 0;
	virtual MsgMgr*         MakeMsgMgr() = 0;
	virtual ResourceMgr*    MakeResourceMgr();
	virtual TimeMgr*        MakeTimeMgr() = 0;

};
extern SCI * sci;

#endif
