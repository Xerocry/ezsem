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
