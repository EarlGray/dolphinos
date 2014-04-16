;
;	Simplest shell 
;	DolphinOS (c) Dmytro Sirenko
; ___________________________________________________________________

%ifndef __SHELL_INC__
%define __SHELL_INC__

%include "stdio.inc"
%include "cmds.inc"
%include "debug.inc"

[section .text]

shell:
; just shell
	
.shell_loop:
	mov si, def_prompt
	call print_string

	call clear_cmdline

	;; read commands
	mov dx, cmdbuf
	call read_string

	;; parse it to arguments
	call parse_cmdline

	mov cx, internal_cmds_size
	mov di, internal_commands
.cmd_loop:
	call cmdcmp
	jne .next
	call word [di + INTCMDS_ENTRY_SIZE - 2]
	jmp .shell_loop
.next:
	add di, INTCMDS_ENTRY_SIZE
	loop .cmd_loop
	
	mov si, m_unknown_cmd
	call print_string

	jmp .shell_loop
.exit:
	retn

; ___________________________________________________________________
cmdcmp:
	push di
	mov si, word [arg.0]
.next:
    	cmpsb
	jne .ret
	mov al, byte [di]
	test al, al
	jnz .next
	cmpsb
	
.ret:	pop di
	retn

; ___________________________________________________________________
parse_cmdline:
;; divides cmdbuf string into words in-place, inserting 0 (endofline) 
;; after each argument in command line, then writing up offsets of 
;; arguments to [arg.n]
	;; clear arg array
	mov di, word arg
	xor ax, ax
	mov cx, MAX_ARGC
	rep stosw
	;; 
	xor bx, bx		;; args counter
	mov di, word cmdbuf	;; iterator
.separator:
	cmp byte [di], SEP_CHAR
	jne .first_non_sep
	mov byte [di], 0
	inc di
	jmp .separator
.first_non_sep:
	; next argument
	inc bl
	;; is there no more than MAX_ARGC args?
	cmp bl, MAX_ARGC
	jg .end
	; calculate offset of arg[bl]
	mov si, bx
	dec si		;; bx=1 for first arg, offs=0
	shl si, 1	;; word, 2 byte
	add si, word arg
	; remember offset of current argument
	mov word [si], di
.non_sep:
	;; whether the end reached
	mov dl, byte [di]
	test dl, dl
	jz .end
	mov al, SEP_CHAR	;; for comparison, won't be changed
	;; test the next one
	inc di
	cmp byte [di], SEP_CHAR
	je .separator
	jmp .non_sep
.end:	
	mov byte [argc], bl
	retn

; ___________________________________________________________________
clear_cmdline:	
	;; there may be additional actions, like saving cmdline in hist
	xor al, al
	mov cx, CMDBUF_SIZE
	mov di, cmdbuf
	rep stosb
	retn

; ___________________________________________________________________
shell_exit:
	pop ax	;; offset of return
	mov si, m_sh_exiting
	call print_string
	mov ax, word shell.exit
	push ax	
	retn

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
[section .data]

MAX_ARGC	equ	8
CMDBUF_SIZE	equ	256
INTCMDS_ENTRY_SIZE	equ	10

internal_commands:
.exit:	
	db	"exit", 0, 0, 0, 0
	dw	shell_exit
.off:	
	db	"off", 0, 0, 0, 0, 0
	dw	turnoff
.memdump:
	db	"memdmp", 0, 0
	dw	sh_mem_dump
.argsout:
	db	"args", 0, 0, 0, 0
	dw 	dbg_show_args

internal_cmds_size equ ($ - internal_commands)/INTCMDS_ENTRY_SIZE

cmdbuf:	times CMDBUF_SIZE db 0x00

;; just 5 possible arguments at the moment

argc:	db	MAX_ARGC	;; just to fill
arg:
%assign i 0
%rep MAX_ARGC
.%[i]:	dw	((i + '0') << 8) + 'A'
%assign i i+1
%endrep

%endif
