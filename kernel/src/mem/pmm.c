

#include <kernel.h>

uint32_t *phys_memory_bitmap = 0;
uint32_t phys_block_count = 0;
uint32_t phys_used_block_count = 0;

uint32_t kernel_phys_map_start;
uint32_t kernel_phys_map_end;

uint64_t phys_installed_memory_size = 0, phys_available_memory_size = 0;

// For initrd
uint64_t initrd_mmap_entry_addr = -1;
uint32_t mmap_avail_entries_count = 0;
multiboot_memory_map_entry mmap_avail_entries_array[100];

inline static void bitmap_set(int bit) {
    phys_memory_bitmap[bit / 32] |= (1 << (bit % 32));
}

inline static void bitmap_unset(int bit) {
    phys_memory_bitmap[bit / 32] &= ~(1 << (bit % 32));
}

inline static bool bitmap_test(int bit) {
    return phys_memory_bitmap[bit / 32] & (1 << (bit % 32));
}

void pmm_parse_memory_map(multiboot_memory_map_entry *mmap_addr, uint32_t length) {
    multiboot_memory_map_entry *mentry = 0;	

    int i = 0;
    /* Entries number in memory map structure */
    int n = length / sizeof(multiboot_memory_map_entry);

    /* Set pointer to memory map */
    mentry = mmap_addr;

    /* Print info about physical memory allocation */
    for (i = 0; i < n; i++) {
        if ((mentry + i)->type == 1) {
            phys_available_memory_size += (mentry + i)->len;
        }


        phys_installed_memory_size += (mentry + i)->len;
    }

    
    tty_printf("Installed memory size: %d KB", phys_installed_memory_size / 1024);
    tty_printf(" = %d MB\n", phys_installed_memory_size / (1024 * 1024));

    tty_printf("Available memory size: %d KB", phys_available_memory_size / 1024);
    tty_printf(" = %d MB\n", phys_available_memory_size / (1024 * 1024));
    
}

int pmm_find_free_block() {
    for (uint32_t i = 0; i < phys_block_count / 32; i++) {
        uint32_t block = phys_memory_bitmap[i];
        if (block != 0xFFFFFFFF) {
            for (uint8_t j = 0; j < 32; j++) {
                int bit = 1 << j;
                if (!(bit & block)) {
                    return (32 * i) + j;
                }
            }
        }
    }
    return -1;
}

int pmm_find_free_blocks(uint32_t count) {
    int starting_block = -1;
    int starting_block_bit = -1;
    uint32_t cur_block_num = 0;

    for (uint32_t i = 0; i < phys_block_count / 32; i++) {
        uint32_t cur_block = phys_memory_bitmap[i];
        if (cur_block == 0xFFFFFFFF) {
            cur_block_num = 0;
            continue;
        }

        for (uint8_t j = 0; j < 32; j++) {
            int bit = 1 << j;
            if (bit & cur_block) { // Bit is set
                cur_block_num = 0;
                continue;
            }

            if (!cur_block_num) {
                starting_block = i;
            }
            if (!cur_block_num) {
                starting_block_bit = j;
            }

            cur_block_num += 1;
            if (cur_block_num == count) {
                return (32 * starting_block) + starting_block_bit;
            }
        }
    }
    return -1;
}

// Functions to manage a single block in memory
physical_addr pmm_alloc_block() {
    if (phys_block_count - phys_used_block_count <= 0) {
        return 0xFFFFFFFF; // Old: return 0;
    }

    int free_block = pmm_find_free_block();
    if (free_block == -1) {
        return 0xFFFFFFFF; // Old: return 0;
    }

    bitmap_set(free_block);
    uint32_t addr = free_block * PHYS_BLOCK_SIZE;
    phys_used_block_count++;

    return addr;
}

void pmm_free_block(physical_addr addr) {
    int block = addr / PHYS_BLOCK_SIZE;
    bitmap_unset(block);
    phys_used_block_count--;
}

bool pmm_is_block_alloced(physical_addr addr) {
    int block = addr / PHYS_BLOCK_SIZE;
    return bitmap_test(block);
}

// Functions to allocate multiple blocks of memory

physical_addr pmm_alloc_blocks(uint32_t count) {
    if (phys_block_count - phys_used_block_count <= 0) {
        return 0xFFFFFFFF; // Old: return 0;
    }

    int free_block = pmm_find_free_blocks(count);
    if (free_block == -1) {
        return 0xFFFFFFFF; // Old: return 0;
    }

    for (uint32_t i = 0; i < count; i++) {
        bitmap_set(free_block + i);
    }

    uint32_t addr = free_block * PHYS_BLOCK_SIZE;
    phys_used_block_count += count;
    return addr;
}

void pmm_free_blocks(physical_addr addr, uint32_t count) {
    int block = addr / PHYS_BLOCK_SIZE;
    for (uint32_t i = 0; i < count; i++) {
        bitmap_unset(block + i);
    }
    phys_used_block_count -= count;
}

// Internal functions to allocate ranges of memory

void pmm_alloc_chunk(uint64_t base_addr, uint64_t length) {
    int cur_block_addr = base_addr / PHYS_BLOCK_SIZE;
    int num_blocks = length / PHYS_BLOCK_SIZE;
    while (num_blocks-- >= 0) {
        bitmap_set(cur_block_addr++);
        phys_used_block_count--;
    }
}

