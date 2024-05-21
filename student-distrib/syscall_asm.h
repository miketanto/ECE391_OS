#ifndef SYSCALL_ASM_H
#define SYSCALL_ASM_H

#include "types.h"


#ifndef ASM

// modifies various control registers to enable paging
void context_switch(uint32_t start_val);

#endif

#endif

