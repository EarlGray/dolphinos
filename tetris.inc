;
;	Tetris for DolphinOS
;	(c) Dmytro Sirenko
; ___________________________________________________________________
%ifndef __TETRIS_INC
%define __TETRIS_INC

%include "stdio.inc"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
segment .text

%macro BEGIN_TXT_DRAW 0
	push es	
	mov dx, TXTBUF_SEGM
	mov es, dx
%endmacro

%macro END_TXTDRAW 0
	pop es
%endmacro

;; __________________________________________________________________
PlayTetris:
	xor ax, ax
	call set_video_mode
	mov si,	tetris.welcome
	call display_string

	BEGIN_TXT_DRAW
	call tetris_start
	
	call tetris_main
	END_TXTDRAW
	
	movzx dx, [tetris.x0]
	call dbg_print_word_dx
	movzx dx, [tetris.y0]
	call dbg_print_word_dx
	
	retf

;; __________________________________________________________________
tetris_start:
	;;  x0-y0:
	movzx ax, byte [ds : tetris.width]
	movzx bx, byte [ds : screen_width]
	sub bx, ax
	shr bx, 1
	mov byte [ds : tetris.x0], bl
	; 
	movzx ax, byte [ds : tetris.height]
	movzx bx, byte [ds : screen_height]
	sub bx, ax
	shr bx, 1
	mov byte [ds : tetris.y0], bl
	
	;; clear tetris fields
	
	movzx ax, byte [ds : tetris.y0]
	mul byte [screen_width]
	movzx dx, byte [ds : tetris.x0]
	add ax, dx
	shl ax, 1

	movzx dx, byte [ds : tetris.height]

	mov bh, TTRS_FREE_CELL		;; free cell color
	mov bl, ' '			;; fill cell

.next_line:
	mov di, ax
	push ax
	mov ax, bx
	movzx cx, byte [ds : tetris.width]
    rep stosw
	pop ax
	movzx cx, byte [ds : screen_width] 
	shl cx, 1
	add ax, cx
	dec dx
	jnz .next_line
	;
	
	retn

;; __________________________________________________________________
tetris_main:
	;; check state and move tetris down if possible
	call ttrs_new_shape
	
.main_loop:
	;; delay
	mov ax, 80	;; ms
	call delay

	;; check keyboard
	mov ah, 0x11
	int 0x16
	jz .fall_further
	;; key is pressed
	; clear keyboard buffer
	mov ah, 0x10
	int 0x16
	; handle
	cmp al, 'q'
	je .end
.fall_further:	
	;; layout checkout
	call tetris_checkout
	jnc .main_loop
	;; run new shape
	call ttrs_new_shape	
	jnc .main_loop
.end:	
	retn

;; __________________________________________________________________
ttrs_new_shape:
;; puts random shape at top of the screen
;; rets:	
;;	cf=1 if there's no place
	;; convert all existing falling cells to mass
	mov bh, byte [tetris.height]
.cnext_line:
	dec bh
	jl .cend
	xor bl, bl
.cnext_cell:
	call ttrs_get_cell
	cmp ah, TTRS_FALL_CELL
	jne .cnext
	mov al, 0x9F
	call ttrs_set_cell
.cnext:	inc bl
	cmp bl, byte [tetris.width]
	jl .cnext_cell
	jmp .cnext_line
.cend:
	
	;; put new figure
	mov bh, 0x00
	mov bl, byte [ds : tetris.width]
	shr bl, 1
	sub bl, 2
	call ttrs_video_addr
	mov di, ax	;; DI will be holder for videomem addr
	; choose shape
	call ttrs_random
	shl ax, 3		;; mul by 8, size of shape images
	add ax, tetr_primitives
	mov si, ax		; now si contains mask
	; read mask:
	mov cx, 0x000F	;; bit counter
