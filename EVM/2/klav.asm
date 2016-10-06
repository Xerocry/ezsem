;str 125
	org 8100h	 
p5:	equ f8h

klav:	mov r0, #30h
	orl p5, #f0h
	mov a, #7fh

m1:	mov p1, a		
	mov r2, a		
	mov a, p5
	anl a,#f0h
	mov @r0, a
	inc r0	
	mov a, r2
	rr a		
	cjne a, #f7h, m1
	ret