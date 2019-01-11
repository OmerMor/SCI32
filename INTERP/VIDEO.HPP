#ifndef ASSEMBLY_HPP
#define ASSEMBLY_HPP

class SOL_Rect;
class SOL_Point;

extern "C"
{
	void	SetVGAPal(const uchar *);
	void	ShowRect(const SOL_Rect *, uchar *);
	void	ShowMovieRect(SOL_Rect *,uchar*,int,SOL_Point*);
	void	ShowMovieRectDbl(SOL_Rect *,uchar*,int,SOL_Point*);
	void	ShowMovieBlack(SOL_Rect *,uchar*,int,SOL_Point*);
	void	ShowMovieBlackDbl(SOL_Rect *,uchar*,int,SOL_Point*);
	void	BlackOutScreen(SOL_Rect *);

	void	MemcpyDBL(uchar* ptrD,uchar* ptrS,int s,int sx);
	#pragma aux MemcpyDBL parm [EDI] [ESI] [ECX] [EBX];
	void	MemcpyBlack(uchar* ptrD,uchar* ptrS,int s);
	#pragma aux MemcpyBlack parm [EDI] [ESI] [ECX];
	void	ReadVideoRect(const SOL_Rect *,uchar *);
	void	WriteVideoRect(const SOL_Rect *,uchar *, int);
	int	SetupVesa();
	int	VesaSupport();
	void 	SetVMapWidth(int width);
	void 	ShakeScreen(int shakeCount, int shakeDir, int *time);
	void  SetVideoMode(int mode);
}
#pragma aux (PushArgs) SetVGAPal;
#pragma aux (PushArgs) ShowRect;
#pragma aux (PushArgs) ReadVideoRect;
#pragma aux (PushArgs) WriteVideoRect;
#pragma aux (PushArgs) SetupVesa;
#pragma aux (PushArgs) VesaSupport;
#pragma aux (PushArgs) SetVMapWidth;
#pragma aux (PushArgs) SetVideoMode;

#endif
