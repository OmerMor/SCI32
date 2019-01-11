//	config.cpp

#include <ctype.h>
#include	<stdio.h>
#include <string.h>

#include "sol.hpp"

#include "config.hpp"
#include "info.hpp"
#include "kernel.hpp"
#include "memmgr.hpp"
#include "msg.hpp"
#include "pmachine.hpp"
#include "stampver.hpp"
#include "string.hpp"
#include "textid.hpp"

ConfigMgr* configMgr;

struct FlagEntry {
	FlagEntry(char letter, char* help = 0);
	
	void	Usage(char* dest);

	char	letter;
	Bool	set;
	Bool	arg;
	char* str;
	char*	help;
	int	val;

} static flags[] = {

	FlagEntry('\0'),
#ifdef DEBUG
	FlagEntry('a', "   alert warning if resources loaded during audio/robot playback"),
	FlagEntry('c', "   cursor display for resource loading (i.e. -c958 for cursor 958)"),
	FlagEntry('e', "   extract resources from vols to the curdir (e.g. -e123.v56 or -e*.v56)"),
	FlagEntry('i', "   integrity checking (default is on; use -i0 to turn off)"),
	FlagEntry('k', "   kernel calls between integrity checks (default is 50,000)"),
	FlagEntry('m', "   max memory to use (examples: -m1M -m1024K -m1048576 -m$fffff)"),
	FlagEntry('M', "   Mono monitor echo Alert messages"),
	FlagEntry('p', "   priority screen allocated, enable related features"),
	FlagEntry('P', "   Play digital audio test (examples: -P100.WAV -PA0010203.045)"),
	FlagEntry('r', "   room-by-room min memory requirements printed to C:\\DUMP"),
	FlagEntry('t', "   translation messages shown on mono monitor"),
	FlagEntry('v', "   vesa mode (obsolete; use videoDrv=vesa.drv in CFG file)"),
	FlagEntry('W', "   WVIDEO compatibility")
#endif
};

const nFlags = sizeof(flags) / sizeof(*flags);

///////////////////////////////////////////////////////////////////////////////

struct ConfigEntry {
	ConfigEntry(char* label, MemType resType, char* help = 0);
	~ConfigEntry();
	
	enum { MaxLine = 240 };

	Bool	Claim(char* line, int nTokens, int len);
	char*	Get(int);

	char*		label;
	int  		nTokens;
	char*		tokens;
	MemType	resType;
	char*		help;

} static entries[] = {

	ConfigEntry("audio",			MemResAudio),
	ConfigEntry("audio36",		MemResAudio36),
	ConfigEntry("bitmap",		MemResBitmap),
	ConfigEntry("cursor",		MemResCursor),
	ConfigEntry("font", 			MemResFont),
	ConfigEntry("heap",			MemResHeap),
	ConfigEntry("map",			MemResMap),
	ConfigEntry("message",		MemResMsg),
	ConfigEntry("palette",		MemResPalette),
	ConfigEntry("patch",			MemResPatch),
	ConfigEntry("pic",			MemResPic),
	ConfigEntry("robot",			MemResRobot),
	ConfigEntry("script",		MemResHunk),
	ConfigEntry("sound",			MemResSound),
	ConfigEntry("sync",			MemResSync),
	ConfigEntry("sync36",		MemResSync36),
	ConfigEntry("translation",	MemResMsgTranslation),
	ConfigEntry("view",			MemResView),
	ConfigEntry("vocab",			MemResVocab),

	// these are special entries that are used by the interpreter

	ConfigEntry("audioDMA",		MemResNone),
	ConfigEntry("audioDrv",		MemResNone),
	ConfigEntry("audioIRQ",		MemResNone),
	ConfigEntry("audioSize",	MemResNone,
		"0K(no audio),...62K(max), 32K default"),
	ConfigEntry("brightness",	MemResNone,
		"0(default),...4(maximum)"),
	ConfigEntry("CDspeed",		MemResNone),
	ConfigEntry("DACSize",		MemResNone,
		"0K(no audio),...32K(max), 2K default"),
	ConfigEntry("hires",			MemResNone),
	ConfigEntry("joyStick",		MemResNone,
		"YES or NO(default)"),
	ConfigEntry("language",		MemResNone,
		"1(English)"),
	ConfigEntry("minMemory",	MemResNone,
		"required after SCI+extender loaded (1M default)"),
	ConfigEntry("movieDir",		MemResNone),
	ConfigEntry("patchDir",		MemResNone),
	ConfigEntry("resAud",		MemResNone),
	ConfigEntry("resCDisc",		MemResNone),
	ConfigEntry("resMap",		MemResNone),
	ConfigEntry("resSfx",		MemResNone),
	ConfigEntry("soundDrv",		MemResNone),
	ConfigEntry("staticMemory",MemResNone,
		"bytes excluded from SCI usage (200K default)"),
#ifdef NOT_SHIPPING_VERSION
	ConfigEntry("thrashDetect",	MemResNone,
		"turns on thrash detection (needs count)"),  
#endif
	ConfigEntry("tracking",		MemResNone),
	ConfigEntry("videoDrv",		MemResNone,
		"VESA.DRV or VGA.DRV(default)")
};

