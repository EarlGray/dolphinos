
BOOT=bfat12.asm
KERN=kernel.asm
KERNEL=KERNEL.BIN
MBR_FILE=boot.mbr
IMG_FILE=dolphin.img
MNT_DIR=dolphin
VBOX_OS=dolphin

default:kernel run 
all: 	boot kernel run

run:
	VirtualBox --startvm $(VBOX_OS)

boot:
	nasm $(BOOT) -o $(MBR_FILE)
	dd if=$(MBR_FILE) of=$(IMG_FILE) conv=notrunc

clean:
	rm $(KERN).lst || /bin/true
	rm $(KERNEL) || /bin/true
	rm $(MBR_FILE) || /bin/true
	rmdir $(MNT_DIR)

image:	
	dd if=/dev/zero of=$(IMG_FILE) bs=1k count=1440
	/sbin/mkfs.msdos -n DOLPHOS $(IMG_FILE)

mount:
	if [ ! -x $(MNT_DIR) ] ; then mkdir $(MNT_DIR); \
	else echo "Already mounted"; exit 0; fi
	sudo mount -t msdos -o loop $(IMG_FILE) $(MNT_DIR) || /bin/true
	echo "Mounted"

umount: 
	sudo umount $(MNT_DIR)
	rmdir $(MNT_DIR)

dump:
	dd if=$(IMG_FILE) bs=1k count=10 | hexdump -C > $(IMG_FILE).hex

kernel:
	nasm $(KERN) -o $(KERNEL) -l $(KERN).lst
	make mount
	sudo cp $(KERNEL) $(MNT_DIR)
	make umount
