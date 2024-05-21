#include "types.h"
#include "lib.h"
#include "i8259.h"


// Define the keyboard interrupt number
#define KEYBOARD_IRQ_NUM            1
#define SCAN_CODE_RELEASE_OFFSET    0x80

// Define the keyboard port number
#define KEYBOARD_PORT 0x60

#define BUFFER_SIZE 128


// Initialize the keyboard
void keyboard_init();

// Keyboard interrupt handler
extern void keyboard_handler();

extern uint8_t keyboard_buffer[BUFFER_SIZE];
extern uint8_t bufffer_head;

extern int terminal_switch_flag;
