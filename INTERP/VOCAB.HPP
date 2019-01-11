// vocab.hpp

#ifndef VOCAB_HPP
#define VOCAB_HPP

// special vocab files containing strings/data need by the system 
const	KERNEL_VOCAB	= 999;
const	OPCODE_VOCAB	= 998;
const	SELECTOR_VOCAB	= 997;
const	CLASSTBL_VOCAB	= 996;
const	HELP_VOCAB		= 995;
const	PROPOFS_VOCAB	= 994;

int	GetVocabStr(int moduleNum, int entryNum, char* buffer);

#endif

