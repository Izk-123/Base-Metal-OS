CC = gcc
CFLAGS = -ffreestanding -m64 -nostdlib
LD = ld

all: os.iso

kernel.bin:
	nasm -f elf64 kernel/entry.asm -o entry.o
	$(CC) $(CFLAGS) -c kernel/kernel.c -o kernel.o
	$(LD) -T linker.ld entry.o kernel.o -o kernel.bin

os.iso: kernel.bin
	mkdir -p iso/boot/grub
	cp kernel.bin iso/boot/kernel.bin
	cp boot/grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o os.iso iso

run:
	qemu-system-x86_64 -cdrom os.iso