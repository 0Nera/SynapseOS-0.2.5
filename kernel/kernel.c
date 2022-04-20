/*
    Ядро SynapseOS

    Ядро является главным модулем операционной системы, без него невозможна работа SynapseOS.
*/

#include <kernel.h>


// Входная точка ядра SynapseOS
void kernel(int magic_number, struct multiboot_info *mboot_info) {
    // Настройка графики
    tty_init(mboot_info);
    
    // Вывод информации о ядр
    tty_printf("\t\tSynapseOS kernel version: %d.%d.%d, Builded: %s\n", 
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,    // Версия ядра 
        __TIMESTAMP__                                   // Время окончания компиляции ядра
        );
    
    // Проверка, является ли сборка дистрибутивом
    if (isDistr) {
        //Вывод информации о дистрибутиве
        tty_printf("\tDistribution \"%s\", version %d.%d.%d",
            dVERSION_MAJOR, dVERSION_MINOR, dVERSION_PATCH
            );
    }

    gdt_init();

    idt_init();


    pmm_init(mboot_info);

    uint32_t initrd_beg = *(uint32_t*) (mboot_info->mods_addr);
    uint32_t initrd_end = *(uint32_t*) (mboot_info->mods_addr + 4);
    tty_printf("initrd_beg = %x initrd_end = %x\n", 
        initrd_beg, initrd_end
        );

    vmm_init();

    kheap_init();

    init_vbe(mboot_info);

    vfs_init();

    initrd_init(initrd_beg, initrd_end);

    syscall_init();

    keyboard_install();
    
    tty_setcolor(VESA_LIGHT_GREY);

    tty_printf("List of files:");
    initrd_list(0, 0);


    ksh_main();
}
