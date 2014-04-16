
%ifndef __INTERNAL_CMDS_INC
%define __INTERNAL_CMDS_INC

%include "stdio.inc"

[section .text]

;; __________________________________________________________________
memory_dump:
;; dump cx bytes from dx:bx
;; ascii format if dh=0xFF
	;; @todo: beginning alignment: cx may not be divisible by 16	
	push dx
	mov si, bx
.iter:
	test si, 0x0F
	jnz .no_endl
	;; we have to wrap the line
	cmp dh, 0xFF
	je .mem_dump_ascii_wrap
	; just a next line
	mov ah, 0x0E
	xor bh, bh
	mov al, 13
	int 0x10
	mov al, 10
	int 0x10
.no_endl:
	; is there a way of doing it in not disgusting way?
	pop dx
	push ds
	mov ds, dx
	lodsb
	pop dx
	push ds
	mov ds, dx

	mov dl, al
	call print_hex_dl
	mov dl, ' '
	call print_ascii_dl 	
	loop .iter
	pop dx
	retn

.mem_dump_ascii_wrap:
	
	jmp .no_endl

;; __________________________________________________________________
sh_mem_dump:
	;; parse cmdbuf
	
	;; dump
	call memory_dump
	retn

[section .data]

%endif
