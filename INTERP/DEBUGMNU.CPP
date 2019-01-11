//	debugmnu.cpp

#include "sol.hpp"

#ifdef DEBUG

#include "debug.hpp"
#include "debugmnu.hpp"
#include "event.hpp"
#include "font.hpp"
#include "window.hpp"

DebugMenus	debugMenus;

DebugMenus::DebugMenus() : visible(False), count(0)
{
}

Bool
DebugMenus::HandleEvent(SOL_Event& event)
{
	assert(count);
	return items[count - 1]->HandleEvent(event, visible);
}

void
DebugMenus::Add(DebugMenu* menu)
{
	items[count++] = menu;
	if (visible)
		menu->Show();
}

void
DebugMenus::Delete()
{
	assert(count);
	items[--count]->Hide();
}

void
DebugMenus::Hide()
{
	for (int i = 0; i < count; i++)
		items[i]->Hide();
}

void
DebugMenus::Show()
{
	for (int i = 0; i < count; i++)
		items[i]->Show();
}

void
DebugMenus::Toggle()
{
	visible = !visible;
	if (visible)
		Show();
	else
		Hide();
}

///////////////////////////////////////////////////////////////////////////////

DebugMenu::DebugMenu(char* label, DebugMenuItem* items) :
	label(label), items(items), isSubmenu(False), window(0)
{
}

DebugMenu::~DebugMenu()
{
	//	this is problematic, since by the time we destruct a static DebugMenu,
	//	the graphMgr may have already been destructed itself
//	delete window;
}

Bool
DebugMenu::HandleEvent(SOL_Event& event, Bool visible)
{
	if (event.type == SOL_Event::KeyDown) {
		Kbd::Key key = event.message;
		if (key == Kbd::Esc) {
			eventMgr->PostCommand(0);
			return True;
		}

		else if (key == '/' && !IsSubmenu()) {
			debugMenus.Toggle();
			return True;
		}
	}
		
	return Claim(event, visible);
}

Bool
DebugMenu::Claim(SOL_Event& event, Bool visible)
{
	for (DebugMenuItem* item = items; *item; item++)
		if (item->Claim(event, visible))
			return True;
	return False;
}

void
DebugMenu::Hide()
{
	delete window;
	window = 0;
}

void
DebugMenu::Show()
{
	if (!window) {
		TextID buf;
		if (label)
			buf.AddF("%s:", label);
		for (DebugMenuItem* item = items; *item; item++)
			item->ShowStr(buf);
	
		const height = 14*YSCALE;
		window = New EventWindow(SOL_Rect(0, SCIRESY - height , SCIRESX-1, SCIRESY-1), *buf, 0,
			255, systemFont, TEJUSTLEFT, False);
	
		buf.Free();
	}
	
	window->Show();
}

/////////////////////////////////////////////////////////////////////////////

DebugMenuItem::DebugMenuItem(char* label, ushort menuKey, int cmd,
									  ushort hotKey) :
	label(label), cmd(cmd, menuKey, hotKey), cmds(0)
{
}

DebugMenuItem::DebugMenuItem(char* label, DebugMenuCmd* cmds) :
	label(label), cmd(0), cmds(cmds)
{
}

DebugMenuItem::DebugMenuItem(char* label, ushort menuKey, DebugMenu* menu) :
	label(label), cmd(menuKey, menu), cmds(0)
{
}

DebugMenuItem::DebugMenuItem(int) : label(0), cmd(0), cmds(cmds)
{
}

Bool
DebugMenuItem::Claim(SOL_Event& event, Bool visible)
{
	if (cmd)
		return cmd.Claim(event, visible);

	for (DebugMenuCmd* cmd = cmds; *cmd; cmd++) {
		if (cmd->Claim(event, visible))
			return True;
	}
	return False;
}

void
DebugMenuItem::ShowStr(TextID& buf)
{
	if (label && *label)
		buf.AddF(" %s", label);
}

/////////////////////////////////////////////////////////////////////////////

DebugMenuCmd::DebugMenuCmd(int cmd, ushort menuKey, ushort hotKey) :
	cmd(cmd), menuKey(menuKey), hotKey(hotKey ? hotKey : menuKey), menu(0)
{
}

DebugMenuCmd::DebugMenuCmd(ushort menuKey, DebugMenu* menu) :
	menuKey(menuKey), menu(menu)
{
	menu->IsSubmenu(True);
}

DebugMenuCmd::DebugMenuCmd(int) : cmd(0), hotKey(0), menuKey(0), menu(0)
{
}

Bool
DebugMenuCmd::Claim(SOL_Event& event, Bool visible)
{
	if (menu)
		if (visible && event.type == SOL_Event::KeyDown &&
			 event.message == menuKey) {
			//	trap all input until submenu is done
			debugMenus.Add(menu);
			do
				event = eventMgr->Wait(SOL_Event::All);
			while (!debugMenus.HandleEvent(event));
			debugMenus.Delete();
			return True;
		} else if (!visible)
			return menu->Claim(event, visible);
		else
			return False;
	
	if (visible && event.type == SOL_Event::KeyDown && event.message== menuKey ||
	    !visible && event.type == SOL_Event::KeyDown && event.message== hotKey) {
		eventMgr->PostCommand(cmd);
		return True;
	}

	return False;		
}

#endif
