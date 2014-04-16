
%ifndef __DBG_INC__
%define __DBG_INC__

[section .text]

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

[section .data]
sdbg_arg1:	db	"<", 0
sdbg_end:	db	">", 13, 10, 0

%endif
