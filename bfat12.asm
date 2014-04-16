;
; 	this is bootloader for FAT12 (nasm)
; 	(c) Dmytro Sirenko, 04-06/04/2010
; Great thanks to ultimite boot-strap loader by Matthew Vea (VnutZ)
; This is beerware; 
; you can copy, modify and steal it without any restrictions :)
;
; Notice: 
;	1) if you read this code for learning, read FAT's spec. first!
;	2) terminology: boot2 means executable to load and jump to, it
;	   may be a kernel or a second-stage bootloader, see variable
;	   boot2 at the end of this file
; ___________________________________________________________________

%include "sysdefs.inc"

USE_MEM	    	 equ 0x0200	; place in memory for temp. data
FAT_ENTRY_SIZE	 equ 0x0020	; sizeof directory entry
BOOT2_SEGM	     equ KERNEL_SEGM	; address in memory for boot2
BOOT2_OFFS  	 equ KERNEL_OFFS	; 
END_CLUSTER_MASK equ 0x0FF8	; marker of last cluster for FAT12

%define DEBUG
%ifdef DEBUG
	%warning "_____________________warning: debug info included"
%endif

[bits 16]

	org 0x0000		; where loader is to be found by bios

entry:
	jmp short begin		; 0x0000 x0001; eb 3c

; ___________________________________________________________________
; 						    BIOS params block

brINT13flag	db 	90h	; 0x0002
brOEM		db	'MSDOS5.0'
brBPS		dw 	512	; bytes per sector
brSPC		db	1	; sectors/cluster
brResCount	dw	1	; reserved sectors
brFATs		db	2	; FAT copies
brRootEntries	dw	00e0h	; root directory entries
brSectorCount	dw	0B40h	; sector in volume
brMedia		db	0xF0	; media descriptor
brSPF		dw	9	; sector per FAT
brSPH		dw	18	; sector per track
brHPC		dw	0x0002	; number of heads
brHidden	dd	0	; hidden sectors
brSectors	dd	0	; [0x0020]
brDriveNum	db	0x00	; phys. drive no
brExtBootSig	db	29h	; extended boot record sig
brSerialNum	dd	350518E3h
brLabel		db	'DolphinOS  '	; volume id, 11 chars 
brFSID		db	'FAT12   '	; fs id, 8 chars

; ___________________________________________________________________
;  0x003E							 code
begin:
	cli
	; adjust code position
	mov ax, 0x07C0			;; base of loader code, phys. addr = 0x07c00
	mov ds, ax
	mov es, ax
	mov fs, ax
	; create stack
	xor ax, ax			;; stack at 0x0FFFF
	mov ss, ax
	mov sp, 0xFFFF
	sti	

	mov si, welcome			;; says 'hello'
	call display_string 

	;; determine root directory size in sectors
	mov ax, word [brRootEntries]
	shl ax, 5		; == * sizeof entry
	div word [brBPS]
	mov cx, ax			; count for dataSector

	;; find sector of root directory
	mov ax, word [brSPF]
	mul byte [brFATs]	
	add ax, word [brResCount]	; now ax - number of first root sector
	mov word [dataSector], ax
	add word [dataSector], cx	
	;; [dataSector] stores position of first Data (the last FAT part) sector
	
	;; load root directory to memory at 07C0:0200 (behind the bootcode)
	mov bx, USE_MEM			; address to load to
	call LoadSectors
	jnc hang.ok
read_fail:				;; display message: disk reading failed
	mov di, err_read_fail
	call display_string
hang: 	jmp hang
.ok:
	;; now root directory is at 07c0:0200, search through it for boot2
	mov cx, word [brRootEntries]	; counter for loop
	mov di, USE_MEM			; where to begin. bx will be changing
.next_root_entry:
	mov bx, 11			; count of symbols to compare
	mov si, boot2			; what with?
		; compare
		push cx
		push di
		mov cx, 11
	    rep cmpsb
		pop di
		pop cx
	je boot2_found

	add di, FAT_ENTRY_SIZE		; look up next entry
	loop .next_root_entry

	; all root entries passed, no match
	mov si, boot2
	call display_string
	mov si, err_not_found
	call display_string
	jmp hang

boot2_found:
	;; di points to root dir entry for boot2 now
	mov dx, word [di + 0x001A]	; field "cluster" in entry structure
	mov word [cluster], dx
	
	;; load the entire FAT into USE_MEM
	mov cx, word [brSPF]
	mov bx, USE_MEM
	mov ax, word [brResCount]
	call LoadSectors
	jc read_fail

	;; prepare memory
	mov bx, BOOT2_SEGM		; kernel segment may change further
	mov es, bx
	mov bx, BOOT2_OFFS		; don't touch bx except memory handling!
	push bx
	
