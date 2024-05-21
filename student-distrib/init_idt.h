//init_idt.h
#ifndef IDT_INIT_H
#define IDT_INIT_H

#include "x86_desc.h"
#include "exc_handler.h"
#include "interrupt_linkage.h"
# include "keyboard.h"
# include "rtc.h"
# include "pit.h"

#ifndef ASM
    void idt_init();
#endif

#endif /*init_idt.h*/
