// #include "idt.h"

#define ASM 1

/* Pointers to all the function exception handlers */

.text

.globl isr_wrapper_1,isr_wrapper_2,isr_wrapper_3,isr_wrapper_4
.globl isr_wrapper_5,isr_wrapper_6,isr_wrapper_7,isr_wrapper_8
.globl isr_wrapper_9,isr_wrapper_10,isr_wrapper_11,isr_wrapper_12
.globl isr_wrapper_13,isr_wrapper_14,isr_wrapper_15,isr_wrapper_16
.globl isr_wrapper_17,isr_wrapper_18,isr_wrapper_19,isr_wrapper_20
.globl isr_wrapper_21,isr_wrapper_22,isr_wrapper_23,isr_wrapper_24
.globl isr_wrapper_25,isr_wrapper_26,isr_wrapper_27,isr_wrapper_28
.globl isr_wrapper_29,isr_wrapper_30,isr_wrapper_31,isr_wrapper_32
.globl general_interrupt, rtc_wrapper, keyboard_wrapper, syscall_handler

/*
 * xx_wrapper
 *   DESCRIPTION: Pushes vector number of the hardware interrupt
 *                 on stack and calls common part saves
 *                 state of machine
 *   Convention : Not C style
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   REGISTERS USED : None
 *   SIDE EFFECTS: Setscr3 to address of page directory
 */

keyboard_wrapper:
    pushl $-6
    jmp common_part

rtc_wrapper:
    pushl $-5
    jmp common_part





/*
 * isr_wrapper_xx
 *   DESCRIPTION: Pushes vector number of the exception on stack and
 *                calls common part saves state of machine
 *   Convention : Not C style
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   REGISTERS USED : None
 *   SIDE EFFECTS: Setscr3 to address of page directory
 */

isr_wrapper_1:

    pushl $0x0
    jmp common_part

isr_wrapper_2:

    pushl $0x1
    jmp common_part

isr_wrapper_3:

    pushl $0x2
    jmp common_part

isr_wrapper_4:

    pushl $0x3
    jmp common_part

isr_wrapper_5:

    pushl $0x4
    jmp common_part

isr_wrapper_6:

    pushl $0x5
    jmp common_part

isr_wrapper_7:

    pushl $0x6
    jmp common_part

isr_wrapper_8:

    pushl $0x7
    jmp common_part

isr_wrapper_9:

    pushl $0x8
    jmp common_part
isr_wrapper_10:

    pushl $0x9
    jmp common_part

isr_wrapper_11:

    pushl $0xA
    jmp common_part

isr_wrapper_12:

    pushl $0xB
    jmp common_part

isr_wrapper_13:

    pushl $0xC
    jmp common_part

isr_wrapper_14:

    pushl $0xD
    jmp common_part

isr_wrapper_15:

    pushl $0xE
    jmp common_part

isr_wrapper_16:

    pushl $0xF
    jmp common_part

isr_wrapper_17:

    pushl $0x10
    jmp common_part

isr_wrapper_18:

    pushl $0x11
    jmp common_part

isr_wrapper_19:

    pushl $0x12
    jmp common_part

isr_wrapper_20:

    pushl $0x13
    jmp common_part

isr_wrapper_21:

    pushl $0x14
    jmp common_part

isr_wrapper_22:

    pushl $0x15
    jmp common_part

isr_wrapper_23:

    pushl $0x16
    jmp common_part

isr_wrapper_24:

    pushl $0x17
    jmp common_part

isr_wrapper_25:

    pushl $0x18
    jmp common_part

isr_wrapper_26:

    pushl $0x19
    jmp common_part

isr_wrapper_27:

    pushl $0x1A
    jmp common_part

isr_wrapper_28:

    pushl $0x1B
    jmp common_part

isr_wrapper_29:

    pushl $0x1C
    jmp common_part

isr_wrapper_30:

    pushl $0x1D
    jmp common_part

isr_wrapper_31:

    pushl $0x1E
    jmp common_part

isr_wrapper_32:

    pushl $0x1F
    jmp common_part

general_interrupt:

    pushl $-1
    jmp common_part

/*
 * common_part
 *   DESCRIPTION: Called by wrapper functions. In turn callls the
 *                Exception Handler and restores state of machine
 *   Convention : C style
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   REGISTERS USED: None
 *   SIDE EFFECTS: Calls the exception handler and then uses iret to
 *                 return
 */

common_part:
/* call the exception handler here */
// PUSH ALL REGS
    PUSHW $0
    PUSHW %fs
    PUSHW $0
    PUSHW %es
    PUSHW $0
    PUSHW %ds
    push %eax
    push %ebp
    push %edi
    push %esi
    push %edx
    push %ecx
    push %ebx

    push %esp

    call do_irq
    addl $4, %esp
    pop %ebx
    pop %ecx
    pop %edx
    pop %esi
    pop %edi
    pop %ebp
    pop %eax
    pop %ds
    pop %es
    pop %fs
    addl $4, %esp
    iret
    // if exception remove error code

