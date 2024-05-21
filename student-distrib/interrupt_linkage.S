#define ASM 1
#include "interrupt_linkage.h"


.global keyboard_interrupt
.global rtc_interrupt
.global pit_interrupt

keyboard_interrupt:
    pushal                  ;\
    pushfl                  ;\
    call keyboard_handler   ;\
    popfl                   ;\
    popal                   ;\
    iret                    

rtc_interrupt:
    pushal                  ;\
    pushfl                  ;\
    call rtc_handler        ;\
    popfl                   ;\
    popal                   ;\
    iret                                    

pit_interrupt:
    pushal                  ;\
    pushfl                  ;\
    call pit_handler        ;\
    popfl                   ;\
    popal                   ;\
    iret                    


