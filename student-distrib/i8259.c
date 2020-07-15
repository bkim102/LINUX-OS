/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */
static i8259A_irq_type i8259A_irq;
/*
#define ICW1                0x11
#define ICW2_MASTER         0x20
#define ICW2_SLAVE          0x28
#define ICW3_MASTER         0x04
#define ICW3_SLAVE          0x02
#define ICW4                0x01
*/


/*
 * i8259_init
 *   DESCRIPTION: initialises the slave and master PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: masks all IRQ's on PIC and sets them up to handle interrupts
 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
    i8259A_irq.name = "PIC";
    i8259A_irq.shutdown = disable_irq;
    i8259A_irq.disable = disable_irq;
    i8259A_irq.enable = enable_irq;
    i8259A_irq.ack = mask_and_ack;
    i8259A_irq.end = send_eoi;

    outb(master_mask, MASTER_8259_PORT_2);
    outb(slave_mask, SLAVE_8259_PORT_2);

    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_PORT_2);
    outb(ICW3_MASTER, MASTER_8259_PORT_2);
    outb(ICW4, MASTER_8259_PORT_2);

    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_PORT_2);
    outb(ICW3_SLAVE, SLAVE_8259_PORT_2);
    outb(ICW4, SLAVE_8259_PORT_2);
}

/* Enable (unmask) the specified IRQ */
/*
 * enable_irq
 *   DESCRIPTION: Enable (unmask) the specified IRQ in irq_num
 *   INPUTS: irq_num : The irq specified to enable
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */

void enable_irq(uint32_t irq_num) {
    // 0 and 7 because the Master PIC has ports 0-7 and Slave has 8-15 thus 7 and 16
    // cli();
    if(irq_num>0 && irq_num<8){
        int master_mask_interim = (0x1 << irq_num);
        master_mask = master_mask & (~ master_mask_interim);
        outb(master_mask, MASTER_8259_PORT_2);
    }
    // 0 and 7 because the Master PIC has ports 0-7 and Slave has 8-15 thus 7 and 16
    else if (irq_num >7 && irq_num <16){
        int master_mask_interim = (0x1 << 2);
        master_mask = master_mask & (~ master_mask_interim);
        outb(master_mask, MASTER_8259_PORT_2);
        int slave_mask_interim = (0x1 << (irq_num-8));
        slave_mask = slave_mask & (~ slave_mask_interim);
        outb(slave_mask, SLAVE_8259_PORT_2);
    }
    // sti();
}


/* Disable (mask) the specified IRQ */
/*
 * enable_irq
 *   DESCRIPTION: Disable (unmask) the specified IRQ in irq_num
 *   INPUTS: irq_num : The irq specified to disable
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */

void disable_irq(uint32_t irq_num) {
    // cli();
    // 0 and 7 because the Master PIC has ports 0-7 and Slave has 8-15 thus 7 and 16
    if(irq_num>0 && irq_num<8){
        int master_mask_interim = (0x1 << irq_num);
        master_mask = master_mask | ( master_mask_interim);
        outb(master_mask, MASTER_8259_PORT_2);
    }
    // 0 and 7 because the Master PIC has ports 0-7 and Slave has 8-15 thus 7 and 16
    else if (irq_num >7 && irq_num <16){
        int slave_mask_interim = (0x1 << (irq_num-8));
        slave_mask = slave_mask | ( slave_mask_interim);
        outb(slave_mask, SLAVE_8259_PORT_2);
    }
    // sti();
}

/* Send end-of-interrupt signal for the specified IRQ */
/*
 * send_eoi
 *   DESCRIPTION: Sends EOI signal to PIC fro sepcified irq_num
 *   INPUTS: irq_num : The irq specified to send EOI to
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void send_eoi(uint32_t irq_num) {
    // cli();
    // 0 and 7 because the Master PIC has ports 0-7 and Slave has 8-15 thus 7 and 16
    if(irq_num >7 && irq_num <16){
        outb((EOI|(irq_num-8)),SLAVE_8259_PORT);
        outb(EOI|(2),MASTER_8259_PORT);

    }
    else
        outb((EOI|irq_num),MASTER_8259_PORT);
    // sti();
}
/*
 * mask_and_ack
 *   DESCRIPTION: Disables specified irq
 *                and then ends EOI signal to PIC fro sepcified irq_num
 *   INPUTS: irq_num : The irq specified to send EOI to
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sends eoi and isables irq's on a specific irq num
 */
void mask_and_ack(uint32_t irq_num) {
    disable_irq(irq_num);
    send_eoi(irq_num);
}
