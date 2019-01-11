#if !defined LZSS_HPP
#define LZSS_HPP

class LZSS {

	public:

	LZSS();
	~LZSS();
	long Encode(char * inBuffer, long remaining, char * outBuffer);
	long Decode(char * inBuffer, long remaining, char * outBuffer, long outputLength);

	private:

	int initError;
	void InitTree(void);
	void InsertNode(short r);
	void DeleteNode(short p);

	long textsize;	/* text size counter */
	long codesize;	/* code size counter */

	unsigned char * text_buf;	/* ring buffer of size N,
		with extra F-1 bytes to facilitate string comparison */

	short match_position;
	short match_length;  /* of longest match.  These are
			set by the InsertNode() procedure. */

	short * lson;
	short * rson;
	short * dad;  /* left & right children & parents -- These constitute binary search trees. */


};


#endif
