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
	ConfigMgr(const char* defaultFilename, int argc, char** argv);
	enum Flag {
#ifdef DEBUG
		None,
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
		WVIDEO,
		OldNames,
		vmdReport
#else
		None,
		OldNames,
		vmdReport
#endif
	};

	Bool				Arg(Flag n);
	Bool				Claimed(char* name);
	char*				Get(char* name, int tokenNum = 0);
	char*				Get(MemType type, int tokenNum = 0);
	Bool				Get(Flag n);
	char*				GetLine(char* name);
	int				GetNum(char* name, int tokenNum = 0, int defalt = 0);
	const char*		GetFileName() const { return fileName; }
	int				Val(Flag n);
	char*				Str(Flag n);

	int	GetNTokens(char* name);
	int	GetNTokens(MemType type);

	void	AddTokenToFront(char* name, char* token);

	static Bool		FindConfigFile(char* fileName, const char* defaultFileName);

protected:
	void				ReadCommandLine(const char* defaultFileName, int argc,char** argv);
	void				ShowUsage() const;
	Flag				IsFlag(char flag);
	void				MakeFileName(const char* rootName);
	void				ReadFile();
	int				Tokenize(char* buf);
	ConfigEntry*	GetEntry(char* name);
	ConfigEntry*	GetEntry(MemType type);
	
	char				fileName[MaxPath + 1];
};

extern ConfigMgr* configMgr;

#endif
