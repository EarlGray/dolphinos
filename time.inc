;;
;;
;;
;; __________________________________________________________________
%ifndef __TIME_INC
%define __TIME_INC

section .text

get_rtc_date:
;; success: cf=0
;; args: none
;; outs:
;;	cx - bcd year (1998h for 1998)
;;	dh - bcd month
;; 	dl - bcd day
	mov ah, 0x04
	int 0x1A
	retn

get_rtc_time:
;; cf = 0 success
;; args: none
;; outs:
;; 	ch:dl:dh - (bcd) hh:mm:ss
	mov ah, 0x02
	int 0x1A
	retn

set_rtc_date:
;; args:
;;	cx:dh:dl - (BCD) yyyy:mm:dd
	mov ah, 0x05
	int 0x1A
	retn

set_rtc_time:
;; args:
;;	ch:cl:dh - (bcd) hh:mm:ss
;;	dl - 01 for summer time, 0 otherwise
	mov ah, 0x03
	int 0x1A
	retn

get_sys_time:
;; args: none
;; outs:
;;	cx:dx - counter
;;	al - counter overflow byte
	xor ah, ah
	int 0x1A
	retn

set_alarm:
;; args:
;;	ch:cl:dh - (bcd) hh:mm:ss for irq8 (int 0x4A)
	mov ah, 0x06
	int 0x1a
	retn

next_second:
;; args: ch:cl:dh - hh:mm:ss (bcd)
;; out:  ch:cl:dh - hh:mm:ss+1 (bcd)
	push ax
	movzx ax, dh
	inc al
	daa
	mov dh, al 
	cmp dh, 0x60
	jl .ok
	xor dh, dh
	mov al, cl
	inc al
	daa
	mov cl, al
	cmp cl, 0x60
	jl .ok
	xor cl, cl
	mov al, ch
	inc al
	daa
	mov ch, al
	cmp ch, 0x24
	jl .ok
	xor ch, ch
.ok:	pop ax
	retn

%endif
