#define ASM 1

#include "syscall_asm.h"
.globl context_switch


/* 
 *  context_switch()
 *  DESCRIPTION: Prepares stack for IRET call then calls IRET  
 *  INPUTS: start_val = eip to fisrt instruction to user program
 *  OUTPUTS: none
 *  Side Effects: begins execution of the user program
 *  RETURN VALUE: none
 */
context_switch:
        movl 4(%esp), %edx  # grab eip to first instruction of user program from stack
        pushl $0x2B         # push USER_DS
        pushl $0x083FFFFC   # push esp
        pushfl              #  push FLAGS
        popl %eax
        orl $0x200, %eax    # re-enabling interrupts
        pushl %eax
        pushl $0x23          # push cs
        pushl %edx           # push eip to first instruction of user program
        
        iret

.globl ret_execute


//
ret_execute:
        pushl %ebp
        movl %esp, %ebp

        xorl %ebx, %ebx
        movl 16(%ebp), %ebx     # get (status)
        movl %ebx, %eax         

        movl 12(%ebp), %ebx     # get execute (esp)
        movl %ebx, %esp   

        movl 8(%ebp), %ebx       # get execute (ebp)
        movl %ebx, %ebp

        leave
        ret


.globl ret_schedule


/* ret_schedule()
 * DESCRIPTION: sets the esp and ebp with the given inputs so that the next task can begin
                execution in scheduling
 * INPUTS: schedule_ebp = value to set the ebp to      
 *         schedule_esp = value to set the esp to
 * OUTPUTS: none
 * RETURN: none
 * SIDE EFFECTS: sets esp and ebp to schedule_esp and schedule_ebp
 */
ret_schedule:
        pushl %ebp
        movl %esp, %ebp
       
        movl 12(%ebp), %ebx     # get schedule (esp)
        movl %ebx, %esp   

        movl 8(%ebp), %ebx       # get schedule (ebp)
        movl %ebx, %ebp
        leave
        ret

