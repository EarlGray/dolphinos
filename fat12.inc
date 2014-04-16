; 
;	This file contains routines for handling FAT12/16 filesystems
; 	(c) Dmytro Sirenko, 2010
; ___________________________________________________________________
;		Unit description
; 	Interface;
; fmount_fat12 : - reads needed data for fat12 filesystem
; fopen_fat12 
;
; ===================================================================

%ifndef __FAT12
%define	__FAT12

%include "sysdefs.inc"
%include "stdio.inc"
%include "debug.inc"

[segment .data]

;;; Global FAT data
;;; _________________________________________________________________
;; _____	BIOS parameter block	_____________________________
bpb:
.oem_name times 8	db	0x00	; oem name, 8 chars		0x00
.BpS			dw	0x0000	; bytes per sector		0x08	
.SpC			db	0x00	; sectors per cluster		0x0A
.reserved		dw	1	; reserved sectors count	0x0B
.FATs			db	2	; count of FAT copies		0x0D
.rtentr			dw	0x0000	; count of root entries		0x0E
.ttl_sctrs		dw	0x0000	; total sectors count		0x10
.type			db	0xF0	; media type			0x12	
.SpF			dw	0x0000	; sectors per FAT		0x13
.SpH			dw	0x0000	; sectors per head		0x15
.HpC			dw	0x0000	; number of heads		0x17
.hddn_sctrs		dd	0	; hidden sectors		0x19
.ttl_sctrs32		dd	0 	; 32bit ttl_sctrs count		0x1D

sizeof_bpb	equ	$ - bpb		;				0x21

;; _____	Extended bpb for FAT12/16	_____________________
ebpb1x:
.drive_num		db	0	; phys. drive number
.ext_boot_sig		db	0x00	; signature of three foll. fields
.serial			dd	0	; serial number of device
.label	times 11	db	0x00	; volume id, 11 chars 
.fs_id	times 8		db	0x00	; fs id, 8 chars

sizeof_ebpb	equ	$ - ebpb1x

;;;; _____ 	Extended bpb for FAT32 	 	_____________________	

;; _____________         FAT entry		_____________________
SIZEOF_FAT_ENTRY	equ	20
		
;;; temporarily!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
fopen 		equ 	fopen_fat12
fclose		equ	fclose_fat12
fmount		equ	fmount_fat12
fread		equ	fread_fat12

chs:
.track		db	0x00
.head		db	0x00
.sector		db	0x00

rootSector	dw	0x0000
dataSector	dw	0x0000

; _________  messages  ______________________________________________
fat_welcome	db  "Mounting FAT...", 10, 13, 0
err_fat_missing	db  13, "Error: FAT12 header is missing", 10, 13, 0


[segment .text]
;;; ;;;;;;;;;;; Unit Interface ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; _________________________________________________________________
fmount_fat12:
;;;// 	__far void fmount_fat12(void)
;;; initializes: fs type, cluster size and so on
;;; args: 

	mov si, fat_welcome
	call display_string
	
	;; copy data to bpb 
	push ds			; save ds
	; from:
	push BOOT_SEG		
	pop ds			

	mov cx, sizeof_bpb	; count
	mov si, BOOT_MEM + 3	
	; bpb structure beginning is located at byte 0x03 in 0th sector
	; to:
	; mov es, KERNEL_SEGM - is set already 
	mov di, bpb		; offset
	call KERNEL_SEGM:memcopy
	
	mov cx, sizeof_ebpb
	mov si, BOOT_MEM + 3 + sizeof_bpb
	mov di, ebpb1x
	call KERNEL_SEGM:memcopy
	
	pop ds			; restore ds

	
	mov dx, [bpb.BpS]
	call dbg_print_word_dx

	;; calculate dataSector
			; dataSector = reserved + fat_count * SpF +
			;		+ (root_entr_count * 20h)/BpS;
	movzx ax, byte [bpb.FATs]
	mul word [bpb.SpF]
	add ax, [bpb.reserved]
	mov word [rootSector], ax
	mov dx, ax
	
	; root size
	mov ax, [bpb.rtentr]
	shl ax, 5			; == mul 20h == mul sizeof(entry)
	div word [bpb.BpS]
	add dx, ax
	mov word [dataSector], dx

	call dbg_print_word_dx

	;; init cache
	call cache_init
	
	retn			; can be called from anywhere, so far ret

;; __________________________________________________________________
cache_init:
	xor ax, ax
	push es
	push word [cache_seg]
	pop es
	mov [es:0000], ax
	mov [es:0002], ax
	pop es
	retn

;;; _________________________________________________________________
fopen_fat12:
;;; args:
;;;	ds:si	- file name
;;;	al	- flags (E_READ | E_WRITE | E_APPEND)
;;; out:
;;;	ax - 	file descriptor, 0 if failed, errcode in bx
;;;	
	;; find file
	; parse path
	
	
	;; 
	
	retn

;;; _________________________________________________________________
fclose_fat12:
	retn
	
;;; _________________________________________________________________
fread_fat12:
	retn


