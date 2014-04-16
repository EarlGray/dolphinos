;
;	Pseudographics routines
;	DolphinOS (c) Dmytro Sirenko
; ___________________________________________________________________

%ifndef __PSEUDOGRAPHICS_INC
%define __PSEUDOGRAPHICS_INC

section .data

psgr:
;; corners
.drt	db	0xBB		;; double right-top
.dlt	db	0xC9		;; 	  left-top
.drb	db	0xBC		;; double right-bottom
.dlb	db	0xC8		;; 	
.srt	db	0xBF
.slt	db	0xDA
.srb	db	0xD9
.slb	db	0xC0

;; lines
.svl	db	0xB3		;; single vertical line
.shl	db	0xC4		;; 	horisontal
.dvl	db	0xBA
.dhl	db	0xCD

;; bars
.ff	db 	0xDB		;; fully filled rect
.hdf	db	0xDC		;; down-half filled rect
.huf	db	0xDF		;; up-half
.hrf	db	0xDE		;; right-half
.hlf	db	0xDD		;; left-half

section .text

;; __________________________________________________________________
psgr_draw_dvl:
;; bh:bl - y:x, cx - length
	mov al, [psgr.dvl]
	
	call put_char
	retn

;; __________________________________________________________________
psgr_dframe:
;; draw double-lined frame
	retn

%endif