.next_kernel_cluster:

	mov ax, word [cluster]			 
	call cluster_to_lba			; ax = LBA(cluster)
	xor cx, cx
	mov cl, byte [brSPC]			; read 1 cluster
	pop bx
	call LoadSectors			;; load -------------
	;; LoadSectors increments bx internally
	push bx
	jc read_fail
	
	;; find next cluster
	mov ax, word [cluster]			; value of next cluster
	; mul 3
	mov dx, ax
	shl ax, 1
	add ax, dx
	; div 2
	shr ax, 1
	add ax, USE_MEM
	mov di, ax		; now di - offset of next cluster
	
	mov dx, word [di]	; read next cluster value
	; dx - two bytes: 0xXCCC for even, 0xCCCX for odd cluster
	test byte [cluster], 0x01		; check parity
	jnz .odd
.even:
	and dx, 0x0FFF
	jmp short .enddif;ference :)
.odd:
	shr dx, 4
.enddif:
	mov word [cluster], dx

	;; whether is the last
	cmp dx, END_CLUSTER_MASK	
	jb .next_kernel_cluster			; last cluster of file is read
	
; premature optimization is the root of all evil, huh.
; an only needed sector in memory is good but the worse is better
;;;next_kernel_cluster:
;;;	;; echo
;;;	push bx
;;;	mov ax, 0x0E40
;;;	mov bh, 0
;;;	int 10h
;;;	pop bx
;;;
;;;	;; what sectors do we need to load
;;;	mov ax, word [cluster]
;;;	call cluster_to_lba
;;;
;;;	;; load brSPC sectors of current cluster
;;;	xor cx, cx
;;;	mov cl, byte [brSPC]
;;;	mov dx, cx			; save for next
;;;	call LoadSectors
;;;
;;;	;; determine next position in memory
;;;	mov ax, word [brBPS]
;;;	mul dx				; bx += brSPC * brBPS
;;;	jnc .this_seg 	; overflow, need to read dx, more than 64 Kb
;;;	mov di, es
;;;	add di, dx
;;;	mov es, di	; next segments
;;;.this_seg:
;;;	add bx, ax
;;;
;;;	;; find next cluster of boot2 file
;;;	;; whether it was the last cluster of file
;;;	mov ax, word [cluster]
;;;	and ax, END_CLUSTER_MASK
;;;	cmp ax, END_CLUSTER_MASK	; end cluster is 0xFF8-0xFFF
;;;	je .done
;;;
;;;	;; search in FAT for the next cluster value
;;;	; determine necessary sector of FAT
;;;		; SectorNum = ResSectors + ((ClusterNum * 3)/2)/brBPS
;;;		; mem_offset = ((ClusterNum * 3)/2) % brBPS
;;;	mov ax, word [cluster]
;;;	;mul 3:	 
;;;	mov dx,ax;
;;;	shl ax, 1
;;;	add ax,dx ; wouldn't be better, i suppose
;;;	shr ax, 1
;;;	div word [brBPS]		; remainder is stored in dx
;;;	add ax, word [brResCount]
;;;	; now ax - sector num, dx - offset
;;;	cmp ax, word [fatSector]	; if is already loaded sector
;;;	; fatSector=0 for the first time, sector will be read
;;;	je .already_loaded
;;;	;; read new sector
;;;	mov word [fatSector], ax	; set new position
;;;	push es
;;;	push bx
;;;	mov bx, ds
;;;	mov es, bx
;;;	mov bx, USE_MEM			; [es:bx] - int 13h reqirement
;;;	call LoadSector
;;;	pop es
;;;	pop bx
;;;.already_loaded:
;;;	;; load next chain
;;;	mov di, dx			; a [dx] call prohibited, [edx]/[di] only
;;;	add di, USE_MEM
;;;	mov cx, word [di]	
;;;	; dx - offset, cx contains new 12bit cluster value now
;;;	; - like 0xXCCC (even) or 0xCCCX (odd) because of fucking 12bitness
;;;	; check a parity
;;;	test byte [cluster], 0x01
;;;	jnz .odd_cluster
;;;.even_cluster:
;;;	and cx, 0x0FFF			; 0xXCCC & 0x0FFF = 0x0CCC
;;;	jmp short .no_difference
;;;.odd_cluster:
;;;	shr cx, 0x04			; 0xCCCX >> 4 = 0x0CCC
;;;.no_difference:
;;;	; now cx - cluster number
;;;	mov word [cluster], cx
;;;	jmp next_kernel_cluster	


