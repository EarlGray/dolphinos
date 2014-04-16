%ifndef __CSTRING_INC__
%define __CSTRING_INC__

; ___________________________________________________________________
strcmp:
; compare bx symbols at [ds:si] with [es:di]
; out : set zf if equal
	push di
	inc bx
cmp_iter:
	dec bx
	jz cmp_ret
	cmpsb
	je cmp_iter		; it's a kind of magic, nyah
cmp_ret:
	pop di
	retf

; ___________________________________________________________________
strncmp:
;//	zf strncmp(ds:si str1, es:di str2, cx n)
    rep cmpsb
	retf


%endif
