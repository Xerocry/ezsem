	org 8700h

P4: 	equ E8h
P5:	equ F8h

indic:	clr P5.0		; Подготовка к вводу КОМАНД: RS = 0
	mov r4, #38h	; 8-битовый режим обмена с выводом обеих строк
	lcall ind_wr	; Запись команды в ЖКИ
	mov r4, #0Ch	; Активизация всех знакомест дисплея без курсора
	lcall ind_wr
	mov r4, #80h	; Адрес нулевой ячейки 1-ой строки
	lcall ind_wr
			 
	mov dptr, #FFD0h
	setb P5.0 	; Подготовка к вводу ДАННЫХ: RS = 1
	
; Выводим 1-ую строку
wr_str1:	movx a, @dptr ; Читаем символ из внешней памяти
	mov r4, a
	lcall ind_wr  ; Запись данных в ЖКИ
	inc dptr	  ; Формируем сл. адрес видеобуфера
	mov a, dpl	; Мл. часть dptr
	cjne a, #E4h, wr_str1; Проверка окончания вывода символов 1 строки
			
	clr P5.0
	mov r4, #C0h
	lcall ind_wr
	setb P5.0
			
; Выводим 2-ую строку
wr_str2:	movx a, @dptr ; Читаем символ из внешней памяти
	mov r4, a	  ; Запись данных в ЖКИ
	lcall ind_wr  ; Формируем сл. адрес видеобуфера
	inc dptr 
	mov a, dpl 
	cjne a, #0F8h, wr_str2; Проверка окончания вывода символов 2 строки
	ret
			
ind_wr: 	mov P4, r4 ; Грузим в порт Р4 передаваемую посылку
	setb p5.3	; Установка сигнала Е
	clr p5.2	; Сигнал R/W=0 (запись)
	lcall delay
	clr p5.3	; Сброс сигнала Е
	lcall delay
	setb p5.3	
	ret

delay:	mov r3, #7
m2:	djnz r3, m2
	ret

memklav:
	mov 20h, #0h ; 0 for clear C
	mov R1, #33h 	; Адрес первой ячейки памяти для просмотра
	mov R3, #3h	; счетчик(по строкам и столбцам)
	mov 35h, #0h 	; Счётчик нажатых клавиш
	mov 37h, #0h 	; Код символа
	mov 38h, #0h	; номер строки
	mov 39h, #0h	; номер столбца
	lcall klav
	
	; Сначала - проверка на ноль (ничего не нажато)
zero_chk:
	mov C, 0h		; clear C
	mov A, @R1 	; Читаем данные из памяти
	;mov 56h, R1
	subb A, #f0h	; Отнимаем 0Fh - если будет ноль, то ничего не нажато.
			; Иначе считаем, что было какое-нибудь нажатие.	
	jz skip_cntr	; A==0 - пропускаем счётчик нажатий
	inc 35h		; Не ноль - инкремент счётчика нажатий
	mov A,@R1
	mov 37h,A		; Сохраняем код нажатой клавиши.
	mov 38h,R3	; Сохранили номер строки нажатой клавиши

skip_cntr:	
	dec R1		; Берём следующий элемент из памяти
			; Пока не достигли конца массива для проверки -
	dec R3		; увеличиваем номер строки
	mov C, 0h		; clear C
	cjne R1, #2Fh, zero_chk ; - продолжаем цикл
	; Вышли из цикла проверки отсутствия нажатий
		

	mov A, 35h	; Грузим в А счётчик нажатий
	jz wr_0		; 0 нажатий - пишем ноль
	mov C, 0h		; clear C
	cjne A, #01h, wr_FF 	; больше 1 нажатия - пишем FF
	
	mov dptr, #cdMask 	; начало массива кодов
	mov R3,#0h;	; обнулили счетчик
	
find_column:
	inc R3;		; счетчик номера столбца
	mov 39h,R3	; сохраняем номер столбца
	mov A,R3;
	mov C, 0h		; clear C
	subb A,#5h
	jz wr_FF ; Т.к. клавишу точно нажали(или несколько)
		; ее код обазятельно должен найтись в массиве
		; иначе - было нажато несколько клавиш, и код не совпал
	movx A, @dptr	; записали элемент 
	inc dptr		; сразу inc индекс в массиве
	mov C, 0h		; clear C
	cjne A, 37h, find_column ; если число не равно найденному,
			; продолжим поиск
get_num:
	; номер строки*4+номер столбца
	mov A, 38h
	mov C, 0h		; clear C	
	rl A
	rl A			; два сдвига числа =*4
	;add A, 39h		; получили число
	add A, #5h
	subb A, 39h
	mov 34h, A	; запись числа 
	sjmp ext
wr_0:	mov 34h, #0h
	sjmp ext
wr_FF:	mov 34h, #FFh
	sjmp ext			

	; Существующие коды клавиш - характерны для столбца.
cdMask: db E0h, D0h, B0h, 70h
	
ext:	ret

p5:	equ f8h

klav:	mov r0, #30h	; задаем адрес карты памяти
	orl p5, #f0h	; настраиваем порт на ввод
	mov a, #7fh	; загружаем код бегущего нуля

mb:	mov r2, a

	rlc a
	mov p1.7, c
	rlc a
	mov p1.6, c
	rlc a
	mov p1.5, c
	rlc a
	mov p1.4, c
	
	mov a, p5	; считываем данные с клавиатуры
	anl a,#f0h
	mov @r0, a	; и запоминаем их
	inc r0		; увеличиваем адрес для записи
	mov a, r2
	rr a		; осуществляем сдвиг
	cjne a, #f7h, mb; выполняем цикл
	ret

;Приведение полученной цифры к десятичному формату
decim:	mov a,34h
	cjne a,#ffh, wrff
	mov a,#46h
	mov dptr, #str2 + 17
	movx @dptr,a
	inc dptr
	movx @dptr,a
	ret

wrff:	mov dptr, #str2 + 17
	mov a, 34h
	mov b, #10	
	div ab 
	add a, #30h
	movx @dptr, a
	inc dptr
	mov a, b 
	add a, #30h
	movx @dptr, a
	ret

; видеобуффер
		org FFD0h
str1:	db	20h, 20h, 20h, 3Ah, 20h, 20h, 20h, 3Ah, 20h, 20h, 20h, 2eh, 20h, 20h, 30h, 30h, 20h, 20h, 20h, 20h	
str2:	db	'BUTTON NUMBER:      '