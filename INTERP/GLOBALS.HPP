// globals.hpp
// 	SCI global variable numbers

#ifndef GLOBALS_HPP
#define GLOBALS_HPP

enum GlobalNum {
	g_ego,				//pointer to ego
	g_theGame,			//ID of the Game instance
	g_curRoom,			//ID of current room
	g_thePlane,			//default plane
	g_quit,				//when TRUE, quit game
	g_cast,				//collection of actors
	g_regions,			//set of current regions
	g_timers,			//list of timers in the game
	g_sounds,			//set of sounds being played
	g_inventory,		//set of inventory items in game
	g_planes,			//list of all active planes in the game
	g_curRoomNum,		//current room number
	g_prevRoomNum,		//previous room number
	g_newRoomNum,		//number of room to change to
	g_debugOn,			//generic debug flag -- set from debug menu
	g_score,				//the player's current score
	g_possibleScore,	//highest possible score
	g_textCode,			//code that handles interactive text
	g_cuees,				//list of who-to-cues for next cycle
	g_theCursor,		//number of the current cursor
	g_normalCursor,	//number of normal cursor form
	g_waitCursor,		//cursor number of "wait" cursor
	g_userFont,			//font to use for Print
	g_smallFont,		//small font for save/restore, etc.
	g_lastEvent,		//the last event (used by save/restore game)
	g_eventMask,		//event mask passed to GetEvent in (uEvt new:)
	g_bigFont,			//large font
	g_version,			//pointer to 'incver' version string
	g_autoRobot,			
	g_curSaveDir,		//address of current save drive/directory string
	g_numCD,				//number of current CD, 0 for file based
	g_perspective,		//player's viewing angle: degrees away
							//	from vertical along y axis
	g_features,			//locations that may respond to events
	g_unused_5,			
	g_useSortedFeatures,	//enable cast & feature sorting?
	g_unused_6,		
	g_overlays,			
	g_doMotionCue,		//a motion cue has occurred - process it
	g_systemPlane,		//ID of standard system plane
	g_saveFileSelText,//text of fileSelector item that's selected.
	g_unused_8,			
	g_unused_2,			
	g_sysLogPath,		//used for system standard logfile path	
	g_endSysLogPath =	62,	//(uses 40 globals)
	g_gameControls,	//pointer to instance of game controls
	g_ftrInitializer,	//pointer to code that gets called from a feature's init
	g_doVerbCode,		//pointer to code that gets invoked if no feature claims a
							//  user event
	g_approachCode,	//pointer to code that translates verbs into bits
	g_useObstacles,	//will Ego use PolyPath or not?
	g_unused_9,			
	g_theIconBar,		//points to TheIconBar or Null	
	g_mouseX,			//last known mouse position
	g_mouseY,				
	g_keyDownHandler,	//our EventHandlers, get called by game
	g_mouseDownHandler,
	g_directionHandler,
	g_speechHandler,	//a special handler for speech events
	g_lastVolume,		
	g_pMouse,			//pointer to a Pseudo-Mouse, or NULL
	g_theDoits,			//list of objects to get doits each cycle
	g_eatMice,			//how many ticks before we can mouse
	g_user,				//pointer to specific applications User
	g_syncBias,			//globals used by sync.sc
	g_theSync,			//		(will be removed shortly)
	g_extMouseHandler,//extended mouse handler
	g_talkers,			//list of talkers on screen
	g_inputFont,		//font used for user type-in
	g_tickOffset,		//used to adjust gameTime after restore
	g_howFast,			//measurment of how fast a machine is
	g_gameTime,			//ticks since game start
	g_narrator,			//pointer to narrator (normally Narrator)
	g_msgType,			//type of messages used
	g_messager,			//pointer to messager (normally Messager)
	g_prints,			//list of Print's on screen
	g_walkHandler,		//list of objects to get walkEvents
	g_textSpeed,		//time text remains on screen
	g_altPolyList,		//list of alternate obstacles
	g_lastSysGlobal =	99
};

#endif
