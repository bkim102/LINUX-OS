#ifndef _KEYBOARD_H
#define _KEYBOARD_H
#include "i8259.h"
#include "lib.h"
#include "syscalls.h"
#define NUM_TERMINALS 3
#define BUF_LEN 128
#define SCREEN_W 25
#define SCRENE_H 80

extern volatile int enter;
extern int len[NUM_TERMINALS];
extern volatile int cur_terminal;
extern volatile int enter_state;
extern unsigned char buf[NUM_TERMINALS][BUF_LEN];
extern volatile int add;
extern volatile int cur_row[NUM_TERMINALS];
extern volatile int cur_col[NUM_TERMINALS];
extern char screen[NUM_TERMINALS][SCREEN_W][SCRENE_H];
extern void scroll();
extern volatile int base_pid[NUM_TERMINALS];
extern volatile int glb_flag;
int32_t get_pcb_from_pid(int pid);
extern volatile int active_pid[NUM_TERMINALS];
#endif
