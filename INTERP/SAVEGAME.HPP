//	savegame.hpp

#ifndef SAVEGAME_HPP
#define SAVEGAME_HPP

#include <setjmp.h>
#include <stdio.h>

#ifndef DOS_HPP
#include "dos.hpp"
#endif

#ifndef SAVEABLE_HPP
#include "saveable.hpp"
#endif

#ifndef TEXTID_HPP
#include "textid.hpp"
#endif

struct SaveGame {
	enum { MaxGames = 20, NameLen = 36 };

	SaveGame();

	Bool		CheckSaveGame(char* gameName, int saveNum, char* gameVersion);
	void		DumpFile(char* gameName, int saveNum);
	void		Dump(char* fmt, ...);
	TextID	GetDir() { return dir; }
	int		GetCDisc() { return cdisc; }
	ulong		Length();
	void		MakeFileName(char* dest, char* gameName, int saveNum);
	void		MakeCatalogName(char* dest, char* gameName);
	int		ReadCatalog(char* name, char* names, SCIWord* nums);
	Bool		Restore(TextID gameName, int saveNum, TextID gameVersion);
	Bool		Save(TextID gameName, int saveNum, TextID desc, TextID gameVersion);
	void		SaveRestore(void* data, size_t len, Bool save);
	void		SaveRestore(char* data, Bool save);
	void		SaveRestore(SCIWord& data, Bool save);
	void		SaveRestore(unsigned& data, Bool save);
	void		SaveRestore(int& data, Bool save);
#ifdef MACINTOSH
	//			Mac Alternate save game functions.
	short		InitGlobalSaveFile(void);
	void		SaveGlobalFile(void);
	short		RestoreGlobalFile(void);
#endif

protected:
	void		Do(Saveable::Func, Bool save);
	void		DumpHeader(char* gameName);
	void		CheckChecksum();
	void		CheckPoint(unsigned checkPointNum, Saveable::Func func, Bool save);
	Bool		CheckVersion(char* gameVersion);
	Bool		FillBuffer();
	Bool		FlushBuffer();
	void		InitBuffer(Bool save);
	void		MakeDumpFileName(char* dest, char* gameName, int saveNum);
	Bool		ReadData(void* output, size_t size);
	void		UnInitBuffer();
	Bool		WriteData(void* data, size_t size);
	Bool		WriteCatalog(char* gameName, char* desc, SCIWord saveNum);
	void		WriteChecksum();

	MemID		bufID;
	size_t	bufIndex;
	int		bufSize;
	int		cdisc;
	long		checksum;
	ulong		dstCnt;
	TextID	dir;
	char		dumpFileName[MaxPath + 1];
	int		fd;
	FILE*		fp;
	jmp_buf	errorBuf;
	char		fileName[MaxPath + 1];
	Bool		gettingLength;
	ulong		length;
	ulong		srcCnt;
	uint		version;

};
extern SaveGame * saveGame;

#endif
