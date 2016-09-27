	org 8500h
res:	equ 34h

	lcall memklav
		
	mov dptr, #str2 + 18
	mov r0, #res	
		
d7:	mov a, @r0
	mov b, #10	
	div ab 
	add a, #30h
	movx @dptr, a
	inc dptr
	mov a, b 
	add a, #30h
	movx @dptr, a

	lcall indic 
	
	ret		

str1:	db 	'Microcontrollers2016'
str2:	db	'Number of button '

	include ASMS\43501_3\bk\2\ind.asm
	include ASMS\43501_3\bk\2\klv.asm