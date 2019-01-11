// savegame.cpp

#include <string.h>
#include <stdarg.h>

#include "sol.hpp"

#include "array.hpp"
#include "config.hpp"
#include "criterr.hpp"
#include "dcomp.hpp"
#include "debug.hpp"
#include "dos.hpp"
#include "event.hpp"
#include "graphmgr.hpp"
#include "kernel.hpp"
#include "math.hpp"
#include "message.hpp"
#include "mouse.hpp"
#include "msg.hpp"
#include "object.hpp"
#include "pmachine.hpp"
#include "resource.hpp"
#include "savegame.hpp"
#include "script.hpp"
#include "sound.hpp"
#include "textid.hpp"
#include "time.hpp"

SaveGame* saveGame;

SaveGame::SaveGame() :
	bufIndex(0), bufSize(0), checksum(0), dstCnt(0), fd(-1), fp(0),
	gettingLength(False), length(0), srcCnt(0), version(21) 
{
	//	initialize savegame directory

	dir.Get(MaxPath + 1, SaveGameDirHandle);
	char* dirP = *dir;

	char drive[MaxDrive + 1];
	char dir[MaxDir + 1];
	_fullpath(dirP, configMgr->GetFileName(), MaxPath);
	_splitpath(dirP, drive, dir, 0, 0);
	_makepath(dirP, drive, dir, 0, 0);
}

void
SaveGame::CheckChecksum()
{
	long writtenChecksum;
	ReadData(&writtenChecksum, sizeof writtenChecksum);
	if (writtenChecksum != checksum)
		msgMgr->Fatal("Invalid save game:  checksum failed");
}

void
SaveGame::CheckPoint(unsigned checkPointNum, Saveable::Func func, Bool save)
{
	if (func == Saveable::SaveRestore || func == Saveable::Dump) {
		unsigned saveNum = checkPointNum;
		SaveRestore(checkPointNum, save);

		if (func == Saveable::Dump)
			Dump("Checkpoint %04X, expected %04X, written %u\n", saveNum,
				checkPointNum, length);

		if (!save && checkPointNum != saveNum)
			msgMgr->Fatal("Error restoring saved game: %X", saveNum);
	}
}

Bool
SaveGame::CheckSaveGame(char* gameName, int saveNum, char* gameVersion)
{
	ResetDisk();
	critErrHandler->SetMethod(CritErrHandler::Retry);
	
	InitBuffer(False);

	Bool rc = True;

	//	open file
	MakeFileName(fileName, gameName, saveNum);
	if ((fd = Open(fileName, O_BINARY)) == -1)
		rc = False;

	if (rc) {
		//	bailout point for errors
		if (setjmp(errorBuf))
			rc = False;
		
		//	make sure it's the correct game version
		if (rc)
			rc = CheckVersion(gameVersion);

		Close(fd);
	}

	UnInitBuffer();

	critErrHandler->SetMethod(CritErrHandler::Abort);
	
	return rc;
}

Bool
SaveGame::CheckVersion(char* gameVersion)
{
	int savedVersion;
	SaveRestore(savedVersion, False);
	if (version != savedVersion)
		return False;

	char savedGameVersion[100];
	SaveRestore(savedGameVersion, False);

	SaveRestore(cdisc, False);

	return !strcmp(savedGameVersion, gameVersion);
}

