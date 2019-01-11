#ifndef MBOX_HPP
#define MBOX_HPP

#include <stdio.h>

extern	char	mbuff[];

void			MBox (char *, char *);
void 			SciDisplay (char *,int xOrig=620,int yOrig=400,int width=140,int height=200,int history=50);
Bool			MBox2 (char *, char *);

#endif
