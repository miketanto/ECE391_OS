//exc_handler.h
#ifndef EXC_HANDLER_H
#define EXC_HANDLER_H

#define EXCEPTION_FLAG 256

void DIVIDE_ERROR_EXC();
void DEBUG_EXC();
void NMASK_INT_EXC();
void BREAKPOINT_EXC();
void OVERFLOW_EXC();
void BOUND_RANGE_EXCEEDED_EXC();
void INVALID_OPCODE_EXC();
void DEVICE_NOT_AVAILABLE_EXC();
void DOUBLE_FAULT_EXC();
void CORPROC_SEGMENT_OVERRUN_EXC();
void INVALID_TSS_EXC();
void SEGMENT_NOT_PRESENT_EXC();
void STACK_SEGMENT_FAULT();
void GENERAL_PROTECTION_FAULT();
void PAGE_FAULT();
void INTEL_RES_EXC();
void FLOATING_POINT_EXC();
void ALIGNMENT_CHECK_EXC();
void MACHINE_CHECK_EXC();
void SIMD_FLOATING_EXC();
void SYSTEM_CALL();

#define push_error_code()               \
do {                                    \
    asm volatile ("pushl $0"            \
            :                           \
            :                           \
            : "memory", "cc"            \
    );                                  \
} while (0)
#endif
