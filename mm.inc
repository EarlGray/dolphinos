;
;	Memory management
;	DolphinOS (c) Dmytro Sirenko, 2010
; ___________________________________________________________________

%ifndef __MEMORY_INC__
%define __MEMORY_INC__

%define MEM_DEBUG (1)
%if MEM_DEBUG
%include "stdio.inc"
%endif

; Memory management

;;
;;   <-- Sysmemory -->		       Userspace
;;     BIOS   kernel     Heap and code         Disk cache  Stack
;;   <----> <-------->   ---->                        <---   <----
;;  |------|---------|----------------------- . . .------|-------|
;;0000  KERN_SEG                                              0xA000

; Entry:
;	first byte is a block type descriptor:
; |---1b----|---1b----|---------2b---------|info...
;   prop    checksum    length in bytes    pointer
;     prop+low(l+p)+high(l+p)

; prop:
MEM_FREE	equ	0x0000
MEM_USED	equ	0x8000	;; used memory

; according to a type, entry can be:

[section .text]

; ___________________________________________________________________
; check checksum at 16b-sheer dx
; in:   di (16b-offset of chunk)
; out   zf=0 if fails
; corrupts 
check_chunk:
    push gs
    sub di, 1
    mov gs, di
    mov ax, word [gs:0xE]   ;; size of chunk in 16b-paragraphes
    add di, 1
    add ax, di
    and ax, 0x7FFF
    mov bx, word [gs:0xC]   ;; 15bit-checksum + 0x8000 if used
    and bx, 0x7FFF
    cmp ax, bx    
    pop gs
    retn

; ___________________________________________________________________
; put used chunk at_16b, length_b, prop
; in:   dx  - 16b-offset where to put, oldest bit is whether to use
;       cx  - reqired size in 16b
;       
put_chunk:
    push gs

    mov bx, 0x7FFF
    or bx, dx  ;; bx is a mask to AND with checksum

    sub dx, 1   
    mov gs, dx
    add dx, 1

    mov word [gs:0xE], cx
    add cx, dx          ;; make checksum as (pointer16 + length16)
    and cx, bx          ;; set 'used' bit

    mov word [gs:0xC], cx
    
    pop gs
    retn

; ___________________________________________________________________
; determine paragraph of next chunk: 
; in:   dx - 16b-offset of current
; out:  dx - 16b-offset of next
next_chunk:
    push gs
    sub dx, 1
    mov gs, dx
    add dx, 1
    ;;
    add dx, word [gs:0xE]
    pop gs
    retn

; ___________________________________________________________________
init_memory:
	;; calculate segment
	mov ax, word kernel_end
	shr ax, 4			;; [kernel_end]>>4                       KERNEL_SEGM+2
	add ax, KERNEL_SEGM + 2		;; |--kernel_end---|---first_pointer_info|memory------
	mov word [heap], ax

	;; put the first pointer info
    

	retn

; ___________________________________________________________________
mem_allocate:
;; args:
;;	bx - size of memory in 16b paragraphs
;; out:
;;	cf=0 if successful
;; 	ax - segment of allocated block
;;	cf=1 if failed: 
;;		ax=7 if heap structure is corrupted
;;		ax=8 if there's no memory, bx - size of biggest free chunk
	;; check whether heap is valid
	jne .heap_corruption
	
	retn
.search_again:
	;; search for sufficient block in existing heap
	;; find the least sufficient piece of memory in heap	
	; go through list of chunks and save the biggest and least sufficient
	mov dx, word [heap]		; dx - pointer, ax - least sufficient 
	
	
	retn

.heap_corruption:
	mov ax, 7	; errcode
	stc 
	retn

; ___________________________________________________________________
mem_reallocate:
	retn

; ___________________________________________________________________
mem_release:
	
	retn


[section .data]

heap:	 	dw	0x0000

%endif
