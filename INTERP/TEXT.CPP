#include <stdio.h>

#include "sol.hpp"

#include "array.hpp"
#include "font.hpp"
#include "kernel.hpp"
#include "rect.hpp"
#include "text.hpp"
#include "textid.hpp"

void
KDisplay(argList)
{
	puts(*(TextID) arg(1));
}

void
KTextFonts(kArgs)
{
}

void
KTextColors(kArgs)
{
}

