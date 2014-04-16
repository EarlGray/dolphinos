;
; 	This file contains routins for text i/o 
;	(c) Dmytro Sirenko, 2010
; ____________________________________________________________________

%ifndef __TEXTMODE_INC
%define __TEXTMODE_INC

%include "sysdefs.inc"
%include "string.inc"

[segment .text]

; ___________________________________________________________________
hex_to_ascii:
	cmp al, 10
	sbb al, 96h
	das
	ret

; ___________________________________________________________________
display_string:
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

; ___________________________________________________________________
set_cursor_pos:
; args: dh - line, dl - column
	xor bx, bx
	mov ah, 02h		; bios func num
	int 10h
	retn

; ___________________________________________________________________
get_cursor_pos:
; args: none
; out:  dh - line, dl - column
	xor bx, bx
	mov ah, 03h
	int 10h
	retn


; ___________________________________________________________________
wait_key:
get_char:
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
	push ds
	push 0
	pop ds
	mov dx, word [041Ah]
	mov word [041Ch], dx	; clear buffer
	cmp al, 03		; Ctrl-C handler
	jne .next0
	pop ds
	push shell.system_loop
	jmp .ret
.next0:
	cmp al, 0x0C
	jne .ret
	;; it should be an interrupt
;	jmp .ret
.ret:
	pop ds
	retn

out_char:
; echo a character in al
	pusha
	mov ah, 0x0E
	movzx bx, byte [text_attr]
	int 0x10
	popa
	retn

out_color_char:
; args: al - ascii, bl - color attrs
	pusha
	mov ah, 0x0E
	mov bh, 0x00
	int 10h
	popa
	retn

put_char:
;; args:
;;	ah - color attr, al - ascii, bh:bl - y:x
	pusha
	mov cx, ax
	movzx ax, bh
	movzx dx, byte [screen_width]
	mul dl
	movzx dx, bl
	add ax, dx
	shl ax, 1
	push es
	mov bx, TXTBUF_SEGM
	mov es, bx
	mov bx, ax
	mov word [es:bx], cx
	pop es
	popa
	retn

; ___________________________________________________________________
press_any:
; show "Press any key" message and waits for it
	mov si, pakmsg
	call display_string
	call wait_key
	ret
pakmsg	db 13, 10, "Press any key to continue...", 13, 10, 0

; ___________________________________________________________________
read_prompt:
; loop until enter is pressed, fills cmd_buf.
; args, outs: none
	mov cx, 0		; counter
	mov di, cmd_buf

.type_loop:			; while enter will be pressed
	call get_char
	; check whether a char is ascii
	cmp al, 20h		; gap ascii code
	jl	.char_skipped
	test al, 80h
	jnz	.char_skipped
	; all is right
	stosb			; add to cmd_buf
	call out_char	; echo
.char_skipped:
.bksp:	cmp al, 0x08
	jne .enter
	; remove last character
	mov word [di], 0
	; is the first?
	cmp di, cmd_buf
	jle .type_loop
	dec di
	; don't leave tracks
	mov al, 0x08
	call out_char
	mov al, ' '
	call out_char
	mov al, 0x08
	call out_char
	jmp .type_loop
.enter:
	cmp al, 0x0D	; enter ?
	jne .type_loop

	; push the end of line into cmd_buf
	mov al, 0
	stosb

	; print end of line
	mov al, 13
	call out_char
	mov al, 10
	call out_char
	retn

; ___________________________________________________________________
set_video_mode:
; sets video mode for text
; arguments: video mode in al (03h by default)
	test al, al
	jnz vm_ok
	mov al, 03h
vm_ok:
	mov ah, 00h
	int 10h
	retn

;; __________________________________________________________________
show_ascii_table:
	mov si, .asciimsg
	call display_string
	xor cx, cx
.asc_l:
	mov si, endl
	call display_string
	xor di, di
.asc_l1:
	mov al, cl
	shl al, 4
	add ax, di
	call out_char
	mov al, ' '
	call out_char
	inc di
	cmp di, 0x10
	jl .asc_l1
	inc cx
	cmp cx, 0x10
	jl .asc_l

	retn
.asciimsg	db 	"Ascii table:", 13, 10, 0

;; __________________________________________________________________
show_key_info:
;; 
	call get_char
	
	mov si, .str_symb
	call display_string
	mov dl, al
	call dbg_print_char_dl	

	mov si, .str_ascii
	call display_string
	call dbg_print_hex_dl

	test dl, dl
	jz .ext
	mov si, .str_scan
	jmp .end
.ext:	mov si, .str_ext
.end:	call display_string
	mov dl, ah
	call dbg_print_hex_dl

	mov si, endl
	call display_string
	retn
.str_symb	db	13, 10, "Symbol: ", 0
.str_ascii	db	13, 10, "Ascii : ", 0
.str_ext	db	13, 10, "ExtAsc: ", 0
.str_scan	db	13, 10, "Scan  : ", 0

;; __________________________________________________________________
;; _________________  DATA and defines  _____________________________
;; __________________________________________________________________
[segment .data]

endl	db	13, 10, 0
clsymb	db	8, ' ', 8, 0

text_attr	db	0x07
screen_width	db	80
screen_height	db	25

%endif
