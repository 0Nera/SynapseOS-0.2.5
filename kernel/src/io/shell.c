#include <kernel.h>

char current_dir[256] = "/initrd/";

void ksh_main() {

    while (1) {

        tty_setcolor(VESA_LIGHT_CYAN);
        tty_printf("\nTestUser ");
        tty_setcolor(VESA_LIGHT_GREEN);
        tty_printf("%s>", current_dir);
        tty_setcolor(VESA_LIGHT_GREY);

        char *cmd = keyboard_gets();

        if (strlen(cmd) == 0) {
            continue;
        }

        if (strcmp(cmd, "about") == 0) {
            tty_printf("SynapseOS is a simple x86 C operating system with a well-documented kernel.");
        } else if (strcmp(cmd, "help") == 0) {
            tty_printf("\nCommands:\n" \
                        "->help                |get list of commands\n" \
                        "->cat   <filename>    |open file to read\n" \
                        "->cd    <folder>      |open folder\n" \
                        "->run   <file>        |run .elf programm\n" \
                        "->ls                  |print list of files\n" 
                        );
        } else if (strlen(cmd) > 4 && strncmp(cmd, "cat ", 4) == 0) {
            char fname[100];
            char *tok = strtok(cmd, " ");
            tok = strtok(0, " "); // tok - now is filename

            if (fname != 0) {
                cat(tok);
            } else {
                tty_printf("\ncat: incorrect argument\n");
            }
        } else if (strlen(cmd) > 3 && strncmp(cmd, "cd ", 3) == 0) {
            char dname[100];
            char *tok = strtok(cmd, " ");
            tok = strtok(0, " "); // tok - now is dirname

            if (dname != 0) {
                cd(tok);
            } else {
                tty_printf("\ncd: incorrect argument\n");
            }
        } else if (strcmp(cmd, "ls") == 0) {
            initrd_list(0, 0);

        } else if (strlen(cmd) > 4 && strncmp(cmd, "run ", 4) == 0) {
            char fname[100];
            char *tok = strtok(cmd, " ");
            tok = strtok(0, " "); // tok - now is filename

            if (fname != 0) {
                run(tok);
            } else {
                tty_printf("run: incorrect argument\n");
            }
        } else if (strcmp(cmd, "") == 0) {

        } else {
            tty_setcolor(VESA_LIGHT_RED);
            tty_printf("\nUnknown: [%s]\n", cmd);
        }
    }
}



void run(char *fname) {
    if (fname[0] != '/') { // TODO: make function
        char temp[256];
        strcpy(temp, current_dir);
        strcat(temp, fname);
        strcpy(fname, temp);
    }


    
    qemu_printf("run fname %s\n", fname);
    if (!vfs_exists(fname)) {
        tty_printf("\run: error file not found\n");
        return;
    }
    run_elf_file(fname);
}

void cd(char *dname) {
    if (dname[0] != '/') {
        char temp[256];
        strcpy(temp, current_dir);
        strcat(temp, dname);
        strcpy(dname, temp);
    }

    //tty_printf("%s\n", dname);
    //tty_printf("e = %x d = %d\n", vfs_exists(dname), vfs_is_dir(dname));

    if (dname[strlen(dname) - 1] != '/') { //very important, otherwise vfs won't see the dir
        strcat(dname, "/");
    }

    if (vfs_exists(dname) && vfs_is_dir(dname)) {
        strcpy(current_dir, dname);
    } else {
        tty_printf("cd: no such directory\n");
    }
}


void cat(char *fname) {
    if (fname[0] != '/') { //TODO: make function
        char temp[256];
        strcpy(temp, current_dir);
        strcat(temp, fname);
        strcpy(fname, temp);
    }

    char *buf = (char*) kheap_malloc(4096);

    if (!vfs_exists(fname)) {
        tty_printf("cat: error file not found\n");
    } else {
        uint32_t fsize = vfs_get_size(fname);
        int res = vfs_read(fname, 0, fsize, buf);
        buf[fsize] = '\0';
        tty_printf("cat: file %s:\n\n%s\n", fname, buf);
    }
    kheap_free(buf);
}
