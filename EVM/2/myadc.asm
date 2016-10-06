	org 8100h

ADCON:   	equ D8h
ADDAT:   	equ D9h
DAPR:	 equ Dah
U1: 	 equ 40h
U2: 	 equ 41h

U1_read: 	mov A,#08h ;выбор нулевого канала
	anl ADCON, #E0h ;инициализация АЦП
	orl ADCON, A
	;mov DAPR, #D4h ;задаем диапазон
	mov DAPR, #00h ;задаем диапазон
	jb D8h.4,$

	mov r7, #15 ;длина паузы
m1: 	djnz r7, m1 ;пауза
	mov U1, ADDAT
			
U2_read: 	mov A,#0fh ; выбор седьмого канала
	anl ADCON, #E0h ; инициализация АЦП
	orl ADCON, A
	;mov DAPR, #D4h ; задаём диапазон
	mov DAPR, #62h ;задаем диапазон
	jb D8h.4, $

	mov r7, #15 ; длина паузы
m2: 	djnz r7, m2 ; пауза
	mov U2, ADDAT

	lcall 128h ;!!!!!!!!!!!!!!
	ljmp U1_read ;!!!!!!!!!!!!!!

	ret