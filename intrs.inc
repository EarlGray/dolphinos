;
;	Interrupts handlers and initializers
;	DolphinOS (c) Dmytro Sirenko, 2010
; ___________________________________________________________________

%ifndef __INTRS_INC__
%define __INTRS_INC__

%include "sysdefs.inc"
%include "stdio.inc"

[section .text]

;; SET_INTR intr_num handler_addr
%macro SET_INTERRUPT 2       
 	mov	si, %1
	shl	si, 2	;; mul by 4, int handler size
	mov	ax, %2
	mov	[si], ax
	mov	ax, KERNEL_SEGM
	mov	[si + 2], ax
%endmacro

%macro INTR_RETURN_TO_SHELL 0
    pop ax
    pop ax
    push word KERNEL_SEGM
    push word shell
%endmacro

; ___________________________________________________________________
init_interrupts:
	push	ds
	xor	ax, ax
	mov 	ds, ax

	;; set SYS_INTR handler to sys_int_handler
    SET_INTERRUPT SYS_INTR, sys_int_handler

    ;; set Ctrl-Break intr handler
    SET_INTERRUPT 0x1B, ctrlbreak_intr_handler

    SET_INTERRUPT 0x00, division_by_zero_handler
    
	pop	ds
	retn

; ___________________________________________________________________
sys_int_handler:
;; args:
;;	ah - function number
;; 	al, bx, cx, dx - args to function

	pusha
	movzx	di, ah
	shl	di, 1	;; size of function table enrty
	mov	si, system_functions_table
	add	di, si
	call	word [di]
	popa
	iret

; ___________________________________________________________________
division_by_zero_handler:
    mov si, m_division_by_zero
    call print_string
    mov di, sp
    mov dx, word [ss : di + 2]
    call print_hex_dx
    mov al, ':'
    call print_char
    mov dx, word [ss : di]
    call print_hex_dx
    call print_endl
    INTR_RETURN_TO_SHELL
    iret

; ___________________________________________________________________
ctrlbreak_intr_handler:
    mov si, m_ctrl_break
    call print_string
    INTR_RETURN_TO_SHELL
    iret

; ___________________________________________________________________
dummy_function:
	mov	dl, ah
	call	print_hex_dl
	mov 	si, m_dummy_intr
	call 	print_string 
	retn

[section .data]

system_functions_table:
.sf00	dw	dummy_function
.sf01	dw	dummy_function	;; wait and read symbol with echo 
.sf02	dw	dummy_function	;; write to stdout with Ctrl-break check
.sf03	dw	dummy_function	;; read from STDAUX
.sf04	dw	dummy_function	;; write to STDAUX
.sf05	dw	dummy_function	;; write byte to PRN
.sf06	dw	dummy_function	;; print ascii char in dl 
.sf07	dw	dummy_function	;; read a symbol from STDIN (no echo, wait, no ^Break);
.sf08	dw	dummy_function	;; read a symbol from stdin (wait, Ctrl-Break check, no echo)
.sf09	dw	print_ds_dx	;; print string at [ds:dx]
.sf0A	dw	read_string	
.sf0B	dw	dummy_function	;; check kbd state
.sf0C	dw	dummy_function	;; clear buffer and read symbol (wait)
.sf0D	dw	dummy_function	;; sync all file buffers
.sf0E	dw	dummy_function
.sf0F	dw	dummy_function

.sf10	dw	dummy_function
.sf11	dw	dummy_function
.sf12	dw	dummy_function
.sf13	dw	dummy_function
.sf14	dw	dummy_function
.sf15	dw	dummy_function
.sf16	dw	dummy_function
.sf17	dw	dummy_function
.sf18	dw	dummy_function
.sf19	dw	dummy_function
.sf1A	dw	dummy_function	;; set DTA offset
.sf1B	dw	dummy_function
.sf1C	dw	dummy_function
.sf1D	dw	dummy_function
.sf1E	dw	dummy_function
.sf1F	dw	dummy_function

.sf20	dw	dummy_function	;; 
.sf21	dw	dummy_function
.sf22	dw	dummy_function
.sf23	dw	dummy_function
.sf24	dw	dummy_function
.sf25	dw	dummy_function
.sf26	dw	dummy_function
.sf27	dw	dummy_function	;; terminate and stay resident
.sf28	dw	dummy_function
.sf29	dw	dummy_function	;; print a teletype char
.sf2A	dw	dummy_function	;; get date
.sf2B	dw	dummy_function	;; set date
.sf2C	dw	dummy_function	;; get time
.sf2D	dw	dummy_function	;; set time
.sf2E	dw	dummy_function	
.sf2F	dw	dummy_function

.sf30	dw	dummy_function
.sf31	dw	dummy_function
.sf32	dw	dummy_function
.sf33	dw	dummy_function	;; mouse driver
.sf34	dw	dummy_function
.sf35	dw	dummy_function
.sf36	dw	dummy_function
.sf37	dw	dummy_function
.sf38	dw	dummy_function
.sf39	dw	dummy_function	;; create directory
.sf3A	dw	dummy_function	;; remove directory
.sf3B	dw	dummy_function	;; set current dirctory
.sf3C	dw	dummy_function	;; create file
.sf3D	dw	dummy_function	;; open file
.sf3E	dw	dummy_function	;; close file
.sf3F	dw	dummy_function	;; read from file

.sf40	dw	dummy_function	;; write to file or device
.sf41	dw	dummy_function	;; remove file
.sf42	dw	dummy_function  ;; seek position in file
.sf43	dw	dummy_function	
.sf44	dw	dummy_function
.sf45	dw	dummy_function
.sf46	dw	dummy_function
.sf47	dw	dummy_function	;; get current directory
.sf48	dw	mem_allocate	;; allocate memory
.sf49	dw	mem_release	;; release memory	
.sf4A	dw	mem_reallocate	;; reallocate memory
.sf4B	dw	dummy_function	;; load and execute program
.sf4C	dw	dummy_function
.sf4D	dw	dummy_function
.sf4E	dw	dummy_function	;; find first file
.sf4F	dw	dummy_function	;; find next file

.sf50	dw	dummy_function
.sf51	dw	dummy_function
.sf52	dw	dummy_function
.sf53	dw	dummy_function
.sf54	dw	dummy_function
.sf55	dw	dummy_function
.sf56	dw	dummy_function
.sf57	dw	dummy_function
.sf58	dw	dummy_function
.sf59	dw	dummy_function
.sf5A	dw	dummy_function	;; create and open temp file	
.sf5B	dw	dummy_function
.sf5C	dw	dummy_function
.sf5D	dw	dummy_function
.sf5E	dw	dummy_function
.sf5F	dw	dummy_function

.sf67	dw	dummy_function
.sf68	dw	dummy_function	;; syncing of file caches

.sf71	dw	dummy_function	;; LFN functions

%endif
