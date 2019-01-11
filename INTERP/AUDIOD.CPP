// audiod.cpp
//		DOS specific digital audio sample playback

#include "sol.hpp"

#include "audio.hpp"
#include "config.hpp"
#include "memid.hpp"

MemID	audioDrv;

unsigned
GetDACDefaultSize(void)
{
	return DAC_HBUFFSIZE*2;
}

void *
LoadAudioDrv(void)
{
	char* fileName = configMgr->Get("audioDrv");
	if (!fileName || !*fileName)
		return 0;

	/* Load the audio driver.
	 */
	if (!(audioDrv.Load(MemDriver, fileName, PERSIST, AudioDriverHandle))) {
		msgMgr->Alert(Msg_NoAudioDrv, fileName);
		return 0;
	}

	return (*audioDrv);
}

void
FreeAudioDrv(void)
{
	audioDrv.Free();
}
