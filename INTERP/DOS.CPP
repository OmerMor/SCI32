//	dos.cpp

#ifdef WIN32S
#define INCLUDE_MMSYSTEM_H
#include "sciwin.hpp"
#include "mbox.hpp"
#endif

#include <direct.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys\stat.h>

#include "sol.hpp"

#include "audio.hpp"
#include "criterr.hpp"
#include "dos.h"
#include "dos.hpp"
#include "event.hpp"
#include "kernel.hpp"
#include "pmachine.hpp"
#include "savegame.hpp"
#include "string.hpp"
#include "textid.hpp"

static int		SetDisk(int disk);
static int		GetDisk();
static void		GetDevice(char* path, char* device);
static void		GetCurDevice(char* device);

void
KFileIO(argList)
{
	enum {
		fileOpen,
		fileClose,
		fileRead,
		fileWrite,
		fileUnlink,
		fileFGets,
		fileFPuts,
		fileSeek,
		fileFindFirst,
		fileFindNext,
		fileExists,
		fileRename,
		fileCopy,
		fileReadWord,
		fileWriteWord
	};

	enum {
		F_APPEND,
		F_READ,
		F_TRUNC
	};

	static FindBuffer	findFileEntry;

	TextID	buf;
	int		mode;
	int		fd;

	critErrHandler->SetMethod(CritErrHandler::Retry);

	switch (arg(1)) {
		case fileOpen:
			buf = (TextID) arg(2);
			mode = arg(3);
			if (mode == F_TRUNC)
				fd = Creat(*buf, 0);
			else {
				fd = Open(*buf, O_RDWR | O_BINARY);
				if (!critErrHandler->Tripped()) {
					if (fd == -1 && mode == F_APPEND)
						fd = Creat(*buf, 0);
					if (fd != -1 && mode == F_APPEND)
						LSeek(fd, 0L, SEEK_END);
				}
			}
			pm.acc = (Acc) fd;
			break;

		case fileClose:
			pm.acc = !Close(arg(2));
			break;

		case fileRead:
#ifdef DEBUG
			if (((TextID)arg(3)).Size() < arg(4)) {
				msgMgr->Fatal("Buffer overflow reading %d bytes into %d bytes",
									arg(4), ((TextID)arg(3)).Size());
			}
#endif
			pm.acc = (Acc) Read(arg(2), *(TextID) arg(3), arg(4));
			break;

		case fileWrite:
#ifdef DEBUG
			if (((TextID)arg(3)).Size() < arg(4)) {
				msgMgr->Fatal("Buffer underflow writting %d bytes into %d bytes",
									arg(4), ((TextID)arg(3)).Size());
			}
#endif
			pm.acc = (Acc) Write(arg(2), *(TextID) arg(3), arg(4));
			break;

		case fileUnlink:
			pm.acc = (Acc) Unlink(*(TextID) arg(2));
			break;

		case fileFGets:
#ifdef DEBUG
			if (((TextID)arg(2)).Size() < arg(3)) {
				msgMgr->Fatal("Buffer overflow getting %d bytes into %d bytes",
									arg(3), ((TextID)arg(2)).Size());
			}
#endif
			pm.acc = Acc(ReadString(*(TextID) arg(2),arg(3), arg(4)) ? arg(2) : 0);
			break;

		case fileFPuts:
#ifdef DEBUG
			if (((TextID)arg(3)).Size() < strlen(*(TextID)arg(3))) {
				msgMgr->Fatal("Buffer underflow putting %d bytes into %d bytes",
									strlen(*(TextID)arg(3)), ((TextID)arg(3)).Size());
			}
#endif
			pm.acc = (Acc) Write(arg(2), *(TextID) arg(3),strlen(*(TextID)arg(3)));
			break;

		case fileSeek:
			pm.acc = (Acc) LSeek(arg(2), arg(3), arg(4));
			break;

		case fileFindFirst:
			pm.acc = (Acc) FirstFile(*(TextID) arg(2), arg(4), &findFileEntry);
			if (pm.acc)
				strcpy(*(TextID) arg(3), findFileEntry.name);
			break;

		case fileFindNext:
			pm.acc = (Acc) NextFile(&findFileEntry);
			if (pm.acc)
				strcpy(*(TextID) arg(2), findFileEntry.name);
			break;

		case fileExists:
			pm.acc = (Acc) Exists(*(TextID) arg(2));
			break;

		case fileRename:
			pm.acc = (Acc) Rename(*(TextID) arg(2), *(TextID) arg(3));
			break;

		case fileCopy:
			{
			int	fd2, cnt;
			const unsigned	CPYBUFLEN =	512;
			char	cpy[CPYBUFLEN];

			buf = (TextID) arg(2);
			fd = Open(*buf, O_RDONLY | O_BINARY);
			if (critErrHandler->Tripped())
				break;
			buf = (TextID) arg(3);
			fd2 = Creat(*buf, 0);
			if (critErrHandler->Tripped()) {
				Close(fd);
				break;
			}
			while ((cnt = Read(fd, cpy, CPYBUFLEN)) != 0) {
				if (critErrHandler->Tripped())
					break;
				Write(fd2, cpy, cnt);
				if (critErrHandler->Tripped())
					break;
			}
			pm.acc = (Acc) critErrHandler->Tripped();
			Close(fd);
			Close(fd2);
			break;
			}
			
		case fileReadWord:
			Read(arg(2), &pm.acc, sizeof(Acc));
			break;
			
		case fileWriteWord:
			pm.acc = Write(arg(2), &arg(3), sizeof(Acc));
			break;
	}

	critErrHandler->SetMethod(CritErrHandler::Abort);
}