; ___________________________________________________________________
print_fat_entry:
;; void print_fat_entry(bx *entry)
	;mov si, .endl
	;call display_string
	mov di, 0
.name_loop:
	mov al, byte [bx + di]
	cmp al, ' '
	je .end_loop
	call out_char
	inc di
	cmp di, 8
	jl .name_loop
.end_loop:
	cmp byte [bx + 8], ' '
	je .no_ext
	mov al, '.'
	call out_char

	mov di, 0
.ext_loop:
	mov al, byte [bx + di + 8]
	cmp al, ' '
	je .no_ext
	call out_char
	inc di
	cmp di, 3
	jl .ext_loop
.no_ext:
	retn

;; __________________________________________________________________
cache_file:
;; 	
	retn

;; __________________________________________________________________
cache_cluster:
;; args:
;;	ax - number of cluster to be cached
;; out:
;;	CACHE_SEG:bx - adrress of mem image
		
	retn

;; __________________________________________________________________
cache_sectors:
;; args:
;;	ax - number of first sector to be cached
;;	cx - count of sectors 
;; rets:
;;	[cache_seg]:bx - address of mem image
		;;; 
	push ax
	push cx
	call get_sector_cache

	pop cx
	pop ax
	test bx, bx
	jnz .ret	;; already loaded
		;;; we have to load it
		;;; find a place for cache
	call get_place_for_cache
	;; now bx - place for sector
	push es

.load_sector:
	call _read_sector
	add bx, word [bpb.BpS]
	inc ax
	loop .load_sector
	pop es
.ret:	retn


;;; _________________________________________________________________
get_place_for_cache:
;; args:
;;	ax - count of sectors
;; rets:
;;	bx - pointer to place

	push es
	mov bx, word [cache_seg]
	mov es, bx
	mov bx, SIZEOF_CACHE_HEADER
	push ax
.find_cache:
	mov dx, word [es: bx-4]
	test dx, dx
	jz .found
	movzx ax, byte [es: bx-2]
	mul word [bpb.BpS]
	add bx, ax
	add bx, SIZEOF_CACHE_HEADER
	jmp .find_cache
.found:	;; is there enough space for new cache?
	pop ax
	; is CACHE_SIZE - 4 - bx > cx * BpS?
	push ax
	mov ax, cx
	mul word [bpb.BpS]
	push bx
	sub bx, CACHE_SIZE - 4
	neg bx
	cmp bx, ax
	pop ax
	jge .enough
	call cache_init
	;; go to first cache entry
	mov bx, SIZEOF_CACHE_HEADER
.enough:pop es
	retn

;;; 	Cache:
;;;   	|--|-----------------|--|--------|00|00000...
;;;	 d1   sector cache 1  d2  cache2   end of cache - 
;;;	d1, d2 - descriptors               zero descriptor	
;;				     v	
;; 	|-----|----------|-----------|------------|
;;	 cache    sect0       sect1  ^    sect2
;;       descr<-------- cx --------->bx 
;; cache desriptor:
;;	|--------|--------|--------|--------|
;;	 <--- 1stSecNum -> <------> <------>
;;			      SC      CS
;;	SC - sectors count after 1st
;;	CS - Check sum


;; __________________________________________________________________
get_sector_cache:
;; args:
;;	ax - sector 
;; rets:
;;	CACHE_SEG:bx - pointer, 0 if not found (first cache lays at 0x0004)
;;	cx - number of this sector in the cache sequence
SIZEOF_CACHE_HEADER	equ	4
	push es
	push word [cache_seg]
	pop es
	mov bx, SIZEOF_CACHE_HEADER
.next_cache:
	mov cx, word [es: bx - 4]
	test cx, cx
	jz .endofcache	;; 0th sector is never cached, cache ends with 0
	movzx dx, byte [es: bx - 2]
	cmp ax, cx
	jl .not_this	;; cache contains older sectors than one being searching for
	add dx, cx
	cmp ax, dx
	jge .not_this	;; given sector is older than ones in cache
	;; yes, this cache contains sought sector
	sub ax, cx
	mov cx, ax	;; cx - number of this sector in cached sequence
	jmp .ret	
.not_this:
	;; go to next cache
	push ax
	movzx ax, byte [es : bx - 2]
	mul word [bpb.BpS]
	add bx, ax
	add bx, SIZEOF_CACHE_HEADER
	;; cache must end with zero descriptor, so don't check the end of cache memory
	pop ax
	jnz .next_cache
.endofcache:
	;; sector not found
	xor bx, bx
.ret:	pop es
	retn
;;
; ___________________________________________________________________
get_path_cluster:
;//	ax get_path_cluster(ds:si dir_path)
	
	retn

; ___________________________________________________________________
get_first_cluster:
;//	ax get_first_cluster(ds:si name, ax dir_cluster)
; 
DIR_MEM	equ	((CACHE_SIZE << 4) - 0x300)
	test ax, ax
	jnz .datapart
	;; root dir
	mov ax, word [rootSector]
.datapart:
	;;
	push es
	mov di, word [cache_seg]
	mov es, di
	mov bx, DIR_MEM
	call _cluster_to_sector
	movzx cx, [bpb.SpC]
	call upload_sectors
	pop es
	;;

	retn


