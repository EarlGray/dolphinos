
%ifndef __STR_RESOURCES_INC__
%define __STR_RESOURCES_INC__

[section .data]

endl:			db	13,10,0
m_system_welcome:	db	13, 10, "     Welcome to DolphinOS", 13, 10, 10, 0

m_dummy_intr:		db	": this function is not implemented", 13, 10, 0
m_press_any:		db	13, 10, "Press any key to continue...", 13, 10, 0
m_unknown_cmd:		db	"Unknown command", 13, 10, 0
m_sh_exiting:		db	"Shell exited...", 13, 10, 0
m_off_failed:		db	13, 10, "Now you can turn computer off", 0

def_prompt:		db	">> ", 0

mdbg_sys_mem:		db	"System memory size in blocks: ", 0

%endif
