    оrg 8500h
	
	mov dptr, #8400h
	mov r0, #0h
	mov r1, #10h
	
m1:	mov a, r0
	movx @dptr, a
	inc r0
	inc dptr
	dec r1
	mov a, r1
	jnz m1
	
	mov r1, #11h
	
m2:	mov a, r0
	movx @dptr, a
	dec r0
	inc dptr
	dec r1
	mov a, r1
	jnz m2
	
	ret
