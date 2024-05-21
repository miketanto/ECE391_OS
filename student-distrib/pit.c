#include "pit.h"
#include "syscall.h"
#include "terminal.h"
#include "syscall_asm.h"
#include "paging.h"
#include "x86_desc.h"

int pit_counter = 0;
int scheduled_terminal = 1;


/* schedule()
 *   DESCRIPTION:  stops currently executing task and begins/resumes executing the next task in a 
 *                 round robin fashion. Saves all relavent data for the currently executing task so
 *                 that when it is possible to return to this task once it is scheduled again. 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: stops currently executing task and begins/resumes executing the next task in a 
 *                 round robin fashion
*/
void schedule(){
    //if(scheduled_terminal<=3)running_terminal = scheduled_terminal - 1;
    
    //saves esp and ebp of currently running task to return at a later point
    pcb_t* pcb;
    if(curr_process_id[running_terminal]!= -1){
        pcb = get_pcb_ptr(curr_process_id[running_terminal]);
        uint32_t esp;
        uint32_t ebp;
        asm("\t movl %%esp, %0" : "=r"(esp));
        asm("\t movl %%ebp, %0" : "=r"(ebp));
        pcb->schedule_esp = esp; 
        pcb->schedule_ebp = ebp;
    }
    
    //get target terminal and update running terminal to target terminal
    int target_terminal = running_terminal;
    target_terminal = ((running_terminal + 1)%3);
    running_terminal = target_terminal;

    //start shell in target terminal if target terminal has nothing running currently
    if(curr_process_id[target_terminal] == -1){
        system_execute((const uint8_t *)"shell");
    }

    //getting data for program running in the target terminal
    pcb = get_pcb_ptr(curr_process_id[target_terminal]);
    uint32_t phys_address = EIGHT_MB + (FOUR_MB)*curr_process_id[target_terminal];
    page_directory[PROG_DIRECTORY_IDX].address_31_12 = phys_address/FOUR_KB_ALIGN;
    flush_tlb();

    //returning from scheduling 
    tss.ss0 = KERNEL_DS;
    tss.esp0 = EIGHT_MB - EIGHT_KB * (curr_process_id[target_terminal]) - 4;
    
    ret_schedule(pcb->schedule_ebp, pcb->schedule_esp);
    return;    
}



/* pit_init()
 *   DESCRIPTION: Initialize the PIT by sending initialization commands to the PIT controller,
 *                and enable the PIT interrupt on the PIC at IRQ0
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Initialize the PIT, enable the PIT interrupt on the PIC at IRQ0
*/
void pit_init(){
    outb(PIT_MODE, CMD_PORT);
    //setting frequency to ~100Hz
    outb(PIT_FREQ & 0xFF, DATA_PORT);
    outb((PIT_FREQ >> 8) & 0xFF, DATA_PORT);
    enable_irq(IRQ_NUM);
    return;
}



/* pit_handler()
 *   DESCRIPTION: Handle the PIT interrupt, calls update_video_memory_paging() and schedule()
 *                to handle scheduling, after pit_handler is done the task that was executing should
 *                be paused and the next task that was scheduled should be executing.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: stops currently executing task and begins/resumes executing the next task in a 
 *                 round robin fashion
*/
void pit_handler(){
    cli();
    send_eoi(IRQ_NUM);
    // if(pit_counter < 3){
    //     view_terminal = pit_counter;
    //     if(pit_counter>0)terminal_switch(pit_counter-1, pit_counter);
    //     pit_counter++;
    //     system_execute((const uint8_t *)"shell");
    //     return;
    // }else if (pit_counter == 3){
    //     pit_counter++;
    //     view_terminal = 0;
    //     terminal_switch(2, 0);
    //     return;
    // }
    // else{
    //     if(scheduled_terminal<=3){
    //         update_video_memory_paging((((scheduled_terminal - 1) + 1)%scheduled_terminal));
    //     }else{
    //         update_video_memory_paging(((running_terminal + 1)%3));
    //     }
    //     schedule();
    // }
    update_video_memory_paging(((running_terminal + 1)%3));
    schedule();
    sti();
}
