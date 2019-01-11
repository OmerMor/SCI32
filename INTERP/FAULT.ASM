

	.386
	.MODEL	FLAT,SYSCALL

	.data

	DivideByZero equ 1
	GeneralProtection equ 2


	.code

	AsmPanicReturn proto faultNo:dword



PanicDiv0 proc far

	mov	ax, ds
	mov ss, ax
	invoke AsmPanicReturn, DivideByZero
	
PanicDiv0 endp

PanicGP proc far

	mov	ax, ds
	mov ss, ax
	invoke AsmPanicReturn, GeneralProtection

PanicGP endp


end

