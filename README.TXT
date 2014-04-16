DolphinOS v0.01

This version is the first working version of DOS-like OS with a valid shell.
I wrote it studying intel assembler language and considering neither the system 
architecture and design, nor compatibility with existing systems, just extending 
bare bones with an every new feature I found in BIOS possibilities.
There's a lot of dirty code and hacks, don't use it as a model.

This version was written for a few weeks, starting with bootloader (3 days in 
Feb, 2010). 

Features:
- FAT12 bootloader;
- simplest shell with internal commands (type help to list it);
- a lot of useless but nice features :) like neat clock at the screen and so on;
- there is no memory manager, no disk driver, no filesystem; don't expect to 
encounter them :)

