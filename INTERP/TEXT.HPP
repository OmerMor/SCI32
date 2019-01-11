//	text.hpp

#ifndef TEXT_HPP
#define TEXT_HPP

/* Defines for text strings with special control codes (fonts, colors, etc.) */
const MAXTEXTFONTS		= 100;
const MAXTEXTCOLORS	= 100;
const char CTRL_CHAR			= '|';

class SOL_Rect;

void		RTextSize(SOL_Rect*, char*, int, int);
int		GetLongest(char**, int, int);
int		GetHighest(char*, int, int);
int*		RTextBox(char*, int, SOL_Rect*, int, int);
int		RStringWidth(char*);
void		DrawString(char*);
void		ShowString(char*);
void		RDrawText(char*, int, int, int, int);
void		ShowText(char*, int, int, int, int);

extern uchar		textColors[];
extern unsigned	textFonts[];
extern uchar		lastTextColor;
extern uchar		lastTextFont;
extern int*		newRect;
extern int		rectIndex;

#endif

