
%ifndef __INTERNAL_CMDS_INC
%define __INTERNAL_CMDS_INC

%include "stdio.inc"

[section .text]

;; __________________________________________________________________
memory_dump:
;; dump cx bytes from dx:bx
;; 
%define MEMDMP_WRAP 0x0F        ;; every 16 bytes
    push gs
    mov gs, dx
    add cx, bx      ;; cx is a limit now
    and bx, 0xFFF0
    mov di, .line

.next_byte:
    ;; whether new line
    test bx, MEMDMP_WRAP
    jz .endl
.no_endl:
    ;; whether a middle
    test bx, MEMDMP_WRAP/4
    jnz .no_middle
    mov al, ' '
    stosb
.no_middle:
    mov al, ' '
    stosb
    mov dl, byte [gs:bx]
    call dl_to_hex8_di

    inc bx
    cmp bx, cx
    jl .next_byte

    call .println
    pop gs
	retn

.endl:
    call .println
    ;; prepare for a new line
    mov di, .line
    ;; print offs:addr
    mov dx, bx
    call dx_to_hex16_di
    mov al, ':'
    stosb
    jmp .no_endl

.println:
    xor al, al  ; finishing zero byte
    stosb
    ;; print this line
    push bx
    mov si, .line
    call print_string
    call print_endl
    pop bx
    retn

.line:  times 0x60  db  0x00
    ;;offs:addr | XX XX XX XX XX XX XX XX  XX XX XX XX XX XX ...
    ;;                                  ... XX XX | asciitex tofthisl'
    ;; -> max count ~ 0x60
    
;; __________________________________________________________________
sh_mem_dump:
    mov si, word [arg.n2]
    call hex16_si_to_dx
    mov bx, dx

    mov si, word [arg.n3]
    call hex16_si_to_dx
    mov cx, dx

    mov si, word [arg.n1]
    call hex16_si_to_dx

	call memory_dump
	retn

[section .data]

%endif
