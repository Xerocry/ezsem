	org 8100h

num_task: equ 20h
	mov 43h, #0h 
	lcall init
progs:
	ljmp prog1

; инициализаци€ таймера
init:
	anl TMOD, #00000000b
	orl TMOD, #00000001b
	mov TH0, #FEh
	mov TL0, #39h
		
	;сброс регистров значений таймера
	mov ms, #0h
	mov sd10, #0h
	mov sec, #0h
	mov min, #0h 
	mov hours, #0h 

	setb ea	;разрешение всех прерываний
	setb et0	;разрешение прерываний от таймера 0
	setb tr0	;разрешение счета таймера 0

	; задаем изначально num_task = 0 Ц номер задачи
	mov num_task ,#0h
	ret

tim0: 	
	mov TH0, #FEh	; 2KHz = 500mks
	mov TL0, #39h

	cpl p1.0

; программа-диспетчер	
dispatcher:	
	
	; сохранение SFR
	push dph
	push dpl
	push psw
	push b
	push a
	push 0
	push 1

	; сохранение контекста
	mov r0, sp	;количество сохран€емых параметров
	inc r0
	lcall form_dptr
	mov r1, #0h
prpm1:	
	mov a, @r1
	movx @dptr, a
	inc r1
	inc dptr
	djnz r0, prpm1
	
	; определение номера следующей задачи
		clr c
		mov a, num_task
		subb a, #2h
		jc inc_num_task
		mov num_task, #0h
		sjmp end_num_task
inc_num_task:	inc num_task
end_num_task:	
	
	; восстановление контекста
		lcall form_dptr
		mov r1, #0h
prpm2:	
	movx a, @dptr
	mov @r1, a
	inc r1
	inc dptr
	djnz r0, prpm2
	
	; восстановление SFR
	dec r1
	mov sp, r1
	pop 1
	pop 0
	pop a
	pop b
	pop psw
	pop dpl
	pop dph
	
	reti	

	; определение DPTR
form_dptr:	mov dph, #E0h
	mov a, num_task
	mov b, #20h
	mul ab
	mov dpl, a
	ret	

; «адача1 - определение номера нажатой клавиши
; и преобразование в ASCII код
	org 9000h	
prog1:	
	cpl P1.1
	;lcall memklav
	;lcall decim
	sjmp prog1

; «адача 2 - индикаци€ номера нажатой клавиши	
	org 9200h
prog2:	
	cpl P1.2
	;lcall indic
	sjmp prog2

; «адача 3 - электронные часы
	org 9400h
prog3:	
	cpl P1.3
	;lcall clock
	sjmp prog3

; дескриптор задачи1	
	org E000h
prog1_d:	db 11h, 1,  0, 0, 0, 0, 0, 0, 00, 90h, 0, 0, 0, 0, 0, 0, 0, 0

; дескриптор задачи2
	org E020h
prog2_d:	db 11h, 1,  0, 0, 0, 0, 0, 0, 00, 92h, 0, 0, 0, 0, 0, 0, 0 ,0

; дескриптор задачи3
	org E040h
prog3_d:	db 11h, 1,  0, 0, 0, 0, 0, 0, 00, 94h, 0, 0, 0, 0, 0, 0, 0, 0

	org 800bh
	ljmp tim0
	
	include ASMS\43501_3\bk\3\p37\clock.asm
	include ASMS\43501_3\bk\3\p37\iklav.asm
