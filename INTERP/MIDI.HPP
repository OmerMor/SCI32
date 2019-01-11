//	midi.hpp

#ifndef MIDI_HPP
#define MIDI_HPP

#ifndef INTRPT_HPP
#include "intrpt.hpp"
#endif

extern "C" {
	int			DoSound(int, ...);
	ServerFunc	SoundServer;
};
#pragma aux (PushArgs) DoSound;

#endif


