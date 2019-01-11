//	mono.hpp

#ifndef MONO_HPP
#define MONO_HPP

extern "C" {
void	MonoStr(char* text);
void	MonoChar(char);
void	MonoSet(int row,int col);
void	MonoInit();
};

#pragma aux (PushArgs) MonoStr;
#pragma aux (PushArgs) MonoChar;
#pragma aux (PushArgs) MonoSet;
#pragma aux (PushArgs) MonoInit;

#endif
