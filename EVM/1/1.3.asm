	org 8400h 
	
	mov a, #0A0h
	mov r0, #50h
m1:	mov @r0, #a

	dec a
	inc r0
	cjne r0, #60h, m1
	ret	
