;
;
;
;

	.386
	.MODEL	FLAT,SYSCALL
        .DATA
	extern	sysTicks:DWORD
	dwLowTickCount dd 0
        .CODE
GetDOSMillisec PROC

;       /*--- Disable interrupts; Issue read-back command for counter 0 ---*/
	push	ebx
	push	ecx
	push	edx
	push	esi

start:  mov     al, 0C2h                ;; read back command
        cli
        out     43h, al                 ;; send command to 8254
	nop

;       /*--- Set carry flag to counter 0 OUT pin status ---*/
        in      al, 40h                 ;; OUT status is in bit 7
	nop
        shl     al, 1                   ;; carry = OUT status

;       /*--- Set CX to where in interval we are ---*/
        in      al, 40h                 ;; read counter low
	nop
        movzx	ecx, al			;; save counter low

        in      al, 40h                 ;; read counter high
	nop
        mov     ch, al                  ;; save counter high
        jcxz    restart                 ;; restart if counter is zero
	jc	xxx
	add	cx, 19886
xxx:
	mov	eax, 39772
	sub	eax, ecx
	mov	ebx, 25
	mul	ebx
	mov	ebx, 59658
	div	ebx
	mov	esi, eax

;       /*--- Convert ticks to milliseconds (1/60 to 1/1000 count) ---*/
	mov	eax, sysTicks
	mov	ebx, 50
	mul	ebx
	mov	bx, 3
	div	ebx
	add	eax, esi

;       /*--- Assure tick count is equal or advancing ---*/
        cmp     eax, DWORD PTR [dwLowTickCount]
        jb      restart

;       /*--- Save last tick count obtained ---*/
done:   mov     DWORD PTR [dwLowTickCount], eax
        sti
	pop	esi
	pop	edx
	pop	ecx
	pop	ebx
	ret

;       /*--- Restart procedure ---*/
restart: sti
         jmp    start

GetDOSMillisec ENDP

        END
