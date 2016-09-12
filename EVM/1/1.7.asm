	;1.7.asm
	
	org 8400h

	mov dptr,#8500h
	mov a, #10h
	movx @dptr, a

	inc dptr
	mov a, #5h
	movx @dptr, a

	inc dptr
	mov a, #ffh
	movx @dptr, a

	inc dptr
	mov a, #3h
	movx @dptr, a

	inc dptr
	mov a, #11h
	movx @dptr, a
	ret

	mov dptr,#8500h
	mov r2, #ffh
	mov r1, #0h
m0:	movx a, @dptr
	subb a, r2
	jnz m1
	mov r2, a 
m1:	
	inc dptr
	cjne r1, #4h, m0
	ret 