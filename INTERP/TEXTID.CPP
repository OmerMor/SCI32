// textid.cpp

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "sol.hpp"

#include "textid.hpp"

TextID::TextID()
{
}

TextID::TextID(const MemID& id) : MemID(id)
{
}

TextID::TextID(SOL_Handle h) : MemID(h)
{
}

TextID&
TextID::operator =(const char* str)
{
	size_t size = strlen(str) + 1;
	if (!handle)
		Get(size);
	else
		Realloc(size);
	strcpy(**this, str);
	
	return *this;
}

void
TextID::Add(char c)
{
	//	add character
	
	char buf[] = { c, 0 };
	Add(buf);
}

void
TextID::Add(const char* str)
{
	//	add text, which should not be a dereferenced TextID itself
	
	AddSize(strlen(str));
	strcat(**this, str);
}

void
TextID::Add(TextID id)
{
	//	add text from id
	
	AddSize(strlen(*id));
	strcat(**this, *id);
}

void
TextID::AddF(const char* fmt, ...)
{
	//	add formatted text
	
	char buf[240];
	va_list	args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	
	Add(buf);
}
	
void
TextID::AddSize(size_t additionalSize)
{
	//	increase the size

	if (!handle) {
		Get(additionalSize + 1);
		***this = 0;
	} else
		Realloc(strlen(**this) + additionalSize + 1);
}

char*
TextID::Lock()
{
	return (char*) MemID::Lock();
}

void
TextID::Snug()
{
	//	realloc to just the right size
	
	if (handle)
		Realloc(strlen(**this) + 1);
}
