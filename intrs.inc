;
;	Interrupts setter and handler for Dolphin OS kernel
;	(c) Dmytro Sirenko
; ___________________________________________________________________

%ifndef __INTR_INC
%define __INTR_INC

%macro SET_INT 1
	mov word [es : %1 * 4], int%1
	mov word [es : %1 * 4 + 2], KERNEL_SEGM
%endmacro
	

[segment .text]

INTRTBL_SEGM	equ 	0x0000

; ___________________________________________________________________
; intr_init
; initializes OS interrupts table 

intr_init:
	cli
	push es
	mov ax, INTRTBL_SEGM
	mov es, ax

	;SET_INT 08h
	;; for Ctrl-Break handling
	SET_INT 1Bh
	;; exit from programs
	SET_INT 20h
	;; every second alarm
	SET_INT 4Ah
	;; seconds start
	int 4Ah

	pop es
	sti
	retn

; ___________________________________________________________________
; int 1Bh
; handler of Ctrl-Break
int1Bh:
	;; clear stack
	popf
	pop ax
	pop ax
	;; go to system loop
	push shell.system_loop
	retn

; ___________________________________________________________________
; int 20h handler
; return to shell

int20h:
	mov si, int20msg
	call display_string
	
	popf				; restore flags
	pop dx				; don't return to program
	pop bx

	call dbg_print_word_dx
	mov dx, bx
	call dbg_print_word_dx
	
	push shell.system_loop		; to system shell loop
	retn
int20msg	db 	"Int 20h executing...",13,10,0


; ___________________________________________________________________
;; called every 1/55 sec
int08h:
	pusha
	push es
	mov bx, KERNEL_SEGM
	mov es, bx
	mov ah, byte [es:show_clock]
	pop es
	test ah, ah
	jz .ret

	;; clock
	call get_rtc_time
	;; show time
	push cx
	push dx
	push es
	mov bx, TXTBUF_SEGM		;; video memory
	mov es, bx
	;; tiny hack: cl needed for color, so cl will be out first
	;; minutes
	mov dl, cl
	mov al, cl
	shr al, 4
	call hex_digit_to_ascii
	mov ah, 0x87		;; TODO: neat clock colors :)
	mov bx, 0x004B		;; position
	call put_char
	mov al, dl
	and al, 0x0F
	call hex_digit_to_ascii
	inc bx
	call put_char
	;; hours
	mov bx, 0x0048
	mov al, ch
	shr al, 4
	call hex_digit_to_ascii
	call put_char
	inc bx
	mov al, ch
	and al, 0x0F
	call hex_digit_to_ascii
	call put_char
	inc bx
	mov al, ':'
	call put_char
	add bx, 3
	call put_char
	;; seconds	
	inc bx
	mov al, dh
	shr al, 4
	call hex_digit_to_ascii
	call put_char
	inc bx
	mov al, dh
	and al, 0x0f
	call hex_digit_to_ascii
	call put_char
	
	pop es
	pop dx
	pop cx
.ret:	popa
	iret
show_clock db	1

; ___________________________________________________________________	
int4Ah:
	pusha
	;; clock
	call get_rtc_time
	;; check whether clock is running
	push es
	mov bx, KERNEL_SEGM
	mov es, bx
	mov ah, byte [es:show_clock]
	pop es
	test ah, ah
	jz .no_clock
	
	;; show time
	push cx
	push dx
	push es
	mov bx, TXTBUF_SEGM		;; video memory
	mov es, bx
	;; tiny hack: cl needed for color, so cl will be out first
	;; minutes
	mov dl, cl
	mov al, cl
	shr al, 4
	call hex_digit_to_ascii
	mov ah, 0x0E		;; TODO: neat clock colors :)
	mov bx, 0x004B		;; position
	call put_char
	mov al, dl
	and al, 0x0F
	call hex_digit_to_ascii
	inc bx
	call put_char
	;; hours
	mov bx, 0x0048
	mov al, ch
	shr al, 4
	call hex_digit_to_ascii
	call put_char
	inc bx
	mov al, ch
	and al, 0x0F
	call hex_digit_to_ascii
	call put_char
	inc bx
	mov al, ':'
	call put_char
	add bx, 3
	call put_char
	;; seconds	
	inc bx
	mov al, dh
	shr al, 4
	call hex_digit_to_ascii
	call put_char
	inc bx
	mov al, dh
	and al, 0x0f
	call hex_digit_to_ascii
	call put_char
	
	pop es
	pop dx
	pop cx
	
	;; set next int4A for next second
.no_clock:
	;call next_second
	mov cx, 0xFFFF
	mov dh, 0xFF
	call set_alarm
	popa
	iret

section .data
	
%endif
