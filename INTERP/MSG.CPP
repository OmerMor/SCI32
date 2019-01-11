// msg.cpp

#include <string.h>
#include <stdio.h>

#include "sol.hpp"

#include "audio.hpp"
#include "config.hpp"
#include "debug.hpp"
#include "dialog.hpp"
#include "dos.hpp"
#include "errormgr.hpp"
#include "kernel.hpp"
#include "msg.hpp"
#include "pmachine.hpp"
#include "savegame.hpp"
#include "sound.hpp"
#include "string.hpp"
#include "textid.hpp"

MsgMgr*  msgMgr;

MsgMgr::MsgMgr() : fileName("SIERRA.ERR")
{
	*quitStr    = 0;
	*fatalStrs  = 0;
	*sciFatalStr= 0;
}

void
MsgMgr::Init()
{
	// make sure file can be found and preload messages that can't be loaded
	// later
	
	int fd;
	if ((fd = OpenFile()) == -1)
		Fatal("Can't find %s", fileName);

	// Pre-load error messages that cannot be loaded later 
	char* tmpBuf = preloadedMsgs;
	for (int i = 1; i <= Msg_LastPreload; i++){
		if (!Read(i, tmpBuf, fd))
			break;
		tmpBuf += strlen(tmpBuf) + 1;
		LSeek(fd, 0L, SEEK_SET);      // Messages not necessarily in order
	}
	Close(fd);
}

void
MsgMgr::AddFatalStr(char* str)
{
	int roomLeft = MaxFatalBuf - strlen(fatalStrs);
	if (roomLeft) {
		if (roomLeft > 1)
			strcat(fatalStrs, "\n");
		if (roomLeft < strlen(str) + 1)
			//	fit as much of the string as possible, truncating the beginning
			strcat(fatalStrs, str + strlen(str) - roomLeft + 1);
		else
			strcat(fatalStrs, str);
	}
}

void
MsgMgr::AddPMIP(char* msg)
{
	if ((int) pm.game && pm.ip.IsValid())
		sprintf(msg + strlen(msg), "\nScript %u/$%x", pm.curScriptNum,
			pm.ip.ScriptOffset());
}

void
MsgMgr::AddSrcLoc(char* msg, char* fileName, int lineNum)
{
	sprintf(msg + strlen(msg), "\n%s(%d)", fileName, lineNum);
}

Bool
MsgMgr::Alert(int errNum, ...)
{
	va_list args;
	va_start(args, errNum);
	char* cp = Make(errNum, args);
	va_end(args);

	return AlertOutput(cp);
}

Bool
MsgMgr::Alert(char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char* cp = Make(fmt, args);
	va_end(args);
	
	return AlertOutput(cp);
}

Bool
MsgMgr::Alert(char, char* fileName, int lineNum, int errNum, ...)
{
	va_list args;
	va_start(args, errNum);
	char* cp = Make(fileName, lineNum, errNum, args);
	va_end(args);
	
	return AlertOutput(cp);
}

Bool
MsgMgr::Alert(char, char* fileName, int lineNum, char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char* cp = Make(fileName, lineNum, fmt, args);
	va_end(args);
	
	return AlertOutput(cp);
}

Bool
MsgMgr::AlertOutput(char* msg)
{
	return MsgBox("Alert", msg);
}

void
MsgMgr::Dump(char* fmt, ...)
{
	// dump information to a file
	
	static Bool inited;
	
	if (!inited) {
		Unlink("C:\\DUMP");
		inited = True;
	}
	
	FILE* fp = fopen("C:\\DUMP", "at");
	va_list args;
	va_start(args, fmt);
	vfprintf(fp, fmt, args);
	va_end(args);
	fclose(fp);
}

void
MsgMgr::Fatal(int errNum, ...)
{
	va_list args;
	va_start(args, errNum);
	char* cp = Make(errNum, args);
	va_end(args);
	
	FatalOutput(cp);
}

void
MsgMgr::Fatal(char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char* cp = Make(fmt, args);
	va_end(args);

	FatalOutput(cp);
}

void
MsgMgr::Fatal(char, char* fileName, int lineNum, int errNum, ...)
{
	va_list args;
	va_start(args, errNum);
	char* cp = Make(fileName, lineNum, errNum, args);
	va_end(args);
	
	FatalOutput(cp);
}

void
MsgMgr::Fatal(char, char* fileName, int lineNum, char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char* cp = Make(fileName, lineNum, fmt, args);
	va_end(args);
	
	FatalOutput(cp);
}

void
MsgMgr::FatalOutput(char* msg)
{
	AddPMIP(msg);

#ifdef DEBUG
	if (pm.game)
      strcat(msg, "\nEnter-debug Esc-exit");
	if (MsgBox("Fatal", msg))
		ExitThroughDebug();
#else
	char buf[MaxFatalBuf];
	AddFatalStr(Get(Msg_Sorry, buf));
	AddFatalStr(msg);
#endif

	exit(255);
}

char*
MsgMgr::Get(int errNum, char *textBuf)
{
	*textBuf = 0;

	if (errNum <= Msg_LastPreload) {
		char* tmpBuf = preloadedMsgs;
		for (int j = 1; j < errNum; j++)
			while (*tmpBuf++)
				;
		strcpy(textBuf, tmpBuf);

	} else {
		int fd;
		if ((fd = OpenFile()) != -1) {
			if (!Read(errNum, textBuf, fd))
				sprintf(textBuf, "Error #%d not found in file %s", errNum,
					fullFileName);
			Close(fd);
		}
	}

	return textBuf;
}