void
SaveGame::Do(Saveable::Func func, Bool save)
{
#if 0
char* when;
if (func == Saveable::SaveRestoreBefore)
	when = "before";
else if (func == Saveable::SaveRestoreAfter)
	when = "after";
else
	when = "during";
char* what = save ? "saving" : "restoring";
#endif

	CheckPoint(0x1000, func, save);	memMgr->SaveRestoreDo(func, this, save);
	CheckPoint(0x1001, func, save);	eventMgr->SaveRestoreDo(func, this, save);
	CheckPoint(0x1002, func, save);	graphMgr->SaveRestoreDo(func, this, save);
	CheckPoint(0x1003, func, save);	

if (func == Saveable::SaveRestoreBefore)
	pmSaveRestoreBefore(this, save);
else if (func == Saveable::SaveRestoreAfter)
	pmSaveRestoreAfter(this, save);

	CheckPoint(0x1004, func, save);	msgMgr->SaveRestoreDo(func, this, save);
	CheckPoint(0x1005, func, save);
#ifdef DEBUG
	debugger->SaveRestoreDo(func, this, save);
#endif
	CheckPoint(0x1006, func, save);	mathMgr.SaveRestoreDo(func, this, save);
	CheckPoint(0x1007, func, save);	messageMgr.SaveRestoreDo(func, this, save);
	CheckPoint(0x1008, func, save);	objectMgr.SaveRestoreDo(func, this, save);
	CheckPoint(0x1009, func, save);	soundMgr.SaveRestoreDo(func, this, save);
	CheckPoint(0x100A, func, save);	scriptMgr.SaveRestoreDo(func, this, save);
	CheckPoint(0x100B, func, save);	timeMgr->SaveRestoreDo(func, this, save);
}

void
SaveGame::DumpFile(char* gameName, int saveNum)
{
	Bool rc = True;
	
	InitBuffer(False);
	
	//	open file
	MakeFileName(fileName, gameName, saveNum);
	if ((fd = Open(fileName, O_BINARY)) == -1)
		rc = False;

	if (rc) {
		MakeDumpFileName(dumpFileName, gameName, saveNum);
		if (!(fp = fopen(dumpFileName, "wt")))
			rc = False;
	}

	if (rc) {
		//	bailout point for errors
		if (setjmp(errorBuf))
			rc = False;

		if (rc) {
			checksum = 0;
			length = 0;
			DumpHeader(gameName);
			Do(&Saveable::Dump, False);

			long writtenChecksum;
			ReadData(&writtenChecksum, sizeof writtenChecksum);
			Dump("Read checksum %lu, compared to actual checksum %lu\n",
				writtenChecksum, checksum);
		}

		Close(fd);
		fclose(fp);
	}

	UnInitBuffer();
}

void
SaveGame::DumpHeader(char* gameName)
{
	fprintf(fp, "%s Saved Game\n", gameName);

	int savedVersion;
	SaveRestore(savedVersion, False);
	fprintf(fp, "Saved version %u, current version %u\n", savedVersion, version);
	
	char savedGameVersion[100];
	SaveRestore(savedGameVersion, False);
	fprintf(fp, "Game version: %s\n", savedGameVersion);

	int cdisc;
	SaveRestore(cdisc, False);
	fprintf(fp, "CD#: %d\n", cdisc);
}

void
SaveGame::Dump(char* fmt, ...)
{
	va_list	args;
	char		buf[100];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	fputs(buf, fp);
}

Bool
SaveGame::FillBuffer()
{
	bufIndex = 0;
	srcCnt = bufSize;
	return Read(fd, (char*) *bufID, bufSize);
}

Bool
SaveGame::FlushBuffer()
{
	char* bufPtr = &bufID[bufIndex];
	dstCnt = 0;
	Compress(&bufPtr, &bufPtr, &dstCnt, &dstCnt, (char*) *resMgr->decompWorkBuffer);
	bufIndex = bufPtr - (char*) *bufID;
	return Write(fd, *bufID, bufIndex) == bufIndex;
}

void
SaveGame::InitBuffer(Bool save)
{
	bufID = resMgr->DecompBuffer(10000);
	bufSize = bufID.Size() - 16;
	bufIndex = save ? 0 : bufSize;
	Initcompress((char*) *resMgr->decompWorkBuffer);
}

void
SaveGame::UnInitBuffer()
{
	resMgr->DecompBuffer(0);
}