void
KGetCWD(argList)
{
	TextID id = arg(1);

	id.Realloc(MaxPath + 1);
	GetCWD(*id);
	id.Snug();

	pm.acc = id;
}

void
KCheckFreeSpace(argList)
{
	//	KCheckFreeSpace functions
	enum CheckFreeSpaceFuncs {
		SAVEGAMESIZE,
		FREESPACE,
		ENOUGHSPACETOSAVE
	};
	
	/* 
	   If only path argument {
			See if there is enough free space on the
	      disk in 'path' to save a game.
	   } else {
	      If Arg2 = SAVEGAMESIZE {
	         Return the amount of space needed to save
	         the game in K
	      }
	      If Arg2 = FREESPACE {
	         Return the amount of free space in K up 
	         to 32Meg
			}
	      IF Arg2 = ENOUGHSPACETOSAVE {
				Return whether there is enough space to save in path
	   	}
	   }
	*/
	char*	path;
	ulong	freeSpace;

	// Find out how much space there is.
	path = *(TextID) arg(1);
	if (*(path+1) == ':')
		freeSpace = RGetFreeSpace(ToLower(*path));
	else
		freeSpace = RGetFreeSpace(0);

	if (argCount < 2)
      // Return Boolean, there is enough free space on the disk in 'path' to
      //	save a game.
		pm.acc = freeSpace > saveGame->Length();

   else
	   switch (arg(2))  {
		   case SAVEGAMESIZE:
            // Return the amount of space  in K needed to save the game 
				pm.acc = (Acc) ((saveGame->Length() +1023) / 1024L);
			   break;

		   case FREESPACE:
            // Return the amount of free space in K up to 32Meg
	         freeSpace = freeSpace / 1024L;
				pm.acc = (Acc) (freeSpace > 32767L ? 32767 : freeSpace);
			   break;

			case ENOUGHSPACETOSAVE:
				// Return Boolean, there is enough free space on the disk in 'path'
				//	to save a game.
				pm.acc = freeSpace > saveGame->Length();
				break;
			
			default:
				assert(!"Invalid KCheckFreeSpace function");
		}
}

