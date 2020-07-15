#ifndef _IDT_H
#define _IDT_H

#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "keyboard.h"
#define SYS_VEC 0x80
#define KEYBOARD_VEC 0x21
#define RTC_VEC 0x28
#define REG_A 0x8A
#define REG_B 0x8B
#define REG_C 0x0C
#define SEL_REG 0x70
#define RW_REG 0x71
#define EN_VAL 0x40

extern void isr_wrapper_1 ();
extern void isr_wrapper_2 ();
extern void isr_wrapper_3 ();
extern void isr_wrapper_4 ();
extern void isr_wrapper_5 ();
extern void isr_wrapper_6 ();
extern void isr_wrapper_7 ();
extern void isr_wrapper_8 ();
extern void isr_wrapper_9 ();
extern void isr_wrapper_10 ();
extern void isr_wrapper_11 ();
extern void isr_wrapper_12 ();
extern void isr_wrapper_13 ();
extern void isr_wrapper_14 ();
extern void isr_wrapper_15 ();
extern void isr_wrapper_16 ();
extern void isr_wrapper_17 ();
extern void isr_wrapper_18 ();
extern void isr_wrapper_19 ();
extern void isr_wrapper_20 ();
extern void isr_wrapper_21 ();
extern void isr_wrapper_22 ();
extern void isr_wrapper_23 ();
extern void isr_wrapper_24 ();
extern void isr_wrapper_25 ();
extern void isr_wrapper_26 ();
extern void isr_wrapper_27 ();
extern void isr_wrapper_28 ();
extern void isr_wrapper_29 ();
extern void isr_wrapper_30 ();
extern void isr_wrapper_31 ();
extern void isr_wrapper_32 ();
extern void do_irq();
extern void rtc_wrapper();
extern void keyboard_wrapper();

// taken from Kernel https://github.com/torvalds/linux/blob/v4.8/arch/x86/include/uapi/asm/ptrace.h

typedef struct pt_regs_idt {
    long ebx;
    long ecx;
    long edx;
    long esi;
    long edi;
    long ebp;
    long eax;
    int  xds;
    int  xes;
    int  xfs;
    long orig_eax;
    long eip;
    int  xcs;
    long eflags;
    long esp;
    int  xss;
}pt_regs_idt ;

#endif
