//exc_handler.c
#include "exc_handler.h"
#include "lib.h"
#include "syscall.h"


void DIVIDE_ERROR_EXC(){
    cli();
    push_error_code();
    printf("Division Error Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void DEBUG_EXC(){
    cli();
    printf("Debug Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void NMASK_INT_EXC(){
    printf("Non-maskable Interrupt Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void BREAKPOINT_EXC(){
    printf("Breakpoint Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void OVERFLOW_EXC(){
    printf("Overflow Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void BOUND_RANGE_EXCEEDED_EXC(){
    cli();
    printf("Bound Range Exceeded Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void INVALID_OPCODE_EXC(){
    printf("Invalid Opcode Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void DEVICE_NOT_AVAILABLE_EXC(){
    printf("Device Not Available Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void DOUBLE_FAULT_EXC(){
    printf("Double Fault Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void CORPROC_SEGMENT_OVERRUN_EXC(){
    printf("Coprocessor Segment Overrun Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void INVALID_TSS_EXC(){
    printf("Invalid TSS Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void SEGMENT_NOT_PRESENT_EXC(){
    printf("Segment Not Present Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void STACK_SEGMENT_FAULT(){
    printf("Stack-Segment Fault Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void GENERAL_PROTECTION_FAULT(){
    printf("General Protection Fault exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void PAGE_FAULT(){
    printf("Page Fault Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void INTEL_RES_EXC(){
    printf("Intel Reserved Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void FLOATING_POINT_EXC(){
    printf("Floating-Point Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void ALIGNMENT_CHECK_EXC(){
    printf("Alignment Check Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void MACHINE_CHECK_EXC(){
    printf("Machine Check Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void SIMD_FLOATING_EXC(){
    printf("SIMD Floating-Point Exception");
    halt((uint8_t) EXCEPTION_FLAG);
}

void SYSTEM_CALL(){
    cli();
    printf("Sys");
    sti();
}