ulong
SaveGame::Length()
{
	if (!setjmp(errorBuf)) {
		length = 0;
		gettingLength = True;
		Do(&Saveable::SaveRestoreBefore, True);
		Do(&Saveable::SaveRestore, True);
		Do(&Saveable::SaveRestoreAfter, True);
		gettingLength = False;
		return length + sizeof checksum + 1000;
	} else {
		gettingLength = False;
		return 0;
	}
}

void
SaveGame::MakeCatalogName(char* dest, char* gameName)
{
	sprintf(dest, "%s%sSG.CAT", AddSlash(*dir), gameName);
}

void
SaveGame::MakeDumpFileName(char* dest, char* gameName, int saveNum)
{
	sprintf(dest, "%s%sDF.%03u", AddSlash(*dir), gameName, saveNum);
}

void
SaveGame::MakeFileName(char* dest, char* gameName, int saveNum)
{
	sprintf(dest, "%s%sSG.%03u", AddSlash(*dir), gameName, saveNum);
}

Bool
SaveGame::ReadCatalog(char* gameName, char* names, SCIWord* nums)
{
	ResetDisk();
	critErrHandler->SetMethod(CritErrHandler::Retry);

	int nSaves = 0;

	MakeCatalogName(fileName, gameName);
	if ((fd = Open(fileName, O_BINARY)) == -1) {
		//	if couldn't open the catalog because of a critical error, return 
		//	error, else the catalog just doesn't exist, so return an empty one
		if (critErrHandler->Tripped())
			nSaves = -1;

	} else {
		if (setjmp(errorBuf)) {
			//	error during read
			nSaves = -1;

		} else {
			//	read in the save game numbers and names
			memset(names, 0, MaxGames * NameLen + 1);
			memset(nums, 0, MaxGames);
			for (;; ++nSaves) {
				//	read the game number
				SCIWord n;
				Read(fd, &n, sizeof n);
				if (n == -1)
					break;
				*nums++ = n;
	
				//	read the game name
				Read(fd, names, NameLen);
				names += NameLen;
			}
		}
		Close(fd);
	}
	
	//	terminate the names list
	*names = 0;

	critErrHandler->SetMethod(CritErrHandler::Abort);

	return nSaves;
}

Bool
SaveGame::ReadData(void* output, size_t size)
{
	Bool	rc 		= True;
	char* dst		= (char*) output;
	ulong	dstCnt	= size;

	while (dstCnt) {
		if (bufIndex >= bufSize)
			if (!(rc = FillBuffer()))
				break;

		char* bufPtr = &bufID[bufIndex];
		while (srcCnt && dstCnt) {
			int drc = Decompress(&bufPtr, &dst, &srcCnt, &dstCnt,
				(char*) *resMgr->decompWorkBuffer);
			if (drc) {
				srcCnt = 0;
				Decompress(&bufPtr, &dst, &srcCnt, &dstCnt,
					(char*) *resMgr->decompWorkBuffer);
				srcCnt = bufSize - (bufPtr - (char*) *bufID);
			}
		}
		bufIndex = bufPtr - (char*) *bufID;
	}
	
	return rc;
}

Bool
SaveGame::Restore(TextID gameName, int saveNum, TextID gameVersion)
{
#ifdef DEBUG
	CheckIntegrity("Begin restore");
#endif
	ResetDisk();
	critErrHandler->SetMethod(CritErrHandler::Retry);

	InitBuffer(False);

	Bool rc = True;

	//	open file
	MakeFileName(fileName, *gameName, saveNum);
	if ((fd = Open(fileName, O_BINARY)) == -1)
		rc = False;

	if (rc) {
		//	bailout point for errors
		if (setjmp(errorBuf))
			rc = False;
		
		//	make sure it's the correct game version
		checksum = 0;
		if (rc)
			rc = CheckVersion(*gameVersion);

		if (rc)
			rc = cdisc == 0 || cdisc == discNum;

		//	call the managers to do the restore
		if (rc) {
			Do(&Saveable::SaveRestoreBefore, False);
			Do(&Saveable::SaveRestore, False);
			CheckChecksum();
			Do(&Saveable::SaveRestoreAfter, False);
		}

		Close(fd);
	}

	UnInitBuffer();

	critErrHandler->SetMethod(CritErrHandler::Abort);
	
#ifdef DEBUG
	CheckIntegrity("End restore");
#endif
	
	if (rc)
//BEW CHANGED		longjmp(pm.restartBuf, 1);
		pmRestartBuf();

//BEW REMOVED	else

	return False;
}

