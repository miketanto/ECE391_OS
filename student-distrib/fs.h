#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "types.h"
#include "lib.h"

#define FILENAME_LEN 32 // up to 32 characters zero padded
#define DATA_BLOCK_SIZE 4096 // 4KB

extern unsigned int FILE_SYS_BASE_ADDR;

typedef struct __attribute__((packed)) dentry_t {
    uint8_t filename[FILENAME_LEN];
    uint32_t filetype;
    uint32_t inode_num;
    uint8_t reserved[24];
} dentry_t;

typedef struct __attribute__((packed)) boot_block_t {
    uint32_t dir_count;
    uint32_t inode_count;
    uint32_t data_count;
    uint8_t reserved[52];
    struct dentry_t direntries[63];
} boot_block_t;

typedef struct __attribute__((packed)) inode_t{
    int32_t length;
    int32_t data_block_num[1023];
} inode_t;

boot_block_t* boot_block;
uint32_t n_inodes;
uint32_t b_data_blocks;
uint32_t num_directories;
dentry_t* direntries;

uint8_t* data_block_start;
inode_t* inode_start;

//Place holder before system calls
uint32_t directory_counter;
uint32_t file_position;
uint32_t curr_inode;

void init_filesystem();
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);


int32_t file_open(const uint8_t* fname);
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_close(int32_t fd);

int32_t directory_open(const uint8_t* fname);
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);
int32_t directory_close(int32_t fd);
#endif
