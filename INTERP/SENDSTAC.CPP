//	sendstac.cpp

#include "sol.hpp"

#ifdef DEBUG
#include <stdio.h>
#include <string.h>

#include "sendstac.hpp"
#include "savegame.hpp"
#include "string.hpp"
#include "textid.hpp"
#include "window.hpp"

SendStackDebug::SendStackDebug() : on(True), lastPtr(0), ptr(-1), window(0) 
{
}

SendStackDebug::~SendStackDebug()
{
	delete window;
}

void
SendStackDebug::Clear()
{
	ptr = -1;
}

void
SendStackDebug::PushObject(ObjectID object)
{
	ptr++;
	if (ptr < Size)
		stack[ptr].object = object;
}

void
SendStackDebug::PushSelector(Selector selector)
{
	//	put selector in slot reserved when object was pushed

	stack[ptr].selector = selector;
	//	force update
	lastPtr = 0;
}

void
SendStackDebug::Pop()
{
	ptr--;
}

SendStack::Entry*
SendStackDebug::Top()
{
	return &stack[ptr];
}

void
SendStackDebug::Toggle()
{
	on = !on;
	if (on)
		Show();
	else
		Hide();
}

void
SendStackDebug::Hide()
{
	delete window;
	window = 0;
}

void
SendStackDebug::Show()
{
	if (!on)
		return;
		
	//	if we're already displaying the window, and the pointer hasn't changed,
	//	then the window contents haven't changed, so don't redisplay it
	if (window && lastPtr == ptr)
		return;
		
	lastPtr = ptr;

	TextID buf;

	// account for clipped send stack
	int sp = ptr;
	if (sp >= Size) {
		sp = Size - 2;
		buf.Add("Stack clipped\n");
	}

	int ss = 0;
	if (sp < ss)
		buf.Add("No send stack");
	else {
		if (sp > (ss + 20))
			ss = sp - 19;			// Max of 20 messages on screen at a time
		for ( ; sp >= ss ; sp--) {
			char	lineBuf[100];
			strcpy(lineBuf, "(");
			ObjectID object = stack[sp].object;

			char name[100];
			char* namePtr;
			if (*object.Name()) {
				strcpy(name, object.Name());
				namePtr = name;
			} else
				namePtr = 0;

			if (!namePtr || !IsPrintable(namePtr))
				sprintf(lineBuf + strlen(lineBuf), "$%04x ", (int) object);
			else
				sprintf(lineBuf + strlen(lineBuf), "%s ", namePtr);
			char select[80];
			const char* msgPtr = GetSelectorName(stack[sp].selector, select);
			if (!msgPtr)
				sprintf(lineBuf + strlen(lineBuf), "%d", select);
			else
				sprintf(lineBuf + strlen(lineBuf), "%s", msgPtr);
			strcat(lineBuf, ":)\n");
			buf.Add(lineBuf);
		}
	}

	if (ss)
		buf.Add("Stack clipped\n");

	delete window;
	const width = 100;
	window = DebugWindow(*buf, 0, width, SOL_Point(0, 14));

	buf.Free();
}

void
SendStackDebug::SaveRestoreBefore(SaveGame*, Bool)
{
	delete window;
	window = 0;
}

#endif

