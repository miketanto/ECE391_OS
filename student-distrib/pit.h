#ifndef PIT_H
#define PIT_H

#include "types.h"
#include "lib.h"
#include "i8259.h"

#define CMD_PORT 0x43
#define DATA_PORT 0x40
#define IRQ_NUM 0x0
#define PIT_MODE 0x36
#define PIT_FREQ 11932

void pit_init();
void pit_handler();

#endif
