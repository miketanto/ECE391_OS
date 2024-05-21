#include "syscall.h"
#include "fs.h"
#include "paging.h"
#include "x86_desc.h"
#include "syscall_asm.h"
#include "rtc.h"                                    //changes here
#include "pit.h"

//Testing Libs
#include "terminal.h"
 
//This process ids need to be terminal based too
int active_process[MAX_TERMINAL] = {0,0,0}; //Process running on terminal currently
int curr_process_id[MAX_TERMINAL] = {-1,-1,-1};
int parent_process_id[MAX_TERMINAL] = {-1,-1,-1};

//This is still the main processId array
int process_id_array[MAX_PROCESS_ID];


/* 
 *  get_curr_pcb_ptr()
 *  DESCRIPTION: gets pointer to current process control block 
 *  INPUTS: none
 *  OUTPUTS: pointer to the PCB currently being used 
 *  Side Effects: none
 *  RETURN VALUE: pointer to the PCB currently being used 
 */
pcb_t* get_curr_pcb_ptr(){
    return((pcb_t*)(EIGHT_MB - (curr_process_id[running_terminal]+1)*EIGHT_KB));
}



/* 
 *  get_pcb_ptr()
 *  DESCRIPTION: gets pointer to start of all process control blocks 
 *  INPUTS: none
 *  OUTPUTS: pointer to start of all process control blocks
 *  Side Effects: none
 *  RETURN VALUE: pointer to start of all process control blocks
 */
pcb_t* get_pcb_ptr(int process_id){
    return ((pcb_t*)(EIGHT_MB - (process_id + 1)*EIGHT_KB));
}



/* 
 *  parse_args()
 *  DESCRIPTION: takes a command inputted by the user and splits the command
 *               seperating the file name and each argument
 *  INPUTS: command: command that the user inputted
 *          fname: location where the filename given in the command should be stored
 *          args: location where the arguments given in the command should be stored 
 *  OUTPUTS: None
 *  Side Effects: Fills Fname and Args with filename and arguments for the command
 *                that the user inputted
 *  RETURN VALUE: None
 */
void parse_args(const uint8_t* command, uint8_t* fname, uint8_t* args){
    // Need to parse args somewhere for next checkpoint;
    int len, i, j;
    // len = sizeof(command);           //changes here sizeof works for ptr arrays   //! this line is causing shell to not work
    len = strlen((const int8_t *)command);          //! debug_1112
    i = 0;
    while(i < len){
        if(command[i] == ' ')break;
        fname[i] = command[i];
        i++;
    }
    for( j = i ; j < FILENAME_LEN ; j++ ){
        fname[j] = '\0';
    }
    j = 0;
    i++;
    while(i < len){
        args[j] = command[i];
        i++;
        j++;
    }
    // printf("FILENAME: %s", fname);
    return;
}



/* 
 *  halt()
 *  DESCRIPTION: system call for halt(). Ends program that is currently
 *               being executed. Returns  
 *  INPUTS: status: 
 *  OUTPUTS: none
 *  Side Effects: exits program currently being executed, returning control
 *                to the parent program.
 *  RETURN VALUE: -1 
 */
int32_t halt (uint8_t status){
    cli();
    int i;

    //restoring parent data
    pcb_t* pcb = get_pcb_ptr(curr_process_id[running_terminal]);

    pcb_t* parent_pcb = get_pcb_ptr(pcb->parent_process_id);
    curr_process_id[running_terminal] = pcb->parent_process_id;
    parent_process_id[running_terminal] = parent_pcb->parent_process_id;
    active_process[running_terminal] -= 1;
    //if(active_process[running_terminal]<2)active_terminals--;
    process_id_array[pcb->process_id] = 0;


    //restoring parent paging
    uint32_t phys_address = EIGHT_MB + (FOUR_MB)*curr_process_id[running_terminal];
    if (pcb->vidmap_flag)
    {
        pcb->vidmap_flag = 0;
        terminal_array[running_terminal].vidmap_flag = 0;
        video_page_table[0].present = 0;
        //page_directory[VIDMEM_DIRECTORY_IDX].present = 1;
        if (running_terminal == view_terminal){
            video_page_table[0].address_31_12 = (VIDMEM_PHY_ADDRESS/FOUR_KB_ALIGN);
        }else{
            video_page_table[0].address_31_12 = (terminal_array[running_terminal].terminal_video_buffer/FOUR_KB_ALIGN);
        }
    }

    page_directory[PROG_DIRECTORY_IDX].address_31_12 = phys_address/FOUR_KB_ALIGN;
    flush_tlb();

    //closing relevant file descriptors
    for(i=0;i<MAX_OPEN_FILES;i++){
        pcb->file_descriptor_array[i].flags = 0;
    }

    //This is the base shell of the shell
    if(active_process[running_terminal] <= 0){
        //context_switch(pcb->task_esp);
        execute((uint8_t*)"shell");
    }

    //jumping to execute return
    tss.ss0 = KERNEL_DS;
    tss.esp0 = EIGHT_MB - (EIGHT_KB*parent_pcb->process_id) - sizeof(int32_t);

    ret_execute(pcb->execute_ebp,pcb->execute_esp,status);

    return -1;
}



