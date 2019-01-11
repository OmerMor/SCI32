//	selector.hpp
//		SCI selector values
//
//		NOTE:  If you add a selector here, you must also add it to SELECTOR.NEW
//			    on S:.  Otherwise the number could change, depending on how the
//				 system is built on S:.

#ifndef SELECTOR_HPP
#define SELECTOR_HPP

typedef SCIUWord	Selector;

//	beginning and ending offset of graphics-critical properties
#ifdef NETWORK
	#define CRITICAL_START	0x30
	#define CRITICAL_END 	0x56
#else
	#define CRITICAL_START	0x34
	#define CRITICAL_END 	0x58
#endif
	
const MinGraphUpdateOffset	= CRITICAL_START;	//	offset of 'plane' property
const MaxGraphUpdateOffset	= CRITICAL_END;	//	offset of 'bitmap' property
const MinGraphUpdateIndex	= MinGraphUpdateOffset / sizeof(Property);
const MaxGraphUpdateIndex	= MaxGraphUpdateOffset / sizeof(Property);

enum {
	actX,
	actY,
	actSignal,
	actView,
	actLoop,
	actCel,
	actPri,
#ifdef NETWORK
	actLeft,
	actTop,
	actRight,
	actBottom,
#else
	actLS,
	actNSLeft,
	actNSTop,
	actNSRight,
	actNSBottom,
#endif
	actBRLeft,
	actBRTop,
	actBRRight,
	actBRBottom,
	actXStep,
	actYStep,
	actLooper,
	actHeading,
	actMover,
	actMoveSpeed,
#ifndef NETWORK
	actIllegalBits,
#endif
	actScaleX,
	actScaleY,
	actMaxScale,
	actScaleSignal,
	actBitmap,
	actPlaneID,
	actFixedPri,
#ifdef NETWORK
	actSeenRect,
#else
	actUseInsetRect,
	actInsetRectL,
	actInsetRectT,
	actInsetRectR,
	actInsetRectB,
#endif
	actXLast,
	actYLast,
	actZ,

	motClient,
	motX,
	motY,
	motDX,
	motDY,
	motI1,
	motI2,
	motDI,
	motIncr,
	motXAxis,
	motMoveCnt,
	motXLast,
	motYLast,

	jmpXStep,
	jmpYStep,

	evX,
	evY,
	evZ,
	evPitch,
	evRoll,
	evYaw,
	evTyp,
	evMsg,
	evMod,
	evClaimed, 

	syncTime,
	syncCue,

	sndHandle,
	sndNumber,
	sndNodePtr,
	sndLoop,
	sndPriority,
	sndVol,
	sndSignal,
	sndMin,
	sndSec,
	sndFrame,
	sndFlags,
	sndDataInc,

	MaxIndexedProperty
};

//#define actSeenRect actUseInsetRect

// Critical properties
const	s_planeID				= 0;
const	s_x					= 1;
const	s_y					= 2;
const	s_z					= 3;
const	s_scaleX				= 4;
const	s_scaleY				= 5;
const	s_maxScale				= 6;
const	s_priority				= 7;
const 	s_fixedPriority				= 8;
const 	s_insetRectL				= 9;
const 	s_insetRectT				= 10;
const 	s_insetRectR				= 11;
const 	s_insetRectB				= 12;
const 	s_useInsetRect				= 13;
const	s_seenRect				= 13;
const	s_view					= 14;
const	s_loop					= 15;
const	s_cel					= 16;
const 	s_bitmap				= 17;

// Non-critical properties

const	s_nowSeen				= 18;
const	s_nowSeenL				= s_nowSeen;
const	s_nowSeenT				= 19;
const	s_nowSeenR				= 20;
const	s_nowSeenB				= 21;

const	s_lastSeen				= 22;
const	s_lastSeenL		   		= s_lastSeen;
const	s_lastSeenT				= 23;
const	s_lastSeenR       			= 24;
const	s_lastSeenB				= 25;

