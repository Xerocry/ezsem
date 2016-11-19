	org 8400h
	
	timertics:	equ 70h
	dsec:	equ 71h
	sec:	equ 72h
	min:	equ 73h
	hour:	equ 74h

	strdata: 	equ 8700h

	lcall timerinit
	lcall init_clk
infloop:	
	lcall memklav		
	mov  a, 34h
	cjne a, #01h, cont
	ljmp infloop
cont:	lcall upd_time
	lcall set_clk
	lcall lcd
	ljmp infloop
	ret

init_clk:
	mov dsec, #00h
	mov sec, #00h
	mov min, #00h
	mov hour, #00h
	ret

upd_time:	
	mov a, timertics
	clr c
	subb a, #200 	; 500 mcs * 200
	jc stop_upd
	mov timertics, a	; tics -= 200
	inc dsec		

	mov a, dsec
	cjne a, #10, stop_upd		; 1 second
	mov dsec, #00h
	inc sec

	mov a, sec
	cjne a, #60, stop_upd		; 1 minute
	mov sec, #00h
	inc min

	mov a, min
	cjne a, #60, stop_upd		; 1 hour
	mov min, #00h
	inc hour

	mov a, hour
	cjne a, #24, stop_upd		; next day
	mov hour, #00h

stop_upd:	ret


set_clk:	
	mov dptr, #strdata	;  
	mov dpl, #20	; points to line2 0 symbol
	mov r5, hour
	lcall hex2dec	; two digits in r6 and r7
	mov a, r6
	lcall put_dig	; save high hour digit
	inc dpl
	mov a, r7
	lcall put_dig	; save low hour digit
	
	inc dpl
	inc dpl		; points to 3 symbol
	mov r5, min
	lcall hex2dec	; minute digits in r6 and r7
	mov a, r6
	lcall put_dig	; save high min digit
	inc dpl
	mov a, r7
	lcall put_dig	; save low min digit
	
	inc dpl
	inc dpl		; points to 6 symbol
	mov r5, sec
	lcall hex2dec	; sec digits in r6 and r7
	mov a, r6
	lcall put_dig	; save high sec digit
	inc dpl
	mov a, r7
	lcall put_dig	; save low sec digit

	inc dpl
	inc dpl		; points to 9 symbol
	mov r5, dsec
	lcall hex2dec	; sec digits in r6 and r7
	mov a, r7		; set ONLY LOW byte
	lcall put_dig	; save high sec digit
	ret

hex2dec:	; r5 - hex num, r6 - high dec byte, r7 - low dec byte
	mov a, r5
	mov b, #10	
	div ab	; int in a, rem in b
	mov r6, a
	mov r7, b

	ret

put_dig:	; puts decimal digit into @dptr from A
	add a, #30h	; 0..9 ASCII are 30h..39h
	movx @dptr, a
	mov b, a
	mov a, dpl
	add a, #40h
	mov r1, a
	mov @r1, b	
	ret



	include C:\SHELL51\ASMS\43501_3\bk\3\p34\timer.asm
	include C:\SHELL51\ASMS\43501_3\bk\3\p34\disp.asm
	include C:\SHELL51\ASMS\43501_3\bk\3\sklav.asm


	org strdata
line1:	db '_______TIMER:_______'
line2:	db 'hh:mm:ss:d__________' ;