#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "fs.h" 
#include "paging.h"
#include "terminal.h"
#include "rtc.h"
#include "syscall.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* Boundary Test
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Boundary Handling
 * Files: None
*/
int bound_range_exception_test(){
	TEST_HEADER;
	int arr[5] = {1, 2, 3, 4, 5};
	int value;
	value = arr[10];
	return FAIL;
}

// add more tests here

/* Divid by 0 Test
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Divide by 0 Handling
 * Files: None
*/
int div_exception_test(){
	TEST_HEADER;
	int a = 0;
	int b;
	b = 1/a;
	return FAIL;
}

/* Null Pointer Dereference Test
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Null Pointer Dereference Handling
 * Files: None
*/
int null_ptr_deref_test(){
	TEST_HEADER;

	int* ptr = NULL;
	int a;
	a = *(ptr);
	
	return FAIL;
}

/* Outside of Bounds Page Test
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Page Fault Handling
 * Files: paging.h/S
*/
int out_of_bounds_page_test(){
	TEST_HEADER;

	int* ptr = (int*)(0x800000 + 8); // This is memory location past 8MB
	int testVar;
	testVar = *(ptr);
	return FAIL;
}

/* Video Memory Page Test
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Page Fault Handling
 * Files: paging.h/S
*/
int vid_mem_page_test(){
	TEST_HEADER;

	int* ptr = (int*)(0xB8000 + 4);
	int testVar; 
	testVar = *ptr;
	return PASS;
}

/* Checkpoint 2 tests */
/* File System Memory Test
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Boot block file location check
 * Files: fs.c/h
*/
int file_system_page_test(){
	TEST_HEADER;
	init_filesystem();
	printf("BOOT BLOCK Inode Count %d\n", n_inodes);
	return PASS;
}

/* Read Directory Entry Test
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Testing read_dentry_by_name function
 * Files: fs.c/h
*/
int read_dentry_test(){
	TEST_HEADER;
	dentry_t dentry;
	int32_t ret;
	const uint8_t* arg;
	ret = read_dentry_by_name(arg, &dentry);
	printf("Dentry FileName: %s\n", dentry.filename);
	printf("Dentry Filetype: %d\n", dentry.filetype);
	return PASS;
}

/*Print all directory entries
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Print all directory entries from the boot block
 * Files: fs.c/h
*/
int print_all_direntry_names(){
	TEST_HEADER;
	int i;
	dentry_t* direntries = boot_block->direntries;
	for( i = 0 ; i < boot_block->dir_count; i++){
		printf("Dentry %d FileName: %s\n", i, direntries[i].filename);
	}
	return PASS;
}

/*Directory read test
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Directory Open and Read function, print all the filenames, types and sizes.
 * Files: fs.c/h
*/

int directory_read_test(){
	TEST_HEADER;
	uint8_t buf[32];
	dentry_t dentry;
	inode_t* inode_block_ptr;
	int ret;
	const uint8_t* arg;
	clear();
	directory_open(arg);
	while(directory_read(0,buf,0) != 0){
		ret = read_dentry_by_name(buf, &dentry);
		inode_block_ptr = (inode_t*)(inode_start + (dentry.inode_num));
		printf("Filename: %s    Inode: %d    Filetype: %d    Filesize: %d \n", buf, dentry.inode_num, dentry.filetype, inode_block_ptr->length);
	}
	return PASS;
}

/*File read test
 * Inputs: fname: Filename for contents to be printed.
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: File open and read, print the contents of file passed in as argument
 * Files: fs.c/h
*/
int read_data_test(const uint8_t* fname){
	int i;
	uint32_t ret;
    dentry_t dentry;
	uint32_t bytes_read;
	uint8_t* address = (uint8_t*)(0x8048000);
    ret = read_dentry_by_name(fname, &dentry);
    bytes_read = read_data(dentry.inode_num, 0, address, 5605);
	clear();
	printf("BYTES READ: %d\n", bytes_read);
	for(i =0; i <bytes_read ;i++){
			if((*address)!=NULL) putc((*address));
			address++;
	}
	return 0;
}

int file_read_test(const uint8_t* fname){
	TEST_HEADER;
	int i;
	uint32_t ret;
	uint8_t buf[4096];
	clear();
	ret = file_open(fname);
	if(ret == -1) return FAIL;
	while((ret = file_read(0, buf, 4096)) != -1){
		for(i =0; i <ret ;i++){
			if(buf[i]!=NULL) putc(buf[i]);
		}
	}
    printf("\n");
	return PASS;
}

int terminal_test(void* buf, int32_t in_size) {
	TEST_HEADER;
	uint32_t ret;
	clear();

	while (1) {
		printf("terminal_read\n");
		ret = terminal_read(0,buf, in_size);
		printf("terminal_read return: %d\n", ret);
		printf("Terminal Read Buffer: %s\n", buf);

		// printf("terminal_write\n");
		// ret = terminal_write(0, terminal_0->ter_buffer, in_size);
		// printf("terminal_write return:", ret);
	}
    printf("\n");
	return PASS;
}

void terminal_test_2(){
	clear();

	char buf[BUFFER_SIZE];
	int read_ret, write_ret;

	TEST_HEADER;
	// printf("reached 1\n");

	while (1)
	{
		printf("Input: ");
		read_ret = terminal_read(0, buf, BUFFER_SIZE);		

		// printf("\nreached 2\n");

		printf("Output: ");
		write_ret = terminal_write(0, buf, read_ret);

		printf("\nterminal_read return: %d\n	terminal_write return: %d\n", read_ret, write_ret);
		printf("*************************************************************************\n");
	}

	clear();
}

