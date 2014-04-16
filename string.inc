;
;	string routines
;	DolphinOS (c) Dmytro Sirenko
; ___________________________________________________________________

%ifndef __STRING_INC
%define __STRING_INC

DEF_DELIMITER	equ	' '

section .text

; ___________________________________________________________________
si_to_hex_dx:
;; reads stream at [si] until hex word will be read
;; default delimiter is ' '
;; args:	si - stream beginning
;; outs:	dx - read word
;; notes: HHHH, HHH, HH, HHHHh options are acceptable
;; 	  does not preserve registers! si is moved to last read position.
	;; read delimiters before
	mov al, DEF_DELIMITER
.next_delim:
	cmp al, byte [si]	;; scasb moves automatically, don't use it
	jne .end_delim
	inc si
	jmp .next_delim
.end_delim:	
	;; leading zeroes
	mov al, '0'
.next_zero:
	cmp al, byte [si]
	jne .end_zero
	inc si
	jmp .next_zero
.end_zero:
	;; number must be here
	mov cx, 4
.next_digit:
		
	loop .next_digit
.error:
.end_number:
	retn



; ___________________________________________________________________
hex_digit_to_ascii:
;; args: al - byte 0..F
;; outs: al - '0'..'9', 'A'..'F'
	push bx
	mov bl, al
	sub al, 10
	jl .dec
.hex:	add al, 'A'
	jmp .ret
.dec:	add bl, '0'
	mov al, bl
.ret:	pop bx
	retn


%endif
