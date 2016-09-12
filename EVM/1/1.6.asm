	;1.6.asm

	org 8400h
	mov dptr,#8500h
	mov r1,#2h
	mov r2, #0

;m0:	mov a,#0h

;m1:	add a,r1
m0:	add a,r1
	movx @dptr, a
;	cjne a, #10h, m1

;m2:	subb a,r1
;	movx @dptr, a
;	cjne a, #0h, m2

	inc dptr
	inc r2
	cjne r2, #10h, m0

	ret