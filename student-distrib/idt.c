#include "idt.h"
// #include "rtc.h"
// #include "lib.h"

void init_idt();
void exception_handler(int vec_num);
void rtc_interrupt_handler();
extern void keyboard_interrupt_handler();
extern void syscall_handler();

char* exception_messages[32] = {
    "Division by zero",
    "Debug",
    "NMI",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device not available",
    "Double fault",
    "Coprocessor Segment Overrun",
    "Invalid Task State Segment",
    "Segment not present",
    "Stack-Segment Fault",
    "General protection fault",
    "Page fault",
    "reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved",
};
volatile int inter;

/*
 * rtc_interrupt handler
 *   DESCRIPTION: handles the rtc interrupt reads from reg c
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: set volatile var to 1 to signify interrupt was raised
 */

void rtc_interrupt_handler(){

    // set volatile variable to 1 so that we kow interrupt was raised
    inter = 1;

    // set to register C and read to get next interrupt
    outb(REG_C, SEL_REG);
    inb(RW_REG);

    // send eoi to say interrupot has been serviced
    send_eoi(8);

}
/*
 * init_idt
 *   DESCRIPTION: Initialises all 256 idt entries
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initialises the whole IDT table
 */

void init_idt(){
  int i;
    idt_desc_t exceptions, hardware_interrupt, system_calls;

    // Initialise a general exception for 0x0 - 0x1F's idt entry
    // DP R0 S  R1 R2 R3 R4
    // 0  0  1  1  1  1  1
    exceptions.reserved4 = 0;
    exceptions.reserved3 = 1;
    exceptions.reserved2 = 1;
    exceptions.reserved1 = 1;
    exceptions.size = 1;
    exceptions.reserved0 = 0;
    exceptions.dpl = 0;
    exceptions.seg_selector = KERNEL_CS;
    exceptions.present = 1;

    // initialise a general hardware interrupt idt entry
    // DP R0 S  R1 R2 R3 R4
    // 0  0  1  1  1  0  1
    hardware_interrupt = exceptions;
    hardware_interrupt.reserved3 = 0;

    // initialise a system call idt entry
    // DP R0 S  R1 R2 R3 R4
    // 3  0  1  1  1  1  1
    system_calls = exceptions;
    system_calls.dpl = 3;

    for (i = 0; i < NUM_VEC; i++) {
        // Set up first 32 entries of the idt specifically as
        // processor exceptions
        if (i < 32) {
            idt[i] = exceptions;
        }
        //  Load up system call idt vector at ox80
        else if (i == SYS_VEC) {
            idt[i] = system_calls;
            SET_IDT_ENTRY(idt[i],syscall_handler);
        }
        // load hardware interrupt for RTC and Keyboard
        else if (i == KEYBOARD_VEC) {
            idt[i] = hardware_interrupt;
            SET_IDT_ENTRY(idt[i], keyboard_wrapper);
        }
        else if (i == RTC_VEC) {
            idt[i] = hardware_interrupt;
            SET_IDT_ENTRY(idt[i], rtc_wrapper);
        }
        else{
            SET_IDT_ENTRY(idt[i],do_irq);
        }
    }

    /* Set all IDT entries here for the handlers */
    SET_IDT_ENTRY(idt[0],isr_wrapper_1);
    SET_IDT_ENTRY(idt[1],isr_wrapper_2);
    SET_IDT_ENTRY(idt[2],isr_wrapper_3);
    SET_IDT_ENTRY(idt[3],isr_wrapper_4);
    SET_IDT_ENTRY(idt[4],isr_wrapper_5);
    SET_IDT_ENTRY(idt[5],isr_wrapper_6);
    SET_IDT_ENTRY(idt[6],isr_wrapper_7);
    SET_IDT_ENTRY(idt[7],isr_wrapper_8);
    SET_IDT_ENTRY(idt[8],isr_wrapper_9);
    SET_IDT_ENTRY(idt[9],isr_wrapper_10);
    SET_IDT_ENTRY(idt[10],isr_wrapper_11);
    SET_IDT_ENTRY(idt[11],isr_wrapper_12);
    SET_IDT_ENTRY(idt[12],isr_wrapper_13);
    SET_IDT_ENTRY(idt[13],isr_wrapper_14);
    SET_IDT_ENTRY(idt[14],isr_wrapper_15);
    SET_IDT_ENTRY(idt[15],isr_wrapper_16);
    SET_IDT_ENTRY(idt[16],isr_wrapper_17);
    SET_IDT_ENTRY(idt[17],isr_wrapper_18);
    SET_IDT_ENTRY(idt[18],isr_wrapper_19);
    SET_IDT_ENTRY(idt[19],isr_wrapper_20);
    SET_IDT_ENTRY(idt[20],isr_wrapper_21);
    SET_IDT_ENTRY(idt[21],isr_wrapper_22);
    SET_IDT_ENTRY(idt[22],isr_wrapper_23);
    SET_IDT_ENTRY(idt[23],isr_wrapper_24);
    SET_IDT_ENTRY(idt[24],isr_wrapper_25);
    SET_IDT_ENTRY(idt[25],isr_wrapper_26);
    SET_IDT_ENTRY(idt[26],isr_wrapper_27);
    SET_IDT_ENTRY(idt[27],isr_wrapper_28);
    SET_IDT_ENTRY(idt[28],isr_wrapper_29);
    SET_IDT_ENTRY(idt[29],isr_wrapper_30);
    SET_IDT_ENTRY(idt[30],isr_wrapper_31);
    SET_IDT_ENTRY(idt[31],isr_wrapper_32);
    lidt(idt_desc_ptr);
}

// take in the ptrace struct
// use that to get information

/*
 * do_irq
 *   DESCRIPTION: Directs to correct interrupt handler
 *                Squashes user elvel program
 *   INPUTS: pt_reg structure from stack
 *   OUTPUTS: none
 *   RETURN VALUE: non
 *   SIDE EFFECTS: handles interrupts/ exception
 */

void do_irq(pt_regs_idt* pt){
    int vec_num = pt->orig_eax;
    if(vec_num == -5){
        rtc_interrupt_handler();
    }
    else if(vec_num == -6){
        keyboard_interrupt_handler();
    }
    else if(vec_num == -1){
        printf("General interrupt was raised \n");
        halt(-1);

    }
    else{
        printf("The exception raised was %s\n",exception_messages[vec_num]);
        halt(-1);
    }
}
