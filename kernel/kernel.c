/*
    Ядро SynapseOS

    Ядро является главным модулем операционной системы, без него невозможна работа SynapseOS.
*/

#include <kernel.h>


// Входная точка ядра SynapseOS
void kernel(int magic_number, struct multiboot_info *mboot_info) {
    
    tty_init(mboot_info);

    tty_printf("SynapseOS %d.%d.%d\n", 
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH
        );
    
    // Проверка, является ли сборка дистрибутивом
    if (isDistr) {
        //Вывод информации о дистрибутиве
        tty_printf("\tDistribution \"%s\", version %d.%d.%d",
            dVERSION_MAJOR, dVERSION_MINOR, dVERSION_PATCH
            );
    }

    qemu_putstring("GDT INIT\n");
    gdt_init();

    qemu_putstring("IDT INIT\n");
    idt_init();


    qemu_putstring("PMM INIT\n");
    pmm_init(mboot_info);

    uint32_t initrd_beg = *(uint32_t*) (mboot_info->mods_addr);
    uint32_t initrd_end = *(uint32_t*) (mboot_info->mods_addr + 4);
    tty_printf("initrd_beg = %x initrd_end = %x\n", 
        initrd_beg, initrd_end
        );

    qemu_putstring("VMM INIT\n");
    vmm_init();

    qemu_putstring("KHEAP INIT\n");
    kheap_init();

    qemu_putstring("VBE INIT\n");
    init_vbe(mboot_info);

    qemu_putstring("VFS INIT\n");
    vfs_init();

    qemu_putstring("RAMDISK INIT\n");
    initrd_init(initrd_beg, initrd_end);

    qemu_putstring("SYSCALL INIT\n");
    syscall_init();

    qemu_putstring("KEYBOARD INIT\n");
    keyboard_install();
    
    tty_setcolor(VESA_LIGHT_GREY);

    tty_printf("List of files:");
    initrd_list(0, "  ");


    ksh_main();
}
