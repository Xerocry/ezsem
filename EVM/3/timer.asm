	;timer.asm
	org 8100h
	lcall init

loop:	sjmp loop

init:	anl TMOD,#11110000b
	orl TMOD,#00000001b
	mov TH0,#ECh
	mov TL0,#77h
	setb ea
	setb et0
	setb tr0
	ret
tim0:	mov TH0,#ECh
	mov TL0,#77h
	cpl P1.3
	reti
	org 800bh
	ljmp tim0