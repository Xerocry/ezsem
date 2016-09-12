	;1.5.asm
	
	org 8400h
	
	mov r0,#20h  ; operands addrs
	mov @r0,#09h ; value, 4 little bits
	
	; f1(08h) = A(0h) and not B(1h)
	mov C, 00h
	anl C, /01h
	mov 08h,C
		
	; f2(09h) = C or D
	mov C, 02h
	orl C, 03h
	mov 09h,c

	; f1(08h) xor f2(09h)
	mov C,08h
	anl C,/09h
	mov 0ah,C
	mov C,09h
	anl C,/08h
	orl C,0ah
	mov 10h,C
	ret