.next_mask_symb:
	bt word [ds : si], cx	;; copy CXth bit from [SI] to CF
	jnc .next	;; if zero, we don't need put smth here
	; check whether is possible to put cell
	mov ax, word [es : di]
	cmp ah, TTRS_FREE_CELL
	je .cell_ok
	;;;;;; end of game, there's no place for new figure
	stc
	retn
.cell_ok:
	mov dh, TTRS_FALL_CELL
	mov dl, TTRS_BLOCK_ASCII 
	mov word [es : di], dx
.next:	
	test cl, 0x03		;; divisible by 4?
	jnz .mask_next
	;; we need to go to next line
	sub di, 8		;; LF :)
	mov ax, di
	call ttrs_down_cell	;; CR :)
	mov di, ax
.mask_next:
	add di, 2
	dec cx
	jge .next_mask_symb
.ret:	clc		;; all is ok
	retn

;; __________________________________________________________________
ttrs_fall_aside:
;; move falling figure by one cell aleft or aright if possible
;; args:
;; 	di=0 - left, di=1 - right
	
	;; find quad that contains falling figure
	;; and check whether all cells can be moved
	mov bh, byte [tetris.height]
	; cx will be (x1, y1), dx (x2, y2)
	mov cx, word [tetris.width] ;; .height is just after .width
	xor dx, dx
	xor si, si	;; si=0 - move is possible
.cnext_line:
	dec bh
	jl .cend
	xor bl, bl
.cnext_cell:
	call ttrs_get_cell
	cmp ah, TTRS_FALL_CELL
	jne .cnext
	;;
	; check whether move is possible
	test di, di
	jz .left
	inc bl
	cmp bl, byte [tetris.width]
	jge .ret
	call ttrs_get_cell
	cmp ah, TTRS_FALL_CELL
	je .fall_ok
	cmp ah, TTRS_FREE_CELL
	jne .ret
.fall_ok:
	dec bl
	jmp .eif
.left:	dec bl
	cmp bl, 0
	jle .ret
	call ttrs_get_cell
	cmp ah, TTRS_FALL_CELL
	je .fa_ok
	cmp ah, TTRS_FREE_CELL
	jne .ret
.fa_ok:	inc bl
.eif:
	; check bounds
	cmp ch, bh
	cmovg ch, bh
	cmp dh, bh
	cmovl dh, bh
	
	cmp cl, bl
	cmovg cl, bl
	cmp dl, bl
	cmovl dl, bl
	;;
.cnext:	inc bl
	cmp bl, byte [tetris.width]
	jl .cnext_cell
	jmp .cnext_line
.cend:  
	mov bx, cx	


.next:
	;; Move this cell
	call ttrs_get_cell
	test di, di
	jz .move_left
	dec bl
	
	inc bl
.move_left:
	
.endif:
	;;
	inc bl
	cmp bl, dl
	jl .next
	mov bl, cl
	inc bh
	cmp bh, dh
	jl .next
.ret:		
	retn

;; __________________________________________________________________
ttrs_down_cell:
; args:   ax - addr of cell
; rets:	  ax - addr down by one cell
	movzx dx, byte [screen_width]
	shl dx, 1
	add ax, dx
	retn

ttrs_up_cell:
; see above, just up 
	movzx dx, byte [screen_width]
	shl dx, 1
	sub ax, dx
	retn

;; __________________________________________________________________
tetris_checkout:
;; check fields from bottom to top
;; if there's a filled line, remove it
;; if there's a falling shape, move it down if possible, 
;;	else return cf=1
	;; calculate last cell addr in videomem
	mov bh, byte [tetris.height]

.next_line:
	dec bh
	jl .ret
	xor bl, bl
	;; cycle init:
	stc 	;; flag of full line
.next_block:
	push bx
	call ttrs_video_addr
	mov di, ax
	mov ax, word [es:di]
	cmp ah, TTRS_FREE_CELL
	jne .else
	clc
	jmp .endif
