	org 8500h
to_int:
	
; ��� ������� ����� ������
	mov a, 40h
	mov dptr, #FFDDh
	lcall overal
	
; ��� ������	
	mov a, 41h
	mov dptr, #FFDAh
	lcall overal
	
	dec dpl
	lcall overal
	
; ��� �����	
	mov a, 42h
	mov dptr, #FFD6h
	lcall overal
	
	dec dpl
	lcall overal

; ��� �����
	mov a, 43h
	mov dptr, #FFD2h
	lcall overal
	
	dec dpl
	lcall overal
	ret
	
overal:	mov b, #10d	;��������� ������� ���������
	div ab
	mov r1, a	
	mov a, b
	add a, #30h	;ASCII �������
	movx @dptr, a	;������
	mov a, r1
	ret
