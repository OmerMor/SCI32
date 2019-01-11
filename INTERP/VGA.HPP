#ifndef VGA_HPP
#define VGA_HPP

#if !defined (SOL_HPP)
#include "sol.hpp"
#endif


class Vga
{
	public:
	Vga();
	~Vga();
	void	Set320x200();
	void	Set640x480();
	void	SetText();

	protected:
	int lenx;
	int leny;
	Int16	startupMode;
};


#endif


