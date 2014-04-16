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

MEM_BLOCK_SIZE	equ	0x200
MEM_TABLE_SIZE	equ	0x800
MAT_ENTRY_SIZE	equ	0x04

; Memory management
; Memory is mapped to Memory Allocation Table just after kernel code

;;
;;   <------- System memory ----->		Userspace
;;     BIOS   kernel       MAT      Heap and code         Disk cache  Stack
;;   <----> <-------->  <--------> ---->                        <---   <----
;;  |------|---------|-|---------||--------------------- . . .------|-------|
;;0000  KERN_SEG       [mat_segm]                                       0xA000

; MAT describes memory blocks with MAT entry (4 byte for each):
; memory from 0x00000 to 0x9FFFF
; Entry:
;	first byte is a block type descriptor:

MEM_FREE_CLUSTER	equ	0x00
MEM_HEAP_CLUSTER	equ	0x01	;; used memory
MEM_DISK_CACHE_CLUSTER	equ	0x02	;; can be used if there are no free clusters
MEM_EXECUTABLE_CLUSTER	equ	0x03
MEM_SYSTEM_CLUSTER	equ	0x0F	;; kernel and BIOS

; according to a type, entry can be:

[section .text]

; ___________________________________________________________________
init_memory:
	;; calculate MAT segment
	mov ax, word kernel_end
	shr ax, 4
	add ax, KERNEL_SEGM
	inc ax
	mov word [mat_segm], ax
	
	;; clear table
	push es
	mov es, ax
	mov di, 0

	mov cx, MEM_TABLE_SIZE
	xor eax, eax		;; free memory cluster
   rep	stosd

	;; set bios memory, kernel space and MAT to system memory
	; MAT size in 16b paragraphs
	mov ax, ( MEM_TABLE_SIZE * MAT_ENTRY_SIZE ) >> 4
	add ax, word [mat_segm]
	; now convert ax to memory blocks
%if (MEM_BLOCK_SIZE != 0x200)
%error "Hack"
%else
	shr ax, 9	
%endif
	inc ax
	mov cx, ax
%if MEM_DEBUG
	mov si, mdbg_sys_mem
	call print_string
	mov dx, cx
	call print_hex_dx
	call print_endl
%endif
	xor eax, eax
	mov di, ax
	mov al, MEM_SYSTEM_CLUSTER
   rep	stosd

	pop es			
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
	
	retn

; ___________________________________________________________________
mem_reallocate:
	retn

; ___________________________________________________________________
mem_release:
	
	retn


[section .data]

mat_segm:	dw	0x0000


%endif