/*Terminal write test
 * Inputs: buf: input buffer with characters to be printed
 * 		   in_size: no of characters to be printed
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Write contents of buf to the screen
 * Files: terminal.c/h
*/
int terminal_write_test(const void* buf, int32_t in_size) {
	TEST_HEADER;
	uint32_t ret;
	clear();

	terminal_write(0, terminal_0->ter_buffer, in_size);
	printf("terminal_write return:", ret);
	return PASS;
}

/* Terminal read test
 * Inputs: buf -- buffer to receive input of terminal
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Terminal read
 * Files: fs.c/h
*/

int terminal_read_test(const void* buf) {
	return PASS;
}

int rtc_driver_test(){
	TEST_HEADER;
	int32_t i,j;
	int32_t good =0;
	good +=rtc_open(NULL);						//testing that the rtc open rets 0
	for(i =2; i<=1024; i*=2 ){					//testing that the rtc writes for all valid freq
		good += rtc_write(NULL, &i, 4);
		for(j =0; j<i; j++){					//testing that rtc read rets 0;
			good+=rtc_read(NULL,NULL,0);
			printf("1");
		}
		printf("\n");
	}
	if(good ==0){								//so far all funcs work and write works with valid inputs 
		i=0;
		good += rtc_write(NULL, &i, 4);			//test rtc write with bad/ invalid input
		if(good == -1){							// if invalid input rets -1 all tests pass
			return PASS;
		}
	}
	return FAIL; 								//if it made it here it failed
	
}


/* Checkpoint 3 tests */
int program_page_test(){
	TEST_HEADER;

	int* ptr = (int*)(0x8048000); // This is memory location past 8MB
	int testVar;
	testVar = *(ptr);
	return PASS;
}

int out_program_page_test(){
	TEST_HEADER;

	int* ptr = (int*)(0x8000000); // This is memory location past 8MB
	int testVar;
	testVar = *(ptr);
	return FAIL;
}

/*For Unit Testing while building*/
/*int copy_program_page_test(){
	TEST_HEADER;

	
	int i;
    uint32_t ret;
	uint8_t buf[4096];
	int* ptr = (int*)(0x8048000); // This is memory location past 8MB
	
	load_program();
	clear();
	ret = file_open("shell");
	while((ret = file_read(0, buf, 4096)) != -1){
		for(i =0; i <ret ;i++){
			if(buf[i]!=NULL){
				putc(*(ptr));
				if(*(ptr) != buf[i])return FAIL;
				ptr++;
			}
		}
	}
	
	return PASS;
}*/

int syscall_test(){
	TEST_HEADER;
	clear();
	init_jump_tables();
	if(system_execute((const uint8_t *)"shell") == -1)return PASS;
	return FAIL;
}

int execute_paging_test(){
	TEST_HEADER;
	
	dentry_t command_dentry;
	uint32_t phys_address = EIGHT_MB;
	uint32_t ret;
	uint8_t buf[4];
	uint32_t start_val;
	
	init_jump_tables();
    page_directory[32].address_31_12 = phys_address/FOUR_KB_ALIGN;
    flush_tlb();

	read_dentry_by_name((const uint8_t *)"shell", &command_dentry);
	uint8_t* program_img_address = (uint8_t*)0x8048000;
    inode_t* length_ptr = (inode_t *)(inode_start+command_dentry.inode_num);
	ret = read_data(command_dentry.inode_num, 0, program_img_address, length_ptr->length); 

	// int i;
	// clear();
	// printf("BYTES READ: %d\n", ret);
	// for(i =0; i <ret ;i++){
	// 		if((*program_img_address)!=NULL) putc((*program_img_address));
	// 		program_img_address++;
	// }

	read_data(command_dentry.inode_num, 24, buf, sizeof(int32_t)); // gets start of user program to run
    start_val = *((int*)buf);
	return 0;
}
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	
	// TEST_OUTPUT("idt_test", idt_test()); //PASS
	// TEST_OUTPUT("null ptr test", null_ptr_deref_test()); //PASS
	// TEST_OUTPUT("Boot block test", file_system_page_test()); //PASS
	// TEST_OUTPUT("inbounds_test", bound_range_exception_test());
	// TEST_OUTPUT("division_test", div_exception_test());//PASS
	
	/*Paging Tests*/
	// TEST_OUTPUT("VidMem Page Check", vid_mem_page_test()); //PASS
	// TEST_OUTPUT("Paging Out of Bounds Test", out_of_bounds_page_test());//PASS
	// launch your tests here

	/*Filesystem tests*/
	// TEST_OUTPUT("Read Dentry Test", read_dentry_test()); //PASS
	// TEST_OUTPUT("Print Direntries", print_all_direntry_names());	
	// TEST_OUTPUT("Read File Data Test", read_data_test());
	// TEST_OUTPUT("Read Directory Test", directory_read_test());
	// TEST_OUTPUT("Read File Test", file_read_test("shell"));

	/*Terminal Tests*/
	// uint8_t input_buf[32] = "Hello World!";
	// TEST_OUTPUT("Terminal Test", terminal_test(input_buf, 12));
	// TEST_OUTPUT("Terminal Test", terminal_write_test(input_buf, 12));
	// terminal_test_2();

	/*RTC driver test*/
	// TEST_OUTPUT("rtc_driver_test", rtc_driver_test());//PASS

	// TEST_OUTPUT("Program Page Test", program_page_test());
	// TEST_OUTPUT("Outside Program Page Test", out_program_page_test());
	// TEST_OUTPUT("Copy Program Page Test", copy_program_page_test());
	// TEST_OUTPUT("System Execute Test", syscall_test());
	//read_data_test("shell");
	// execute_paging_test();

}
