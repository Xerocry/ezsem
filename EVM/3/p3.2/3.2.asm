	org 8400h
T2CON: 	equ 0C8h

pwm:
	;lcall U1_read
	lcall calc
	lcall init
	lcall 128h
	ljmp pwm
	ret

calc:
	mov a, 40h
	mov b, #D9h ;#F7     D9
	mul ab
	mov r3, b
	mov r4, a
	mov a, r4
	clr c
	addc a, #27h ;#F9
	mov r4, a
	mov a, r3
	addc a, #D8h ;#F6
	mov r3, a	
	ret

init:
	orl p1, #00000010b
	mov T2CON, #0

	mov CBh, #EFh
	mov CAh, #D8h

	mov C1h, #00001000b
	mov C3h, r3
	mov C2h, r4
	mov T2CON, #00010001b
	ret

	;include ASMS\43501_3\bk\2\myadc.asm