;
;	This is a simplest kernel
; 	DolphinOS (c) Dmytro Sirenko, 2010
; ___________________________________________________________________

%include "sysdefs.inc"

kernel_start:
	org KERNEL_OFFS
[segment .text]
	cli
	; prepare memory state
	mov ax, KERNEL_SEGM
	mov ds, ax		; use segment 0x0100
	mov es, ax		; 

	mov ax, STACK_SEGM
	mov ss, ax		;
	mov sp, STACK_SIZE	; global stack size 0xFFFF
	sti
	
	; initialize
	call init_interrupts
	call init_memory

	; run
	call press_any

	mov si, m_system_welcome
	call print_string

	call shell
	
	call turnoff

;; ___________________________________________________________________

%include "msgs.inc"

%include "time.inc"
%include "stdio.inc"
%include "acpi.inc"
%include "intrs.inc"
%include "mm.inc"

%include "shell.inc"


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
[segment .data]

build_date:  	db	__DATE__, 0
build_time:  	db	__TIME__, 0

; the DATA segment! Otherwise kernel_end will be at the end of code, 
;	but before data and heap will overwrite kernel data
kernel_end:
