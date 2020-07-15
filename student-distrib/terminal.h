#ifndef _TERMINAL_H_
#define _TERMINAL_H_
#include "lib.h"
#include "keyboard.h"
#define NUM_TERMINAL 3

void scroll();
void terminal_write(unsigned char * terminal_buf, int num_to_write);
int terminal_read(unsigned char * terminal_buf);
void display_screen();
typedef struct regs{
    // to store all regs
    long eax;
    long ebx;
    long ecx;
    long edx;
    long esi;
    long edi;
    long esp;
    long ebp;
    long eflags;

}regs_t;

typedef struct ter_detail{
    // all things needed when doing a context switch
    int init;
    regs_t regs;
    int esp0;
    int ss0;
    void* pcb_ptr;

} ter_detail_t;

ter_detail_t * ter_struct [NUM_TERMINAL];
#endif
