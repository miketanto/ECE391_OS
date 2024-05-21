/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include "types.h"

/* Ports that each PIC sits on */
#define MASTER_8259_PORT    0x20        // IO Base Address of the Primary Chip, (PIC1, PIC1_COMMAND)
#define SLAVE_8259_PORT     0xA0        // IO Base Address of the Secondary Chip, (PIC2, PIC2_COMMAND)

// added
#define MASTER_8259_DATA	(MASTER_8259_PORT + 1)
#define SLAVE_8259_DATA	    (SLAVE_8259_PORT + 1)

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1                0x11        // Initialization control word 1
#define ICW2_MASTER         0x20        //? Interrupt vector offset
#define ICW2_SLAVE          0x28        //? Interrupt vector offset
#define ICW3_MASTER         0x04        // Tell Master PIC that there is a slave PIC at IRQ2    //? Call address interval 4 (8) for master (slave)?
#define ICW3_SLAVE          0x02        // Tell Slave PIC it in cascade mode
#define ICW4                0x01        // Enable 8086 mode

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI                 0x60        // End of interrupt

/* Externally-visible functions */

/* Initialize both PICs */
void i8259_init(void);
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);

#endif /* _I8259_H */
