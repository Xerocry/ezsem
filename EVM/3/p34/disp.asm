; вариант 6
; P4 - 8-битная линия данных-команд  
; P1.5 - RS (0 - команды, 1 - данные) -> P5.4
; .6 - RW (0 - запись, 1-чтение) -> P5.6
; 7 - E (строб 0 для записи) -> P5.7
; !!! проверить адреса битов, получаемые для P5
	
lcd:	
	P4: equ E8h
	P5: equ F8h
indic:
	;clr P5.4
	clr P5.0
	mov r4,#38h
	lcall ind_wr
	mov r4,#0Ch
	lcall ind_wr
	mov r4,#80h
	lcall ind_wr

str1_set:	mov dptr,#strdata
	;setb P5.4
	setb P5.0

wr_str1: 	movx a,@dptr 
	mov r4,a
	lcall ind_wr
	inc dptr
	mov a,dpl
	cjne a,#14h,wr_str1 ; check value precisely

str2_set:	;clr P5.4
	clr P5.0
	mov r4,#C0h
	lcall ind_wr
	;setb P5.4
	setb P5.0

wr_str2: 	movx a,@dptr
	mov r4,a
	lcall ind_wr
	inc dptr
	mov a,dpl
	cjne a,#026h,wr_str2  ; CHECK IT !!!

	ret

ind_wr: 	mov P4, r4
	setb P5.3
	;clr P5.6
	clr P5.2
	lcall delay
	clr P5.3
	lcall delay
	setb P5.3
	ret

delay:	mov r3, #7
m1:	djnz r3, m1
	ret ; 
