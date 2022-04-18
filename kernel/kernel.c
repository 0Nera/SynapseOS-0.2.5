/*
    Ядро SynapseOS

    Ядро является главным модулем операционной системы, без него невозможна работа SynapseOS.
*/

#include <kernel.h>


// Входная точка ядра SynapseOS
void kernel(int magic_number, struct multiboot_info *mboot_info) {
    // Настройка графики
    tty_init(mboot_info);
    
    // Вывод информации о ядре
    tty_printf("\t\tSynapseOS kernel version: %d.%d.%d, Builded: %s\n", 
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
        __TIMESTAMP__
        );
    // Проверка, является ли сборка дистрибутивом
    if (isDistr) {
        //Вывод информации о дистрибутиве
        tty_printf("\tDistribution \"%s\", version %d.%d.%d",
            dVERSION_MAJOR, dVERSION_MINOR, dVERSION_PATCH
            );
    }

    gdt_init();             //  \ 
    idt_init();             //  / Установка и настройка прерываний 
    pmm_init(mboot_info);   // Инициализация менеджера физической памяти

    uint32_t initrd_beg = *(uint32_t*) (mboot_info->mods_addr);     //  Получение начала памяти рамдиска
    uint32_t initrd_end = *(uint32_t*) (mboot_info->mods_addr + 4); //  Получение конца памяти рамдиска

    vmm_init();                         // Инициализация менеджера виртуальной памяти
    kheap_init();                       // Инициализация "кучи" ядра
    init_vbe(mboot_info);               // Инициализация графики
    vfs_init();                         // Инициализация виртуальной файловой системы
    initrd_init(initrd_beg, initrd_end);// Инициализация рамдиска
    syscall_init();                     // Установка сисфункций
    keyboard_install();                 // Установка драйвера PS/2 клавиатуры
    
    tty_setcolor(VESA_LIGHT_GREY);      // Смена цвета в консоли на светло-серый

    tty_printf("List of files:");       // \ Вывод списка файлов
    initrd_list(0, "  ");               // /

    ksh_main();                         // Активация консольного интерпретатора
}
