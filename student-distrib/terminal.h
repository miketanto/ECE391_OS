#ifndef TERMINAL_T_DEFINED
#define TERMINAL_T_DEFINED


#include "lib.h"
#include "keyboard.h"

#define MAX_TERMINAL 3
#define BUFFER_SIZE 128

#define T1_VID_ADDRESS 0xBA000
#define T2_VID_ADDRESS 0xBB000
#define T3_VID_ADDRESS 0xBC000

typedef struct terminal_t {
    char keyboard_buffer[BUFFER_SIZE];
    uint8_t ter_buffer[BUFFER_SIZE];
    uint8_t bufffer_head;
    uint32_t terminal_video_buffer;
    int cur_x;
    int cur_y;
    int vidmap_flag;
    //int open;
    volatile int enter_pressed;
} terminal_t;


// terminal_t* view_terminal_struct;
extern terminal_t terminal_array[MAX_TERMINAL];
terminal_t* terminal_0;

extern int running_terminal;
extern int view_terminal;

// volatile int enter_pressed_flag;


void terminal_init();
int terminal_switch(int prev_terminal, int target_terminal);
int terminal_open();
int terminal_close();

// int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
// int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);


void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor();
void update_cursor(int x, int y);
uint16_t get_cursor_position(void);

void update_video_memory_paging(int terminal);

#endif
