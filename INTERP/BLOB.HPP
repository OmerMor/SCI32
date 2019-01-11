
const int blobListSize = 10;
class  BlobRect {

	BlobRect(){};

	protected:
	int top,left,bottom,right; 	// corner offsets in the source bitmap
	int bitmapWidth;
	int squareSize;

	public:
	BlobRect(int squareSize, int bitmapWidth,int top, int left, int bottom,int right);
	~BlobRect() {};
	void BlobIt(char * bitmap);
};

class Blob {
	protected:
	int bitmapWidth;
	int bitmapHeight;
	BlobRect * blobRects[blobListSize];
	
	public:
	Blob(int bitmapWidth,int bitmapHeight);
	~Blob();
	int AddRect(int squareSize, int top, int left,int bottom,int right);	// return rect no
	void DeleteRect(int rectNo);
	void Process(char * bitmap);	// do all the blobs in the list
};
