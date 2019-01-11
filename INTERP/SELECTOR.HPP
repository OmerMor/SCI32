//	selector.hpp
//		SCI selector values

#ifndef SELECTOR_HPP
#define SELECTOR_HPP

typedef SCIUWord	Selector;

//	beginning and ending offset of graphics-critical properties
const MinGraphUpdateOffset	= 0x34;	//	offset of 'plane' property
const MaxGraphUpdateOffset	= 0x58;	//	offset of 'bitmap' property
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
	actLS,
	actNSLeft,
	actNSTop,
	actNSRight,
	actNSBottom,
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
	actIllegalBits,
	actScaleX,
	actScaleY,
	actMaxScale,
	actScaleSignal,
	actBitmap,
	actPlaneID,
	actFixedPri,
	actUseInsetRect,
	actInsetRectL,
	actInsetRectT,
	actInsetRectR,
	actInsetRectB,
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
	avClient,
	avHeading,
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

// Critical properties
const	s_planeID			= 0;
const	s_x					= 1;
const	s_y					= 2;
const	s_z					= 3;
const	s_scaleX				= 4;
const	s_scaleY				= 5;
const	s_maxScale			= 6;
const	s_priority			= 7;
const s_fixedPriority	= 8;
const s_insetRectL		= 9;
const s_insetRectT		= 10;
const s_insetRectR		= 11;
const s_insetRectB		= 12;
const s_useInsetRect		= 13;
const	s_view				= 14;
const	s_loop				= 15;
const	s_cel					= 16;
const s_bitmap				= 17;

// Non-critical properties
const	s_nowSeen			= 18;
const	s_nowSeenL			= s_nowSeen;
const	s_nowSeenT			= 19;
const	s_nowSeenR			= 20;
const	s_nowSeenB			= 21;
const	s_lastSeen			= 22;
const	s_lastSeenL		   = s_lastSeen;
const	s_lastSeenT			= 23;
const	s_lastSeenR       = 24;
const	s_lastSeenB			= 25;
const	s_signal				= 26;
const	s_illegalBits		= 27;
const	s_baseRect			= 28;
const	s_baseRectL			= s_baseRect;
const	s_baseRectT			= 29;
const	s_baseRectR			= 30;
const	s_baseRectB			= 31;
const	s_name				= 32;
const	s_key					= 33;
const	s_time				= 34;
const	s_text				= 35;
const	s_elements			= 36;
const	s_fore				= 37;
const	s_back				= 38;
const	s_mode				= 39;
const	s_style				= 40;
const	s_state				= 41;
const	s_font				= 42;
const	s_type				= 43;
const	s_window				= 44;
const	s_cursor				= 45;
const	s_max					= 46;
const	s_mark				= 47;
const	s_who					= 48;
const	s_message			= 49;
const	s_edit				= 50;
const	s_play				= 51;
const	s_number				= 52;
const	s_nodePtr			= 53;
const	s_client 			= 54;
const	s_dx 					= 55;
const	s_dy 					= 56;
const	s_bMoveCnt 			= 57;
const	s_bI1 				= 58;
const	s_bI2 				= 59;
const	s_bDi 				= 60;
const	s_bXAxis 			= 61;
const	s_bIncr 				= 62;
const	s_xStep 				= 63;
const	s_yStep 				= 64;
const	s_moveSpeed			= 65;
const	s_cantBeHere 		= 66;
const	s_heading			= 67;
const	s_mover				= 68;
const	s_doit				= 69;
const	s_isBlocked			= 70;
const	s_looper				= 71;
const	s_modifiers			= 72;
const	s_replay				= 73;
const	s_setPri				= 74;
const	s_at					= 75;
const	s_next				= 76;
const	s_done				= 77;
const	s_width				= 78;
const	s_intFail			= 79;
const	s_syntaxFail		= 80;
const	s_semanticFail		= 81;
const	s_pragmaFail		= 82;
const	s_said				= 83;
const	s_claimed			= 84;
const	s_value				= 85;
const	s_save				= 86;
const	s_restore			= 87;
const	s_title				= 88;
const	s_button				= 89;
const	s_icon				= 90;
const	s_draw				= 91;
const	s_delete				= 92;
const	s_parseLang			= 93;
const	s_printLang			= 94;
const	s_subtitleLang		= 95;
const	s_size				= 96;
const	s_points				= 97;
const	s_palette			= 98;
const	s_dataInc			= 99;
const	s_handle				= 100;
const	s_min					= 101;
const	s_sec					= 102;
const	s_frame				= 103;
const	s_vol					= 104;
const	s_BLANK				= 105;
const	s_perform			= 106;
const	s_moveDone			= 107;
const s_topString			= 108;
const s_flags				= 109;
const s_quitGame			= 110;
const s_restart			= 111;
const s_hide				= 112;
const	s_scale_signal		= 113; 
const	s_vanishingX		= 114;
const	s_vanishingY		= 115;
const s_picture			= 116;
const s_resX				= 117;
const s_resY				= 118;
const s_coordType			= 119;
const s_struct				= 120;
const s_data				= 121;
const s_skip				= 122;
const s_left				= 123;
const s_right				= 124;
const s_center				= 125;
const s_all					= 126;
const s_show				= 127;
const s_textRect			= 128;
const s_textLeft			= s_textRect;
const s_textTop			= 129;
const s_textRight			= 130;
const s_textBottom		= 131;
const s_borderColor	   = 132;
const s_titleFore		   = 133;
const s_titleBack		   = 134;
const s_titleFont		   = 135;
const s_dimmed		      = 136;
const s_frameOut	      = 137;
const s_lastKey	      = 138;
const s_magnify			= 139;
const s_power				= 140;
const s_mirrored			= 141;
const s_pitch  			= 142;
const s_roll   			= 143;
const s_yaw    			= 144;
const s_input				= 508;

#endif




