/*Info on structures from https://wiki.osdev.org/Paging*/

#ifndef _PAGING_H
#define _PAGING_H

#ifndef ASM
#include "fs.h"
#include "types.h"

#define FOUR_KB_ALIGN 4096 
#define TABLE_SIZE 1024
#define KERNEL_PHY_ADDRESS 0x400000 //Location for 4MB past 0
#define VIDMEM_PHY_ADDRESS 0xB8000  //Specific location of Video memory

#define PROG_DIRECTORY_IDX 32
#define VIDMEM_DIRECTORY_IDX 33

#define PROG_PHY_ADDRESS 0x800000 // 8 MB
#define PROG_OFFSET 0x48000

#define VIDMAP_ADDR 0x8400000  // for vidmap
//extern void init_paging();
/* Page Directory Entry */
typedef struct __attribute__((packed)) page_directory_entry {
    /*P, or 'Present'. If the bit is set, the page is actually in physical memory at the moment.*/
    uint16_t present : 1;
    uint16_t read_write : 1;

    /*U/S, the 'User/Supervisor' bit, controls access to the page based on privilege level. If the bit is set, then the page may be accessed by all; if the bit is not set, however, only the supervisor can access it. For a page directory entry, the user bit controls access to all the pages referenced by the 
    page directory entry. Therefore if you wish to make a page a user page, you must set the user bit in the relevant page directory entry as well as the page table entry.*/
    uint16_t user_supervisor :1; // Kernel space will be 0

    /*PWT, controls Write-Through' abilities of the page. If the bit is set, write-through caching is enabled. 
    If not, then write-back is enabled instead.*/
    uint16_t write_through :1;

    /*PCD, is the 'Cache Disable' bit. If the bit is set, the page will not be cached. Otherwise, it will be.*/
    uint16_t cache_disable :1;

    /*A, or 'Accessed' is used to discover whether a PDE or PTE was read during virtual 
    address translation. If it has, then the bit is set, otherwise, it is not. Note that, 
    this bit will not be cleared by the CPU, so that burden falls on the OS (if it needs this bit at all).*/
    uint16_t accessed :1;

    /*D, or 'Dirty' is used to determine whether a page has been written to*/
    uint16_t dirty :1; // Is Free to use in 4KB page size

    /*PS, or 'Page Size' stores the page size for that specific entry.*/
    uint16_t page_size :1; // 1 if 4MB and 0 if 4KB page size

    /*
     G, or global tells the processor not to invalidate the TLB entry corresponding 
     to the page upon a MOV to CR3 instruction. Bit 7 (PGE) in CR4 must be set to enable 
     global pages.*/
    uint16_t global :1;

    uint16_t available : 3; // Free to use
    uint32_t address_31_12 : 20;
} page_directory_entry;

/* Page Directory Entry */
typedef struct __attribute__((packed)) page_table_entry {
    /*P, or 'Present'. If the bit is set, the page is actually in physical memory at the moment.*/
    uint16_t present : 1;
    uint16_t read_write : 1;

    /*U/S, the 'User/Supervisor' bit, controls access to the page based on privilege level. If the bit is set, then the page may be accessed by all; if the bit is not set, however, only the supervisor can access it. For a page directory entry, the user bit controls access to all the pages referenced by the 
    page directory entry. Therefore if you wish to make a page a user page, you must set the user bit in the relevant page directory entry as well as the page table entry.*/
    uint16_t user_supervisor :1; // Kernel space will be 0

    /*PWT, controls Write-Through' abilities of the page. If the bit is set, write-through caching is enabled. 
    If not, then write-back is enabled instead.*/
    uint16_t write_through :1;

    /*PCD, is the 'Cache Disable' bit. If the bit is set, the page will not be cached. Otherwise, it will be.*/
    uint16_t cache_disable :1;

    /*A, or 'Accessed' is used to discover whether a PDE or PTE was read during virtual 
    address translation. If it has, then the bit is set, otherwise, it is not. Note that, 
    this bit will not be cleared by the CPU, so that burden falls on the OS (if it needs this bit at all).*/
    uint16_t accessed :1;

    /*D, or 'Dirty' is used to determine whether a page has been written to*/
    uint16_t dirty :1; // Is Free to use in 4KB page size

    uint16_t page_attribute_table : 1; // 1 if 4MB and 0 if 4KB page size

    /*
     G, or global tells the processor not to invalidate the TLB entry corresponding 
     to the page upon a MOV to CR3 instruction. Bit 7 (PGE) in CR4 must be set to enable 
     global pages.*/
    uint16_t global : 1;

    uint16_t available : 3; // Free to use
    uint32_t address_31_12 : 20;
} page_table_entry;

struct page_directory_entry page_directory[TABLE_SIZE] __attribute__((aligned (FOUR_KB_ALIGN)));
struct page_table_entry page_table[TABLE_SIZE] __attribute__((aligned (FOUR_KB_ALIGN)));
struct page_table_entry video_page_table[TABLE_SIZE] __attribute__((aligned (FOUR_KB_ALIGN)));
/* enablePaging()
 *   Description: Set the paging (PG) and protection (PE) bits of CR0 to enable Paging
 *   Inputs: none
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: none
*/
extern void enablePaging();

/* setCR4()
 *   Description: Set CR4 to enable PSE(4MB Pages)
 *   Inputs: none
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: Initialize the Page table and directory
 *                 set the proper bits for CR0 and CR4 and fill CR3 with the memory location of page directory
*/
extern void setCR4();

/* loadPageDirectory()
 *   Description: Loads the pointer to page directory table to Cr3
 *   Inputs: unsigned int* page_directory -- Pointer to page directory table
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: none
 * */
extern void loadPageDirectory(unsigned int* page_directory);

extern void flush_tlb();
void init_paging();

#endif
#endif


