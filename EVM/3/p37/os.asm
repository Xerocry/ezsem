	org 8100h

num_task: equ 50h
	
	mov SP, #7h
	lcall init

; Задача1 - определение номера нажатой клавиши
; и преобразование в ASCII код	
prog1:	
	cpl P1.1
	lcall memklav
	lcall decim
	sjmp prog1

; Задача 2 - индикация номера нажатой клавиши
prog2:	
	cpl P1.2
	lcall indic
	sjmp prog2

; Задача 3 - электронные часы
prog3:	
	cpl P1.3
	lcall clock
	sjmp prog3

; дескриптор задачи1
prog1_d:	db 11h, 1,  0, 0, 0, 0, 0, 0, 00, 00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

; дескриптор задачи2
prog2_d:	db 11h, 1,  0, 0, 0, 0, 0, 0, 00, 00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

; дескриптор задачи3
prog3_d:	db 11h, 1,  0, 0, 0, 0, 0, 0, 00, 00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

; инициализация дескрипторов и таймера
init:
	mov num_task, #0
        	; обнуление декод. значения клавиши
        	mov 34h, #0

	mov dptr, #prog2
	mov r0, DPL
	mov r1, DPH
	mov dptr, #prog2_d
	lcall init_rout

	mov dptr, #prog3
	mov r0, DPL
	mov r1, DPH
	mov dptr, #prog3_d
	lcall init_rout

	;anl TMOD, #00000000b
	;orl TMOD, #00000001b
	mov TMOD, #11110001b
	mov TH0, #F6h ; fe
	mov TL0, #3Bh ; 39
		
	;сброс регистров значений таймера
	mov ms, #0h
	mov sd10, #0h
	mov sec, #0h
	mov min, #0h 
	mov hours, #0h 

	setb ea	;разрешение всех прерываний
	setb et0	;разрешение прерываний от таймера 0
	setb tr0	;разрешение счета таймера 0
	ret

init_rout:
	mov a, DPL
	add a, #8
	mov DPL, a

	mov a, r0
	movx @dptr, a
	inc dptr
	mov a, r1
	movx @dptr, a
	ret

tim0: 	
	mov TH0, #F6h	; 2,5 ms F6 47
	mov TL0, #47h

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

	lcall def_context_addr

	; сохранение контекста
	mov r0, sp	;количество сохраняемых параметров
	inc r0
	mov r1, #0h
prpm1:	
	mov a, @r1
	movx @dptr, a
	inc r1
	inc dptr
	djnz r0, prpm1

	lcall def_ntx_tsk
	lcall def_context_addr
	
	movx a, @dptr
	mov b, a
	mov r1, #0h
prpm2:	
	movx a, @dptr
	mov @r1, a
	inc r1
	inc dptr
	djnz r0, prpm2
	
	; восстановление SFR
	dec b
	mov SP, b
	pop 1
	pop 0
	pop a
	pop b
	pop PSW
	pop DPL
	pop DPH
	
	reti	

	; определение DPTR
def_ntx_tsk:
	inc num_task
	mov a, num_task
	cjne a, #3h, exit_def_ntf
	mov num_task, #0h
exit_def_ntf:
	ret	

def_context_addr:
	mov a, num_task
	rr a
	rr a
	rr a
	mov r0, a
	mov dptr, #prog1_d
	mov a, DPL
	add a, r0
	mov DPL, a
	jnc exit_def
	inc DPH
exit_def:
	ret

	org 800bh
	ljmp tim0
	
	include ASMS\43501_3\bk\3\p37\clock.asm
	include ASMS\43501_3\bk\3\p37\iklav.asm
