#include "keyboard.h"
#include "terminal.h"
// Reference: https://wiki.osdev.org/PS/2_Keyboard
// Reference: https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html


// special keys flags
// 0: not pressed, 1: pressed
int caps_lock_pressed = 0;
int shift_pressed = 0;
int ctrl_pressed = 0;
int alt_pressed = 0;        // dont know if I need to handle this

// int line_wrap_around = 0;

uint8_t keyboard_buffer[BUFFER_SIZE];
uint8_t bufffer_head = 0;
int terminal_switch_flag = 0;


// mapping scan_codes to ASCII characters     // make it into a 3D array
uint8_t keyboard_scan_code_to_ascii[3][128] = 
{
    {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8',           // 0x00 - 0x09
        '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r',     // 0x0A - 0x13
        't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,        // 0x14 - 0x1D
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',       // 0x1E - 0x27
        '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n',       // 0x28 - 0x31
        'm', ',', '.', '/', 0, '*', 0, ' ', 0, ' ',               // 0x32 - 0x3B
        ' ', ' ', 0, 0, 0, 0, 0, 0, 0, 0,                           // 0x3C - 0x45
        0, '7', '8', '9', '-', '4', '5', '6', '+', '1',         // 0x46 - 0x4F      // numpad
        '2', '3', '0', '.', 0, 0, 0, 0, 0, 0,                   // 0x50 - 0x59
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                           // 0x5A - 0x63
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                           // 0x64 - 0x6D
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                           // 0x6E - 0x77
        0, 0, 0, 0, 0, 0, 0, 0                                  // 0x78 - 0x80
    },

    // unsigned char shift_scan_code_to_ascii[128] = 
    {
        0, 0, '!', '@', '#', '$', '%', '^', '&', '*',           // 0x00 - 0x09
        '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R',     // 0x0A - 0x13
        'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,        // 0x14 - 0x1D
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',       // 0x1E - 0x27
        '\"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N',        // 0x28 - 0x31
        'M', '<', '>', '?', 0, '*', 0, ' ', 0, ' ',             // 0x32 - 0x3B
        ' ', ' ', 0, 0, 0, 0, 0, 0, 0, 0,                       // 0x3C - 0x45
        0, '7', '8', '9', '-', '4', '5', '6', '+', '1',         // 0x46 - 0x4F      // numpad
        '2', '3', '0', '.', 0, 0, 0, 0, 0, 0,                   // 0x50 - 0x59
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                           // 0x5A - 0x63
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                           // 0x64 - 0x6D
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                           // 0x6E - 0x77
        0, 0, 0, 0, 0, 0, 0, 0                                  // 0x78 - 0x80
    },

    // unsigned char caps_lock_scan_code_to_ascii[128] = 
    {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8',           // 0x00 - 0x09
        '9', '0', '-', '=', '\b', '\t', 'Q', 'W', 'E', 'R',     // 0x0A - 0x13
        'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', 0,        // 0x14 - 0x1D
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',       // 0x1E - 0x27
        '\'', '`', 0, '\\', 'Z', 'X', 'C', 'V', 'B', 'N',       // 0x28 - 0x31
        'M', ',', '.', '/', 0, '*', 0, ' ', 0, ' ',               // 0x32 - 0x3B
        ' ', ' ', 0, 0, 0, 0, 0, 0, 0, 0,                           // 0x3C - 0x45
        0, '7', '8', '9', '-', '4', '5', '6', '+', '1',         // 0x46 - 0x4F      // numpad
        '2', '3', '0', '.', 0, 0, 0, 0, 0, 0,                   // 0x50 - 0x59
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                           // 0x5A - 0x63
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                           // 0x64 - 0x6D
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                           // 0x6E - 0x77
        0, 0, 0, 0, 0, 0, 0, 0                                  // 0x78 - 0x80
    }
};


/* keyboard_init
 *   Description: Initialize the keyboard by sending initialization commands to the keyboard controller, 
 *                and enable the keyboard interrupt on the PIC at IRQ1
 *   Inputs: none
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: Initialize the keyboard, enable the keyboard interrupt on the PIC at IRQ1
*/
void keyboard_init() {
    enable_irq(KEYBOARD_IRQ_NUM);               // enable the keyboard interrupt on the PIC
}


