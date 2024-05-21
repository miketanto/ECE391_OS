#include "terminal.h"
#include "paging.h"
#include "syscall.h"

#define NUM_COLS        80
#define NUM_ROWS        25
#define VIDEO_MEM_SIZE  4096



terminal_t terminal_array[MAX_TERMINAL];
int running_terminal = 0;
int view_terminal = 0;

/* 
 *  terminal_init()
 *  DESCRIPTION: initialize the terminals array, set the default terminal to be the first terminal
 *               and set the keyboard buffer to be the first terminal's keyboard buffer
 *               (for next week, only one terminal now)    
 *  INPUTS: noun
 *  OUTPUTS: none
 *  Side Effects: change the content in the terminals array accordingly
 *  RETURN VALUE: none
 */
 void terminal_init() {		
	int i;
	for (i = 0; i < MAX_TERMINAL; i++) {
		terminal_array[i].terminal_video_buffer = (uint32_t) (VIDMEM_PHY_ADDRESS + (i + 1)*FOUR_KB_ALIGN);
		terminal_array[i].cur_x = 0;
		terminal_array[i].cur_y = 0;
		terminal_array[i].enter_pressed = 0;
	}
	terminal_0 = &terminal_array[0];
	set_screen_x(&terminal_array[0].cur_x);
	set_screen_x(&terminal_array[0].cur_y);
	running_terminal = 0;
	view_terminal = 0;
}


/* 
 *  terminal_open()
 *  DESCRIPTION: open a new terminal   
 *  INPUTS: noun
 *  OUTPUTS: none
 *  Side Effects: create a new terminal
 *  RETURN VALUE: none
 */
int terminal_open(){
	return 0;
}




/* 
 * 	terminal_switch()
 *  DESCRIPTION: switches terminal from the terminal indexed at prev_terminal to the terminal indexed at 
 * 				 target terminal
 *  INPUTS: prev_terminal = terminal to switch from, all relavent data to return to this terminal at 
 * 							a later time will be saved
 * 			target_terminal = terminal to switch to, video memory is updated so that this terminal is
 * 							  displayed and becomes the terminal that all keyboard inputs go to.
 *  OUTPUTS: returns -1 upon failure
 * 			 returns 0 if successful
 *  SIDE EFFECTS: updates video memory so that the correct terminal is displayed to the screen
 * 
 */
int terminal_switch(int prev_terminal, int target_terminal) {
	//testing if the target_terminal is invalid
	if(target_terminal < 0 || target_terminal > 2){					//if we somehow get an invalid term return -1 as a fail
		return -1;
	}

	//save the stuff from prev term
	//uint16_t x = get_cursor_position();						//get the prev terms cursor postion and save it in the struc
	//uint16_t y= x/NUM_COLS;
	//x = x%NUM_ROWS;

	//save terminal info
	terminal_array[prev_terminal].bufffer_head = bufffer_head; //save the buffer head of prev term 
	terminal_0 = &terminal_array[target_terminal];
	set_screen_x(&terminal_0->cur_x);
	set_screen_y(&terminal_0->cur_y);
	update_cursor(terminal_0->cur_x, terminal_0->cur_y);

	//save keyboard buffer
	memcpy((void*)terminal_array[prev_terminal].keyboard_buffer, (void*)keyboard_buffer, BUFFER_SIZE);
	memcpy((void*)keyboard_buffer, (void*)terminal_0->keyboard_buffer, BUFFER_SIZE);

	//updating video memory
	update_video_memory_paging(prev_terminal);
	//put everthing in video mem in a different place
	memcpy((void*)terminal_array[prev_terminal].terminal_video_buffer,(void*) VIDMEM_PHY_ADDRESS, FOUR_KB_ALIGN);
	memcpy((void*)VIDMEM_PHY_ADDRESS, (void*)terminal_0->terminal_video_buffer, FOUR_KB_ALIGN);
	view_terminal = target_terminal;
	update_video_memory_paging(running_terminal);

	return 0;
}


/* 
 *  terminal_close()
 *  DESCRIPTION: close the terminal   
 *  INPUTS: noun
 *  OUTPUTS: none
 *  Side Effects: none
 *  RETURN VALUE: none
 */
int terminal_close() {
	return -1;
}


/* 
 *  terminal_write()
 *  DESCRIPTION: write the content in the buffer to the terminal
 *  INPUTS: *input_buf - pointer to the buffer to be written
 * 			in_size - size of the buffer to be written
 *  OUTPUTS: content in the buffer
 *  Side Effects: none
 *  RETURN VALUE: size of the buffer written if successful,
 * 				  -1 if failed
 */
// int32_t terminal_write (int32_t fd, const void* input_buf, int32_t in_size) {
int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes) {
	if (buf == NULL)
		return -1;			// nothing to write

	int i;
	//writing data to terminal
	for (i = 0; i < nbytes; i++) {
		if(((char*) buf)[i] != '\0')putc(((char*)buf)[i]);
	}
	if(running_terminal == view_terminal){
		update_cursor(get_screen_x(), get_screen_y());
	}
	return nbytes;
}


/* 
 *  terminal_read()
 *  DESCRIPTION: read the content in the buffer to the terminal
 *  INPUTS: *buf - pointer to the buffer to be read
 * 			in_size - size of the buffer to be read
 *  OUTPUTS: none
 *  Side Effects: change the terminal buffer
 *  RETURN VALUE: size of the buffer read if successful,
 * 				  -1 if failed
 */