Bool
SaveGame::Save(TextID gameName, int saveNum, TextID desc, TextID gameVersion)
{
#ifdef DEBUG
	CheckIntegrity("Begin save");
#endif

	InitBuffer(True);

	ResetDisk();
	critErrHandler->SetMethod(CritErrHandler::Retry);
	
	Bool rc = True;

	//	open file
	MakeFileName(fileName, *gameName, saveNum);
	if ((fd = Creat(fileName, 0)) == -1)
		rc = False;
		
	if (rc) {
		//	bailout point for errors
		if (setjmp(errorBuf))
			rc = False;
			
		//	call the managers to do the save
		if (rc) {
			Do(&Saveable::SaveRestoreBefore, True);
			checksum = 0;
			SaveRestore(version, True);
			SaveRestore(*gameVersion, True);
			SaveRestore(discNum, True);
			Do(&Saveable::SaveRestore, True);
			WriteChecksum();
			rc = FlushBuffer();
			Do(&Saveable::SaveRestoreAfter, True);
		}
		
		Close(fd);
	}

	UnInitBuffer();

	critErrHandler->SetMethod(CritErrHandler::Abort);

	if (rc)
		WriteCatalog(*gameName, *desc, saveNum);
		
#ifdef DEBUG
	CheckIntegrity("End save");
#endif

	return rc;
}

void
SaveGame::SaveRestore(void* data, size_t size, Bool save)
{
	//	save or restore arbitrary data

	length += size;

	if (!gettingLength) {
		//	read or write the data
		Bool rc;
		if (save)
			rc = WriteData(data, size);
		else
			rc = ReadData(data, size);
		if (!rc)
			longjmp(errorBuf, 1);

		//	update the checksum
		char* cp = (char*) data;
		while (size--)
			checksum += *cp++;
	}
}

void
SaveGame::SaveRestore(char* str, Bool save)
{
	//	save or restore a string (including terminating 0)

	do
		SaveRestore(str, 1, save);
	while (*str++);
}

void
SaveGame::SaveRestore(SCIWord& num, Bool save)
{
	//	save or restore an SCIWord

	SaveRestore(&num, sizeof num, save);
}

void
SaveGame::SaveRestore(int& num, Bool save)
{
	//	save or restore an int

	SaveRestore(&num, sizeof num, save);
}

void
SaveGame::SaveRestore(unsigned& num, Bool save)
{
	//	save or restore an unsigned int

	SaveRestore(&num, sizeof num, save);
}

Bool
SaveGame::WriteCatalog(char* gameName, char* desc, SCIWord saveNum)
{
	int		n;
	char		names[MaxGames * NameLen + 1];
	SCIWord	nums[MaxGames];
	char		writtenDesc[NameLen];
	
	memset(writtenDesc, 0, sizeof writtenDesc);
	strcpy(writtenDesc, desc);

	Bool rc = True;

	int nSaves = ReadCatalog(gameName, names, nums);
	if (nSaves == -1)
		rc = False;

	ResetDisk();
	critErrHandler->SetMethod(CritErrHandler::Retry);

	if (rc) {
		MakeCatalogName(fileName, gameName);
		if ((fd = Creat(fileName, 0)) == -1)
			rc = False;

		if (rc) {
			//	bailout point for errors
			if (setjmp(errorBuf))
				rc = False;
	
			if (rc) {
				// rewrite the directory with the requested file first
				Write(fd, &saveNum, sizeof saveNum);
				Write(fd, writtenDesc, NameLen);
				for (n = 0; n < nSaves; n++) {
					if (saveNum != nums[n]) {
						Write(fd, &nums[n], sizeof nums[n]);
						Write(fd, &names[n * NameLen], NameLen);
					}
				}
				saveNum = -1;
				Write(fd, &saveNum, sizeof saveNum);
			}

			Close(fd);
		}
	}

	critErrHandler->SetMethod(CritErrHandler::Abort);

	return rc;
}