const nEntries = sizeof(entries) / sizeof(*entries);

///////////////////////////////////////////////////////////////////////////////

ConfigMgr::ConfigMgr(int argc,char **argv)
{
	//	read the command line and the config file

	strcpy(fileName, "resource.cfg");

	ReadCommandLine(argc,argv);
	ReadFile();
}

void
ConfigMgr::ReadCommandLine(int argc,char** argv)
{
	//	parse the command line, setting Flags and their arguments

	for (int i = 1; i < argc; ++i) {
		if (*argv[i] != '-' && *argv[i] != '/') {
			if (strlen(argv[i]) >= MaxPath) {
				printf("Config file name can not exceed %u characters\n",MaxPath);
				exit(1);
			}
			strcpy(fileName, argv[i]);
			continue;
		}

		Flag flag = IsFlag(argv[i][1]);

		if (!flag)
			ShowUsage();  // won't be comming back...
		
		flags[flag].set = True;
		flags[flag].arg = False;
		flags[flag].str = &argv[i][2];
		if (argv[i][2] == '$' || argv[i][2] == 'x' || argv[i][2] == 'X') {
			sscanf(&argv[i][3], "%x", &flags[flag].val);
			flags[flag].arg = True;
		} else {
			sscanf(&argv[i][2], "%d", &flags[flag].val);
			if (isdigit(argv[i][2]))
				flags[flag].arg = True;
		}

#ifdef DEBUG
		switch(flag) {
			case MemAllocate:
				switch(argv[i][strlen(argv[i])-1]) {
					case 'M':
					case 'm':
						flags[MemAllocate].val *= 1024;
					case 'K':
					case 'k':
						flags[MemAllocate].val *= 1024;
						break;
				}
				break;
		}
#endif

	}
}

ConfigMgr::Flag
ConfigMgr::IsFlag(char flag)
{
	//	return the number of the flag specified by the argument

	for (int i = None + 1; i < nFlags; ++i)
		if (flags[i].letter == flag)
			return (Flag) i;
	return None;
}

void
ConfigMgr::ShowUsage() const
{

	char	buf[MsgMgr::MaxStr];
#if 0
	char* cp = buf;
	
	strcpy(cp, "Usage:\n");
	for (int i = None + 1; i < nFlags; i++)
		flags[i].Usage(cp += strlen(cp));
	sprintf(cp += strlen(cp), "system memory = %d\n", memMgr->FreeMemory());
	sprintf(cp += strlen(cp), "version stamp = %d\n", GetVerStamp());
	configMgr = 0;
	msgMgr->Fatal(buf);
#endif

	printf("PATHS:  ");
	for (int i = 0; i < nEntries; i++) {
		if (entries[i].resType == MemResNone)
			break;
		printf("%s ",entries[i].label);
	}
	printf("\nTOKENS:\n");
	for ( ; i < nEntries; i++)
		printf("  %-12s  %s\n",entries[i].label,entries[i].help);
	printf("COMMAND-LINE ARGUMENTS:\n");
	for (i = None + 1; i < nFlags; i++) {
		flags[i].Usage(buf);
		printf("  %s",buf);
	}
	printf("SYSTEM MEMORY: %d   \n", memMgr->FreeMemory());
	printf("INTERPRETER VERSION : %s   ", version);
	printf("VERSION STAMP: %d   ", GetVerStamp());
	exit(1);
}

void
ConfigMgr::ReadFile()
{
	//	read the config file and assign each line (if possible) to a
	//	ConfigEntry

	int fd = Open(fileName, O_RDONLY);
	if (fd == -1) {
		printf("Can't open configuration file: %s\n", fileName);
		exit(1);
	}

	char buf[ConfigEntry::MaxLine + 1];
	while (ReadString(buf, sizeof(buf), fd)) {
		int len = strlen(buf);
		int nTokens;
		if (!(nTokens = Tokenize(buf)))
			continue;
		for (int i = 0 ; i < nEntries; i++)
			if (entries[i].Claim(buf, nTokens, len))
				break;
	}
	
	Close(fd);
}

int
ConfigMgr::Tokenize(char* buf)
{
	//	convert a string into null-terminated tokens
	//	return the number of tokens, not counting the first label
	
	char buf2[ConfigEntry::MaxLine + 1];
	char*	tmp = buf2;

	strcpy(tmp, buf);
	
	if (!(tmp = strtok(tmp, " \t=")))
		return 0;
	strcpy(buf, tmp);
	buf += strlen(tmp) + 1;

	for (int nTokens = 0; tmp = strtok(0, " =\t;,"); nTokens++) {
		strcpy(buf, tmp);
		buf += strlen(tmp) + 1;
	}
	
	return nTokens;
}

