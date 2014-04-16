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
	call intr_init
	call init_heap
	call fmount_fat12
	mov si, endl
	call display_string
	call press_any

	; shell mode
	call shell

; ___________________________________________________________________
reboot:
; tries to reboot computer
	int 0x19

; ___________________________________________________________________
turnoff:
; turn off power for all bios-managed devices
; args: none
	mov ax, 5307h	; bios power management
	mov bx, 0001h	; all devices, where power managed by bios
	mov cx, 0003h	; state: off
	int 0x15
; ___________________________________________________________________
delay:
; void delay(ax ms) 
	pusha
	mov cx, 1000
	mul cx
	jc .long
	xor cx, cx
	jmp .endif	
.long:	mov cx, dx
.endif:	mov dx, ax
	mov ah, 86h
	int 15h
	popa
	retn

;; __________________________________________________________________
execute:
;//	__far ax:status execute(ds:dx name)
	
	retf

%include "heap.inc"
%include "intrs.inc"	
%include "shell.inc"
%include "time.inc"

;;; %include "fs.inc" we're not ready now to encapsulate
%include "fat12.inc"		; \\\TODO: encapsulate it in fs.inc!


[segment .data]
build_date  	db	__DATE__, 0
build_time  	db	__TIME__, 0

; the DATA segment! Otherwise kernel_end will be at the end of code, 
;	but before data and heap will overwrite kernel data
kernel_end:
