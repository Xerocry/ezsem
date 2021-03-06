	org 8300h
memklav:
	mov 20h, #0h ; 0 for clear C
	mov R1, #33h 	; ����� ������ ������ ������ ��� ���������
	mov R3, #3h	; �������(�� ������� � ��������)
	mov 35h, #0h 	; ������� ������� ������
	mov 37h, #0h 	; ��� �������
	mov 38h, #0h	; ����� ������
	mov 39h, #0h	; ����� �������
	lcall klav
	
	; ������� - �������� �� ���� (������ �� ������)
zero_chk:
	mov C, 0h		; clear C
	mov A, @R1 	; ������ ������ �� ������
	;mov 56h, R1
	subb A, #f0h	; �������� 0Fh - ���� ����� ����, �� ������ �� ������.
			; ����� �������, ��� ���� �����-������ �������.	
	jz skip_cntr	; A==0 - ���������� ������� �������
	inc 35h		; �� ���� - ��������� �������� �������
	mov A,@R1
	mov 37h,A		; ��������� ��� ������� �������.
	mov 38h,R3	; ��������� ����� ������ ������� �������

skip_cntr:	
	dec R1		; ���� ��������� ������� �� ������
			; ���� �� �������� ����� ������� ��� �������� -
	dec R3		; ����������� ����� ������
	mov C, 0h		; clear C
	cjne R1, #2Fh, zero_chk ; - ���������� ����
	; ����� �� ����� �������� ���������� �������
		

	mov A, 35h	; ������ � � ������� �������
	jz wr_0		; 0 ������� - ����� ����
	mov C, 0h		; clear C
	cjne A, #01h, wr_FF 	; ������ 1 ������� - ����� FF
	
	mov dptr, #cdMask 	; ������ ������� �����
	mov R3,#0h;	; �������� �������
	
find_column:
	inc R3;		; ������� ������ �������
	mov 39h,R3	; ��������� ����� �������
	mov A,R3;
	mov C, 0h		; clear C
	subb A,#5h
	jz wr_FF ; �.�. ������� ����� ������(��� ���������)
		; �� ��� ����������� ������ ������� � �������
		; ����� - ���� ������ ��������� ������, � ��� �� ������
	movx A, @dptr	; �������� ������� 
	inc dptr		; ����� inc ������ � �������
	mov C, 0h		; clear C
	cjne A, 37h, find_column ; ���� ����� �� ����� ����������,
			; ��������� �����
get_num:
	; ����� ������*4+����� �������
	mov A, 38h
	mov C, 0h		; clear C	
	rl A
	rl A			; ��� ������ ����� =*4
	;add A, 39h		; �������� �����
	add A, #5h
	subb A, 39h
	mov 34h, A	; ������ ����� 
	sjmp ext
wr_0:	mov 34h, #0h
	sjmp ext
wr_FF:	mov 34h, #FFh
	sjmp ext			

	; ������������ ���� ������ - ���������� ��� �������.
cdMask: db E0h, D0h, B0h, 70h
	
ext:	ret

p5:	equ f8h

klav:	mov r0, #30h	; �����������������������
	orl p5, #f0h	; ���������������������
	mov a, #7fh	; ������������������������

m1:	mov p1, a		
	mov r2, a		
	mov a, p5	; ��������������������������
	anl a,#f0h
	mov @r0, a	; �������������
	inc r0		; ��������������������������
	mov a, r2
	rr a		; �����������������
	cjne a, #f7h, m1; �������������
	ret