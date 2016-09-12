	;1.4.asm

	org 8400h

	mov r0, #10h  ;A
	mov r1, #22h  ;B
	mov r2, #40h  ;C

	;X = B + C
	mov a, r1
	add a, r2
	mov r3, a

	;X = X / 2
	mov a, r3
	mov b, #2h
	div ab
	mov r3, a

	;B = 2 * B / C 
	mov a, r1
	mov b, #2h
	mul ab
	mov b, r2
	div ab
	mov r1, a

	;ACC = X - B	
	mov a, r3
	subb a, r1
	
	;(AB) = ACC * A
	mov b, r0
	mul ab
	mov r1,b
	mov r2,a
	ret

	

	
	
