#ifndef CELASM_HPP
#define CELASM_HPP

extern "C" uchar*	colorptr;
extern "C" uchar*	destptr;
extern "C" uchar*	controlptr;
extern "C" int*	scaleptr;
extern "C" int		pixelcount;
extern "C" int		beginindex;
extern "C" uchar	skipval;

extern "C"  {
	void		WriteUncompLine();
	void		WriteUncompLineBack();
	void		WriteUncompLineBackNoSkip();
	void		WriteUncompScaleLine();
	void		WriteUncompScaleLine2();
	void		WriteUncompScaleLine2Mirr();
	void		WriteCompSkipLine();
	void		WriteCompSkipLineBack();
	void		WriteWholeCompSkipLine();
};

#endif
