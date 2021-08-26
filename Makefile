CC = gcc
CFLAGS = -m32 -g -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles  -nodefaultlibs -std=gnu99
AS = nasm
ASFLAGS = -felf
INCLUDE = ./include
LD = ld
LDFLAGS = -T link.ld -melf_i386

OBJECTS =   objects/loader.o 	\
			objects/x86/gdt.o \
			objects/x86/idt.o \
			objects/x86/exc.o \
			objects/x86/tss.o \
			objects/x86/isr_wrap.o \
			objects/x86/exc_wrap.o \
			objects/x86/spinlock.o \
			objects/sys/syscall.o \
			objects/sys/sys_wrap.o \
			objects/process/process.o \
			objects/process/schedule.o \
			objects/pci/pci.o \
			objects/memory/memory.o \
			objects/manager/device.o \
			objects/kernel/kernel.o \
			objects/ipc/semaphore.o \
			objects/ipc/shm.o \
			objects/fs/efs2.o \
			objects/fs/termios.o \
			objects/fs/vfs.o \
			objects/fs/dcache.o \
			objects/fs/devfs.o \
			objects/driver/ata.o \
			objects/driver/io.o \
			objects/driver/kbd.o \
			objects/driver/rtc.o \
			objects/driver/serial.o \
			objects/driver/rtl8139.o \
			objects/net/ethernet.o 	\
			objects/net/ip.o 	\
			objects/net/tcp.o 	\
			objects/net/udp.o 	\
			objects/net/arp.o 	\
			objects/net/icmp.o  \
			objects/net/socket.o
			
objects/%.o: src/%.c
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) -I$(INCLUDE) -D__DEBUG__ -o $@ $<

objects/%.o: src/%.s
	mkdir -p $(@D)
	$(AS) $(ASFLAGS) -o $@ $<

kernel.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

vm: kernel.elf
	cp $< iso/boot;	\
	genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 500 -A os -input-charset utf8 -quiet -boot-info-table -o eos.iso iso; \
	qemu-system-i386 -boot d -cdrom eos.iso -m 1024 -hda hda.img & 

clean: $(OBJECTS)
	rm -r objects/*