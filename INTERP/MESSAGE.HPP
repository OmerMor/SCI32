// message.hpp

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#ifndef SAVEABLE_HPP
#include "saveable.hpp"
#endif

struct MessageMgr : Saveable {
	void	SaveRestore(SaveGame*, Bool save);
} extern messageMgr;

const MessageMajorVersion	= 4000;

_Packed struct IndexEntry {
	uchar		noun;
	uchar		verb;
	uchar		cond;
	uchar		seq;
	uchar		talker;
	UInt16	offset;
	uchar		refNoun;
	uchar		refVerb;
	uchar		refCond;
	uchar		refSeq;
};

_Packed struct MsgData {
	UInt32		version;
	UInt16		length;			// size of file starting at count field, excluding
										//	comments
	UInt16		lastMsgNum;
	UInt16		nEntries;
	IndexEntry	entries[1];
};

#endif
