import os, shutil, sys, tarfile, os.path


OUTK        = "bin/kernel"
KD          = "kernel"
KS          = "kernel/src"
x86         = "kernel/arch/x86"
ISODIR      = "isodir/"
INCLUDE_DIR = "kernel/include"

CC          = f"i686-elf-gcc -g -ffreestanding -Wall -Wextra -w -O0 -I kernel/include//"

LDFLAGS     = "-T linker.ld -nostdlib -lgcc"

SYS_OBJ         = f"{OUTK}//kernel.o {OUTK}//sys/elf.o"
ARCH_OBJ        = f"{OUTK}//starter.o {OUTK}//interrupts.o {OUTK}//paging.o"
MEM_OBJ         = f"{OUTK}//mem/pmm.o {OUTK}//mem/vmm.o {OUTK}//mem/kheap.o"
DRIVERS_OBJ     = f"{OUTK}//drivers/vfs.o {OUTK}//drivers/ramdisk.o {OUTK}//drivers/keyboard.o"
IO_OBJ          = f"{OUTK}//io/tty.o {OUTK}//io/vgafnt.o {OUTK}//io/ports.o {OUTK}//io/shell.o"
INTERRUPTS_OBJ  = f"{OUTK}//interrupts/gdt.o {OUTK}//interrupts/idt.o {OUTK}//interrupts/tss.o {OUTK}//interrupts/syscalls.o"
LIBK_OBJ        = f"{OUTK}//libk/stdlib.o {OUTK}//libk/string.o"

OBJ             = f"{SYS_OBJ} {ARCH_OBJ} {MEM_OBJ} {DRIVERS_OBJ} {IO_OBJ} {INTERRUPTS_OBJ} {LIBK_OBJ}"


def build_all():
    print("Building kernel")

    os.system(f"{CC} -c kernel/kernel.c 		        -o {OUTK}//kernel.o")
	os.system(f"{CC} -c {x86}//starter.s 			-o {OUTK}//starter.o")
	os.system(f"{CC} -c {x86}//interrupts.s 		-o {OUTK}//interrupts.o")
	os.system(f"{CC} -c {x86}//paging.s 		    -o {OUTK}//paging.o")

	os.system(f"{CC} -c {KS}//mem/pmm.c 		    -o {OUTK}//mem/pmm.o")
	os.system(f"{CC} -c {KS}//mem/vmm.c 		    -o {OUTK}//mem/vmm.o")
	os.system(f"{CC} -c {KS}//mem/kheap.c 	        -o {OUTK}//mem/kheap.o")

	os.system(f"{CC} -c {KS}//drivers/vfs.c 	    -o {OUTK}//drivers/vfs.o")
	os.system(f"{CC} -c {KS}//drivers/ramdisk.c     -o {OUTK}//drivers/ramdisk.o")
	os.system(f"{CC} -c {KS}//drivers/keyboard.c    -o {OUTK}//drivers/keyboard.o")

	os.system(f"{CC} -c {KS}//io/tty.c 		        -o {OUTK}//io/tty.o")
	os.system(f"{CC} -c {KS}//io/vgafnt.c 	        -o {OUTK}//io/vgafnt.o")
	os.system(f"{CC} -c {KS}//io/ports.c 	        -o {OUTK}//io/ports.o")
	os.system(f"{CC} -c {KS}//io/shell.c 	        -o {OUTK}//io/shell.o")

	os.system(f"{CC} -c {KS}//interrupts/gdt.c      -o {OUTK}//interrupts/gdt.o")
	os.system(f"{CC} -c {KS}//interrupts/idt.c      -o {OUTK}//interrupts/idt.o")
	os.system(f"{CC} -c {KS}//interrupts/tss.c      -o {OUTK}//interrupts/tss.o")
	os.system(f"{CC} -c {KS}//interrupts/syscalls.c -o {OUTK}//interrupts/syscalls.o")

	os.system(f"{CC} -c {KS}//libk/stdlib.c 	    -o {OUTK}//libk/stdlib.o")
	os.system(f"{CC} -c {KS}//libk/string.c 	    -o {OUTK}//libk/string.o")

	os.system(f"{CC} -c {KS}//sys/elf.c 		    -o {OUTK}//sys/elf.o")

	os.system(f"{CC} {LDFLAGS} -o {ISODIR}//boot/kernel.elf {OBJ} ")

if __name__ == "__main__":
    try:
        build_all()

        if os.path.exists("ata.qcow2"):
            pass
        else:
            os.system("qemu-img create -f qcow2 -o compat=1.1 ata.qcow2 8G")

        if sys.platform == "linux" or sys.platform == "linux2":
            os.system("grub-mkrescue -o SynapseOS.iso {ISODIR)")
        else:
            os.system("wsl grub-mkrescue -o SynapseOS.iso {ISODIR)")
        
        os.system("qemu-system-i386 -m 4 -name SynapseOS -soundhw all -cdrom SynapseOS.iso -hda ata.qcow2 -serial  file:Qemu.log -no-reboot")
    except Exception as E:
        print(E)