void
KValidPath(argList)
{
	/* Return True if the passed path is valid, False otherwise.
		Implementation is to do a firstfile() for the directory, specifying
		directory only. */

	char			path[MaxPath + 1];
	FindBuffer	dta;

	//	don't mess with original copy
	strcpy(path, *(TextID) arg(1));
	RemoveTrailingSlash(path);

	// if critical error occurs -- don't display fail/retry message
	critErrHandler->SetMethod(CritErrHandler::Fail);

	if (!strlen(path) || strlen(path) == 1 && *path == '.')
		// Current directory is valid.
		pm.acc = True;

	else if (path[strlen(path) - 1] == ':') {
		// Current directory on a specified drive is same as validity of drive.
		char c = ToLower(path[0]);
		if (pm.acc = (Acc) ExistDrive(c))
			RGetFreeSpace(c);
		if (critErrHandler->Tripped())
			pm.acc = False;

	} else if (FirstFile(path, FA_HIDDEN | FA_SYSTEM | FA_DIREC, &dta))
		// Check to see if the path is a subdirectory.
		pm.acc = Acc(dta.attrib | FA_DIREC);
	else
		// Not valid.
		pm.acc = False;

	critErrHandler->SetMethod(CritErrHandler::Abort);
}

void
KDeviceInfo(argList)
{
	enum {
		GETDEVICE,
		CURDEVICE,
		SAMEDEVICE,
		DEVREMOVABLE,
		CLOSEDEVICE,
		SAVEDEVICE,
		SAVEDIRMOUNTED,
		MAKESAVEDIRNAME,
		MAKESAVEFILENAME
	};

	switch (arg(1)) {
		case GETDEVICE:
			GetDevice(*(TextID) arg(2), *(TextID) arg(3));
			pm.acc = arg(3);
			break;

		case CURDEVICE:
			GetCurDevice(*(TextID) arg(2));
			pm.acc = arg(2);
			break;

		case SAMEDEVICE:
			pm.acc = !strcmp(*(TextID) arg(2), *(TextID) arg(3));
			break;

		case DEVREMOVABLE:
			pm.acc = (Acc) IsDriveAOrB(*(TextID) arg(2));
			break;

		case CLOSEDEVICE:
			// A do-nothing on the IBM, but needed on the Mac and Amiga.
			break;

		case SAVEDEVICE:
			// Return letter of writeable device in case of CD-ROM-based game.
			// (Presume same device as where file used to start game)
			assert(!"No support for SAVEDEVICE call yet");
			break;

		case SAVEDIRMOUNTED:
			// A do-nothing on the IBM, but needed on the Mac and Amiga.
			pm.acc = True; // assume save disk is mounted by user
			break;

		case MAKESAVEDIRNAME:
			saveGame->MakeCatalogName(*(TextID) arg(2), *(TextID) arg(3));
			pm.acc = arg(2);
			break;

		case MAKESAVEFILENAME:
			saveGame->MakeFileName(*(TextID) arg(2), *(TextID) arg(3), arg(4));
			pm.acc = arg(2);
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////

char*
AddSlash(char* dir)
{
	//	add trailing slash to directory so file name can be concatenated onto it

	int len = strlen(dir);
	char lastChar = dir[len - 1];
	if (len && lastChar != '\\' && lastChar != '/' && lastChar != ':')
		strcat(dir, "\\");
	return dir;
}

int
Creat(const char* fileName, uint mode)
{
	//	create a file with name pointed to by 'fname' with attributes set in
	//	'mode'

	critErrHandler->Clear();
	_fmode = O_BINARY;
	return creat(fileName, mode);
}

int
Open(const char* name, uint mode)
{
	critErrHandler->Clear();
	_fmode = O_BINARY;
	return open(name, mode);
}

int
ReadMemID(int fd, MemID id, int size)
{
	//	read 'n' bytes from file 'fd' into the hunk pointed to by 'id'

	return Read(fd, *id, size);
}

int
Read(int fd, void* buf, int n)
{
	static Bool checking = False;

	if (activeSamples && n > 65536 && !checking) {
		int requested, received, total = n;
		char* cp = (char *)buf;
		do {
			checking = True;
			AsyncEventCheck();
			checking = False;
			critErrHandler->Clear();
			cp += received = read(fd, cp, requested = Min(n,65536));
			if (received != requested)
				return received;
		} while (n -= requested);
		return total;
	} else {
		critErrHandler->Clear();
		return read(fd, buf, n);
	}
}

int
Write(int fd, void* buf, int n)
{
	critErrHandler->Clear();
	return write(fd, buf, n);
}

int
Close(int fd)
{
	critErrHandler->Clear();
#ifndef WIN32S
	return close(fd);
#else
	// NOTE: Under WIN32S, in Phantasmagoria, a call was made to Close with
	// an fd value of -1.  Subsequent calls to Open failed on the second
	// file which was opened simultaneously.  The failure looked like a null
	// pointer problem.  The following code is a work-around.

	if (fd > 0)
		return close(fd);
	else
		return -1;
#endif
}

int
Rename(const char* oldName, const char* newName)
{
	critErrHandler->Clear();
	return rename(oldName, newName);
}

char*
ReadString(char* str, int len, int fd)
{
	//	similar to fgets(), but uses a file handle instead of a stream

	int	count = 0;
	char	c;
	char*	cp = str;

	critErrHandler->Clear();

	--len;	//	account for trailing	0
	while (count < len) {
		if (Read(fd, &c, 1) <= 0)
			break;
		count++;
		if (c == '\n')
			break;
		if (c == '\r')
			continue;
		*cp++ = c;
	};
	*cp = '\0';

	return count ? str : 0;
}

Bool
Unlink(const char* name)
{
	//	erase the file 'fname'

	critErrHandler->Clear();
	return unlink(name) != -1;
}

ulong
FileLength(int fd)
{
	//	return the (long) length of the file corresponding to a handle

	ulong loc = LSeek(fd, 0, SEEK_CUR);
	ulong len = LSeek(fd, 0, SEEK_END);
	LSeek(fd, loc, SEEK_SET);
	return len;
}

ulong
LSeek(int fd, ulong offset, int fromWhere)
{
	critErrHandler->Clear();
	return lseek(fd, offset, fromWhere);
}

char*
GetCWD(char* buf)
{
	return getcwd(buf, MaxPath);
}

#ifdef WIN32S
WIN32_FIND_DATA	wfd;
HANDLE				fHandle = INVALID_HANDLE_VALUE;
#endif

Bool
FirstFile(const char* fname, uint attr, FindBuffer* entry)
{
	critErrHandler->Clear();
#ifndef WIN32S
	return _dos_findfirst(fname, attr, entry) == 0;
#else
	attr = attr;
	if (fHandle != INVALID_HANDLE_VALUE)
		FindClose(fHandle);
	fHandle = FindFirstFile((LPSTR)fname,(LPWIN32_FIND_DATA)&wfd);
	if (fHandle == INVALID_HANDLE_VALUE)
		return False;
	else {
		strcpy(entry->name, wfd.cFileName);
		return True;
	}
#endif
}

Bool
NextFile(FindBuffer* entry)
{
	critErrHandler->Clear();
#ifndef WIN32S
	return _dos_findnext(entry) == 0;
#else
	BOOL bFound = FindNextFile(fHandle,(LPWIN32_FIND_DATA)&wfd);
	if (!bFound)
		return False;
	else {
		strcpy(entry->name, wfd.cFileName);
		return True;
	}
#endif
}

Bool
ExistsDriveAB(int drive)
{
#ifndef WIN32S
	//	Check for the validity of drive a: or b: by checking
	//	if there is more than one floppy drive, and how it is assigned
	//
	//		input: drive=0 for drive a: check, 1 for drive b: check
	//
	//	  returns:
	//	    whether drive requested exists

	REGS	regs;

#ifdef WINDOWS
	// use int86 instead of int386 since int386 is not supported by Windows
	int86(0x11, &regs, &regs);
#else
	int386(0x11, &regs, &regs);
#endif


	//	any floppy drives at all?
	if (!(regs.w.ax & 1))
		return False;

	//	more than one?
	if (regs.w.ax & 0xC0)
		return True;

	//	if only one, see whether it's currently A: or B:
	Bool driveIsA = !*(char*) 0x504;

	return driveIsA && drive == 0;
#else
	MBox("ExistsDriveAB called","");
	drive = drive;
	return False;
#endif
}

Bool
ExistDrive(char theDrive)
{
	//	does this drive exist?

	critErrHandler->Clear();

	int curDisk = GetDisk();
	int driveNum = ToLower(theDrive) - 'a';

	if (driveNum <= 1)
		return ExistsDriveAB(driveNum);

	//	change to new drive to see if it's valid
	SetDisk(driveNum);
	int newDisk = GetDisk();
	SetDisk(curDisk);
	return newDisk == driveNum;
}

void
ResetDisk()
{
	//	reset the disk subsystem

#ifndef WIN32S
	bdos(0x0D, 0, 0);
#else
	// the bdos call won't link under WIN32S
	flushall();
#endif
}

ulong
RGetFreeSpace(char drive)
{
	//	get the amount of free space on the drive passed in 'drive'
	//	if 'drive' is 0 or blank use the default drive

	critErrHandler->Clear();

	if (drive == ' ')
		drive = 0;
	if (drive)
		drive = (char) (ToLower(drive) - 'a' + 1);

	diskfree_t d;
	if (_dos_getdiskfree(drive, &d))
		return 0;

	return (ulong) (d.avail_clusters - 1) * d.sectors_per_cluster *
		d.bytes_per_sector;
}

Bool
Exists(const char* fileName)
{
	//	tell whether a file exists without alerting user

	Bool rc = True;

	//	special handling if referencing drive A: or B:, since they don't
	//	generate critical errors if there's only one drive and it's not
	//	the current one
	if (fileName[1] == ':') {
		char drive = ToLower(fileName[0]);
		if (drive == 'a' || drive == 'b')
			rc = ExistDrive(drive);
	}

	if (rc) {
		critErrHandler->SetMethod(CritErrHandler::Fail);
		rc = access(fileName, 0) == 0 && !critErrHandler->Tripped();
		critErrHandler->SetMethod(CritErrHandler::Abort);
	}

	return rc;
}

Bool
IsDriveAOrB(const char* str)
{
	return str[1] == ':' && (ToLower(*str) == 'a' || ToLower(*str) == 'b');
}


int
OpenFileInPath(char* fileName, uint mode, char* dest)
{
	char	path[MaxPath + 1];
	int	fd;
	
	strcpy(path, fileName);

	//	look first in the current directory
	fd = Open(path, mode);
	
	//	then in the executable's directory
	if (fd == -1) {
		strcpy(path, _argv[0]);
	
		//	strip off the program name by truncating after last backslash
		char* cp;
		if (cp = strrchr(path, '\\'))
			cp[1] = 0;
		strcat(path, fileName);

		fd = Open(path, mode);
	}
	
	//	then along the path
	if (fd == -1) {
		_searchenv(fileName, "PATH", path);
		if (*path)
			fd = Open(path, mode);
	}
	
	//	return the name of the file that was found
	if (fd != -1 && dest)
		strcpy(dest, path);

	return fd;
}

char*
RemoveTrailingSlash(char* dir)
{
	// Make sure that the last character of a directory is NOT a '/' or '\'.

	int len = strlen(dir);
	if (len) {
		char* dp = &dir[len - 1];
		if (*dp == '/' || *dp == '\\')
			*dp = 0;
	}

	return dir;
}

static int
GetDisk()
{
	unsigned disk;
	_dos_getdrive(&disk);
	return disk - 1;
}

static int
SetDisk(int disk)
{
	unsigned dummy;
	_dos_setdrive(disk + 1, &dummy);
	return disk - 1;
}

static void
GetDevice(char* path, char* device)
{
	if (path[1] != ':')
		GetCurDevice(device);
	else {
		*device++ = *path++;
		*device++ = *path++;
		*device = '\0';
	}
}

static void
GetCurDevice(char* device)
{
	char	path[MaxPath + 1];

	GetCWD(path);
	GetDevice(path, device);
}
