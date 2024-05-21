//init_idt.c 
# include "init_idt.h"
# include "syscall.h"

void idt_init(){
    int i;
    for(i = 0; i < NUM_VEC; i++) {
        idt[i].present = 0;     // IDT presence will be enabled on SET_IDT_ENTRY call
        idt[i].dpl = (i == 0x80) ? 3 : 0; // system calls in ring3
                                                        // all others in kernel ring0
        idt[i].reserved0 = 0;   // Read ISA Reference Manual for x86, Volume 3,
        idt[i].reserved1 = 1;   // 5.11 IDT Descriptors for why these "magic numbers"
        idt[i].reserved2 = 1;   // are set
        idt[i].reserved3 = (i >= 0x20 && i <= 0x2F) ? 0 : 1;
        idt[i].reserved4 = 0;
        idt[i].size = 1;        // All our handlers are 32 bit.
        idt[i].seg_selector = KERNEL_CS;    // All handlers run in kernel space.
    }

    //exception vectors set up
    SET_IDT_ENTRY(idt[0x00], DIVIDE_ERROR_EXC);
    SET_IDT_ENTRY(idt[0x01], DEBUG_EXC);
    SET_IDT_ENTRY(idt[0x02], NMASK_INT_EXC);
    SET_IDT_ENTRY(idt[0x03], BREAKPOINT_EXC);
    SET_IDT_ENTRY(idt[0x04], OVERFLOW_EXC);
    SET_IDT_ENTRY(idt[0x05], BOUND_RANGE_EXCEEDED_EXC);
    SET_IDT_ENTRY(idt[0x06], INVALID_OPCODE_EXC);
    SET_IDT_ENTRY(idt[0x07], DEVICE_NOT_AVAILABLE_EXC);
    SET_IDT_ENTRY(idt[0x08], DOUBLE_FAULT_EXC);
    SET_IDT_ENTRY(idt[0x09], CORPROC_SEGMENT_OVERRUN_EXC);
    SET_IDT_ENTRY(idt[0x0A], INVALID_TSS_EXC);
    SET_IDT_ENTRY(idt[0x0B], SEGMENT_NOT_PRESENT_EXC);
    SET_IDT_ENTRY(idt[0x0C], STACK_SEGMENT_FAULT);
    SET_IDT_ENTRY(idt[0x0D], GENERAL_PROTECTION_FAULT);
    SET_IDT_ENTRY(idt[0x0E], PAGE_FAULT);
    SET_IDT_ENTRY(idt[0x0F], INTEL_RES_EXC);
    SET_IDT_ENTRY(idt[0x10], FLOATING_POINT_EXC);
    SET_IDT_ENTRY(idt[0x11], ALIGNMENT_CHECK_EXC);
    SET_IDT_ENTRY(idt[0x12], MACHINE_CHECK_EXC);
    SET_IDT_ENTRY(idt[0x13], SIMD_FLOATING_EXC);
    //SET_IDT_ENTRY(idt[0x21], DIVIDE_ERROR_EXC);
    SET_IDT_ENTRY(idt[0x20], pit_interrupt);
    SET_IDT_ENTRY(idt[0x21], keyboard_interrupt);
    SET_IDT_ENTRY(idt[0x28], rtc_interrupt);
    SET_IDT_ENTRY(idt[0x80], syscall_handler);
}