/* keyboard_handler
 *   Description: Handle the keyboard interrupt, print the corresponding character 
 *                to the screen if it's a key press
 *   Inputs: none
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: Print the corresponding character to the screen
*/
void keyboard_handler() {
    /* Checkpoint 1
    cli();

    uint8_t scan_code = inb(KEYBOARD_PORT);                 // read the scan code

    if (scan_code < SCAN_CODE_RELEASE_OFFSET) {             // if the scan code is a key press, not a key release
        putc(keyboard_scan_code_to_ascii[scan_code]);       // print the corresponding character to the screen.
    }

    send_eoi(KEYBOARD_IRQ_NUM);
    sti();
    */

    // TODO: Checkpoint 2
    // printf("enter keyboard_handler\n");                  // debug

    cli();
    uint8_t scan_code = inb(KEYBOARD_PORT);                 // read the scan code
    //printf("%d", scan_code);
    send_eoi(KEYBOARD_IRQ_NUM);     // end interrupt first so we can receive the next key press

    if (scan_code == 0x3A) {                                // caps lock
        caps_lock_pressed = !caps_lock_pressed;
    }
    else if ((scan_code == 0x2A) || (scan_code == 0x36)) {  // shift
        // printf("shift_pressed = 1\n");                              // debug
        shift_pressed = 1;
    }
    else if ((scan_code == 0xAA) || (scan_code == 0xB6)) {  // shift release
        shift_pressed = 0;
    }
    else if (scan_code == 0x1D) {                           // ctrl
        ctrl_pressed = 1;
    }
    else if (scan_code == 0x9D) {                           // ctrl release
        ctrl_pressed = 0;
    }
    else if (scan_code == 0x38) {                           // alt
        alt_pressed = 1;
    }
    else if (scan_code == 0xB8) {                           // alt release
        alt_pressed = 0;
    }
    // else if (scan_code == 0x0E) {                           // backspace
    //     backspace();        // TODO
    // }
    // else if (ctrl_pressed && ((scan_code == 0x26))) {            // ctrl + L
    // // if (scan_code == 0x26) {            // L
    //     clear();
    // }


    if ((scan_code < SCAN_CODE_RELEASE_OFFSET) && (keyboard_scan_code_to_ascii[0][scan_code] != 0)) {
        // printf("enter 1 ");

        if (bufffer_head == BUFFER_SIZE) {
            // printf("buffer full");
            if (scan_code == 0x1C) {            // enter pressed
                // todo: new line and everything
                // printf("enter pressed");
                keyboard_buffer[bufffer_head] =  keyboard_scan_code_to_ascii[0][scan_code];
                update_video_memory_paging(view_terminal);
                putc(keyboard_scan_code_to_ascii[0][scan_code]);
                update_cursor(get_screen_x(), get_screen_y());
                update_video_memory_paging(running_terminal);
                bufffer_head = 0;               // empty the buffer

                // terminal_0->enter_pressed = 1;  // set enter_pressed flag to 1
                terminal_array[view_terminal].enter_pressed = 1;        //! debug_1112
            }
            else if (scan_code == 0x0E) {       // backspace pressed
                // TODO: need to keep track of where the prev line's last char is
                update_video_memory_paging(view_terminal);
                putc(keyboard_scan_code_to_ascii[1][scan_code]);  // todo: need to handle backspace in putc
                update_cursor(get_screen_x(), get_screen_y());
                update_video_memory_paging(running_terminal);
                      
                if (bufffer_head > 0) {
                    bufffer_head--;     // remove one char from the buffer
                }
            }
            else if (ctrl_pressed) {            // ctrl + L
                uint8_t scan_code = inb(KEYBOARD_PORT);
                if (scan_code == 0x26) {
                    clear();
                }
                bufffer_head = 0;       // empty the buffer
            }
        } 
        else {      // if buffer is not full
            // printf("enter 2 ");
            if ((bufffer_head == 73) && (scan_code != 0x0E)) {        // TODO: line wrap around
                // printf("line wrap around\n");              // debug
                update_video_memory_paging(view_terminal);
                putc('\n');
                update_cursor(get_screen_x(), get_screen_y());
                update_video_memory_paging(running_terminal);
            }

            if (scan_code == 0x1C) {            // enter pressed
                // todo: new line and everything
                // printf("enter pressed");
                keyboard_buffer[bufffer_head] =  keyboard_scan_code_to_ascii[0][scan_code];
                update_video_memory_paging(view_terminal);
                putc(keyboard_scan_code_to_ascii[0][scan_code]);
                update_cursor(get_screen_x(), get_screen_y());
                update_video_memory_paging(running_terminal);
                bufffer_head = 0;       // empty the buffer
                terminal_array[view_terminal].enter_pressed = 1;     //! debug_1112
                // line_wrap_around = 0;

                // terminal_0->enter_pressed = 1;  // set enter_pressed flag to
            }
            else if (scan_code == 0x0E) {        // backspace pressed
                // TODO: need to keep track of where the prev line's last char is
                
                // todo: need to handle backspace in putc
                if (bufffer_head > 0) {
                    update_video_memory_paging(view_terminal);
                    putc(keyboard_scan_code_to_ascii[1][scan_code]);
                    update_cursor(get_screen_x(), get_screen_y());
                    update_video_memory_paging(running_terminal);
                    bufffer_head--;     // remove one char from the buffer
                }
            }
            else {
                // printf("enter 3 ");
                // printf("scan_code: %d\n", scan_code);                       // debug
                
                if (shift_pressed) {    // Upper case
                    // printf("enter shift_pressed if statement\n");           // debug
                    uint8_t scan_code = inb(KEYBOARD_PORT);
                    // printf("scan_code: %d\n", scan_code);                   // debug
                    if (scan_code < SCAN_CODE_RELEASE_OFFSET) {
                        keyboard_buffer[bufffer_head] = keyboard_scan_code_to_ascii[1][scan_code];
                        update_video_memory_paging(view_terminal);
                        putc(keyboard_scan_code_to_ascii[1][scan_code]);
                        update_cursor(get_screen_x(), get_screen_y());
                        update_video_memory_paging(running_terminal);
                        bufffer_head++;
                    }
                }
                else if (caps_lock_pressed) {   // Upper case
                    uint8_t scan_code = inb(KEYBOARD_PORT);
                    if (scan_code < SCAN_CODE_RELEASE_OFFSET) {
                        keyboard_buffer[bufffer_head] =  keyboard_scan_code_to_ascii[2][scan_code];
                        update_video_memory_paging(view_terminal);
                        putc(keyboard_scan_code_to_ascii[2][scan_code]);
                        update_cursor(get_screen_x(), get_screen_y());
                        update_video_memory_paging(running_terminal);
                        bufffer_head++;
                    }
                }
                else if (ctrl_pressed) {    // ctrl + L
                    // printf("enter ctrl_pressed if statement\n");               // debug
                    uint8_t scan_code = inb(KEYBOARD_PORT);
                    if (scan_code == 0x26) {
                        // printf("ctrl + L\n");                                  // debug
                        clear();
                        terminal_array[view_terminal].enter_pressed = 1;       //! debug_1202
                    }
                    bufffer_head = 0;       // empty the buffer
                }
                else if (alt_pressed){
                    uint8_t scan_code = inb(KEYBOARD_PORT);
                    //printf("%d", scan_code);
                    if(scan_code == 0x3B){//&& !terminal_switch_flag
                        terminal_switch(view_terminal,0);
                        
                        //printf("TERMINAL 1\n");
                    }else if (scan_code == 0x3C){
                        terminal_switch(view_terminal,1);
                        terminal_switch_flag = 1;
                        //printf("TERMINAL 2\n");
        
                    }else if (scan_code == 0x3D){
                        terminal_switch(view_terminal,2);
                        terminal_switch_flag = 1;
                        //printf("TERMINAL 3\n");
                    }
                }else {    // Lower case
                    // printf("enter 4 ");
                    if(scan_code != 0x3B && scan_code != 0x3C && scan_code != 0x3D){
                        keyboard_buffer[bufffer_head] = keyboard_scan_code_to_ascii[0][scan_code];
                        update_video_memory_paging(view_terminal);
                        putc(keyboard_scan_code_to_ascii[0][scan_code]);
                        update_cursor(get_screen_x(), get_screen_y());
                        update_video_memory_paging(running_terminal);
                        bufffer_head++;
                    }
                }
            }
        }
    }

    // send_eoi(KEYBOARD_IRQ_NUM);
    sti();
}
