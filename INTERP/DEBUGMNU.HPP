//	debugmnu.hpp
#ifdef DEBUG
#ifndef DEBUGMNU_HPP
#define DEBUGMNU_HPP

class TextID;
class SOL_Event;

enum {
	cmOK = 100,
	cmAcc,
	cmQuit,
	cmExit,
	cmCur,
	cmInspect,
	cmStepOver,
	cmStep,
	cmStepSource,
	cmStepOverSource,
	cmCallKernel,

	cmDisplayMode,
	cmDisplaySendStack,
	cmDisplayObjects,
	cmDisplayObjectsWithAddrs,
	cmDisplayPalette,
	cmDisplayResources,
	cmDisplayKernelCalls,
	cmDisplayBitmaps,
	cmDisplayTexts,

	cmEditAcc,
	cmEditLocal,
	cmEditGlobal,
	cmEditTemp,
	cmEditParameter,
	cmEditMemory,

	cmBreakpointClear,
	cmBreakpointKernelAny,
	cmBreakpointCall,
	cmBreakpointProc,
	cmBreakpointSend,
	cmBreakpointKernel,
	cmBreakpointKernelSticky,
	cmBreakpointMemory,
	cmBreakpointMemorySticky,
	cmBreakpointMethod,
	cmBreakpointMethodSticky,
	cmBreakpointScript,
	cmBreakpointScriptSticky,
	cmBreakpointProperty,
	cmBreakpointPropertySticky,
	cmBreakpointStepOut,
	
	cmMemoryCheckIntegrity,
	cmMemoryCheckIntegrityOn,
	cmMemoryCheckIntegrityOff,
	cmMemoryList,
	cmMemoryGraph,
	cmMemorySummary,
	
	cmHelp
};

class DebugMenu;
class DebugMenuItem;
class SOL_Event;
class EventWindow;

class DebugMenus {
public:
	DebugMenus();

	void	Add(DebugMenu*);
	void	Delete();

	Bool	HandleEvent(SOL_Event&);

	void 	Hide();
	void 	Show();
	void	Toggle();

	int	count;
	Bool	visible;

protected:
	DebugMenu*	items[10];

};

extern DebugMenus debugMenus;

class DebugMenu {
public:
	DebugMenu(char* label, DebugMenuItem* items);
	virtual ~DebugMenu();
	
	Bool	Claim(SOL_Event&, Bool visible);
	Bool	HandleEvent(SOL_Event&, Bool visible);

	void 	Hide();
	void 	Show();

	Bool	IsSubmenu() 			{ return isSubmenu; }
	void	IsSubmenu(Bool is)	{ isSubmenu = is; }

protected:
	Bool				isSubmenu;
	char*				label;
	DebugMenuItem*	items;
	EventWindow*	window;
};

class DebugMenuCmd {
public:
	DebugMenuCmd(int cmd, ushort menuKey, ushort hotKey = 0);
	DebugMenuCmd(ushort menuKey, DebugMenu*);
	DebugMenuCmd(int);
	
	Bool	Claim(SOL_Event&, Bool visible);
	
	operator int() { return cmd || hotKey || menuKey; }

protected:
	int			cmd;
	ushort		hotKey;
	ushort		menuKey;
	DebugMenu*	menu;
};

class DebugMenuItem {
public:
	DebugMenuItem(char* label, ushort menuKey, int cmd, ushort hotKey = 0);
	DebugMenuItem(char* label, DebugMenuCmd*);
	DebugMenuItem(char* label, ushort menuKey, DebugMenu*);
	DebugMenuItem(int);
	
	operator	int()	{ return label || cmd; }
	
	Bool	Claim(SOL_Event&, Bool visible);
	
	void 	ShowStr(TextID& buf);

protected:
	char*				label;
	DebugMenuCmd	cmd;
	DebugMenuCmd*	cmds;
};

#endif
#endif

