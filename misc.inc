;
;	Miscel
;	DolphinOS	(c) Dmytro sirenko, 2010
; ___________________________________________________________________

%ifndef __MISC_STUFF_INC__
%define __MISC_STUFF_INC__

%macro AL_HEX_TO_ASCII 0
	cmp 	al, 10
	sbb 	al, 0x96
	das
%endmacro

%endif