const	s_signal				= 26;
const	s_illegalBits				= 27;
const	s_baseRect				= 28;
const	s_baseRectL				= s_baseRect;
const	s_baseRectT				= 29;
const	s_baseRectR				= 30;
const	s_baseRectB				= 31;
const	s_name					= 32;
const	s_key					= 33;
const	s_time					= 34;
const	s_text					= 35;
const	s_elements				= 36;
const	s_fore					= 37;
const	s_back					= 38;
const	s_mode					= 39;
const	s_style					= 40;
const	s_state					= 41;
const	s_font					= 42;
const	s_type					= 43;
const	s_window				= 44;
const	s_cursor				= 45;
const	s_max					= 46;
const	s_mark					= 47;
const	s_who					= 48;
const	s_message				= 49;
const	s_edit					= 50;
const	s_play					= 51;
const	s_number				= 52;
const	s_nodePtr				= 53;
const	s_client 				= 54;
const	s_dx 					= 55;
const	s_dy 					= 56;
const	s_bMoveCnt 				= 57;
const	s_bI1 					= 58;
const	s_bI2 					= 59;
const	s_bDi 					= 60;
const	s_bXAxis 				= 61;
const	s_bIncr 				= 62;
const	s_xStep 				= 63;
const	s_yStep 				= 64;
const	s_moveSpeed				= 65;
const	s_cantBeHere 				= 66;
const	s_heading				= 67;
const	s_mover					= 68;
const	s_doit					= 69;
const	s_isBlocked				= 70;
const	s_looper				= 71;
const	s_modifiers				= 72;
const	s_replay				= 73;
const	s_setPri				= 74;
const	s_at					= 75;
const	s_next					= 76;
const	s_done					= 77;
const	s_width					= 78;
const	s_pragmaFail				= 79;
const	s_claimed				= 80;
const	s_value					= 81;
const	s_save					= 82;
const	s_restore				= 83;
const	s_title					= 84;
const	s_button				= 85;
const	s_icon					= 86;
const	s_draw					= 87;
const	s_delete				= 88;
const	s_printLang				= 89;
const	s_size					= 90;
const	s_points				= 91;
const	s_palette				= 92;
const	s_dataInc				= 93;
const	s_handle				= 94;
const	s_min					= 95;
const	s_sec					= 96;
const	s_frame					= 97;
const	s_vol					= 98;
const	s_perform				= 99;
const	s_moveDone				= 100;
const 	s_topString				= 101;
const 	s_flags					= 102;
const 	s_quitGame				= 103;
const 	s_restart				= 104;
const 	s_hide					= 105;
const	s_scale_signal				= 106; 
const	s_vanishingX				= 107;
const	s_vanishingY				= 108;
const 	s_picture				= 109;
const 	s_resX					= 110;
const 	s_resY					= 111;
const 	s_coordType				= 112;
const 	s_data					= 113;
const 	s_skip					= 114;
const 	s_center				= 115;
const 	s_all					= 116;
const 	s_show					= 117;
const 	s_textRect				= 118;
const 	s_textLeft				= s_textRect;
const 	s_textTop				= 119;
const 	s_textRight				= 120;
const 	s_textBottom				= 121;
const 	s_borderColor	   			= 122;
const 	s_titleFore		   		= 123;
const 	s_titleBack		   		= 124;
const 	s_titleFont		   		= 125;
const 	s_dimmed		      		= 126;
const 	s_frameOut	      			= 127;
const 	s_lastKey	      			= 128;
const 	s_magnify				= 129;
const 	s_power					= 130;
const 	s_mirrored				= 131;
const 	s_pitch  				= 132;
const 	s_roll   				= 133;
const 	s_yaw    				= 134;

const	s_left					= 135;
const	s_right					= 136;
const	s_top					= 137;
const	s_bottom				= 138;
const	s_numLines        			= 139;

//const	s_intFail			= 79;
//const	s_syntaxFail		= 80;
//const	s_semanticFail		= 81;
//const	s_said				= 83;
//const	s_parseLang			= 93;
//const	s_subtitleLang		= 95;
//const	s_BLANK				= 105;
//const s_struct				= 120;

const s_input				= 503;

#endif

