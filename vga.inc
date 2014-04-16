;
;	VGA drawing routins
;	(c) Dmytro Sirenko
; ___________________________________________________________________
%ifndef __VGA_INC
%define __VGA_INC


;;; _________________________________________________________________
set_vga_mode:
;;//		void set_vga_mode(al mode)
	xor ah, ah
	int 10h
	retn

;;; _________________________________________________________________
draw_pixel:
;;//	void put_pixel(bx X, ax Y, cl color)
	cmp ax, 0
	jl .done
	cmp ax, VGA13h_MAX_Y
	jge .done
	cmp bx, 0
	jl .done
	cmp bx, VGA13h_MAX_X
	jge .done

	mov dx, 320
	mul dx
	add ax, bx
	mov di, ax
	mov byte [es:di], cl
.done:	retn

;;; _________________________________________________________________
draw_line:
;;//	void put_line(si *point1, di *point2, al color)
;;			point {   dw x, y;   }
	;; whether is horisontal
	mov bx, word [di + 2]	
	cmp bx, word [si + 2]
	jne .not_horisontal
	;; draw horisontal line
	push es
	push ax		;; save color
	call get_pointer_for_xy		;; [si] -> ax 
	
	mov cx, word [di]
	sub cx, word [si]
	jge .endif
	;; df = 1, cx = abs(cx)
	std
	neg cx		;; cx > 0
.endif:
	mov bx, VGAMEM_SEGM
	mov es, bx	;; es = 0xA000
	mov di, ax	;; stosb: al -> es:di until cx > 0
	pop ax		;; restore color
    rep stosb
	cld		;; clear DF if set
	pop es
	retn
	
.not_horisontal:
	mov bx, word [di]
	cmp bx, word [si]
	jne .not_vertical
	;; draw vertical line
	push ax
	mov cx, word [di + 2]
	sub cx, word [si + 2]
	call get_pointer_for_xy
	mov di, ax
	pop ax
	push es
	mov bx, VGAMEM_SEGM
	mov es, bx
.next_vert:
	mov byte [es:di], al

	test cx, cx
	jns .dnup
	sub di, VGA13h_MAX_X
	inc cx
	jmp .eif
.dnup:	add di, VGA13h_MAX_X
	dec cx
.eif:	jnz .next_vert
	
	pop es
	retn

.not_vertical:
	;; anal magic...
	retn
;;; _________________________________________________________________
draw_rect:
;;//	void draw_rect(si *p1, cx width, dx height, al color);

	push ax
	call get_pointer_for_xy
	mov di, ax		; now si - pointer to pixel di.x:di.y
	pop ax

	push es
	mov bx, VGAMEM_SEGM
	mov es, bx
		
.next_line:
	push di
	push cx
    rep stosb	
	pop cx
	pop di
	add di, VGA13h_MAX_X
	dec dx
	jnz .next_line

	pop es
	retn

; ___________________________________________________________________
get_pointer_for_xy:
;; args: si *point
;; out:  ax pos, preserves other regs 
	mov ax, word [si + 2]
	push dx
	mov dx, VGA13h_MAX_X
	mul dx
	add ax, word [si]
	pop dx
	retn

;;; _________________________________________________________________
vga_trial:
;;//		void tetris(void)
	mov al, 13h
	call set_vga_mode

	mov ax, 20
	mov bx, 256
	push es
	;; begin paint:
	push VGAMEM_SEGM
	pop es
	
	;; draw line with all available colors
.loop:	
	mov cx, bx
	pusha
	call draw_pixel
	popa
	dec bx
	cmp bx, 0
	ja .loop
	pop es

	;; draw rect
	mov cx, 20
	mov dx, 20
	mov al, 10
	mov si, .point1
	call draw_rect

	;; draw line
	mov word [.point1], 120
	mov word [.point1 + 2], 60
	;; point1 = (120, 60)
	mov si, .point1
	mov di, .point2
	mov al, 20
	call draw_line
	
	mov word [.point2], 120
	mov word [.point2 + 2],	50 
	mov di, .point2
	mov si, .point1
	call draw_line

	call wait_key

	xor al, al
	call set_video_mode

	retn
.point1	dw	50, 40
.point2	dw 	20, 60


%endif
