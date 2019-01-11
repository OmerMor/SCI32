#ifndef CURSORD_HPP
#define CURSORD_HPP

#ifndef CURSOR_HPP
#include "cursor.hpp"
#endif

#ifndef RECT_HPP
#include "rect.hpp"
#endif

#ifndef BUFFERD_HPP
#include "bufferd.hpp"
#endif

class PointerDevice;


class CursorDOS : public SOL_Cursor
{
	public:
	CursorDOS(const BufferStd&);
	~CursorDOS();

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
		uchar*	buffer;
		SOL_Rect	rect;
		uchar		skip;
	};

	void		PushAndDisableInterrupts();
	void		PopInterrupts();
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
	short			interruptState;
	int			disableCount;
	PointerDevice*	posDevice;

	int			restoreHideCount;
	SOL_Rect		restoreRestrict;
};


#endif
