#include <kernel.h>


#define SET_IDT_ENTRY(idx) \
    set_idt_entry(idx, (uint32_t) &interrupt_handler_##idx,\
                  0x08, 0x8E);

//....{}
#define DECLARE_INTERRUPT_HANDLER(i) void interrupt_handler_##i(void)

#define ICW1 0x11
#define ICW4 0x01
#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave  PIC */

#define PIC1_COMMAND PIC1
#define PIC1_DATA    (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA    (PIC2 + 1)

void interrupt_enable_all() {
    asm volatile("sti");
}

void interrupt_disable_all() {
    asm volatile("cli");
}

// Defines an IDT entry
struct idt_entry {
    uint16_t handler_lo;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t handler_hi;
} __attribute__((packed));
typedef struct idt_entry idt_entry_t;

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));
typedef struct idt_ptr idt_ptr_t;

// Declare an IDT of 256 entries
idt_entry_t idt[IDT_NUM_ENTRIES];
idt_ptr_t idtp;

// Function arch/i386/idt.S, loads IDT from a pointer to an idt_ptr
extern void idt_load(struct idt_ptr *idt_ptr_addr);

/* ISRs */
//void interrupt_handler_0(void) {};
DECLARE_INTERRUPT_HANDLER(0);
DECLARE_INTERRUPT_HANDLER(1);
DECLARE_INTERRUPT_HANDLER(2);
DECLARE_INTERRUPT_HANDLER(3);
DECLARE_INTERRUPT_HANDLER(4);
DECLARE_INTERRUPT_HANDLER(5);
DECLARE_INTERRUPT_HANDLER(6);
DECLARE_INTERRUPT_HANDLER(7);
DECLARE_INTERRUPT_HANDLER(8);
DECLARE_INTERRUPT_HANDLER(9);
DECLARE_INTERRUPT_HANDLER(10);
DECLARE_INTERRUPT_HANDLER(11);
DECLARE_INTERRUPT_HANDLER(12);
DECLARE_INTERRUPT_HANDLER(13);
DECLARE_INTERRUPT_HANDLER(14);
DECLARE_INTERRUPT_HANDLER(15);
DECLARE_INTERRUPT_HANDLER(16);
DECLARE_INTERRUPT_HANDLER(17);
DECLARE_INTERRUPT_HANDLER(18);
DECLARE_INTERRUPT_HANDLER(19);

DECLARE_INTERRUPT_HANDLER(20);
DECLARE_INTERRUPT_HANDLER(21);
DECLARE_INTERRUPT_HANDLER(22);
DECLARE_INTERRUPT_HANDLER(23);
DECLARE_INTERRUPT_HANDLER(24);
DECLARE_INTERRUPT_HANDLER(25);
DECLARE_INTERRUPT_HANDLER(26);
DECLARE_INTERRUPT_HANDLER(27);
DECLARE_INTERRUPT_HANDLER(28);
DECLARE_INTERRUPT_HANDLER(29);
DECLARE_INTERRUPT_HANDLER(30);
DECLARE_INTERRUPT_HANDLER(31);

/* IRQs */
DECLARE_INTERRUPT_HANDLER(32);
DECLARE_INTERRUPT_HANDLER(33);
DECLARE_INTERRUPT_HANDLER(34);
DECLARE_INTERRUPT_HANDLER(35);
DECLARE_INTERRUPT_HANDLER(36);
DECLARE_INTERRUPT_HANDLER(37);
DECLARE_INTERRUPT_HANDLER(38);
DECLARE_INTERRUPT_HANDLER(39);
DECLARE_INTERRUPT_HANDLER(40);
DECLARE_INTERRUPT_HANDLER(41);
DECLARE_INTERRUPT_HANDLER(42);
DECLARE_INTERRUPT_HANDLER(43);
DECLARE_INTERRUPT_HANDLER(44);
DECLARE_INTERRUPT_HANDLER(45);
DECLARE_INTERRUPT_HANDLER(46);
DECLARE_INTERRUPT_HANDLER(47);


DECLARE_INTERRUPT_HANDLER(128);//for syscalls

void set_idt_entry(uint8_t num, uint64_t handler, uint16_t sel, uint8_t flags) {
    idt[num].handler_lo = handler & 0xFFFF;
    idt[num].handler_hi = (handler >> 16) & 0xFFFF;
    idt[num].always0 = 0;
    idt[num].flags = flags;
    idt[num].sel = sel;
}

void IRQ_set_mask(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;

    if (IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }

    value = inb(port) | (1 << IRQline);
    outb(port, value);
}

void IRQ_clear_mask(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;

    if (IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }

    value = inb(port) & ~(1 << IRQline);
    outb(port, value);
}

void init_pics(int pic1, int pic2) {
    outb(PIC1, ICW1);
    outb(PIC2, ICW1);
    outb(PIC1 + 1, pic1);
    outb(PIC2 + 1, pic2);
    outb(PIC1 + 1, 4);
    outb(PIC2 + 1, 2);
    outb(PIC1 + 1, ICW4);
    outb(PIC2 + 1, ICW4);
    outb(PIC1 + 1, 0xFF);
}