void pmm_free_chunk(uint64_t base_addr, uint64_t length) {
    int cur_block_addr = base_addr / PHYS_BLOCK_SIZE;
    int num_blocks = length / PHYS_BLOCK_SIZE;
    while (num_blocks--) {
        bitmap_unset(cur_block_addr++);
        phys_used_block_count--;
    }
}

// Functions to initialize the Physical Memory Manager

void pmm_free_available_memory(struct multiboot_info *mb) {
    multiboot_memory_map_entry *mm = (multiboot_memory_map_entry*) mb->mmap_addr;

    while ((unsigned int) mm < mb->mmap_addr + mb->mmap_length) {
        if (mm->type == 1) { 
            if (mm->addr != initrd_mmap_entry_addr) {
                pmm_free_chunk(mm->addr, mm->len);
            } else {
                uint32_t initrd_beg = *(uint32_t*) (mb->mods_addr);
                uint32_t initrd_end = *(uint32_t*) (mb->mods_addr + 4);
                uint32_t initrd_size = initrd_end - initrd_end;

                pmm_alloc_chunk(initrd_beg, initrd_size);
                pmm_free_chunk(mm->addr, mm->len - initrd_size - 2); 
            }
        }

        mm = (multiboot_memory_map_entry*) ((unsigned int) mm + mm->size + sizeof(mm->size));
    }

    bitmap_set(0);
}

void pmm_relocate_initrd_to_high_mem(struct multiboot_info *mb) {
    //tty_printf("test");
    uint32_t initrd_beg = *(uint32_t*) (mb->mods_addr);
    uint32_t initrd_end = *(uint32_t*) (mb->mods_addr + 4);
    uint32_t initrd_size = initrd_end - initrd_beg;

    multiboot_memory_map_entry *mm = (multiboot_memory_map_entry*) mb->mmap_addr;
    mmap_avail_entries_count = 0;

    while ((unsigned int) mm < mb->mmap_addr + mb->mmap_length) {
        if (mm->type == 1) { //if == MULTIBOOT_MEMORY_AVAILABLE
            mmap_avail_entries_array[mmap_avail_entries_count] = *mm;
            mmap_avail_entries_count++;
        }
        mm = (multiboot_memory_map_entry*) ((unsigned int) mm + mm->size + sizeof(mm->size));
    }

    int i;
    for (i = mmap_avail_entries_count - 1; i >= 0; i--) {
        if (mmap_avail_entries_array[i].len >= initrd_size) {
            initrd_mmap_entry_addr = mmap_avail_entries_array[i].addr;
            memcpy(initrd_mmap_entry_addr + mmap_avail_entries_array[i].len - initrd_size - 1, initrd_beg, initrd_size);
            initrd_beg = initrd_mmap_entry_addr + mmap_avail_entries_array[i].len - initrd_size - 1;
            initrd_end = initrd_beg + initrd_size;
            break;
        }
    }

    *(uint32_t*) (mb->mods_addr) = initrd_beg;
    *(uint32_t*) (mb->mods_addr + 4) = initrd_end;
}

void pmm_init(struct multiboot_info *mboot_info) {
    multiboot_memory_map_entry *mmap = (multiboot_memory_map_entry*) mboot_info->mmap_addr;
    pmm_parse_memory_map(mmap, mboot_info->mmap_length); // It also calculates the phys_installed_memory_size


    pmm_relocate_initrd_to_high_mem(mboot_info);

    phys_block_count = (phys_installed_memory_size /** 1024*/) / PHYS_BLOCK_SIZE; // How many blocks will be
    phys_used_block_count = phys_block_count; // Initially all blocks are used
    phys_memory_bitmap = (uint32_t*) KERNEL_END_PADDR; // Physical memory bitmap starts after kernel
    memset(phys_memory_bitmap, 0xFF, phys_block_count / PHYS_BLOCKS_PER_BYTE); // Initially we mark all installed memory as used
    

    // Frees memory GRUB considers available
    pmm_free_available_memory(mboot_info);

    // From the freed memory, we need to allocate the ones used by the Kernel
    pmm_alloc_chunk(KERNEL_START_PADDR, KERNEL_SIZE);

    
    // We also need to allocate the memory used by the Physical Map itself
    pmm_alloc_chunk(phys_memory_bitmap, phys_block_count); // WHY first argument *phys_memory_bitmap ???????????????????????????????????????????????????
    kernel_phys_map_start = (uint32_t) phys_memory_bitmap;
    kernel_phys_map_end = kernel_phys_map_start + (phys_block_count / PHYS_BLOCKS_PER_BYTE);


    qemu_printf("Physical memory manager installed. Physical memory bitmap start: %x, end: %x, size = %d bytes\n", kernel_phys_map_start, kernel_phys_map_end, kernel_phys_map_end - kernel_phys_map_start);
}

void pmm_test() {
    tty_printf("TEST: ");
    physical_addr myptr= pmm_alloc_block();
    tty_printf("myptr = %x,  ", myptr);

    char *str = "Hello world!";
    memcpy(myptr, str, 13);

    char buf[25];
    memcpy(buf, myptr, 13);
    tty_printf("read from memory str = %s\n", buf);
}

void update_phys_memory_bitmap_addr(physical_addr addr) {
    phys_memory_bitmap = (uint32_t*) addr;
}