;; memory look up
;;;%ifdef DEBUG
;;;	mov dl, 'd'				; "d"one
;;;	call display_char
;;;	;; debug: what is in memory?
;;;	mov bx, BOOT2_OFFS
;;;.rep:
;;;	mov dx, [es:bx]
;;;	;; hex output of dx
;;;	mov cx, 4
;;;.for:
;;;	mov dl, ah
;;;	shr dl, 4
;;;	; convert to ascii
;;;	cmp dl, 0x0A
;;;	jl .dec
;;;	add dl, 7 		; abcdef: 'A' - 10 - '0'
;;;.dec:	add dl, 0x30
;;;	call display_char	
;;;	shl ax, 4
;;;	loop .for
;;;	;; next word
;;;	add bx, 0x0002
;;;	cmp bx, 0x0200
;;;	jne .rep
;;;%endif

	;; jump to destination at last :)	
	push word BOOT2_SEGM
	push word BOOT2_OFFS
	retf
	
; ___________________________________________________________________
LoadSector:
; loads 1 sector at FAT's [ax] (LBA address) into memory at [es:bx].
; doesn't preserve cx, dx
; out: cf = 0, loaded sector in [es:bx] if ok, cf = 1 otherwise

	mov di, 5		; retries 5 times, due to hardware reasons
.effort:
	push bx
	push ax
	call lba_to_chs
	mov ax, 0x0201		; ah = 02h - BIOS func for int 13h: read sector
				; al = 01h, just an only sector
	mov ch, byte [absTrack]
	mov cl, byte [absSector]
	mov dh, byte [absHead]
	mov dl, byte [brDriveNum]
	int 13h
	test al, al
	jz .fail
	pop ax
	pop bx
	jnc .ok			; load ok?

	; next effort
.fail:	push ax
	xor ax, ax			; int 13h, AH=0 
	int 13h				; reset disk

	pop ax
	dec di
	jnz .effort			; try again
	stc
.ok:	
	retn

; ___________________________________________________________________
LoadSectors:
; loads cx sectors beginning at ax to memory at es:bx
; args: ax - address of first sector, 
;	bx - memory address to load to, 
;	cx - count of sectors
; out:	
read_sector:			; loop for sectors, 1 per time 
	push cx
	push bx
	call LoadSector
%ifdef DEBUG
	mov dl, '.'
	call display_char
%endif
	jnc load_ok 
	pop bx
	pop cx
	jmp ls_ret
	
load_ok:
	pop bx
	pop cx
	add bx, word [brBPS]		; next position in buffer
	inc ax				; next sector
	loop read_sector		; read next sector
	 
ls_ret:	
%ifdef DEBUG
	mov dl, 10
	call display_char
	mov dl, 13
	call display_char
%endif
	retn

; ___________________________________________________________________
cluster_to_lba:
; converts fat cluster number to lba pointer
; LBA = (cluster - 2) * sectors per cluster
; args: ax - cluster
; out:  ax - lba
	sub al, 0x02
	xor cx, cx
	mov cl, byte [brSPC]
	mul cx
	add ax, word [dataSector]
	retn

; ___________________________________________________________________
lba_to_chs:
; converts LBA sector number to CHS
;  absSector = (logicalSector / sectors_per_track ) + 1
;  absHead   = (logicalSector / sectors_per_track ) mod number_of_heads
;  absTrack  = logicalSector / (sectors_per_track * number_of_heads)	 
; args: ax - logical sector number
; out: 	absSector, absHead, absTrack
	xor dx, dx
	div word [brSPH]	; ax = ax / brSPH
	inc dl			; dx - absSector
	mov byte [absSector], dl
	xor dx, dx
	div word [brHPC]
	mov byte [absHead], dl
	mov byte [absTrack], al 	
	retn

%ifdef DEBUG
; ___________________________________________________________________
display_char:
; dl - ASCII
	push ax
	push bx
	mov al, dl
	mov ah, 0x0E
	mov bx, 0x0007
	int 0x10
	pop bx
	pop ax
	retn
%endif

; ___________________________________________________________________
display_string:
; just put an ASCIIZ string [ds:si] on display using BIOS
	lodsb		; al = [ds:si]
	test al, al	; set zero flag if al = 0
	jz .ret	
	mov ah, 0x0E	; video func 0E
	mov bx, 0x0007	; color
	int 0x10	
	jmp display_string
.ret:   retn

; ___________________________________________________________________
; 								DATA
;
welcome	 	db	'DolphinOS bootloader', 13, 10, 0
err_read_fail	db	"Read failure", 0
err_not_found	db	" not found", 0
boot2	 	db 	"KERNEL  BIN"		; 11 bytes

fatSector	dw 	0x0000
cluster		dw	0x0000
dataSector	dw	0x0000

absSector	db 	0x00
absHead		db 	0x00
absTrack	db	0x00

size	equ	$ - entry
%if size > 510 
	%error "code is too large"
%endif
	times (512 - size - 2) db 0
	db 0x55, 0xAA		; boot signature
