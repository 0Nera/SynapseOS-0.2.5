#include <interrupts/idt.h>


#define SC_CODE_puts 0
#define SC_CODE_getscancode 1
#define SC_CODE_putpixel 32


void syscall_init();
void syscall_handler(struct regs *r);