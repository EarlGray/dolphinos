;
;	Some debug routines
;	(c) Dmytro Sirenko
; ___________________________________________________________________

%ifndef __DEBUG_INC__
%define __DEBUG_INC__

%include "pseudogr.inc"

;
dbg_print_char_dl:
; args: dl - ASCII
; out: none
; preserves all fags
	pusha
	pushf
	mov ah, 0x0E
	mov bx, 0x0007
	mov al, dl
	int 10h
	popf 
	popa
	retn

;
dbg_print_hex_dl:
	pusha
	pushf
	mov ah, 0x0E
	mov bx, 0x0007
	mov al, dl
	shr al, 4
	cmp al, 10
	jl .dec
	add al, 'A' - '9' - 1
.dec:	add al, '0'
	int 10h
	mov al, dl
	and al, 0x0F
	cmp al, 10
	jl .dec2
	add al, 'A' - '9' - 1
.dec2:	add al, '0'
	int 10h
	mov al, ' '
	call out_char
	
	popf
	popa
	retn
; 
dbg_print_word_dx:
; args:
;	dx - word
; out:	none
; preserves all flags
	pusha
	pushf

	mov ah, 0x0E
	mov bx, 0x0007
	
	mov cx, 0x0004
.loop:
	mov al, dh
	shr al, 4	; al = digit
	; convert to ascii
	cmp al, 10
	jl .dec
	add al, 'A' - '9' - 1
.dec:	add al, '0'
	int 10h
	
	shl dx, 4
	loop .loop
	mov al, ' '
	int 10h
	
	popf
	popa
	retn

;
dbg_print_string:
	pusha
	pushf
	mov ah, 0x0E
	mov bx, 0x0007
.iter:
	mov al, [si]
	and al, al
	jz .done
	int 0x10
	lodsb
	jmp .iter
.done:	
	popf
	popa
	retn

;
dbg_dump_mem:
; hex output of memory at [es:di] to [es:di+cx]		
; 
	pusha
	pushf
	xor bx, bx
	mov al, 1
.loop: 	dec al
	jnz .next
	mov al, 0x10
	mov dl, 13
	call dbg_print_char_dl
	mov dl, 10
	call dbg_print_char_dl
.next:	mov dl, byte [es : di+bx]
	call dbg_print_hex_dl
	inc bx
	loop .loop

	popf
	popa
	retn
.endl	db	13, 10, 0

;; __________________________________________________________________
dbg_hex_input_dx:
;; args: none
;; rets: dx - word
	pusha
	xor dx, dx
	xor cl, cl
.loop:	
	push dx
	call get_char
	pop dx
	
	cmp cl, 4
	jge .no_digit

	cmp al, '0'
	jl .is_A_F
	cmp al, '9'
	jg .is_A_F
	call out_char
	sub al, '0'
	jmp .digit
.is_A_F:
	cmp al, 'A'
	jl .is_a_f
	cmp al, 'F'
	jg .is_a_f
	call out_char
	sub al, 'A' - 10
        jmp .digit
.is_a_f:
	cmp al, 'a'
	jl .no_digit
	cmp al, 'f'
	jg .no_digit
	call out_char
	sub al, 'a' - 10
	jmp .digit
.no_digit:
	cmp al, 13	;; enter?
	je .break
	cmp al, 8	;; bksp?
	jne .loop
	test cl, cl
	jle .loop
	dec cl
	;; clear last symbol
	mov si, clsymb
	call display_string
	;; dlete it from dx
	shr dx, 4
	jmp .loop
.digit:
	shl dx, 4
	inc cl
	add dl, al
	jmp .loop
.break:
	mov si, endl
	call display_string
	mov word[.res], dx
	popa
	mov dx, word[.res]
	retn
.res	dw	0x0000


%macro SHOW_SEGREG 1
	mov si, dbg_seg_regs.s_%1
	call display_string
	mov al, '='
	call out_char
	
	mov dx, %1
	call dbg_print_word_dx
	mov al, ' '
	call out_char
%endmacro
;; __________________________________________________________________
dbg_window_cpu:						
;;;						       

REG_STR_LENGTH	equ	3

	pusha
		
	;; adrress where this function called from
	mov si, .caller
	call display_string
	mov si, sp
	add si, 16
	mov dx, [ss : si]
	call dbg_print_word_dx
	mov si, endl
	call display_string

	;; general registers
	mov cx, 8
	mov di, dbg_regs
.next_reg:
	mov ax, cx
	dec ax
	mov bl, 0x03
	mul bl
	mov si, di
	add si, ax
	call display_string
	
	mov al, '='
	call out_char

	mov si, sp
	;inc si			;; cause of flags
	mov bx, cx
	dec bx
	shl bx, 1
	add si, bx		;; offset
	mov dx, word [ss : si]
	call dbg_print_word_dx

	mov al, ' '
	call out_char
	loop .next_reg
	
	mov si, endl
	call display_string
	;; segment registers
	SHOW_SEGREG ds
	SHOW_SEGREG es
	SHOW_SEGREG fs
	SHOW_SEGREG gs
	SHOW_SEGREG ss
	SHOW_SEGREG cs
	
	;popf
	popa
	retn
.caller	db	"Called from ", 0
dbg_regs:
.s_di	db	"DI", 0
.s_si	db	"SI", 0
.s_bp	db	"BP", 0
.s_sp	db	"SP", 0

.s_bx	db	"BX", 0
.s_dx	db	"DX", 0
.s_cx	db	"CX", 0
.s_ax	db	"AX", 0

dbg_seg_regs:
.s_ds	db	"DS", 0
.s_es	db	"ES", 0
.s_fs	db	"FS", 0
.s_gs	db	"GS", 0

.s_cs	db	"CS", 0
.s_ss	db	"SS", 0
.s_ip	db	"IP", 0

.s_flags	db	"FLAGS", 0


%endif
