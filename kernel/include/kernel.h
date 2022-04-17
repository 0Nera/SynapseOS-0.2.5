#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include <multyboot.h>

#include <mem/mem.h>
#include <mem/pmm.h>
#include <mem/vmm.h>
#include <mem/kheap.h>

#include <drivers/vfs.h>
#include <drivers/ramdisk.h>

#include <io/tty.h>
#include <io/vgafnt.h>
#include <io/ports.h>
#include <drivers/keyboard.h>

#include <interrupts/gdt.h>
#include <interrupts/idt.h>
#include <interrupts/tss.h>
#include <interrupts/syscalls.h>

#include <libk/string.h>
#include <libk/stdlib.h>

#include <sys/elf.h>


enum {
    VERSION_MAJOR = 0, 
    VERSIOn_MINOR = 2, 
    VERSION_PATCH = 4
};

