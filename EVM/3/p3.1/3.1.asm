	org 8100h
	mov  r5, #ECh  		; TH
	mov  r6, #77h  		; TL

	lcall   init		; Иниц. таймера и системы прерываний
			
loop: 					; рабочий цикл
	lcall memklav		
	mov  dptr, #8300h   ; Коды для TH TL
	mov  a, 34h			; Читаем номер нажатой клавиши
	clr  c				; Очищаем бит С
	rlc  a				; Сдвиг влево А (на место A[0] идёт 0 - бит С)
	push a				; Сохраняем А в стеке
	movc  a, @a+dptr	; В А идёт значение по адресу [А + DPTR]
	mov  r5, a			; Кладём А в ТН
	pop a				; Восстанавливаем А из стека
	inc  a				; Инкрементируем для перехода на TL
	movc  a, @a+dptr	; В А идёт значение по адресу [А + DPTR]
	mov  r6, a			; Кладём А в ТL
	sjmp  loop
	
init:
	anl  TMOD,#11110000b; Иниц. таймера для работы
	orl  TMOD,#00000001b; в режиме 16-битного счётчика
	mov  TH0, r5		; Иниц. счётчика Т/С0 для
	mov  TL0, r6		; Формирования тика 5 мс (по-умолчанию)
	setb  ea			; Разрешение всех прерываний
	setb  et0			; Разрешение  прерывания Т/С0
	setb  tr0			; Разрешение счёта
	ret
tim0: 
	mov  TH0, r5		; Иниц. счётчика Т/С0 для
	mov  TL0, r6		; Формирования тика 5 мс (по-умолчанию)
	cpl  P1.3			; Формирование меандра
	reti
	
	org  800Bh			; Обработчик прерывания Т/С0
	ljmp  tim0
	
	include ASMS\43501_3\bk\3\sklav.asm
	
	org  8300h 			; Массив кодов для TH TL
cod db EDh,0Dh, EDh,0Dh, F6h,9Eh, F9h,CFh, FBh,59h, FCH,3Fh, FCh,ECh, FDh,58h, FDh,A2h, FDh,EBh, FEh,1Fh, FEh,24h, FEh,54h, FEh,79h, FEh,92h, FEh,B3h	
;cod db ECh,77h, ECh,77h, F6h,3Bh, F9h,7Ch, FBh,1Dh, FCH,17h, FCh,BEh, FDh,34h, FDh,8Eh, FDh,D3h, FEh,0Bh, FEh,5Eh, FEh,7Eh, FEh,9Ah, FEh,B2h, FEh,C6h	
	