// int32_t terminal_read(int32_t fd, uint32_t offset, void* buf, int32_t in_size) {
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {
	// printf("terminal_read reached\n");			// debug

	if (buf == NULL)
		return -1;			// nothing to read
	if (nbytes < 0)
		return 0;			// nothing to read
	if (nbytes > BUFFER_SIZE) 
		nbytes = BUFFER_SIZE;			// overflow


	// printf("waiting for enter press\n");		// debug
	// while (!(terminal_0->enter_pressed)) {		// wait until enter is pressed
	while (!terminal_array[running_terminal].enter_pressed) {			// wait until enter is pressed
		// do nothing
	}
	
	//local variables
	int i;
	int count = 0;

	//reading data from terminal into buffer
	for (i = 0; i < nbytes; i++) {
		((char*)buf)[i] = keyboard_buffer[i];
		keyboard_buffer[i] = ' ';
		if (((char*)buf)[i] == '\n') {
			count = i + 1;
			break;
		}
		if((i == (nbytes - 1)) && (((char*)buf)[i] != '\n')){
                ((char*) buf)[i] = '\n';
                count = i + 1;
                break;
        }else {
			count++;
		}
	}
	terminal_array[running_terminal].enter_pressed = 0;
	return count;
}


// cursor reference: https://wiki.osdev.org/Text_Mode_Cursor

/* 
 *  enable_cursor()
 *  DESCRIPTION: enable the cursor to be displayed on the screen
 *  INPUTS: noun
 *  OUTPUTS: none
 *  Side Effects: enable the cursor to be displayed on the screen
 *  RETURN VALUE: none
 */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
	outb(0x0A, 0x3D4);
	outb((inb(0x3D5) & 0xC0) | cursor_start, 0x3D5);
 
	outb(0x0B, 0x3D4);
	outb((inb(0x3D5) & 0xE0) | cursor_end, 0x3D5);
}


/* 
 *  disable_cursor()
 *  DESCRIPTION: disable the cursor to be displayed on the screen
 *  INPUTS: noun
 *  OUTPUTS: none
 *  Side Effects: dsable the cursor to be displayed on the screen
 *  RETURN VALUE: none
 */
void disable_cursor()
{
	outb(0x0A, 0x3D4);
	outb(0x20, 0x3D5);
}


/* 
 *  update_cursor()
 *  DESCRIPTION: update the cursor position to be displayed on the screen
 *  INPUTS: x - x coordinate of the cursor
 * 			y - y coordinate of the cursor
 *  OUTPUTS: none
 *  Side Effects: update the cursor position to be displayed on the screen
 *  RETURN VALUE: none
 */
void update_cursor(int x, int y)
{
	uint16_t pos = y * NUM_COLS + x;

	outb(0x0F, 0x3D4);
	outb((uint8_t) (pos & 0xFF), 0x3D5);
	outb(0x0E, 0x3D4);
	outb((uint8_t) ((pos >> 8) & 0xFF), 0x3D5);
}


/* 
 *  get_cursor_position()
 *  DESCRIPTION: get the cursor position displayed on the screen
 *  INPUTS: none
 *  OUTPUTS: none
 *  Side Effects: none
 *  RETURN VALUE: cursor position = y * NUM_COLS + x
 * 				  To obtain the coordinates: y = pos / NUM_COLS; x = pos % NUM_COLS;
 */
uint16_t get_cursor_position(void)
{
    uint16_t pos = 0;
    outb(0x0F, 0x3D4);
    pos |= inb(0x3D5);
	outb(0x0E, 0x3D4);
    pos |= ((uint16_t)inb(0x3D5)) << 8;
    return pos;
}


/* 
 *  update_video_memory_paging()
 *  DESCRIPTION: updates main video memory to point to the terminal given by the input terminal
 *  INPUTS: terminal = terminal whose video memory we want to be able to access through paging
 *  OUTPUTS: none
 *  SIDE EFFECTS: updates video memory so that future accesses to video memory access the video memory
 * 				  for the correct terminal
 */
void update_video_memory_paging(int terminal){

	//Update the main video memory to point to the appropriate terminal or current terminal
	//Update the Vidmem index to either present or not depending on the vidmap flag that is set in vidmap?
	//Update the video page table to point to the appropriate 4kb page.
	set_screen_x(&terminal_array[terminal].cur_x);
	set_screen_y(&terminal_array[terminal].cur_y);
	if(terminal != view_terminal){
		page_table[(VIDMEM_PHY_ADDRESS/FOUR_KB_ALIGN)].address_31_12 = (terminal_array[terminal].terminal_video_buffer/FOUR_KB_ALIGN);
        video_page_table[0].present = terminal_array[terminal].vidmap_flag;
        video_page_table[0].address_31_12 = (terminal_array[terminal].terminal_video_buffer/FOUR_KB_ALIGN);
	}else{
		page_table[(VIDMEM_PHY_ADDRESS/FOUR_KB_ALIGN)].address_31_12 = (VIDMEM_PHY_ADDRESS/FOUR_KB_ALIGN);
        video_page_table[0].present = terminal_array[terminal].vidmap_flag;
        video_page_table[0].address_31_12 = (VIDMEM_PHY_ADDRESS/FOUR_KB_ALIGN);
	}
	flush_tlb();
}
