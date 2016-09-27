	;1.7.asm
	
	org 8400h

	mov dptr,#8500h
	mov a, #10h
	movx @dptr, a

	inc dptr
	mov a, #1h
	movx @dptr, a

	inc dptr
	mov a, #fh
	movx @dptr, a

	inc dptr
	mov a, #3h
	movx @dptr, a

	inc dptr
	mov a, #11h
	movx @dptr, a

	mov dptr, #8500h
	mov r1, #ffh ;min
	mov r2, #0h ;max
	mov r3, #0h ;sum
	mov r4, #0h ;cntr

m0:	movx a, @dptr
	mov b, r1

	cjne a, f0h , m1
	jmp minres
min:	mov r1, a
	jmp minres
m1:	jc min

minres:	mov b, r2

	cjne a, f0h , m2
	jmp maxres
max:	mov r2, a
	jmp maxres
m2:	jnc max

maxres:	add a, r3
	mov r3, a
	inc r4
	inc dptr
	cjne r4, #5h,m0
	mov a, r3
	mov b, #5h
	div ab
	mov r3, a
	ret