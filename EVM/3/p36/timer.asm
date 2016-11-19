	; reload constants should be at 70h and 71h
	; ??? is it possible to do "mov SFR, mem" ???

timerinit:	
	anl TMOD,#11110000b
	orl TMOD,#00000001b ;16-bit counter
	
	mov TH0, #FEh
	mov TL0, #0Bh 	 ; 500 mcs

	setb ea
	setb et0
	setb tr0

	mov	A9h, #00010000b
	mov	B9h, #00010000b
	setb	es
	ret

	
 timer_ir:	
	mov TH0, #FEh	; actual irq processing
	mov TL0, #0Bh	; set 500 mcs
	inc timertics	; tic per 500 mcs
	reti
	
	org 800bh		; interrupt processor
	ljmp timer_ir

