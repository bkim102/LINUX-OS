#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include "filesys.h"
#include "rtc.h"
#include "types.h"
#include "terminal.h"
#include "lib.h"
#include "paging.h"
#include "x86_desc.h"

#define SYS_HALT    1
#define SYS_EXECUTE 2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_GETARGS 7
#define SYS_VIDMAP  8
#define SYS_SET_HANDLER  9
#define SYS_SIGRETURN  10
#define EXEC_M_NUM_0 0x7f
#define EXEC_M_NUM_1 0x45
#define EXEC_M_NUM_2 0x4c
#define EXEC_M_NUM_3 0x46

#define BIT_NUM_24 24
#define BIT_NUM_25 25
#define BIT_NUM_26 26
#define BIT_NUM_27 27
#define BYTE_SHIFT_3 24
#define BYTE_SHIFT_2 16
#define BYTE_SHIFT_1 8

#define USERSPACE_END 138412032
#define USERSPACE_START 134217728
#define COMMAND_LEN 128
#define FILENAME_LEN 32
#define NUM_FD 8
#define ARG_LEN 64

extern long pid_bit;

typedef struct PCB{

    struct PCB *parent;
    struct PCB *child;
    int pid;
    uint8_t filename[FILENAME_LEN];
    uint8_t command[COMMAND_LEN];
    file_desc_t fd_arr[NUM_FD];
    long esp_orig;
    long eip_orig;
    long ebp_orig;
    long esp0_orig;
    long ss0_orig;
    long kernel_stack_base;
    char arg1[ARG_LEN];
    char arg2[ARG_LEN];
    int base_shell;
    long keyboard_esp;
    long keyboard_ebp;
}PCB;

typedef struct gp_regs{

    long edi;
    long esi;
    long ebp;
    long esp;
    long ebx;
    long edx;
    long ecx;
    long eax;

}gp_regs;

// helper functions
// int parse_args(char *line, char **argv);

// all system calls
extern int32_t halt (uint8_t status);
extern int32_t execute (const uint8_t* command);
extern int32_t read (int32_t fd , void* buf, int32_t nbytes);
extern int32_t write (int32_t fd,const void* buf, int32_t nbytes);
extern int32_t open (const uint8_t* filename);
extern int32_t close (int32_t fd);
extern int32_t getargs (uint8_t * buf, int32_t nbytes);
extern int32_t vidmap (uint8_t ** screen_start);
extern int32_t set_handler (int32_t signum, void* handler_address);
extern int32_t sigreturn (void);

int get_next_pid ();
void clear_fd(file_desc_t * fd_arr);
void free_pid (int pid);
int fill_pcb (PCB* pcb, uint8_t * command, char *filename);
char* parse_filename(char* arr,char* filename, PCB* pcb);
int32_t copy_file_to_buf (uint8_t * filename, int * firs_ins_ptr, int cur_pid, int* file_size_ptr);
int32_t set_paging (int cur_pid, int file_size);
int get_firs_ins(uint8_t* buffer);
int32_t check_exec(void * buf);
int32_t pid_from_esp(void);
#endif