.else:	cmp ah, TTRS_FALL_CELL
	jne .endif
	clc
	;; move this cell by one cell down 
	inc bh	;; check cell below
	cmp bh, byte [tetris.height]
	jl .bottom_ok
.landing:
	pop bx
	stc
	retn
.bottom_ok:
	;; check whether cell below is occupied
	;push bx		;; internal push bx
	call ttrs_get_cell
	cmp ah, TTRS_FREE_CELL	
	je .occupied_ok
	cmp ah, TTRS_FALL_CELL
	jne .landing
.occupied_ok:
	; set cell below as this
	mov al, TTRS_FALL_CELL
	call ttrs_set_cell
	dec bh 
	; clear trace)
	mov al, TTRS_FREE_CELL
	call ttrs_set_cell
.endif:		
	pop bx
	inc bl
	cmp bl, byte [tetris.width]
	jl .next_block

	;; if full, delete, sheer and go to next line
	jnc .not_full
	clc
	;;; sheer all
.not_full:
	jmp .next_line
.ret:	clc
	retn

;; __________________________________________________________________
ttrs_video_addr:
;; args: 
;;	bh - y, 
;;	bl - x (for tetris)
;; rets:
;;	ax - addr of field in videomem
;; notes:
;;	warning: corrupts GS value!
	pusha
	movzx ax, bh
	add al, byte [tetris.y0]
	mul byte [screen_width]
	movzx dx, byte [tetris.x0]
	add ax, dx
	movzx dx, bl
	add ax, dx
	shl ax, 1
	mov word[.res], ax
	popa
	mov ax, word[.res]
	retn
.res	dw	0x0000

;; __________________________________________________________________
ttrs_get_cell:
;; args:
;; 	bh - y	rel. to x0-y0
;;	bl - x 	rel. to x0-y0
;; outs:	
;;	al - text_attr for this cell
;;	ah - ascii there
	push bx
	call ttrs_video_addr
	mov bx, ax
	mov ax, word [es : bx]
	pop bx
	retn

;; __________________________________________________________________
ttrs_set_cell:
;; args:
;;	bh - y 
;;	bl - x
;;	al - attr
	pusha
	mov dh, al
	call ttrs_video_addr
	mov bx, ax
	mov dl, TTRS_BLOCK_ASCII
	mov word [es:bx], dx
	popa
	retn	

;; __________________________________________________________________
ttrs_random:
;; gets number from 0 to 6 in ax
	;;;;; just a STUB!
	mov ax, 4
	;; call get_rtc_time
	retn

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
section .data

TTRS_FREE_CELL		equ	0x8F
TTRS_FALL_CELL		equ	0x0C
TTRS_MASS_MASK		equ	0x77

tetris:
.welcome	db	 "Tetris for DolphinOS", 13, 10, \
			"[under development]", 13, 10, \
			"(c) Dmytro Sirenko, 2010", 13, 10, 0
.width	db	26
.height	db	20

.x0	db	00
.y0	db	00

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
tetr_primitives:
;; this records contains mask 4x4 bit for shapes:
;;    4x4 (2 byte) mask for shape:   4 times for 4 turns
;;	
.line	dw	0x4444, 0x0F00, 0x4444, 0x0F00
.block	dw	0xCC00, 0xCC00, 0xCC00, 0xCC00
.rbreak	dw	0x4620, 0x6C00, 0x4620, 0x6c00
.lbreak	dw	0x4c80, 0xC600, 0x4C80, 0xc600
.rangle	dw	0x6440, 0x0E20, 0x44C0, 0x8E00
.langle dw	0xC440, 0x2E00, 0x4460, 0x0E80
.trngl	dw	0x4E00, 0x4640, 0x0E40, 0x4C40

.debug	dw	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF

TTRS_PRIM_COUNT		equ	8
TTRS_BLOCK_ASCII	equ	' '

%endif
