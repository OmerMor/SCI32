//	dos.hpp

#ifndef DOS_HPP
#define DOS_HPP

#ifndef SOL_HPP
#include "sol.hpp"
#endif

//	the following system files are included here to give access to
//	the defines that functions like Creat() and LSeek() use
#ifndef _DOS_H_INCLUDED
#include <dos.h>
#endif
#ifndef _STDLIB_H_INCLUDED
#include <stdlib.h>
#endif
#ifndef _FCNTL_H_INCLUDED
#include <fcntl.h>
#endif

//	map some compiler-specific constants and types onto our own
#define MaxPath	_MAX_PATH
#define MaxDrive	_MAX_DRIVE
#define MaxDir		_MAX_DIR
#define MaxFName	_MAX_FNAME
#define MaxExt		_MAX_EXT

#ifndef FA_HIDDEN
#define FA_HIDDEN	_A_HIDDEN
#define FA_SYSTEM	_A_SYSTEM
#define FA_DIREC	_A_SUBDIR
#endif
typedef find_t		FindBuffer;

class MemID;

Bool	Exists(const char*);
ulong	FileLength(int handle);
Bool	Unlink(const char* name);
Bool	FirstFile(const char* spec, uint atr, FindBuffer*);
Bool	NextFile(FindBuffer*);
char*	GetCWD(char* dir);
Bool	ExistDrive(char);
char*	FullPath(char* dest, const char* fileName);
void	ResetDisk();
ulong	RGetFreeSpace(char drive);
Bool	IsDriveAOrB(const char*);
long	LSeek(int handle, long offset, int mode);
int	Creat(const char* name, uint mode);
int	Open(const char* name, uint mode);
int	ReadMemID(int file, MemID id, int length);
int	Read(int file, void* buffer, int length);
int	Write(int handle, void* buffer, int length);
int	Close(int handle);
int	Rename(const char* oldName, const char* newName);
char*	ReadString(char* str, int len, int fd);
int	OpenFileInPath(char* file, uint mode, char* dest = 0);
char*	RemoveTrailingSlash(char* dir);
char*	AddSlash(char*);

#endif
