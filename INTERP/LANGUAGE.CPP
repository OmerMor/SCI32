//	language.cpp

#include "sol.hpp"

#include "config.hpp"
#include "language.hpp"
#include "msg.hpp"
#include "object.hpp"
#include "pmachine.hpp"
#include "selector.hpp"

//	Country codes per the phone book
const	ENGLISH		= 1;
const	JAPANESE		= 81;
const	GERMAN		= 49;
const	FRENCH		= 33;
const	SPANISH		= 34;
const	ITALIAN		= 39;
const	PORTUGUESE	= 351;

void
InitLanguage()
{
	int language = configMgr->GetNum("language", 0, ENGLISH);
	if ((int) pm.game)
		switch(language) {
			case ENGLISH:
			case JAPANESE:
			case GERMAN:
			case FRENCH:
			case SPANISH:
			case ITALIAN:
			case PORTUGUESE:
				pm.game.SetProperty(s_printLang, language);
				return;
		}

	msgMgr->Fatal(SrcLoc, Msg_UnknownLanguage, language);
}
