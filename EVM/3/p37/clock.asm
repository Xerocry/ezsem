	org 8300h

ms:	equ 3Fh	;тики
sd10:	equ 40h	;100 мс
sec:	equ 41h	;с
min:	equ 42h	;м
hours:	equ 43h	;ч

; Инициализация
initt:	
	;orl TMOD, #00010000b ;для работы в режиме 16-битного счётчика

	anl TMOD, #1Fh
	mov TH1, #FEh 	;инициализация счётчика T/C1 для
	mov TL1, #0Bh   ;формирования "тика" 5 мс
	
	; наивысший приоритет для T/C1
        mov A9h, #08h
        mov B9h, #00h

	;setb ea	;разрешение всех прерываний
	setb et1	;разрешение прерывания
	setb tr1	;разрешение счёта
	ret
clock:

tim1: 	mov TH1, #FEh	 ;2KHz = 500mks
	mov TL1, #17h    ; 17

	inc ms	  	 ;инкремент тиков
	mov r5, ms
	clr c
	mov a, r5
	subb a, #200
	jc end_tim	;если количество тиков равно 200
	mov ms, #0h
	lcall inc_dec_sec

end_tim:	reti

; Инкрементируем мс, сек и мин
inc_dec_sec:
	inc sd10	; инкремент 0,1 сек
	mov r5, sd10
	cjne r5, #Ah, end	; проверка сек == 0,1	
	inc sec		; инкремент секунд
	mov r5, sec
	mov sd10, #0h
	cjne r5, #3Ch, end	; проверка сек == 60
	inc min		; инкремент минуты
	mov r5, min
	mov sec, #0h	
	cjne r5, #3Ch, end	; проверка мин == 60
	inc hours		; инкремент часы
	mov min, #0h

end:	lcall to_int
	ret

	org 801bh
	ljmp tim1

	org 8500h
	
to_int:
	
; Для десятых долей секунд
	mov a, 40h
	mov dptr, #FFDDh
	lcall overal
	
; Для секунд	
	mov a, 41h
	mov dptr, #FFDAh
	lcall overal
	
	dec dpl
	lcall overal
	
; Для минут	
	mov a, 42h
	mov dptr, #FFD6h
	lcall overal
	
	dec dpl
	lcall overal

; Для часов
	mov a, 43h
	mov dptr, #FFD2h
	lcall overal
	
	dec dpl
	lcall overal
	ret
	
overal:	mov b, #10d	;основание системы счисления
	div ab
	mov r1, a	
	mov a, b
	add a, #30h	;ASCII символа
	movx @dptr, a	;символ
	mov a, r1
	ret

	