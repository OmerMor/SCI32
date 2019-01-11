#include "blob.hpp"
#include <stdlib.h>
#include <time.h>
#include "sol.hpp"

BlobRect::BlobRect(int squareSize, int bitmapWidth, int top, int left, int bottom,int right) :
	squareSize(squareSize), bitmapWidth(bitmapWidth), top(top),left(left), 
	bottom(bottom), right(right)
{
}

void BlobRect::BlobIt(char * bitmap)
{
	int x,y;

	y = top;
	char * ptr = bitmap + y * bitmapWidth + left;

	while(y <= bottom) {
		x = left;
		char * squareStart = ptr;

		// Do one row of squares
		while(x < right) {
			// do the square
			char * pixel = squareStart;
			char * lineStart = pixel;
			char color = *pixel;

			// do one square

			// clip
			int yCount;
			if(y + squareSize > bottom)
				yCount = bottom - y + 1;
			else
				yCount = squareSize;

			int xCount;
			if(x + squareSize > right)
				xCount = right - x + 1;
			else
				xCount = squareSize;

			for(int row = 0; row < yCount; row++) {
				for(int col = 0; col < xCount ; col++) {
					*pixel++ = color;
				}
				pixel = lineStart + bitmapWidth;
				lineStart += bitmapWidth;
			}

			x += squareSize;
			squareStart += squareSize;
		}

		y += squareSize;
		ptr += bitmapWidth * squareSize;
	}
}

Blob::Blob(int bitmapWidth, int bitmapHeight) :
	bitmapWidth(bitmapWidth), bitmapHeight(bitmapHeight)
{
	for(int i = 0;  i < blobListSize;  i++)
		blobRects[i] = NULL;

}

Blob::~Blob() 
{
	for(int i = 0;  i < blobListSize;  i++)
		delete blobRects[i];
}

int Blob::AddRect(int squareSize, int top, int left, int bottom, int right)
{
	// returns the rectangle no or -1 if full

	// find an empty array entry;
	int i = 0;
	while(blobRects[i] && i < blobListSize) {
		i++;
	}

	if(i == blobListSize)
		return -1;

	if(bottom >= bitmapHeight)
		bottom = bitmapHeight - 1;

	if(right >= bitmapWidth)
		right = bitmapWidth - 1;
		
	blobRects[i] = New BlobRect(squareSize, bitmapWidth,top,left,bottom,right);
	return i;
}

void Blob::DeleteRect(int which)
{
	delete blobRects[which];;
	blobRects[which] = NULL;
}

void Blob::Process(char * bitmap)
{
	for(int i = 0; i < blobListSize; i++) {
		if(blobRects[i]) {
			blobRects[i]->BlobIt(bitmap);
		}
	}
}