char*
MsgMgr::Make(char* fileName, int lineNum, int errNum, va_list args)
{
	*buf = 0;
#ifndef DEBUG
	sprintf(buf, "Error %d: ", errNum);
#endif
	char msg[MaxStr + 1];
	vsprintf(buf + strlen(buf), Get(errNum, msg), args);
	AddSrcLoc(buf, fileName, lineNum);
	return buf;
}

char*
MsgMgr::Make(int errNum, va_list args)
{
	*buf = 0;
#ifndef DEBUG
	sprintf(buf, "Error %d: ", errNum);
#endif
	char msg[MaxStr + 1];
	vsprintf(buf + strlen(buf), Get(errNum, msg), args);
	return buf;
}

char*
MsgMgr::Make(char* fileName, int lineNum, char* fmt, va_list args)
{
	vsprintf(buf, fmt, args);
	AddSrcLoc(buf, fileName, lineNum);
	return buf;
}

char*
MsgMgr::Make(char* fmt, va_list args)
{
	vsprintf(buf, fmt, args);
	return buf;
}

void
MsgMgr::Mono(char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	MonoArgs(fmt, args);
}

void
MsgMgr::MonoArgs(char* fmt, va_list args)
{
	vsprintf(buf, fmt, args);
	MonoOutput(buf);
}

void
MsgMgr::MonoPause(char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	MonoArgs(fmt, args);
	if (eventMgr) {
		SOL_Event event = eventMgr->Wait();
		if (event.message == Kbd::Esc)
			exit(2);
	}
}

void
MsgMgr::MonoStr(char* str)
{
	MonoOutput(str);
}

int
MsgMgr::OpenFile()
{
	return OpenFileInPath(fileName, O_TEXT, fullFileName);
}

char*
MsgMgr::Read(int errNum, char* textBuf, int msgfile)
{
	int   num;
	char  fileBuf[2000];
	char*	filePtr = fileBuf;
	char*	linePtr = fileBuf;
	char*	textPtr = textBuf;
	int   count;
	
	*textPtr = 0;
	count = ::Read(msgfile, fileBuf, sizeof fileBuf);
	while (count) {
		if (!strncmp(linePtr, "\\\\", 2)) {
			// found '\\', so read message #
			linePtr += 2;
			// skip over whitespace
			while (*linePtr == '\t' || *linePtr == ' ')
				++linePtr;
			num = 0;
			while (*linePtr >= '0' && *linePtr <= '9') {
				num *= 10;
				num += *linePtr++ - '0';
			}

			// found error message 
			if (num == errNum) {
				// advance to next line 
				while (count) {
					if (!--count)
						count = ::Read(msgfile, filePtr = fileBuf, sizeof(fileBuf));
					if (*filePtr++ == '\n')
						break;
				}
				linePtr = filePtr;

				while (count) {
					// if terminating '\\' is found return error message
					if (!strncmp(linePtr, "\\\\", 2)) {
						// terminate the string 
						textPtr[-1] = 0;
						return textBuf;
					}

					// advance to next line while copying to textBuf
					while (count) {
						if (!--count)
							count = ::Read(msgfile, filePtr = fileBuf,sizeof(fileBuf));
						*textPtr++ = *filePtr;
						if (*filePtr++ == '\n')
							break;
					}
					linePtr = filePtr;
				}
			}
		}

		// advance to next line
		while (count) {
			if (!--count)
				count = ::Read(msgfile, filePtr = fileBuf, sizeof(fileBuf));
			if (*filePtr++ == '\n')
				break;
		}
		linePtr = filePtr;
	}

	return 0;
}

void
MsgMgr::SetQuitStr(char* text)
{
	assert(strlen(text) < MaxStr);
	strcpy(quitStr, text);
}

void
MsgMgr::SetSCIFatalStr(char* text)
{
	assert(strlen(text) < MaxStr);
	strcpy(sciFatalStr, text);
}

void
MsgMgr::SaveRestore(SaveGame* saveGame, Bool save)
{
	saveGame->SaveRestore(quitStr, sizeof quitStr, save);
	saveGame->SaveRestore(sciFatalStr, sizeof sciFatalStr, save);
}

/////////////////////////////////////////////////////////////////////////////

int
AssertFail(char* file, int line, char* expression)
{
	// return value only useful for the way this function is used in
	// assert() macro

	if (msgMgr)
		msgMgr->Fatal("Assertion failed in %s(%d): %s", file, line, expression);
	else {
		printf("Assertion failed in %s(%d): %s\n", file, line, expression);
		abort();
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

void
KSetQuitStr(argList)
{
	msgMgr->SetQuitStr(*(TextID) arg(1));
}

#ifdef DEBUG
void
KSetFatalStr(argList)
{
	msgMgr->SetSCIFatalStr(*(TextID) arg(1));
}

void
KMonoOut(argList)
{
	TextID formatID = arg(1);
	msgMgr->MonoArgs(*formatID, *MakeVprintfArgs(*formatID, &arg(2)));
}
#endif
