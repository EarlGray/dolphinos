;;; _________________________________________________________________f
find_first_fat12:
;//	zf find_first(ax dir_cluster, dx *entry)
	pusha
	push es
	push ds
	test ax, ax		; 0 - root sector
	jz .root
	call _cluster_to_sector
	jmp .endif
.root:	mov ax, word [rootSector]
.endif:	xor bx, bx
	mov cx, word [cache_seg]
	mov es, cx
	call _read_sector		; read in es:bx
	
	push ds
	push es
	pop ds	
	pop es

	mov si, bx
	mov di, dx
	mov cx, SIZEOF_FAT_ENTRY
    rep movsb 

	pop ds
    	pop es
	popa
	retn

;;; _________________________________________________________________
find_next_fat12:
;//	zf find_next(ax dir_cluster, dx *entry)
	test ax, ax
	jz .root

.root:	
	mov 
	
	retn

;;; _________________________________________________________________


lookup_cluster_value:
;;//	ax get_next_cluster(ax cluster)
;;;	0 if the end of file
	;; find number of FAT sector which contains given cluster
	;; 	sector_for_clustval = bpb.reserved +
       	;;		(clust_num * 3 / 2) / bpb.BpS		
	push ax			;; save cluster num
	; mul by 3
       	mov dx, ax
	shl dx, 1
	add ax, dx
	; 
	shr ax, 1
	div word [bpb.BpS]	
	mov di, dx		;; store remainder 
	add word [bpb.reserved]
	;; load it into CACHE_SEG:0000
	push es
	mov bx, word [cache_seg]
	mov es, bx
	xor bx, bx
	call _read_sector
	;jnc .ok
	mov dx, word [es:di]
	pop es
	pop ax		;; restore cluster number
	;; now dx - 0xXCCC for  or 0xCCCX
	test al, 0x01
	jnz .odd
	and dx, 0x0FFF
	jmp .no_dif
.odd:	shr dx, 4
.no_dif:mov ax, dx
	retn

;;; ;;;;;;;;;;;; Service routines ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; ___________________________________________________________________
read_sectors:
;; args:
;;	ax - num of sector
;; out:
;;	CACHE_SEG:bx - cached image of sector
	call get_cache_of_sector
	jz .read
	retn
	
.read:	call cache_sectors
	retn

; ___________________________________________________________________
cache_sector:
;; args: 
;;	ax	- sector logical address
;;	cx	- count of sectors
;; out:  bx 	- pointer to cache of sector
	;; find place for sectors
	
	retn

; ___________________________________________________________________
get_cache_of_sector:
;; args:
;;	ax - sector num
;; out:
;;	bx - pointer to cache at [CACHE_SEG:di] if cached 
;;			0 and zf if no cached image
	pusha
	;; enumerate chunks of cache:
	push es
	mov bx, CACHE_SEG
	mov es, bx
	mov bx, 0x0004
.next_chunk:
	mov dx, word [es:bx-4]
	;; 0th sector is never cached, so
	test dx, dx
	jz .no_cache
	;; some cached chunk
	cmp dx, ax
	je .ret
	;; go to next
	add bx, [bpb.BpS]
	cmp bx, CACHE_SIZE
	jl .next_chunk
.no_cache:
	xor bx, bx
.ret:	pop es
	popa
	retn


