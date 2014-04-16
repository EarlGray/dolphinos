;
;	Simplest DolphinShell	
;	(c) Dmytro Sirenko, 2010
; ___________________________________________________________________
%ifndef __DSHELL_INC__
%define __DSHELL_INC__

%include "sysdefs.inc"

; ___________________________________________________________________
; shell_main
; 
; args, out: none
shell:

	; set video mode
	mov bx, 10bh
	call set_svga
	jne .svga_ok
	mov al, 03h		; set 80x25 16 colors mode
	call set_video_mode
.svga_ok:
	
	mov al, 0x0B
	mov byte [text_attr], al
	mov dl, 20		; cursor at the centre of screen
	mov dh, 10
	call set_cursor_pos
	mov si, welcome		; welcome message
	call display_string

	mov ax, 0x08
	mov byte [text_attr], al
	mov dl, 60		;
	mov dh, 11
	call set_cursor_pos
	mov si, wlcm_help	; help message
	call display_string

	mov ax, 0x07
	mov byte [text_attr], al

.system_loop:			; shell loop: prompt-command-run
	
	; prompt
	mov si, msg_prompt
	call display_string
	
	; read input
	call read_prompt
	; is empty?
	mov al, byte [cmd_buf]
	test al, al
	jz .system_loop
	; echo 
	;mov si, cmd_buf
	;call display_string
	
	; simplest actions
	mov si, .off
	call strcmpcmd
	je .turnoff
	cmp byte [cmd_buf], 'q'		; "q"uit
	jne .next0
.turnoff:
	call turnoff
	mov si, .acpi_error
	jmp .system_loop
.acpi_error	db "Can`t shut down system: no valid ACPI", 13, 10, 0
.off		db "off", 0
.next0:
	;jmp .system_loop
.next0a:
	mov si, .vga
	call strcmpcmd
	jne .next1
	call vga_trial
	jmp .system_loop
.vga 	db "vga", 0
.next1:
	mov si, .info		; "i"nfo   ;;put ;;terrupts
	call strcmpcmd
	jne .next1a
	mov si, .lstr_heap_seg
	call display_string
	mov dx, [heap_seg]
	call dbg_print_word_dx
	mov si, .lstr_cache_seg
	call display_string
	mov dx, [cache_seg]
	call dbg_print_word_dx
	mov si, .lstr_cache_size
	call display_string
	mov dx, CACHE_SIZE
	call dbg_print_word_dx
	jmp .system_loop
.info	db	"info", 0
.lstr_heap_seg	db	"Heap segment at  ", 0
.lstr_cache_seg	db 	13, 10, "Cache segment at ", 0
.lstr_cache_size db	13, 10, "Cache last byte: ", 0
.next1a:
	mov si, .int
	call strcmpcmd
	jne .next2
	int 20h
	jmp .system_loop
.int	db	"int", 0
.next2:
	cmp byte [cmd_buf], 'b'		; "b"uild date/time
	jne .next2a
	mov si, endl
	call display_string
	mov si, build_date
	call display_string
	mov al, ' '
	call out_char
	mov si, build_time
	call display_string
	jmp .system_loop
.next2a:
	mov si, .help
	call strcmpcmd
	jne .next3
	mov si, .lstr_h
	call display_string

	jmp .system_loop
.help	db	"help", 0
.lstr_h	db	"Commands:", 13, 10, \
		"   q - turn off computer", 13, 10, \
		"   ascii - print ascii table", 13, 10, \
		"   keys - keys info, press q for exit", 13, 10, \
		"   dump - dump memory area, you`ll be prompted for params", 13, 10, \
		"   info - info about memory allocation", 13, 10, \
		"   cpu - cpu registers state", 13, 10, \
		"   cls/clear - clear screen", 13, 10, \
		"   delay - 5 second time delay", 13, 10, \
		"   vga - try some VGA graphics", 13, 10, \
		"   tetris - play tetris", 13, 10, \
		"   [ls - list of files in current directory]", 13, 10, \
		0
.next3:
	cmp byte [cmd_buf], 'm'		; allocate "m"emory
	jne .next3a
	mov cx, 10
	push es
	call KERNEL_SEGM:memalloc
	mov [pointer], di
	mov dx, [heap_seg]
	call dbg_print_word_dx
	pop es
	jmp .system_loop
.next3a:
	cmp byte [cmd_buf], 'r'		; "r"elease memory
	jne .next4
	mov si, [pointer]
	call KERNEL_SEGM:memfree
	jnc .system_loop
	mov si, .local_str01
	call display_string
	jmp .system_loop
