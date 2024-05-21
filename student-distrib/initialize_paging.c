#include "paging.h"

/* init_paging()
 *   Description: Initialize Page directory table and page table for Paging and identity
 *                mapping of first 4 MB of memory. 
 *   Inputs: none
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: Initialize the Page table and directory
 *                 set the proper bits for CR0 and CR4 and fill CR3 with the memory location of page directory
*/
void init_paging(){
    int i;
    //Initialize Page Directory
    //Initialize page directory entry for first 4 MB
    page_directory[0].present = 1;
    page_directory[0].read_write = 1;
    page_directory[0].user_supervisor = 0;
    page_directory[0].write_through = 0;
    page_directory[0].cache_disable = 1;
    page_directory[0].accessed = 0;
    page_directory[0].dirty = 1;
    page_directory[0].page_size = 0;
    page_directory[0].global = 0;
    page_directory[0].available = 0;
    page_directory[0].address_31_12 = ((int)page_table)/FOUR_KB_ALIGN;
    
    //Initialize Page Directory for the Kernel Memory Page
    page_directory[1].present = 1;
    page_directory[1].read_write = 1;
    page_directory[1].user_supervisor = 0;
    page_directory[1].write_through = 0;
    page_directory[1].cache_disable = 1;
    page_directory[1].accessed = 0;
    page_directory[1].dirty = 0;
    page_directory[1].page_size = 1;
    page_directory[1].global = 0;
    page_directory[1].available = 0;
    page_directory[1].address_31_12 = KERNEL_PHY_ADDRESS/FOUR_KB_ALIGN;

    page_directory[PROG_DIRECTORY_IDX].present = 1;
    page_directory[PROG_DIRECTORY_IDX].read_write = 1;
    page_directory[PROG_DIRECTORY_IDX].user_supervisor = 1;
    page_directory[PROG_DIRECTORY_IDX].write_through = 0;
    page_directory[PROG_DIRECTORY_IDX].cache_disable = 1;
    page_directory[PROG_DIRECTORY_IDX].accessed = 0;
    page_directory[PROG_DIRECTORY_IDX].dirty = 0;
    page_directory[PROG_DIRECTORY_IDX].page_size = 1;
    page_directory[PROG_DIRECTORY_IDX].global = 0;
    page_directory[PROG_DIRECTORY_IDX].available = 0;
    page_directory[PROG_DIRECTORY_IDX].address_31_12 = PROG_PHY_ADDRESS/FOUR_KB_ALIGN;

    page_directory[VIDMEM_DIRECTORY_IDX].present = 0;
    page_directory[VIDMEM_DIRECTORY_IDX].read_write = 1;
    page_directory[VIDMEM_DIRECTORY_IDX].user_supervisor = 1;
    page_directory[VIDMEM_DIRECTORY_IDX].write_through = 0;
    page_directory[VIDMEM_DIRECTORY_IDX].cache_disable = 1;
    page_directory[VIDMEM_DIRECTORY_IDX].accessed = 0;
    page_directory[VIDMEM_DIRECTORY_IDX].dirty = 0;
    page_directory[VIDMEM_DIRECTORY_IDX].page_size = 0;
    page_directory[VIDMEM_DIRECTORY_IDX].global = 0;
    page_directory[VIDMEM_DIRECTORY_IDX].available = 0;
    page_directory[VIDMEM_DIRECTORY_IDX].address_31_12 = 0;
    //Initialize Page Table for 0-4MB with 4Kb Pages that are aligned
    for( i = 0 ; i < TABLE_SIZE ; i++){
        // if(i * FOUR_KB_ALIGN == VIDMEM_PHY_ADDRESS || (i * FOUR_KB_ALIGN >= T1_VID_ADDRESS && i * FOUR_KB_ALIGN <=T3_VID_ADDRESS)){
        //     page_table[i].present = 1; // This memory space is used for video memory
        // }else{
        //     page_table[i].present = 0;
        // }
        page_table[i].present = 1;
        page_table[i].read_write = 1;
        page_table[i].user_supervisor = 0;
        page_table[i].write_through = 0;
        page_table[i].cache_disable = 1;
        page_table[i].accessed = 0;
        page_table[i].dirty = 0;
        page_table[i].page_attribute_table = 0;
        page_table[i].global = 1;
        page_table[i].available = 0;
        page_table[i].address_31_12 = i;

        video_page_table[i].present = 0;
        video_page_table[i].read_write = 1;
        video_page_table[i].user_supervisor = 1;
        video_page_table[i].write_through = 0;
        video_page_table[i].cache_disable = 1;
        video_page_table[i].accessed = 0;
        video_page_table[i].dirty = 0;
        video_page_table[i].page_attribute_table = 0;
        video_page_table[i].global = 0;
        video_page_table[i].available = 0;
        video_page_table[i].address_31_12 = i;
        
    }

    setCR4(); // Set CR4 with specific mask to allow for 4MB Page for Kernel
    loadPageDirectory((unsigned int*)page_directory); // Load the memory location of start of page directory to CR3
    enablePaging(); // Set CR0 with specific mask to enable paging
    
    return;
}

