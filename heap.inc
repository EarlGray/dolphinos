;;;
;;;	DolphinOS	(c) Dmytro Sirenko
;;;	Memory handling routines
;;; _________________________________________________________________
;;; =================================================================
%ifndef __MEM16_INC__
%define __MEM16_INC__

[segment .text]
;;; _________________________________________________________________
init_heap:
;;
	;; the heap begins behind the kernel end
	mov ax, kernel_end
	add ax, 0x0100
	shr ax, 4
	add ax, KERNEL_SEGM
	mov word [heap_seg], ax			; behind 
	;; set fs and gs to heap segment
	mov fs, ax
	mov gs, ax
	add ax, HEAP_SIZE 
	mov word [cache_seg], ax
	add ax, CACHE_SIZE
	mov word [exe_seg], ax
	jnz .ok
	mov si, heap_error
	call display_string
.hang:	jmp .hang
.ok:	
	push es
	mov es, ax
	xor dx, dx
	mov word [es:0x0000], dx
	mov byte [es:0x0002], dl
	pop es

	mov si, heap_welcome
	call display_string
	mov dx, word[heap_seg]
	call dbg_print_word_dx	
	mov si, endl
	call display_string
		
	retn
heap_welcome	db 	"Heap segment at ", 0
heap_error	db	"ERROR initializing heap", 0

;; __________________________________________________________________
set_es_heap_seg:
;//	es get_heap_segmnet(void)
	push word [heap_seg]
	pop es
	retf

set_ds_heap_seg:
;//	es get_heap_segmnet(void)
	push word [heap_seg]
	pop ds
	retf
set_fs_heap_seg:
	push word [heap_seg]
	pop fs
	retf

;;; _________________________________________________________________
;;; memcopy
;;; args:
;;;	cx 	- count of bytes to copy
;;;	ds:si	- where from
;;;	es:di	- where to
;;; out:
;;;	cf 	- if failed
memcopy:	
    rep movsb	
	retf

;;; _________________________________________________________________
memalloc:
;//	__far es:di memalloc(cx size) [ds!]
;;; args:
;;;	cx 	- size of memory
;;; out: 
;;;	es:di	- memory, NULL and CF if failed
SIZEOF_HEADER	equ	4
	push ds
	call KERNEL_SEGM : set_ds_heap_seg
	mov bx, SIZEOF_HEADER		; see from beginning
.next_chunk:
	; read header of chunk
	mov dx, word[bx - 4]		; dx - count of byte behind header
	mov al, byte[bx - 2]		; al - chunk info, 0 if free
	
	test dx, dx
	jz .heap_end			; if the end of heap
	;; memory already had been allocated, but maybe free now
	test al, al
	jnz .goto_next			; occupied

	;; is there enough space?
	mov ax, cx
	add ax, SIZEOF_HEADER
	sub ax, dx
	jge .goto_next			
	; - if equal, length of body will be 0, so jge.
	
	;; allocate:
	;;           <-------- dx --------->
	;;           <---- cx --->
	;;        h1    new data   h2 <-ax->
	;;      |----|-----------|----|----|
	;;     bx-4  bx        bx+cx     bx+dx
	;;      <------ old chunk --------->
	; write h2 at [dx + cx + 4]:
	neg ax
	mov di, cx
	mov word [bx + di], ax
	mov byte [bx + di + 2], 0	; free part

	; write this chunk:
	mov word [bx - 4], cx
	mov byte [bx - 2], 0x01		; occupied from now
	add cl, ch
	add cl, bl
	mov byte [bx - 1], cl		; control sum
	jmp .done

.goto_next:				; occupied, go to next
	add bx, dx
	add bx, SIZEOF_HEADER
	jnc .next_chunk	
	;; no place
	xor bx, bx			; NULL
	jmp .done

.heap_end:
	push dx
	mov dx, bx
	call dbg_print_word_dx
	pop dx
	; allocate at the end
	mov word [bx - 4], cx
	mov byte [bx - 2], 0x01
	mov di, cx
	add cl, ch
	add cl, bl
	mov byte [bx - 1], cl		; check sum
	
	;; dx == 0
	add di, bx
	mov word [di], dx
	mov word [di + 2], dx
		

.done:	; result:
	;lcall KERNEL_SEGM:set_es_heap_seg
	mov di, bx
				
	pop ds
	retf


;;; _________________________________________________________________
memfree:
;//	__far cf memfree(ds:si pointer)
;;; args:
;;;	si	- where
;;; out: 
;;;	CF if failed
	push ds
	call KERNEL_SEGM:set_ds_heap_seg
	;; check control sum, if addr is valid
	mov cx, si
	and cx, 0x00FF 
	add cl, byte [si - 4]
	add cl, byte [si - 3]
	cmp cl, byte [si - 1]
	je .ok
	stc				; si is not a valid pointer
	jmp .done
.ok:	
	;; clear
	xor ax, ax
	mov word[si - 2], ax		; clear state: now chunk is free
	mov bx, word [si - 4]		; look up next chunk
	;; is the last?
	mov ax, word [si + bx]		; test if there is a next chunk
	test ax, ax
	jnz .done
	;; no next chunk:
	mov word [si - 4], ax		; mark this as last
.done:	pop ds
	retf

;;; _________________________________________________________________
;;; memset
;;; args:
;;;	al	- value
;;;	cx 	- count
;;;	es:di	- where to

;;; _________________________________________________________________
;;; memsetw
;;; -- "" --
;;; 	ax - 	value

[segment .data]

heap_seg	dw	0x0000
cache_seg	dw	0x0000
exe_seg		dw	0x0000

%endif