.local_str01	db 	"Memory release failed", 13, 10,0 
.next4:
	mov si, .dump
	call strcmpcmd
	jne .next5
	push es
	mov si, .lstr_memdump
	call display_string
	call dbg_hex_input_dx
	mov es, dx
	mov si, .lstr_dmoffs
	call display_string
	call dbg_hex_input_dx
	mov di, dx
	mov si, .lstr_dmcount
	call display_string
	call dbg_hex_input_dx
	mov cx, dx
	call dbg_dump_mem
	pop es
	jmp .system_loop
.dump	db	"dump", 0
.lstr_memdump	db	"Dump memory:", 13, 10, "segm: ", 0
.lstr_dmoffs	db	"offs:  ", 0
.lstr_dmcount	db	"bytes: ", 0
.next5:
	mov si, .delay
	call strcmpcmd 
	jne .next6
	;; delay
	mov si, .lstr_delay
	call display_string
	mov ax, 5000
	call delay
	;;
	jmp .system_loop
.delay	db 	"delay", 0
.lstr_delay	db "There will be delay for 5 second...", 13, 10, 0
.next6:
	mov si, .cls
	call strcmpcmd
	je .keya
	mov si, .clear
	call strcmpcmd
	jne .next7
.keya:	mov al, 03h		; set 80x25 16 colors mode
	call set_video_mode
	jmp .system_loop
.cls	db 	"cls", 0
.clear	db	"clear", 0
.next7:
	cmp byte [cmd_buf], 'l'		; "l"oad
	jne .next8
	mov dx, entry
	mov ax, 0
	call find_file_fat12
	mov bx, dx
	call print_fat_entry
	mov si, endl
	call display_string
	
	jmp .system_loop
.next8:
	mov si, .tetris
	call strcmpcmd
	jne .next9
	;call KERNEL_SEGM:PlayTetris
	jmp .system_loop
.tetris db	"tetris", 0
.next9:
	mov si, .ascii
	call strcmpcmd
	jne .nextA
	call show_ascii_table
	jmp .system_loop
.ascii  db	"ascii", 0
.nextA:
	mov si, .cpu_state
	call strcmpcmd
	jne .nextB
	call dbg_window_cpu
	jmp .system_loop
.cpu_state	db	"cpu", 0
.nextB:
	mov si, .keys
	call strcmpcmd
	jne .nextC
.next_key:
	call show_key_info
	cmp al, 'q'
	jne .next_key
	jmp .system_loop
.keys	db	"keys", 0
.nextC:	
	mov si, .color
	call strcmpcmd
	mov byte [text_attr], 0x70
	jne .nextD
.color	db	"white", 0
.nextD:
	jmp .not_found
	;; here should be parameters parsing

	;; try to find executable
	push es
	mov es, word [exe_seg]
	mov di, COM_OFFSET
	mov si, cmd_buf
	call upload_file
	;; if all ok, jump to file
	jc .not_found
	push es
	push di
	retf
	;; return with int 20h
.not_found:
	pop es
	
	;; not found
	mov si, unknown_cmd
	call display_string
	jmp .system_loop
	; end of system body


; ___________________________________________________________________
set_svga:
; tries to set svga video mode
; args: bx - number of svga mode, 108h for 80x60, 10Bh for 132x50
	mov al, 02
	mov ah, 4Fh
	int 10h
	cmp al, 4Fh
	je svga_ret
	mov si, svga_err
	call display_string 
svga_ret:
	retn

; ___________________________________________________________________
strcmpcmd:
	pusha
	mov di, cmd_buf
.next:	cmpsb
	jne .ret
	mov ah, byte [si]
	test ah, ah
	jnz .next
.ret:	popa
	retn

%include "vga.inc"
%include "stdio.inc"
;%include "tetris.inc"

[segment .data]
; ___________________________________________________________________
; _________________ DATA ____________________________________________
; ___________________________________________________________________

; ___________________________________________________________________
; standard messages
welcome		db 	"Welcome to DolphinOS", 13, 10, 0
wlcm_help	db	"type help for help", 13, 10, 0
svga_err	db 	'SVGA not supported', 10, 0
unknown_cmd	db	"Unknown command or file not found", 13, 10, \
			"Type help for command list", 13, 10, 0
msg_prompt	db	13, 10, '>> ', 0

; buffers
cmd_buf		times MAX_CMDLINE_LENGTH 	db 0
dir_path	times MAX_PATH_LENGTH		db 0
path		db	".;/bin;" 
	times PATH_VAR_LENGTH-($-path)	db 0

;;;; TEMP
pointer		dw 	0x0000
entry times 20    db	0

%endif
