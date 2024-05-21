#define ASM     1
#include "x86_desc.h"
#include "sysnum.h"

#define DO_CALL(name,number)   \
.globl name                   ;\
name:   PUSHL	%EBX          ;\
	MOVL	$number,%EAX  ;\
	MOVL	8(%ESP),%EBX  ;\
	MOVL	12(%ESP),%ECX ;\
	MOVL	16(%ESP),%EDX ;\
	INT	$0x80         ;\
	POPL	%EBX          ;\
	RET

DO_CALL(system_execute,SYS_EXECUTE)
DO_CALL(system_halt, SYS_HALT)
DO_CALL(system_write, SYS_WRITE)
DO_CALL(system_read, SYS_READ)
DO_CALL(system_open, SYS_OPEN)
DO_CALL(system_close, SYS_CLOSE)

.align 4
syscall_jump_table:
    .long 0x0       
    .long halt
    .long execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap
    .long set_handler
    .long sigreturn

.globl syscall_handler
.align 4
syscall_handler:
    pushl   %esp
    pushl   %ebp
    pushl   %esi
    pushl   %edi
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushfl

    # Push arguments
    pushl %edx
    pushl %ecx
    pushl %ebx

    # Syscall Number error check
    cmpl    $10, %eax       # Syscall Number < 10
    ja      syscall_err
    cmpl    $0, %eax        # Syscall Number > 0
    jle      syscall_err
    

    # Call syscall
    call    *syscall_jump_table(, %eax, 4)
    jmp     syscall_leave

syscall_err:
    movl    $-1, %eax       # Return -1 as error

syscall_leave:
    addl    $12, %esp       # Clear arguments from stack
    popfl
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %edi
    popl    %esi
    popl    %ebp
    popl    %esp
    iret
