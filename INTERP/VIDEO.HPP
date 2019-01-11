#ifndef ASSEMBLY_HPP
#define ASSEMBLY_HPP

class SOL_Rect;

extern "C"
{
	void	SetVGAPal(const uchar *);
	void	ShowRect(const SOL_Rect *, uchar *);
	void	ReadVideoRect(const SOL_Rect *,uchar *);
	void	WriteVideoRect(const SOL_Rect *,uchar *, int);
	int	SetupVesa();
	int	VesaSupport();
	void 	SetVMapWidth(int width);
	void 	ShakeScreen(int shakeCount, int shakeDir, int *time);
	void SetVideoMode(int mode);
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
