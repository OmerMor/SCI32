//	config.hpp

#ifndef CONFIG_HPP
#define CONFIG_HPP

#ifndef DOS_HPP
#include "dos.hpp"
#endif

#ifndef RESTYPE_HPP
#include "restype.hpp"
#endif

class ConfigEntry;

class ConfigMgr {
public:
	ConfigMgr(int argc, char** argv);

	enum Flag {
		None
#ifdef DEBUG
		,
		AudioMonitor,
		DiskCursor,
		ExtractResource,
		IntegrityChecking,
		KernelIntegCheck,
		MemAllocate,
		MonoEcho,
		PriScreen,
		PlayAudio,
		RoomMinK,
		Translation,
		VESA,
		WVIDEO
#endif
	};

	Bool			Arg(Flag n);
	char*			Get(char* name, int tokenNum = 0);
	char*			Get(MemType type, int tokenNum = 0);
	Bool			Get(Flag n);
	char*			GetLine(char* name);
	int			GetNum(char* name, int tokenNum = 0, int defalt = 0);
	const char*	GetFileName() const { return fileName; }
	int			Val(Flag n);
	char*			Str(Flag n);

	int	GetNTokens(char* name);
	int	GetNTokens(MemType type);

	void	AddTokenToFront(char* name, char* token);

protected:
	void				ReadCommandLine(int argc,char** argv);
	void				ShowUsage() const;
	Flag				IsFlag(char flag);
	void				ReadFile();
	int				Tokenize(char* buf);
	ConfigEntry*	GetEntry(char* name);
	ConfigEntry*	GetEntry(MemType type);
	
	char				fileName[MaxPath + 1];
};

extern ConfigMgr* configMgr;

#endif