void
SaveGame::WriteChecksum()
{
	WriteData(&checksum, sizeof checksum);
}

Bool
SaveGame::WriteData(void* data, size_t size)
{
	Bool	rc = True;
	char* src = (char*) data;
	ulong	srcCnt = size;

	while (srcCnt) {
		//	make sure there's at least twice the size needed in the buffer
		dstCnt = bufSize - bufIndex;
		if (bufIndex && bufIndex + size * 2 > bufSize) {
			if (Write(fd, *bufID, bufIndex) != bufIndex) {
				rc = False;
				break;
			}
			bufIndex = 0;
			dstCnt = bufSize;
		}

		char* bufPtr = &bufID[bufIndex];
		Compress(&src, &bufPtr, &srcCnt,&dstCnt,(char*)*resMgr->decompWorkBuffer);
		bufIndex = bufPtr - (char*) *bufID;
	}
	
	return rc;
}

////////////////////////////////////////////////////////////////////////////////

void
KSave(argList)
{
	enum {
		SaveGame,
		RestoreGame,
		GetSaveDir,
		CheckSaveGame,
		GetSaveCDisc,
		GetSaveFiles,
		MakeSaveCatName,
		MakeSaveFileName,
		GameIsRestarting,
		Restart
	};

	switch (arg(1)) {
		case SaveGame:
			pm.acc = saveGame->Save(
				arg(2),		//	game name
				arg(3),		//	save game number
				arg(4),		//	description
				arg(5)		// game version
			);
			break;

		case RestoreGame:
			pm.acc = saveGame->Restore(
				arg(2),		//	game name
				arg(3),		//	save game number
				arg(4)		// game version
			);
			break;

		case GetSaveDir:
			// SCI programmers will modify the returned string
			pm.acc = saveGame->GetDir();
			break;

		case CheckSaveGame:
			pm.acc = saveGame->CheckSaveGame (
				*(TextID) arg(2),		//	game name
				arg(3),					//	save game num
				*(TextID) arg(4)		//	game version
			);
			break;

		case GetSaveCDisc:
			pm.acc = saveGame->GetCDisc();
			break;

		case GetSaveFiles: {
			TextID names = arg(3);
			names.Realloc(SaveGame::MaxGames * SaveGame::NameLen + 1);

			pm.acc = saveGame->ReadCatalog (
				*(TextID) arg(2),
				*names,
				(SCIWord *) ((ArrayID) arg(4)).Data()
			);

			break;
		}

		case MakeSaveCatName: {
			TextID text(arg(2));
			text.Realloc(MaxPath + 1);

			saveGame->MakeCatalogName(
				*text,				// dest
				*(TextID) arg(3)	// game name
			);
			text.Snug();
			break;
		}

		case MakeSaveFileName: {
			TextID text(arg(2));
			text.Realloc(MaxPath + 1);

			saveGame->MakeFileName(
				*text,						// dest
				*((TextID) arg(3)),		// game name
				arg(4)						// game number
			);
			break;
		}
         
		case GameIsRestarting:
		case Restart:
			break;

		default:
			msgMgr->Fatal(SrcLoc, Msg_InvalidSaveFunc, arg(1));
			break;
	}
}
