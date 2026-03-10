CC      = gcc
AS      = nasm
LD      = ld

CFLAGS  = -m32 -ffreestanding -fno-builtin -fno-stack-protector \
          -nostdlib -nodefaultlibs -fno-pic -fno-pie \
          -Wall -Wextra -O2

ASFLAGS = -f elf32

LDFLAGS = -m elf_i386 -T linker.ld

C_SRCS  = src/kernel/kernel.c src/drivers/vga.c
ASM_SRCS = src/boot/boot.asm

C_OBJS  = $(C_SRCS:.c=.o)
ASM_OBJS = $(ASM_SRCS:.asm=.o)

KERNEL  = myos.elf
ISO     = myos.iso

all: $(ISO)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

$(KERNEL): $(ASM_OBJS) $(C_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(ISO): $(KERNEL)
	cp $(KERNEL) iso/boot/myos.elf
	grub-mkrescue -o $(ISO) iso

run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 32M

debug: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -m 32M -s -S &
	gdb -ex "target remote :1234" -ex "symbol-file $(KERNEL)"

clean:
	rm -f $(C_OBJS) $(ASM_OBJS) $(KERNEL) $(ISO)
	rm -f iso/boot/myos.elf iso/boot/myos.bin

.PHONY: all run debug clean
