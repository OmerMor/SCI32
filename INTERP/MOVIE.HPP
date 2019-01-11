// Header file for movie.c -- animated movie display

#ifndef MOVIE_HPP
#define MOVIE_HPP

class MemID;

void RunMovie(char * fileName,int speed,int xOff,int yOff);
void RunMovieX(MemID & buffer,int handle, int speed);
void RunMovieVesa(MemID & buffer,int handle, int speed);
void LoadFrame(char page,int handle, MemID & buffer);
void DisplayFrame(int handle, char * bufferHandle);
void DisplayBlock(char * buffer,int short xOff,int short yOff,int short xDim,int short yDim);
void LoadMoviePalette(int  handle);
unsigned short LoadCelCount(int  handle);

extern "C" {
void FillVideoPage(void * xCel,unsigned char * minOffset, unsigned char * maxOffset);
void SetVideoMode(int mode);
void ShowPage(int pageOffset);
void CopyPage(int destPage,int srcPage);
void Copy2VMAP(unsigned char *);
}

#pragma aux (PushArgs) FillVideoPage;
#pragma aux (PushArgs) SetVideoMode;
#pragma aux (PushArgs) ShowPage;
#pragma aux (PushArgs) CopyPage;
#pragma aux (PushArgs) Copy2VMAP;

#endif
