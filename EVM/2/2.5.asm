	org 8500h
	ljmp start_main
start_main:
	lcall U1_read
convert1:
	mov 35h, U1 ; преобразуем значение из двоичной СС в десятичную СС
	mov a, 35h ; для этого делим на 10 два раза
	mov b, #10
	div ab
	mov 36h, b ; выделяя остаток от деления
	mov b, #10
	div ab
	mov 37h, b
	mov 38h, a
	mov r0, #36h
ascii1:
	mov a, @r0 ; преобразуем полученную цифру
	add a, #30h ; в HEX-код для вывода на экран
	mov @r0, a
	inc r0
	cjne r0, #39h, ascii1 ; делаем так до ячейки с адресом 39h
	mov r0, #36h
	mov dptr, #str1+19
show1: ; осуществляем вывод полученного десятичного числа
	mov a, @r0
	movx @dptr, a
	dec dpl
	inc r0
	cjne r0, #39h, show1 ;преобразуем значение из двоичной СС в десятичную СС
convert2:
	mov 39h, U2
	mov a, 39h
	mov b, #10
	div ab ; для этого делим на 10 два раза
	mov 3Ah, b ; выделяя остаток от деления
	mov b, #10
	div ab
	mov 3Bh, b
	mov 3Ch, a
	mov r0, #3Ah
ascii2:
	mov a, @r0
	add a, #30h ; преобразуем полученную цифру
	mov @r0, a ; в HEX-код для вывода на экран
	inc r0
	cjne r0, #3Dh, ascii2 ; делаем так до ячейки с адресом 3Dh
	mov r0, #3Ah
	mov dptr, #str2+19
show2:
	mov a, @r0 ; осуществляем вывод полученного десятичного числа
	movx @dptr, a
	dec dpl
	inc r0
	cjne r0, #3Dh, show2
	lcall indic
	inc r0
	ret

	include ASMS\43501_3\bk\2\myadc.asm; считываем значения с АЦП
	include ASMS\43501_3\bk\2\indic.asm; подключаем программу индикации

	;str1: db "potenciometr"
	;str2: db "integrator"