/* 
 *  execute()
 *  DESCRIPTION: System call used to start running a program that
 *               the user has specified   
 *  INPUTS: command: command inputted by user indicating program to run
 *  OUTPUTS: none
 *  Side Effects: Completes all setup and begins execution of the 
 *                program specified by the user. 
 *  RETURN VALUE: 0
 */
int32_t execute(const uint8_t* command){
    cli();
    dentry_t command_dentry;
    uint8_t fname[FILENAME_LEN];
    uint8_t args[FILENAME_LEN];
    uint8_t buf[4];
    int i;
    uint32_t ret;
    uint32_t start_val;
    uint32_t task_esp;
    // printf("EXECUTE CALLED\n");
    // printf("%s\n", command);
    /*Parse Args*/
    parse_args(command, fname, args);
    /*Check Validity of File*/
    // Check if the command file name exists
    if(read_dentry_by_name(fname, &command_dentry) == -1)return -1;
    // Check if the file reads the first 4 bytes
    if(read_data(command_dentry.inode_num, 0, buf, 4) == -1)return -1;
    // Check with Magic Numbers if it is an executable
    if(buf[0]!=0x7f || buf[1]!=0x45 || buf[2]!=0x4C || buf[3]!=0x46)return -1;

    /*Set up paging*/
    //Virtual Memory 128MB need to map to 8MB + process_id * 4MB;
    // Find empty process id
    i = 0;
    while(i < MAX_PROCESS_ID){
        if(process_id_array[i] == 0){
            process_id_array[i] = 1;
            active_process[running_terminal] += 1;
            curr_process_id[running_terminal]= i;
            // running_terminal = running_terminal;
            break;
        }
        i++;
    }
    if(i == MAX_PROCESS_ID) return -1; // No Empty processes
    //printf("PROCESS NUMBER: %d\n", i);
    uint32_t phys_address = EIGHT_MB + (FOUR_MB)*curr_process_id[running_terminal];
    page_directory[PROG_DIRECTORY_IDX].address_31_12 = phys_address/FOUR_KB_ALIGN;
    flush_tlb();

    /*Load File Into Memory*/ // TESTED
    uint8_t* program_img_address = (uint8_t*)0x8048000;
    inode_t* length_ptr = (inode_t *)(inode_start+command_dentry.inode_num);
    ret = read_data(command_dentry.inode_num, 0, program_img_address, length_ptr->length); 

    /*Get PCB and open FD*/ // TESTED
     pcb_t* pcb = get_pcb_ptr(curr_process_id[running_terminal]);
     pcb->vidmap_flag = 0;
     pcb->process_id = curr_process_id[running_terminal];
      if( parent_process_id[running_terminal] != -1 ){
        //If it isn't the first process in the terminal
         pcb->parent_process_id = parent_process_id[running_terminal];
         parent_process_id[running_terminal] = curr_process_id[running_terminal];
     }else{
         pcb->parent_process_id = curr_process_id[running_terminal];
         parent_process_id[running_terminal] = curr_process_id[running_terminal];
     }

     /*Initialize FD Array*/ // TESTED
     for(i = 0 ; i<MAX_OPEN_FILES ; i++){
         pcb->file_descriptor_array[i].fop_jump_ptr = &empty_jump_table;
         pcb->file_descriptor_array[i].flags = 0;
         pcb->file_descriptor_array[i].file_pos = 0;
         pcb->file_descriptor_array[i].inode = 0;
     }

     pcb->file_descriptor_array[0].fop_jump_ptr = &stdin_jump_table;
     pcb->file_descriptor_array[0].flags = 1;

     pcb->file_descriptor_array[1].fop_jump_ptr = &stdout_jump_table;
     pcb->file_descriptor_array[1].flags = 1;
    
    /*save arguments to PCB*/
    strncpy((int8_t*)pcb->argument, (int8_t*)args, FILENAME_LEN+1);

     /*Prepare for Context Switch*/
     read_data(command_dentry.inode_num, 24, buf, 4); // gets start of user program to run
     start_val = *((uint32_t*)buf);
     task_esp = 0x8000000 + FOUR_MB - sizeof(int32_t);//Program Stack This is context to be pushed
     pcb->task_eip = start_val;
     pcb->task_esp = task_esp;

     /*Saved for Jumping to execute return*/
     uint32_t esp;
     uint32_t ebp;
     asm("\t movl %%esp, %0" : "=r"(esp));
     asm("\t movl %%ebp, %0" : "=r"(ebp));
     pcb->execute_esp = esp; 
     pcb->execute_ebp = ebp;

     /*TSS for Stack Switching*/
     tss.ss0 = KERNEL_DS;
     tss.esp0 = EIGHT_MB - EIGHT_KB * (curr_process_id[running_terminal]) - 4;
     pcb->tss_esp0 = tss.esp0;

    /*Scheduling Initialization*/
    pcb->schedule_esp = 0;
    pcb->schedule_ebp = 0;
     /*Push IRET Context to Stack*/
    context_switch(start_val);

    return 0;
}



