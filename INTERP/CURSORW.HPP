#ifndef CURSORW_HPP
#define CURSORW_HPP

#ifndef CURSOR_HPP
#include "cursor.hpp"
#endif

#ifndef RECT_HPP
#include "rect.hpp"
#endif

#ifndef BUFFERD_HPP
#include "bufferd.hpp"
#endif

#ifndef BUFFERW_HPP
#include "bufferw.hpp"
#endif

class PointerDevice;


class CursorWin : public SOL_Cursor
{
	public:
	CursorWin();
	~CursorWin();

	void		Hide();
	void		UnHide();
	void		Show();
	void		Set(int v, int l, int c);
	Bitmap&	Get() {return bitmap;}
	void		DeviceMovedTo(const SOL_Point&);
	void		ForcePos(SOL_Point);
	void		GonnaPaint(SOL_Rect) {}
	void		PaintStarting() {}
	void		DonePainting() {}
	void		SetRestrictRect(const SOL_Rect&);
	void		ClearRestrictRect();
	void		SetDevice(PointerDevice&) {}
	int		GetHideCount() {return hideCount;}


	protected:
	Bitmap		bitmap;
	int			hideCount;
	SOL_Rect		fullSize;
	uchar*		imageData;
	SOL_Rect		restrict;
};



class CursorWinClr : public SOL_Cursor
{
	public:
	CursorWinClr(const BufferWin&);
	~CursorWinClr();

	void		Hide();
	void		UnHide();
	void		Show();
	void		Set(int v, int l, int c);
	Bitmap&	Get() {return bitmap;}
	void		SetDevice(PointerDevice&);
	void		DeviceMovedTo(const SOL_Point&);
	void		ForcePos(SOL_Point);
	void		GonnaPaint(SOL_Rect);
	void		PaintStarting();
	void		DonePainting();
	void		SetRestrictRect(const SOL_Rect&);
	void		ClearRestrictRect();
	void		SaveRestore(SaveGame*, Bool);
	void		SaveRestoreAfter(SaveGame*, Bool);
	int		GetHideCount() {return hideCount;}


	protected:
	struct Info  {
		Info(Bool invertedFlag) : invertedFlag(invertedFlag) {}
		uchar*	buffer;
		SOL_Rect	rect;
		uchar		skip;
		Bool		invertedFlag;
	};

	void		Paint(const Info& dest, const Info& src);
	void		Copy(const Info& dest, const Info& src);
	void		Move();
	void		DrawToHardware(const Info&);
	void		ReadVideo(const Info&);
	void		ReadVideoFromVMAP(const Info&);
	void		ReadVideoFromHardware(const Info&);
	void		RevealCursor();



	Bool			gonnaPaint;
	Bool			putCursorInVMAP;
	int			hideCount;
	Bitmap		bitmap;
	int			xhot, yhot;
	Info			cursorData;
	Info			cursorBack;
	Info			saveVmap;
	Info			vmapData;
	Info			drawBuff1;
	Info			drawBuff2;

	SOL_Rect		restrict;
	SOL_Point	nextPos;
	int			disableCount;
	PointerDevice*	posDevice;

	int			restoreHideCount;
	SOL_Rect		restoreRestrict;
};


#endif
