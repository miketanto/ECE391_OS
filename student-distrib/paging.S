.text
.globl loadPageDirectory
loadPageDirectory:
push %ebp
mov %esp, %ebp
mov 8(%esp), %eax
mov %eax, %cr3
mov %ebp, %esp
pop %ebp
ret

.text
.globl enablePaging
enablePaging:
push %ebp
mov %esp, %ebp
mov %cr0, %eax
or $0x80000000, %eax
mov %eax, %cr0
mov %ebp, %esp
pop %ebp
ret

.text
.globl setCR4
setCR4:
push %ebp
mov %esp, %ebp
mov %cr4, %eax
or $0x00000010, %eax
mov %eax, %cr4
mov %ebp, %esp
pop %ebp
ret

.text
.globl flush_tlb
flush_tlb:
    movl %cr3, %eax    
    movl %eax, %cr3
    ret
