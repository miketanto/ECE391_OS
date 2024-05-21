#ifndef SYSCALL_H
#define SYSCALL_H

#include "lib.h"
#include "types.h"

#define MAX_OPEN_FILES 8
#define MAX_PROCESS_ID 6
#define EIGHT_MB 0x800000
#define FOUR_MB 0x400000
#define ONE_TWENTY_EIGHT_MB 0x8000000
#define EIGHT_KB 8192

//Understanding execute, PCB, TSS, and IRET
// EXECUTE
// Parse Args || Check File Validity || Set up Paging 
// Load File into Mem || Create PCB/Open FD
// Prepare for Context Switch
    // Current Kernel ESP is needed for TSS esp0 to switch back
    // Need to specify ESP of Task which is the bottom of 4MB of Program IMage
    // Specify EIP of Task which is Byte 24-27 of the File
    //
    // Need to keep the current ESP to return to 
    // Keep current ebp for ? 
// Push IRET Context to stack 
    // IRET Context needed:
    // USER DS, EIP, ESP, EFLAG, CS
// IRET

//struct for jumptable
typedef struct __attribute__((packed)) fop_jump_t{
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* fname);
    int32_t (*close)(int32_t fd);
}fop_jump_t;

//struct for File Descriptor 
typedef struct __attribute__((packed)) file_desc_t{
    fop_jump_t* fop_jump_ptr; // Appropriate pointer to
    uint32_t inode; // Inode index of reading
    uint32_t file_pos; // Current offset in file
    uint32_t flags; // Mark as in use
}file_desc_t;

//struct for Process Control Block
typedef struct __attribute__((packed)) pcb_t{
    uint8_t process_id;
    uint8_t parent_process_id;
    uint32_t task_esp;// Need to specify ESP of Task which is the bottom of 4MB of Program IMage
    uint32_t task_eip;// Specify EIP of Task which is Byte 24-27 of the File
    uint32_t execute_esp; // Need to keep the current ESP to return to execute
    uint32_t execute_ebp; // EBP needs to be saved because of something
    uint32_t tss_esp0;
    uint32_t schedule_esp;
    uint32_t schedule_ebp;
    uint8_t vidmap_flag;
    file_desc_t file_descriptor_array[MAX_OPEN_FILES];
    uint8_t argument[128];
}pcb_t;

//jumptables for various system calls
fop_jump_t file_jump_table;
fop_jump_t directory_jump_table;
fop_jump_t exec_jump_table;
fop_jump_t stdin_jump_table;
fop_jump_t stdout_jump_table;
fop_jump_t empty_jump_table;
fop_jump_t rtc_jump_table;                      //changes here


//Functions for system calls
extern void syscall_handler();
extern int32_t system_execute(const uint8_t* command);
extern int32_t system_halt(uint8_t status);
extern int32_t system_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t system_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t system_open (const uint8_t* filename);
extern int32_t system_close (int32_t fd);

extern void ret_execute(uint32_t execute_ebp, uint32_t execute_esp, uint8_t status);
extern void ret_schedule(uint32_t schedule_ebp, uint32_t schedule_esp);
void init_jump_tables();

int32_t halt (uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler);
int32_t sigreturn (void);

pcb_t* get_curr_pcb_ptr();
pcb_t* get_pcb_ptr(int process_id);

extern int curr_process_id[MAX_TERMINAL];
extern int active_process[MAX_TERMINAL];
#endif

