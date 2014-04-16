
%ifndef __DBG_INC__
%define __DBG_INC__

[section .text]

%macro BP 1
    mov al, %1
    call print_char
%endmacro

;; __________________________________________________________________
dbg_show_args:

	mov dl, byte [argc]
	call print_hex_dl
	call print_endl

	mov cx, MAX_ARGC
	mov di, word arg
.iter:
	mov si, word [di]
	test si, si
	jz .next

	push si	
	mov si, sdbg_arg1
	call print_string
	
	pop si
	call print_string

	mov si, word sdbg_end
	call print_string
	
.next:	
	add di, 2
	loop .iter
	retn


%macro SHOW_REG 1
    mov si, cpu_seg_regs.%1
    call print_string
    mov al, '='
    call print_char
    mov dx, %1
    call print_hex_dx
    mov si, .spaces
    call print_string
%endmacro

;; __________________________________________________________________
dbg_cpu_state:
    pusha
    pushf
    ;; common registers
    mov cx, 8
    mov di, sp
    add di, 16
    mov si, cpu_reg_name + 21
.reg:
    push si
    call print_string
    mov al, '='
    call print_char
    mov dx, word[ss:di]
    call print_hex_dx
    mov si, .spaces
    call print_string
    pop si
    sub di, 2
    sub si, 3
    loop .reg
    ;; segment_registers
    call print_endl
    SHOW_REG ds
    SHOW_REG es
    SHOW_REG fs
    SHOW_REG gs
    mov si, .flags
    call print_string
    mov dx, word[ss:di]
    call print_hex_dx
    call print_endl
    popf
    popa
    retn
.regval:    db  'REGV', 0
.spaces:    db  '  ', 0
.flags:     db  'flags=', 0

; ___________________________________________________________________
dbg_something:
;; to test temporary stuff
    mov dx, kernel_end
    call print_hex_dx
    call print_endl
    mov ax, 0
    div cx
    retn

[section .data]
sdbg_arg1:	db	"<", 0
sdbg_end:	db	">", 13, 10, 0

cpu_reg_name:       ;; order is important!
.di:    db  "DI", 0
.si:    db  "SI", 0
.bp:    db  "BP", 0
.sp:    db  "SP", 0
.bx:    db  "BX", 0
.dx:    db  "DX", 0
.cx:    db  "CX", 0
.ax:    db  "AX", 0
cpu_seg_regs:
.ds:    db  "DS", 0
.es:    db  "ES", 0
.fs:    db  "FS", 0
.gs:    db  "GS", 0
;cpu_flag:
;.cf:    db  "CF", 0
;.pf:    db  "PF", 0
;.af:    db  "AF", 0
;.zf:    db  "ZF", 0
;.sf:    db  "SF", 0
;.tf:
;.if:
;.df:
;.of:
;.nt:

%endif
