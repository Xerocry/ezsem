	;1.3.asm

	org 8400h
	mov a, #0h
	mov r0, #50h
m1:	
		mov @r0, a

		mov r1, a
		mov a, r0
		add a, #2h
		mov r0, a
		mov a, r1

	cjne r0,#60h,m1
	ret