int
ConfigMgr::GetNum(char* name, int tokenNum, int defaultN)
{
	// return the 'tokenNumth' token for 'name', converted to number

	ConfigEntry* entry = GetEntry(name);
	if (!entry || tokenNum >= entry->nTokens)
		return defaultN;
	int num;
	char* str = entry->Get(tokenNum);
	if (strchr(str,'X') || strchr(str,'x'))
		sscanf(str, "%x", &num);
	else {
		sscanf(str, "%d", &num);
		if (strchr(str,'K') || strchr(str,'k'))
			num *= 1024;
		else if (strchr(str,'M') || strchr(str,'m'))
			num *= 1024 * 1024;
	}
	return num;
}

char*
ConfigMgr::Get(char* name, int tokenNum)
{
	// return the 'tokenNumth' token for 'name'

	ConfigEntry* entry = GetEntry(name);
	return entry ? entry->Get(tokenNum) : "";
}

char*
ConfigMgr::Get(MemType type, int tokenNum)
{
	ConfigEntry* entry = GetEntry(type);
	return entry ? entry->Get(tokenNum) : "";
}

char*
ConfigMgr::Str(Flag n)
{
	return flags[n].str;
}

Bool
ConfigMgr::Get(Flag n)
{
	return flags[n].set;
}

Bool
ConfigMgr::Arg(Flag n)
{
	return flags[n].arg;
}

int
ConfigMgr::Val(Flag n)
{
	return flags[n].val;
}

int
ConfigMgr::GetNTokens(char* name)
{
	// return the number of tokens for an entry

	ConfigEntry* entry = GetEntry(name);
	return entry ? entry->nTokens : 0;
}

int
ConfigMgr::GetNTokens(MemType type)
{
	// return the number of tokens for an entry

	ConfigEntry* entry = GetEntry(type);
	return entry ? entry->nTokens : 0;
}

void
ConfigMgr::AddTokenToFront(char* name, char* token)
{
	if (strlen(token)) {
		ConfigEntry* entry = GetEntry(name);

		// get total length of old entry
		int oldLen = 0;
		for (int tokenNum = 0; tokenNum < GetNTokens(name); tokenNum++)
			oldLen += strlen(Get(name,tokenNum)) + 1;
		int newLen = strlen(token) + 1;

		// make new copy starting with new token
		char* newTokens = New char[newLen+oldLen];
		strcpy(newTokens, token);
		memcpy(newTokens+newLen, entry->tokens, oldLen);

		delete entry->tokens;
		entry->tokens = newTokens;
		entry->nTokens++;
	}
}

ConfigEntry*
ConfigMgr::GetEntry(char* name)
{
	for (int i = 0; i < nEntries; i++)
		if (!stricmp(name, entries[i].label))
			return &entries[i];
	return 0;
}

ConfigEntry*
ConfigMgr::GetEntry(MemType type)
{
	for (int i = 0; i < nEntries; i++)
		if (entries[i].resType == type)
			return &entries[i];
	return 0;
}

char*
ConfigMgr::GetLine(char* name)
{
	//	return an arbitrary line from the config file

   static char buf[ConfigEntry::MaxLine + 1];

   int fd = Open(GetFileName(), O_RDONLY);
   if (fd == -1) 
		msgMgr->Fatal(SrcLoc, Msg_ConfigFileOpen, GetFileName());

	char* value = 0;
   while (ReadString(buf, sizeof buf, fd)) {
   	char* cp = strtok(buf, "= \t\n");
   	if (cp && !stricmp(cp, name)) {
   		value = strtok(0, " \t\n");
			break;
      }
	}

   Close(fd);
   
   return value;
}

void 
KGetConfig(argList)
{
	//	find an entry in the config file and copy its value into dest.
	//	return dest if entry is found, else 0

	char* value = configMgr->GetLine(*StrGetData(arg(1)));
	if (!value)
		pm.acc = 0;
	else {
		strcpy(*StrGetData(arg(2)), value);
		pm.acc = arg(2);
	}
}


//////////////////////////////////////////////////////////////////////////////

FlagEntry::FlagEntry(char letter, char* help) :
	letter(letter), set(False), help(help), val(0)
{
}

void
FlagEntry::Usage(char* dest)
{
	sprintf(dest, "-%c %s\n", letter, help);
}

//////////////////////////////////////////////////////////////////////////////

ConfigEntry::ConfigEntry(char* label, MemType resType, char* help) :
	label(label), tokens(0), nTokens(0), resType(resType), help(help)
{
}

ConfigEntry::~ConfigEntry()
{
	delete[] tokens;
}

Bool
ConfigEntry::Claim(char* line, int nTokens, int len)
{
	//	if this (tokenized) line is ours, grab it
	
	//	is it ours?
	if (stricmp(line, label))
		return False;
	
	//	skip over label name and make our own copy of the line
	line += strlen(line) + 1;
	this->nTokens = nTokens;
	tokens = New char[len];
	memcpy(tokens, line, len);

	return True;
}

char*
ConfigEntry::Get(int n)
{
	//	return the nth token, or 0 if invalid

	if (n >= nTokens)
		return "";

	char* cp = tokens;
	for (int i = 0; i < n; i++)
		cp += strlen(cp) + 1;

	return cp;
}
