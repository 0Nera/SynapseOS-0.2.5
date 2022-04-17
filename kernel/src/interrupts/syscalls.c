#include <kernel.h>



uint32_t sc_puts(char *str) {
    tty_printf("%s", str);
    return (uint32_t)0;
}

uint32_t sc_getscancode() {
    return keyboard_getscancode();
}

uint32_t sc_getchar() {
    return keyboard_getchar();
}

uint32_t sc_gets() {
    return keyboard_gets();
}

uint32_t sc_putpixel(int x, int y, uint32_t color) {
    set_pixel(x, y, color);
    return (uint32_t)0;
}

void syscall_init() {
    register_interrupt_handler(SYSCALL_IDT_INDEX, &syscall_handler);
}

void syscall_handler(struct regs *r) {
    uint32_t result = -1;

    uint32_t* argptr = (uint32_t*) (r->ebx);


    switch (r->eax) {
        case SC_CODE_puts:
            //tty_printf("str = %x\n", (char*) (argptr[0]));
            result = sc_puts((char*) (argptr[0]));
            break;
        case SC_CODE_getscancode:
            result = sc_getscancode();
            break;
        case SC_CODE_putpixel:
            result = sc_putpixel((int) (argptr[0]), (int) (argptr[1]), (uint32_t)(argptr[2]));
            break;
        default: 
            tty_printf("Invalid syscall #%x\n", r->eax);
    }
    //tty_printf("result = %d, [%c] [%s]\n", result, result, result);
    
    r->eax = result;

}