; ___________________________________________________________________
get_next_cluster:
;; ax get_next_cluster(ax cluster)
	
	retn

; ___________________________________________________________________
find_file_fat12:
;; args:
;;	ax 	- dir_cluster, 0 if root
;;	ds:si 	- name (11)
;;  out dx	- pointer to entry where to copy file info (20h bytes)
;;			0 if not found
	pusha
	mov word [.dir_cluster], ax
	jz .root
	;; read directory:
	jmp .ret
.root:	;; read root:
	mov cx, [bpb.rtentr]
	mov bx, 0h
.next_rootentry:
	cmp bx, word [bpb.BpS]
	jl .no_load
.no_load:
	add bx, SIZEOF_FAT_ENTRY
	loop .next_rootentry
	;; not found
	xor dx, dx

.ret:	popa
	retn
.dir_cluster	dw 	0x0000

; ___________________________________________________________________
fat12_internal_name:
;; returns e.g. "KERNEL  BIN" (11 chars) from "Kernel.bin",0
;; redundant symbols (>8th for name and >3 for ext) are ignored
;; spaces is always padding!
;; args:
;;	ds:si - pointer to usual name
;; 	ds:bx - pointer to 11 bytes buffer			
;;							TODO: do it!!
	pusha
	mov cx, 8
.next_name_symb:
	mov al, byte [si]
	;; check whether is valid
	test al, 0x80
	jnz .valid_symb
	;bt fat12_valid_symbs, al
	jnc .invalid_symb
.valid_symb:
	;; valid
	;; transform to uppercase if needed
	mov byte [bx], al
	
	loop .next_name_symb
.invalid_symb:
	cmp al, '.'
	je .ext
	cmp al, 0
	jne .invfname
	
.ext:
	
.invfname:
	popa
	mov ax, E_INVALID_FILE_NAME
	retn
.ok:
	popa
	mov ax, NO_ERROR
	retn
;; bitfield for valid ascii (0-127) symbols in fat names
fat12_valid_symbs db	0x00, 0x00, 0x00, 0x00, 0x6F, 0xC4, \
			0xFF, 0xC0, 0xFF, 0xFF, 0xFF, 0xE3, \
			0xFF, 0xFF, 0xFF, 0xF6

; ___________________________________________________________________
upload_sectors:
;; args:
;;	ax - lba address of first sector
;;	es:bx - pointer where to load
;;	cx - count
;; outs:
;;	cf = 0 if success
;; changes bx, ax, cx
	
.next_sector:
	call _read_sector
	jc .ret
	inc ax
	add bx, word [bpb.BpS]
	loop .next_sector
.ret	retn

; ___________________________________________________________________
upload_file:
;; args:
;;	ds:si - string of file name
;;	es:di - memory to load in
;;	ax - cluster of directory, 0 for root
;; outs:
;;	cf = 0 if success
	
	call get_first_cluster

	mov bx, di
.next_cluster:
	;; load cluster
	push ax
	call _cluster_to_sector
	movzx cx, byte [bpb.SpC]
	call upload_sectors
	jc .ret
	pop ax
	;; last?
	cmp ax, 0x0FF8
	jge .ret
	;; find next
	call get_next_cluster
	jmp .next_cluster
.ret:	retn

; ___________________________________________________________________
_cluster_to_sector:
_cluster_to_lba:
; converts fat cluster number to lba pointer
; LBA = (cluster - 2) * sectors per cluster
; args: ax - cluster
; out:  ax - lba
;// ax _cluster_to_sector(ax cluster)
	sub al, 0x02
	xor cx, cx
	mov cl, byte [bpb.SpC]
	mul cx
	add ax, word [dataSector]
	retn

; ___________________________________________________________________
_lba_to_chs:
; converts LBA sector number to CHS
;  absSector = (logicalSector / sectors_per_track ) + 1
;  absHead   = (logicalSector / sectors_per_track ) mod number_of_heads
;  absTrack  = logicalSector / (sectors_per_track * number_of_heads)	 
; args: ax - logical sector number
; out: 	chs	
	xor dx, dx
	div word [bpb.SpH]	; ax = ax / brSPH
	inc dl			; dx - absSector
	mov byte [chs.sector], dl
	xor dx, dx
	div word [bpb.HpC]
	mov byte [chs.head], dl
	mov byte [chs.track], al 	
	retn

; ___________________________________________________________________
_read_sector:
; loads 1 sector at FAT's [ax] (LBA address) into memory at [es:bx].
; out: cf = 0, loaded sector in [es:bx] if ok, cf = 1 otherwise
; preserves all registers
	pusha
	mov di, 5		; retries 5 times, due to hardware reasons
.effort:
	push bx
	push ax
	call _lba_to_chs
	mov ax, 0x0201		; ah = 02h - BIOS func for int 13h: read sector
				; al = 01h, just 1
	mov ch, byte [chs.track]
	mov cl, byte [chs.sector]
	mov dh, byte [chs.head]
	mov dl, byte [ebpb1x.drive_num]
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
.ok:	popa
	retn

%endif
