	org 8400h
P4: 	equ E8h
P5:	equ F8h

indic:	clr P5.0
	mov r4,#38h		; Установка 8-битного режима обмена с ЖКИ с выводом обеих строк
	lcall ind_wr
	mov r4,#0Ch		; Отображение экрана без курсоров
	lcall ind_wr

	mov r4,#80h		; Загрузка в счетчик АС адреса нулевой ячейки 1-й строки памяти ЖКИ
	lcall ind_wr		; Подготовка к вводу данных (R/S=1)
	
	mov dptr, #str1+20
	mov E4h, dpl
	mov dptr, #str1+40
	mov F8h, dpl	

	mov dptr, #FFD0h		;Адрес внешней памяти, где хранится выводимые строки
	setb P5.0	

wr_str1:	movx a,@dptr		
	mov r4,a
	lcall ind_wr	; Запись данных в ЖКИ
	inc dptr
	mov a,dpl		
	cjne a,#E4h,wr_str1	; проверка окончания 1-й строки
	
	clr P5.0
	mov r4,#C0h	; Команда для адресации 1-й ячейки второй строки
	lcall ind_wr	
	setb P5.0

wr_str2:	movx a,@dptr
	mov r4,a
	lcall ind_wr
	inc dptr
	mov a,dpl
	cjne a,#0F8h,wr_str2
	ret

ind_wr:	mov P4, r4		; Загрузка в порт P5 записываемой в ЖКИ информации
	setb P5.3			; Установка сигнала E
	clr P5.2			; R/W=0 (запись)
	lcall delay
	clr P5.3			; Сброс сигнала E
	lcall delay
	setb P5.3			; Установка сигнала E
	ret

delay:	mov r3, #7
m1:	djnz r3, m1	
	ret
	
	org FFD0h

str1:	db 'Microcontrollers2016'
str2:	db 20h, 20h, 20h 20h, A0h, 4Fh, B1h, 50h, 4Bh, A5h, 48h, 20h, 20h, 4Bh, 41h, 48h, 20h, 20h, 20h, 20h