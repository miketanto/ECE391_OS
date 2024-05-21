/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF;     /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;      /* IRQs 8-15 */

// Reference: https://wiki.osdev.org/8259_PIC


/* i8259_init
 *   Description: Initialize the 8259 PIC
 *   Inputs: none
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: Initialize the 8259 PIC
*/
void i8259_init(void) {
    outb(ICW1, MASTER_8259_PORT);
	outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_DATA);    // Master PIC and Slave PIC
    outb(ICW2_SLAVE, SLAVE_8259_DATA);      // are initialized differently.
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);
    outb(ICW4, MASTER_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);

	outb(master_mask, MASTER_8259_DATA);
	outb(slave_mask, SLAVE_8259_DATA);

    enable_irq(ICW3_SLAVE);
}


/* enable_irq
 *   Description: Enable (unmask) the specified IRQ
 *   Inputs: irq_num - the IRQ number to enable
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: Enable the specified IRQ port
*/
void enable_irq(uint32_t irq_num) {
    uint8_t mask;
    if(irq_num < 8) {
        // Change Master IRQ
        mask = 1 << irq_num;
        master_mask &= ~mask; //need to be inverse so that only the irq that is enabled becomes 0
        outb(master_mask, MASTER_8259_DATA);
    } else {
        mask = 1 << (irq_num - 8);
        slave_mask &= ~mask;
        outb(slave_mask, SLAVE_8259_DATA);
    }
}


/* disable_irq
 *   Description: Disable (mask) the specified IRQ
 *   Inputs: irq_num - the IRQ number to disable
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: Disable the specified IRQ port
*/
void disable_irq(uint32_t irq_num) {
   uint8_t mask;
    if(irq_num < 8) {
        // Change Master IRQ
        mask = 1 << irq_num;
        master_mask |= mask; //need to be inverse so that only the irq that is enabled becomes 0
        outb(master_mask, MASTER_8259_DATA);
    } else {
        mask = 1 << (irq_num - 8);
        slave_mask |= mask;
        outb(slave_mask, SLAVE_8259_DATA);
    }
}


/* send_eoi
 *   Description: Send end-of-interrupt signal for the specified IRQ
 *   Inputs: irq_num - the IRQ number to send EOI to
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: Send EOI to the specified IRQ port
*/
void send_eoi(uint32_t irq_num) {
    if(irq_num >= 8) {
        outb(EOI | (irq_num - 8), SLAVE_8259_PORT);
        send_eoi(2);
    }
	outb(EOI | irq_num, MASTER_8259_PORT);
}
