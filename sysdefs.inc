%ifndef __SYS_DEFS_INC
%define __SYS_DEFS_INC

MAX_CMDLINE_LENGTH	equ 	256
MAX_PATH_LENGTH		equ	256
PATH_VAR_LENGTH		equ	256

VGAMEM_SEGM	equ 	0xA000
TXTBUF_SEGM	equ	0xB800
VGA13h_MAX_X	equ	320
VGA13h_MAX_Y	equ	200

KERNEL_SEGM	equ	0x0100
KERNEL_OFFS	equ	0x0000

STACK_SEGM	equ 	0x9000

BOOT_SEG	equ	0x07c0
BOOT_MEM	equ	0x0000

HEAP_SIZE	equ	0x1000		;; 
CACHE_SIZE	equ	0x0800		;; 
STACK_SIZE	equ	0x1000		;; 

COM_OFFSET	equ	0x0100

;SCREEN_WIDTH	equ	80
;SCREEN_HEIGHT	equ	25

;; __________   KEYBOARD   __________________________________________
;; state at 0x00417
KB1_RIGHT_SHIFT	equ	0x01
KB1_LEFT_SHIFT	equ	0x02
KB1_ANY_CTRL	equ	0x04
KB1_ANY_ALT	equ	0x08
KB1_SCRLK_ON	equ	0x10
KB1_NUMLK_ON	equ	0x20
KB1_CPSLK_ON	equ	0x40
KB1_INSERT_ON	equ	0x80

;; state at 0x00418
KB2_LEFT_CTRL_PRESSED	equ	0x01
KB2_LEFT_ALT_PRESSED	equ	0x02
KB2_RIGHT_CTRL_PRESSED	equ	0x04
KB2_RIGHT_ALT_PRESSED	equ	0x08
KB2_SCRLK_PRESSED	equ	0x10
KB2_NUMLK_PRESSED	equ	0x20
KB2_CPSLK_PRESSED	equ	0x40
KB2_SYSRQ_PRESSED	equ	0x80



; _________ fopen  function params  _________________________________
F_READ		equ	0x01
F_WRITE		equ	0x02
F_APPEND	equ	0x03

; _________  fopen error codes  _____________________________________
E_NOT_FOUND	equ	0xFF
E_READONLY	equ	0xFE

E_INVALID_FILE_NAME equ	0xF0

NO_ERROR	equ	0

%endif