/* 
 *  read()
 *  DESCRIPTION: general system call for read(). uses jumptable
 *               to locate the proper read function then calls it. 
 *  INPUTS:  fd, buf, nbytes: standard inputs for read system call
 *  OUTPUTS: ret : return value from specific read function, meaning varies depending on device 
 *  Side Effects: fills buf with data that was read
 *  RETURN VALUE: ret : return value from specific read function, meaning varies depending on device
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes){
    sti(); // Allow interrupts
    
    if(fd < 0 || fd > MAX_OPEN_FILES || buf == NULL || nbytes < 0){
        return -1;
    }

    // making sure file descriptor is valid
    pcb_t* pcb = get_curr_pcb_ptr();
    if(pcb->file_descriptor_array[fd].flags == 0) {
        return -1;
    }

    //finding and calling specific read() system call
    int32_t ret = pcb->file_descriptor_array[fd].fop_jump_ptr->read(fd,buf,nbytes);
    return ret;
}



/* 
 *  write()
 *  DESCRIPTION: general system call for write(). uses jumptable
 *               to locate the proper write function then calls it. 
 *  INPUTS:  fd, buf, nbytes: standard inputs for write system call
 *  OUTPUTS: ret : return value from specific write function, meaning varies depending on device 
 *  Side Effects: writes data to terminal/device
 *  RETURN VALUE: ret : return value from specific write function, meaning varies depending on device
 */
int32_t write (int32_t fd, const void* buf, int32_t nbytes){
     if(fd < 0 || fd > MAX_OPEN_FILES || buf == NULL || nbytes < 0){
        return -1;
    }

    pcb_t* pcb = get_curr_pcb_ptr();
    if(pcb->file_descriptor_array[fd].flags == 0) {
        return -1;
    }
    int32_t ret = pcb->file_descriptor_array[fd].fop_jump_ptr->write(fd,buf,nbytes);
    return ret;
}



/* 
 *  open()
 *  DESCRIPTION: general system call for open(). uses jumptable
 *               to locate the proper open function then calls it. 
 *  INPUTS:  filename: standard inputs for open system call
 *  OUTPUTS: ret : return value from specific open function, meaning varies depending on device 
 *  Side Effects: depends, whatever the side effects of the specific open function called are
 *  RETURN VALUE: ret : return value from specific open function, meaning varies depending on device
 */
