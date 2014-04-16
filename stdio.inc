;
; 	This file contains routines for text i/o 
;	(c) Dmytro Sirenko, 2010
; ____________________________________________________________________

%ifndef __STDIO_INC_
%define __STDIO_INC_

%include "misc.inc"
%include "msgs.inc"

[section .text]

; ___________________________________________________________________
press_any:
; show "Press any key" message and waits for it
	mov si, m_press_any
	call print_string
	call wait_key
	ret

; ___________________________________________________________________
print_char:
print_ascii_al:
	mov ah, 0x0E
	mov bh, 0x00
	int 0x10
	retn

; ___________________________________________________________________
print_ascii_dl:
	mov al, dl
	mov ah, 0x0E
	mov bh, 0x00
	int 0x10
	retn

; ___________________________________________________________________
print_endl:
	mov bh, 0x00
	mov ax, 0x0E0D
	int 0x10
	mov al, 0x0A
	int 0x10
	retn

substitute_keys:
    cmp ah, 0x1A
    jne .notlqb
    mov al, '('
.notlqb:
    cmp ah, 0x1B
    jne .notrqb
    mov al, ')'
.notrqb:
    ret

; ___________________________________________________________________
wait_key:
read_char:
; waits for a key press, returns it
; args: none
; out:  al - ASCII-code, 0, or scan-code prefix
;;;	mov ah, 10h			; 101/102 keys
;;;	int 16h	; - this stinking interrupt loads 95% CPU!
	;int 0x4A
.loop:
	mov ax, 20
	call delay
	mov ah, 11h
	int 16h
	jz .loop
	;; clear keyboard buffer
	push ds
	push 0
	pop ds
	mov dx, word [041Ah]
	mov word [041Ch], dx	; clear buffer
	;; process some specific shortcuts
    call substitute_keys
	;call shortcuts
	pop ds
	retn

; ___________________________________________________________________
print_string:
print_ds_si:
; displays string in ds:si
; args: si - offset of a string to display
	pusha
	mov ah, 0x0E
	movzx bx, byte [text_attr]
.iter:
	mov al, [si]
	and al, al
	jz .done
	int 0x10
	lodsb
	jmp .iter
.done:	popa
	retn

print_ds_dx:
	pusha
	mov si, dx
	mov ah, 0x0E
	movzx bx, byte [text_attr]
.iter:
	mov al, [si]
	and al, al
	jz .done
	cmp al, '$'
	je .done
	int 0x10
	lodsb
	jmp .iter
.done:	popa
	retn

; ___________________________________________________________________
read_string_dos:
;; implements DOS 0Ah function behavior;
	retn
	
; ___________________________________________________________________
read_string:
;; args:
;;	ds:dx - address of a buffer up to 255 symbols, first symbol is
;;		maximum count of symbols
	pusha
	mov cx, 0
	mov di, dx
	mov si, dx	; store it

.type_loop:
	call read_char
	;; is valid ascii
	cmp al, 0x20
	jl .char_skipped
	test al, 0x80
	jnz .char_skipped
	stosb
	; echo
	mov ah, 0x0E
	mov bh, 0x00
	int 0x10
	jmp .type_loop

.char_skipped:
.bksp:	cmp al, 0x08
	jne .enter
	; remove last
	mov word [di], 0
	; is the first?
	cmp di, si
	jle .type_loop
	dec di
	; clear symbols
	mov ax, 0x0E08
	xor bh, bh
	int 0x10
	mov al, ' '
	int 0x10
	mov al, 0x08
	int 0x10
	jmp .type_loop	
.escape:
	; 
	jmp .type_loop
.enter:
	cmp al, 0x0D
	jne .type_loop
	
	; push the end if line into si
	mov al, 0
	stosb
	; print end of line
	call print_endl
	popa		
	retn

; ___________________________________________________________________
print_hex_digit_al:
	and al, 0x0F
	movzx si, al
	add si, hextable
	mov al, byte [si]
	mov ah, 0x0E
	mov bh, 0x00
	int 0x10
	retn

; ___________________________________________________________________
print_hex_dl:
;; args:
;; 	dl - hexadecimal number to output
	pusha
    mov di, .arg
    call dl_to_hex8_di
    mov si, .arg
    call print_string
	popa
	retn
.arg:   db  'AB', 0x00

; ___________________________________________________________________
print_hex_dx:
;; args:
;; 	dx - hexadecimal number to output
	pusha 
	mov di, .arg
	call dx_to_hex16_di
    mov si, .arg
	call print_string
	popa
	retn
.arg:   db  'ABCD', 0x00

; ___________________________________________________________________
read_hex_dx:
;; args:
;;	es:di - address of asciiz string in memory
;; rets:
;;	dx - hexadecimal number in [es:di]
	
	retn

[section .data]
text_attr	db	0x07	;; default

%endif ;; __STDIO_INC_
