import os, shutil, sys, tarfile, os.path

CCFLAGS = "-g -I include -ffreestanding -Wall -Wextra -w -O0"
LDFLAGS = CCFLAGS + ""
LD = "-nostdlib -lgcc -T link.ld -o"
CC = f"i686-elf-gcc {LDFLAGS}"
OUTPUT = f"./bin"
data = []
files = []


def ony_bytes(input_file):
    content = None
    output = [] #0x90, 0x53, 0x45,0x41
    output_c = ""

    # Записываем всё содержимое файла в байтовую строку
    with open(input_file, 'rb') as fh:
        content = fh.read()

    for i in content:
        output.append(i)


    #with open(f'{input_file}.sea', 'wb+') as fh:
    #    fh.write(bytes(output))

    for i in output:
        output_c += str(i)
        output_c += ","

    output_c = output_c[:len(output_c)-1]
    return output_c

def reset(tarinfo):
    tarinfo.uid = tarinfo.gid = 0
    tarinfo.uname = tarinfo.gname = "root"
    return tarinfo

def build_all():
    try:
        shutil.rmtree(OUTPUT, ignore_errors=True)
        os.mkdir(OUTPUT)
        shutil.rmtree("./initrd/", ignore_errors=True)
        os.mkdir("./initrd/")
    except Exception as E:
        print(E)
    print("Building kernel")
    os.system(f"{CC}  -c kernel.c -o {OUTPUT}/kernel.o")
    os.system(f"{CC}  -c starter.s -o {OUTPUT}/starter.o")

    os.system(f"{CC}  -c arch/x86/interrupts.s -o {OUTPUT}/interrupts.o")
    os.system(f"{CC}  -c arch/x86/paging.s -o {OUTPUT}/paging.o")

    os.system(f"{CC}  -c src/mem/pmm.c -o {OUTPUT}/pmm.o")
    os.system(f"{CC}  -c src/mem/vmm.c -o {OUTPUT}/vmm.o")
    os.system(f"{CC}  -c src/mem/kheap.c -o {OUTPUT}/kheap.o")

    os.system(f"{CC}  -c src/drivers/vfs.c -o {OUTPUT}/vfs.o")
    os.system(f"{CC}  -c src/drivers/ramdisk.c -o {OUTPUT}/ramdisk.o")
    os.system(f"{CC}  -c src/drivers/keyboard.c -o {OUTPUT}/keyboard.o")

    os.system(f"{CC}  -c src/io/tty.c -o {OUTPUT}/tty.o")
    os.system(f"{CC}  -c src/io/vgafnt.c -o {OUTPUT}/vgafnt.o")
    os.system(f"{CC}  -c src/io/ports.c -o {OUTPUT}/ports.o")
    os.system(f"{CC}  -c src/io/shell.c -o {OUTPUT}/shell.o")

    os.system(f"{CC}  -c src/interrupts/gdt.c -o {OUTPUT}/gdt.o")
    os.system(f"{CC}  -c src/interrupts/idt.c -o {OUTPUT}/idt.o")
    os.system(f"{CC}  -c src/interrupts/tss.c -o {OUTPUT}/tss.o")
    os.system(f"{CC}  -c src/interrupts/syscalls.c -o {OUTPUT}/syscalls.o")

    os.system(f"{CC}  -c src/libk/stdlib.c -o {OUTPUT}/stdlib.o")
    os.system(f"{CC}  -c src/libk/string.c -o {OUTPUT}/string.o")

    os.system(f"{CC}  -c src/sdk/elf.c -o {OUTPUT}/elf.o")
    
    #os.system(f"fasm apps/qemu_log.asm  bin/qemu_log")

    #print(ony_bytes(f"{OUTPUT}/qemu_log"))

    print("Building apps")
    os.system(f"i686-elf-gcc -nostdlib -lgcc -ffreestanding -I apps/include -c apps/examples/C/HelloWorld.c -o {OUTPUT}/HelloWorld.o")
    os.system(f"i686-elf-gcc -nostdlib -lgcc -ffreestanding -I apps/include -c apps/examples/C/sound.c -o {OUTPUT}/beep.o")
    os.system(f"i686-elf-gcc -nostdlib -lgcc -ffreestanding -I apps/include -c apps/examples/C/popsort_int_test.c -o {OUTPUT}/popsort_int_test.o")
    os.system(f"i686-elf-gcc -nostdlib -lgcc -ffreestanding -I apps/include -c apps/examples/C/vesa_Russia.c -o {OUTPUT}/vesa_Russia.o")
    
    os.system(f"i686-elf-gcc -nostdlib -lgcc -T apps/link.ld -o initrd/hi.elf {OUTPUT}/HelloWorld.o")
    os.system(f"i686-elf-gcc -nostdlib -lgcc -T apps/link.ld -o initrd/sort_int.elf {OUTPUT}/popsort_int_test.o")
    os.system(f"i686-elf-gcc -nostdlib -lgcc -T apps/link.ld -o initrd/Russia.elf {OUTPUT}/vesa_Russia.o")
    os.system(f"i686-elf-gcc -nostdlib -lgcc -T apps/link.ld -o initrd/beep.elf {OUTPUT}/beep.o")
    print("Create tar-fs")
    with open("initrd/readme.txt", 'w+') as fh:
        fh.write("""SynapseOS is simple OS by Aren Elchinyan""")


    OBJ = f"kernel.elf {OUTPUT}/kernel.o {OUTPUT}/starter.o {OUTPUT}/pmm.o {OUTPUT}/elf.o  {OUTPUT}/shell.o {OUTPUT}/syscalls.o {OUTPUT}/keyboard.o {OUTPUT}/kheap.o {OUTPUT}/vfs.o {OUTPUT}/ramdisk.o {OUTPUT}/paging.o {OUTPUT}/vmm.o {OUTPUT}/interrupts.o {OUTPUT}/tss.o {OUTPUT}/tty.o {OUTPUT}/vgafnt.o {OUTPUT}/ports.o {OUTPUT}/gdt.o {OUTPUT}/tss.o {OUTPUT}/idt.o  {OUTPUT}/stdlib.o {OUTPUT}/string.o"
    
    #print(ony_bytes('./bin/qemu_log.asm'))
    os.system(f"{CC} {LD} {OBJ}")


def qemu_run():
    print("Testing")
    #os.system("qemu-system-i386  -m 256 -boot d -kernel kernel.elf  -M pc-i440fx-3.1 -serial file:Qemu_log.log -no-reboot")
    os.system("qemu-system-i386 -m 4 -name SynapseOS -soundhw all -cdrom SynapseOS.iso -hda ata.qcow2 -serial  file:Qemu.log -no-reboot")
    input()

def create_env():
    if os.path.exists("ata.qcow2"):
        pass
    else:
        os.system("qemu-img create -f qcow2 -o compat=1.1 ata.qcow2 8G")


if __name__ == "__main__":
    try:
        build_all()
        create_env()
        qemu_run()
    except Exception as E:
        print(E)
        input()