// Installs the IDT
void idt_init() {
    outb(0x21,0xfd);
    outb(0xa1,0xff);
    init_pics(0x20,0x28);

    // Sets the special IDT pointer up
    idtp.limit = (sizeof(struct idt_entry) * IDT_NUM_ENTRIES) - 1;
    idtp.base = (uint32_t) &idt;

    //tty_printf("idtp.base = %x, idtp.limit = %x\n", idtp.base, idtp.limit); //!!!!

    // Clear out the entire IDT, initializing it to zeros
    memset(&idt, 0, sizeof(struct idt_entry) * IDT_NUM_ENTRIES);

    idt_load(&idtp);

    int i;
    for (i = 0; i < IDT_NUM_ENTRIES; i++) {
        idt[i].handler_lo = 0;
        idt[i].handler_hi = 0;
        idt[i].always0 = 0;
        idt[i].flags = 0;
        idt[i].sel = 0;
    }

    /* ISRs */
    //set_idt_entry(0, (uint32_t) &interrupt_handler_0,0x08, 0x8E);
    SET_IDT_ENTRY(0);
    SET_IDT_ENTRY(1);
    SET_IDT_ENTRY(2);
    SET_IDT_ENTRY(3);
    SET_IDT_ENTRY(4);
    SET_IDT_ENTRY(5);
    SET_IDT_ENTRY(6);
    SET_IDT_ENTRY(7);
    SET_IDT_ENTRY(8);
    SET_IDT_ENTRY(9);
    SET_IDT_ENTRY(10);
    SET_IDT_ENTRY(11);
    SET_IDT_ENTRY(12);
    SET_IDT_ENTRY(13);
    SET_IDT_ENTRY(14);
    SET_IDT_ENTRY(15);
    SET_IDT_ENTRY(16);
    SET_IDT_ENTRY(17);
    SET_IDT_ENTRY(18);
    SET_IDT_ENTRY(19);
    SET_IDT_ENTRY(20);
    SET_IDT_ENTRY(21);
    SET_IDT_ENTRY(22);
    SET_IDT_ENTRY(23);
    SET_IDT_ENTRY(24);
    SET_IDT_ENTRY(25);
    SET_IDT_ENTRY(26);
    SET_IDT_ENTRY(27);
    SET_IDT_ENTRY(28);
    SET_IDT_ENTRY(29);
    SET_IDT_ENTRY(30);
    SET_IDT_ENTRY(31);

    /* IRQs */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    //SET_IDT_ENTRY(32); // !!!
    // Install scheduler by timer interrupt
    //set_idt_entry(TIMER_IDT_INDEX, (uint32_t) &task_switch, 0x08, 0x8E);
    //timer_set_frequency(TICKS_PER_SECOND);

    SET_IDT_ENTRY(33);
    SET_IDT_ENTRY(34);
    SET_IDT_ENTRY(35);
    SET_IDT_ENTRY(36);
    SET_IDT_ENTRY(37);
    SET_IDT_ENTRY(38);
    SET_IDT_ENTRY(39);
    SET_IDT_ENTRY(40);
    SET_IDT_ENTRY(41);
    SET_IDT_ENTRY(42);
    SET_IDT_ENTRY(43);
    SET_IDT_ENTRY(44);
    SET_IDT_ENTRY(45);
    SET_IDT_ENTRY(46);
    SET_IDT_ENTRY(47);


    SET_IDT_ENTRY(128); // Need for system calls - int 0x80 , 0x80 = 128 in decimal

    for (int i = 0; i < 16; ++i) {
        IRQ_clear_mask(i);
    }

}





static interrupt_handler_t interrupt_handlers[IDT_NUM_ENTRIES];

int register_interrupt_handler(uint32_t idt_index, interrupt_handler_t handler) {
    if (idt_index >= IDT_NUM_ENTRIES) {
        return 0;
    }

    if (interrupt_handlers[idt_index] != NULL) {
        return 0;
    }

    interrupt_handlers[idt_index] = handler;
    return 1;
}

void fault_handler(struct regs *r) {
    //void *linearAddress;
    // Retrieve the linear address of the page fault stored in CR2
    //ASM( "movl %%cr2, %0" : "=r" (linearAddress) );
    //asm volatile( "movl %cr2, %eax");
    //for (;;);

    uint32_t adr;
    asm volatile("movl %%cr2, %0" : "=r" (adr));
    qemu_printf("System Exception. System Halted! cr2 = %x  r->idt_index = %x eax = %x  ebx = %x  ecx = %x  edx = %x  esp = %x  ebp = %x  eip = %x\n", adr, r->idt_index, r->eax, r->ebx, r->ecx, r->edx, r->esp, r->ebp, r->eip);
    tty_printf("System Exception. System Halted! cr2 = %x  r->idt_index = %x eax = %x  ebx = %x  ecx = %x  edx = %x  esp = %x  ebp = %x  eip = %x\n", adr, r->idt_index, r->eax, r->ebx, r->ecx, r->edx, r->esp, r->ebp, r->eip);
    for (; ; );
}

void irq_handler(struct regs *r) {
    //if (r->idt_index != 32) tty_printf("idt_index = %d\n", r->idt_index);
    // Blank function pointer
    void (*handler)(struct regs *r);

    // If there's a custom handler to handle the IRQ, handle it
    handler = interrupt_handlers[r->idt_index];
    if (handler) {
        handler(r);
    }

    // If the IDT entry that was invoked was greater than 40, sends an EOI
    // to the slave controller
    if (r->idt_index >= 40) {
        outb(0xA0, 0x20);
    }

    // Sends an EOI to the master interrupt controller
    outb(0x20, 0x20);
}

void run_interrupt_handler(struct regs* r) {
    size_t idt_index = r->idt_index;
    if (idt_index < 32) {
        fault_handler(r);
        return;
    }

    if (idt_index >= 32 && idt_index <= 47) {
        irq_handler(r);
        return;
    }

    if (interrupt_handlers[r->idt_index] != NULL) {
        interrupt_handlers[r->idt_index](r);
    }
}