;
;
;
; ___________________________________________________________________

%ifndef __TIME_INC_
%define __TIME_INC_

[section .text]

; ___________________________________________________________________
delay:
; void delay(ax ms) 
	pusha
	mov cx, 1000
	mul cx
	jc .long
	xor cx, cx
	jmp .endif	
.long:	mov cx, dx
.endif:	mov dx, ax
	mov ah, 86h
	int 15h
	popa
	retn


%endif
