#ifndef INTERRUPT_LINKAGE_H
#define INTERRUPT_LINKAGE_H


#ifndef ASM
    extern void keyboard_interrupt();
    extern void rtc_interrupt();
    extern void pit_interrupt();
#endif

#endif
