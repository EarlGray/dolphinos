;
;	Miscel
;	DolphinOS	(c) Dmytro sirenko, 2010
; ___________________________________________________________________

%ifndef __MISC_STUFF_INC__
%define __MISC_STUFF_INC__

%include "debug.inc"

[section .data]
hextable:   db  '0123456789ABCDEF'


[section .text]

%macro AL_HEX_TO_ASCII 0
	cmp 	al, 10
	sbb 	al, 0x96
	das
%endmacro

; ___________________________________________________________________
ascii_to_hex_digit_dl:
	cmp dl, '0'
	jl .fail
	cmp dl, '9'
	jg .next
	sub dl, '0'
	jmp .ret
.next:
	cmp dl, 'A'
	jl .fail
	cmp dl, 'F'
	jg .next1
	sub dl, 'A' - 10
	jmp .ret
.next1:	cmp dl, 'a'
	jl .fail
	cmp dl, 'f'
	jg .fail
	sub dl, 'a' - 10
.ret:
    clc
	retn
.fail:
    stc
	retn

;; __________________________________________________________________
dl_to_hex8_di:
;; write hex of dx to [es:di] moving di
;; in:      dl, di
;; side:    al
;; out:     di
    push bx
    xor bx, bx

    mov bl, dl
    shr bl, 4
    mov al, byte [hextable + bx]
    stosb

    mov bl, dl
    and bl, 0x0F
    mov al, byte [hextable + bx]
    stosb
    pop bx
    retn

;; __________________________________________________________________
dx_to_hex16_di:
;; write hexadecimal representation of dx to [es:di] and move di over 
;;  the 'memory stream'.
;; in:      dx - hex value
;;          es:di - where to write
;; side:    ax
;; out:     di 
    xchg dh, dl
    call dl_to_hex8_di
    xchg dh, dl
    call dl_to_hex8_di
    retn

;; __________________________________________________________________
hex16_si_to_dx:
;; reads stream at [ds:si] until incorrect symbol encountered. Last
;;  processed symbols are used, e.g. 123456D78ABq -> dx = 0x78AB, 42
;;  -> dx = 0x42
;; in:  si
;; out: dx
;; corrupts:    none
    push cx
    xor cx, cx      ;; accumulator
    clc
.next:
    lodsb
    mov dl, al
    call ascii_to_hex_digit_dl
    jc .end
    shl cx, 4
    add cl, dl
    jmp .next
.end:
    mov dx, cx
    pop cx
    retn

%endif