int32_t open (const uint8_t* filename){
    //Error Checks
    int i;
    dentry_t dentry;
    if(filename == NULL) return -1;
    if(read_dentry_by_name(filename, &dentry) == -1) return -1;

    //Allocate fdarray in pcb
    pcb_t* pcb = get_curr_pcb_ptr();
    for(i = 0; i<MAX_OPEN_FILES; i++){
        if(pcb->file_descriptor_array[i].flags == 0){
            pcb->file_descriptor_array[i].flags = 1;
            pcb->file_descriptor_array[i].inode = dentry.inode_num;
            pcb->file_descriptor_array[i].file_pos = 0;

            if(dentry.filetype == 0){
                //RTC Driver
                pcb->file_descriptor_array[i].fop_jump_ptr = &rtc_jump_table;                       // changes here
                // pcb->file_descriptor_array[i].fop_jump_ptr = &file_jump_table;                       //! debug_1112     // idk whcih one to use
            }else if (dentry.filetype == 1){
                pcb->file_descriptor_array[i].fop_jump_ptr = &directory_jump_table;
            }else if (dentry.filetype == 2){
                pcb->file_descriptor_array[i].fop_jump_ptr = &file_jump_table;
            }
            return i;
        }
    }
    return -1;
}



/* 
 *  close()
 *  DESCRIPTION: general system call for close(). uses jumptable
 *               to locate the proper close function then calls it. 
 *  INPUTS:  fd: standard input for close system call
 *  OUTPUTS: ret : return value from specific close function, meaning varies depending on device 
 *  Side Effects: depends, whatever the side effects of the specific close function called are
 *  RETURN VALUE: ret : return value from specific close function, meaning varies depending on device
 */
int32_t close (int32_t fd){
    if(fd < 2 || fd > MAX_OPEN_FILES){
        return -1;
    }
    int ret;
    pcb_t* pcb = get_curr_pcb_ptr();

    if(pcb->file_descriptor_array[fd].flags == 0)return -1;

    pcb->file_descriptor_array[fd].flags = 0;
    pcb->file_descriptor_array[fd].inode = 0;
    pcb->file_descriptor_array[fd].file_pos = 0;
    ret = pcb->file_descriptor_array[fd].fop_jump_ptr->close(fd);
    pcb->file_descriptor_array[fd].fop_jump_ptr = &empty_jump_table;

    return ret;
}


/* 
 *  getargs()
 *  DESCRIPTION: read the program’s command line arguments 
 *               and copy them into a user-level buffer
 *  INPUTS: buf: buffer to copy arguments into
 *          nbytes: maximum number of bytes for the arguments
 *  OUTPUTS: none
 *  Side Effects: arguments are copied into buf
 *  RETURN VALUE: 0 if success, -1 if failure
 */
int32_t getargs (uint8_t* buf, int32_t nbytes){
    pcb_t* cur_pcb = get_curr_pcb_ptr();
    // error checks
    if (buf == NULL) {
        return -1;
    }
    if (cur_pcb == NULL) {
        return -1;
    }
    if ((cur_pcb->argument[0] == '\0') || ((int32_t)strlen((int8_t*)cur_pcb->argument) > nbytes)) {      //changes here strlen doesnt work for ints so changed it to size of and added [0]
        return -1;
    }

    //flush_tlb();

    // memcpy(buf, cur_pcb->argument, nbytes);         // copy arguments into buf     //changes here strncpy doesnt copy ints so changed it to memcpy
    strncpy((int8_t*)buf, (int8_t*)cur_pcb->argument, nbytes);
    //*buf = (uint8_t)(0x84b8000);
    return 0;   // success
}

/* 
 *  vidmap()
 *  DESCRIPTION: unused for now
 *  INPUTS: screenstart
 *  OUTPUTS: 0
 *  Side Effects: none
 *  RETURN VALUE: 0
 */
int32_t vidmap (uint8_t** screen_start){    
    if((uint32_t)screen_start < ONE_TWENTY_EIGHT_MB || (uint32_t)screen_start > (VIDMAP_ADDR)){
        //Needs to be tested
        return -1;
    }
    page_directory[VIDMEM_DIRECTORY_IDX].present = 1;
    page_directory[VIDMEM_DIRECTORY_IDX].page_size = 0;
    page_directory[VIDMEM_DIRECTORY_IDX].user_supervisor = 1;// DPL = 3;
    page_directory[VIDMEM_DIRECTORY_IDX].address_31_12 = ((int)video_page_table)/FOUR_KB_ALIGN;


    video_page_table[0].present = 1;
    video_page_table[0].user_supervisor = 1; // DPL 3;

    if(running_terminal == view_terminal){
        video_page_table[0].address_31_12 = (VIDMEM_PHY_ADDRESS)/FOUR_KB_ALIGN;
    }else{
        video_page_table[0].address_31_12 = (terminal_array[running_terminal].terminal_video_buffer/FOUR_KB_ALIGN);
    }
    flush_tlb();

    *screen_start = (uint8_t*)(VIDMAP_ADDR);
    pcb_t* pcb = get_curr_pcb_ptr();
    pcb->vidmap_flag = 1;
    terminal_array[running_terminal].vidmap_flag = 1;
    return 0;
}

