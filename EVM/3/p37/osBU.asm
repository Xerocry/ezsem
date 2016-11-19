	org 8100h

num_task: equ 20h
	mov 43h, #0h 
	lcall init
progs:
	ljmp prog1

; ������������� �������
init:
	anl TMOD, #00000000b
	orl TMOD, #00000001b
	mov TH0, #FEh
	mov TL0, #39h
		
	;����� ��������� �������� �������
	mov ms, #0h
	mov sd10, #0h
	mov sec, #0h
	mov min, #0h 
	mov hours, #0h 

	setb ea	;���������� ���� ����������
	setb et0	;���������� ���������� �� ������� 0
	setb tr0	;���������� ����� ������� 0

	; ������ ���������� num_task = 0 � ����� ������
	mov num_task ,#0h
	ret

tim0: 	
	mov TH0, #FEh	; 2KHz = 500mks
	mov TL0, #39h

	cpl p1.0

; ���������-���������	
dispatcher:	
	
	; ���������� SFR
	push dph
	push dpl
	push psw
	push b
	push a
	push 0
	push 1

	; ���������� ���������
	mov r0, sp	;���������� ����������� ����������
	inc r0
	lcall form_dptr
	mov r1, #0h
prpm1:	
	mov a, @r1
	movx @dptr, a
	inc r1
	inc dptr
	djnz r0, prpm1
	
	; ����������� ������ ��������� ������
		clr c
		mov a, num_task
		subb a, #2h
		jc inc_num_task
		mov num_task, #0h
		sjmp end_num_task
inc_num_task:	inc num_task
end_num_task:	
	
	; �������������� ���������
		lcall form_dptr
		mov r1, #0h
prpm2:	
	movx a, @dptr
	mov @r1, a
	inc r1
	inc dptr
	djnz r0, prpm2
	
	; �������������� SFR
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

	; ����������� DPTR
form_dptr:	mov dph, #E0h
	mov a, num_task
	mov b, #20h
	mul ab
	mov dpl, a
	ret	

; ������1 - ����������� ������ ������� �������
; � �������������� � ASCII ���
	org 9000h	
prog1:	
	cpl P1.1
	;lcall memklav
	;lcall decim
	sjmp prog1

; ������ 2 - ��������� ������ ������� �������	
	org 9200h
prog2:	
	cpl P1.2
	;lcall indic
	sjmp prog2

; ������ 3 - ����������� ����
	org 9400h
prog3:	
	cpl P1.3
	;lcall clock
	sjmp prog3

; ���������� ������1	
	org E000h
prog1_d:	db 11h, 1,  0, 0, 0, 0, 0, 0, 00, 90h, 0, 0, 0, 0, 0, 0, 0, 0

; ���������� ������2
	org E020h
prog2_d:	db 11h, 1,  0, 0, 0, 0, 0, 0, 00, 92h, 0, 0, 0, 0, 0, 0, 0 ,0

; ���������� ������3
	org E040h
prog3_d:	db 11h, 1,  0, 0, 0, 0, 0, 0, 00, 94h, 0, 0, 0, 0, 0, 0, 0, 0

	org 800bh
	ljmp tim0
	
	include ASMS\43501_3\bk\3\p37\clock.asm
	include ASMS\43501_3\bk\3\p37\iklav.asm
