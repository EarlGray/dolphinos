;
;
; 

%ifndef __ACPI_INC__
%define __ACPI_INC__

%include "stdio.inc"

[section .text]

; ___________________________________________________________________
turnoff:
; turn off power for all bios-managed devices
; args: none
	mov ax, 5307h	; bios power management
	mov bx, 0001h	; all devices, where power managed by bios
	mov cx, 0003h	; state: off
	int 0x15
	mov si, m_off_failed
	call print_string
.hang:	hlt
	jmp .hang

%endif ;; __ACPI_INC__