/* 
 *  set_handler()
 *  DESCRIPTION: unused for now
 *  INPUTS: signum, handler
 *  OUTPUTS: none
 *  Side Effects: none 
 *  RETURN VALUE: 0
 */
int32_t set_handler (int32_t signum, void* handler){
    return -1;
}


/* 
 *  sigreturn()
 *  DESCRIPTION: unused for now   
 *  INPUTS: none
 *  OUTPUTS: none
 *  Side Effects: none
 *  RETURN VALUE: 0
 */
int32_t sigreturn (void){
    return -1;
}



/* 
 *  empty_read()
 *  DESCRIPTION: function used for a device that should not need the read system call   
 *  INPUTS: fd, buf, nbyters: standard inputs for read system call, all unused
 *  OUTPUTS: -1
 *  Side Effects: none
 *  RETURN VALUE: -1
 */
int32_t empty_read (int32_t fd, void* buf, int32_t nbytes){
    return -1;
}


/* 
 *  empty_write()
 *  DESCRIPTION: function used for a device that should not need the write system call   
 *  INPUTS: fd, buf, nbyters: standard inputs for write system call, all unused
 *  OUTPUTS: -1
 *  Side Effects: none
 *  RETURN VALUE: -1
 */
int32_t empty_write (int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}


/* 
 *  empty_open()
 *  DESCRIPTION: function used for a device that should not need the open system call   
 *  INPUTS: filename: standard input for open system call
 *  OUTPUTS: -1
 *  Side Effects: none
 *  RETURN VALUE: -1
 */
int32_t empty_open (const uint8_t* filename){
    return -1;
}


/* 
 *  empty_close()
 *  DESCRIPTION: function used for a device that should not need the close system call 
 *  INPUTS: fd: standard input for close system call
 *  OUTPUTS: -1
 *  Side Effects: none
 *  RETURN VALUE: -1
 */
int32_t empty_close (int32_t fd){
    return -1;
}


/* 
 *  init_jump_tables()
 *  DESCRIPTION: initializes jumptables used for system calls  
 *  INPUTS: none
 *  OUTPUTS: none
 *  Side Effects: fills jumptables with proper functions for read, write, open, and close system calls 
 *  RETURN VALUE: none
 */
void init_jump_tables(){
    //empty jumptable functions
    empty_jump_table.read = empty_read;
    empty_jump_table.write = empty_write;
    empty_jump_table.open = empty_open;
    empty_jump_table.close = empty_close;

    //file jumptable functions
    file_jump_table.read = file_read;
    file_jump_table.write = file_write;
    file_jump_table.open = file_open;
    file_jump_table.close = file_close;

    //directory jumptable functions
    directory_jump_table.read = directory_read;
    directory_jump_table.write = directory_write;
    directory_jump_table.open = directory_open;
    directory_jump_table.close = directory_close;

    //stdin jumptable functions
    stdin_jump_table.read = terminal_read;
    stdin_jump_table.write = empty_write; // Just a empty function;
    stdin_jump_table.open = terminal_open;
    stdin_jump_table.close = terminal_close;

    //stdout jumptable functions
    stdout_jump_table.read = empty_read;
    stdout_jump_table.write = terminal_write; 
    stdout_jump_table.open = terminal_open;
    stdout_jump_table.close = terminal_close;

    //rtc jumptable functions                           //changes here
    rtc_jump_table.read = (void*)rtc_read;               //idk why im getting assign from incomp ptr type
    rtc_jump_table.write = rtc_write; 
    rtc_jump_table.open = rtc_open;
    rtc_jump_table.close = rtc_close;

    return;
}
