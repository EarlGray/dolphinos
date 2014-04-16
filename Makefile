
BOOT=bfat12.asm
KERN=kernel.asm
KERNEL=KERNEL.BIN
MBR_FILE=boot.mbr
IMG_FILE=dolphin.img
MNT_DIR=dolphin
VBOX_OS=dolphin

default:kernel run 
all: 	image boot kernel run clean

run:
	@echo "### Starting DolphinOS..."
	VirtualBox --startvm $(VBOX_OS)

boot:
	@echo "### Compiling bootloader..."
	@nasm $(BOOT) -o $(MBR_FILE) && echo "# Bootloader compiled..."
	@dd if=$(MBR_FILE) of=$(IMG_FILE) conv=notrunc

clean:
	@rm $(KERN).lst || /bin/true
	@rm $(KERNEL) || /bin/true
	@rm $(MBR_FILE) || /bin/true
	@rmdir $(MNT_DIR) || /bin/true
	@echo "### Build is cleaned."

image:	
	@echo -e "\n\n### Creating and formatting image..."
	@dd if=/dev/zero of=image.img bs=1k count=1440
	@/sbin/mkfs.msdos -n DOLPHOS image.img

mount:
	@if [ ! -x $(MNT_DIR) ] ; then mkdir $(MNT_DIR); \
	else echo "### Already mounted"; exit 0; fi
	@sudo mount -t msdos -o loop $(IMG_FILE) $(MNT_DIR) || /bin/true
	@echo "### Mounted successfully..."

umount: 
	@sudo umount $(MNT_DIR) || /bin/true
	@rmdir $(MNT_DIR)
	@echo "### Unmounted"

dump:
	@dd if=$(IMG_FILE) bs=1k count=10 | hexdump -C > $(IMG_FILE).hex

kernel:
	@echo -e "\n### Compiling kernel..."
	@nasm $(KERN) -o $(KERNEL) && echo "### The kernel compiled successfully..."
	@make mount
	@sudo cp $(KERNEL) $(MNT_DIR) && echo "### The kernel has been installed..." || /bin/true
	@make umount
