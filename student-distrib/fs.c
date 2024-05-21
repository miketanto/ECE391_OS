#include "fs.h"
#include "syscall.h"

/* init_filesystem()
 *   Description: Initialize global variables and pointers to memory locations that contain file system data
 *   Inputs: none
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: Initialize global variables from fs.h
*/

void init_filesystem(){
      boot_block = (boot_block_t*)(FILE_SYS_BASE_ADDR);
      num_directories = boot_block->dir_count;
      n_inodes = boot_block->inode_count;
      b_data_blocks = boot_block->data_count;
      //Initialize first dentry
      direntries = boot_block->direntries;

      inode_start = (inode_t* ) (boot_block + 1); //Inodes start in the block after the boot_block
      data_block_start = (uint8_t*) (inode_start + n_inodes); //Data blocks start after all the inode blocks
    return;
}

/* int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
 *   Description: Find dentry that matches the file name and if successful fill in the dentry t block passed as second argument with the file name, file
 *                type, and inode number for the file, then return 0
 *   Inputs: -- const uint8_t* fname: byte array of name (up to 32 characters, zero-padded, but not necessarily including a terminal EOS or 0-byte)
 *           -- dentry_t* dentry: dentry struct that will have data of corresponding dentry if filename found.
 *   Outputs: Fill dentry_t block passed as argument with directory entry data.
 *   Return Value: -- Return 0 when successful
                    -- Return -1 when fname not found and if index not found.
 *   Side Effects: 
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    if(!fname) return -1; // Nonexistent file check
    int i;
    
    for( i = 0 ; i < num_directories ; i++){
        if( !(strncmp((int8_t*) fname, (int8_t*)direntries[i].filename, FILENAME_LEN))){
            read_dentry_by_index(i, dentry);
            return 0;
        }
    }
    return -1;
}

/* int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
 *   Description: Copy data from the directory entry with corresponding index passed in to the dentry_t struct that was passed in the argument.
 *   Inputs: -- uint32_t index : index from directory entry array 
 *           -- dentry_t* dentry: dentry struct that will have data of corresponding dentry if index is in bounds.
 *   Outputs: Fill dentry_t block passed as argument with directory entry data. 
 *   Return Value: -- Return 0 when successful
 *                 -- Return -1 when fname not found and if index not found.
 *   Side Effects: 
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    if(index<0 || index>=num_directories) return -1;
    strncpy((int8_t*)dentry->filename, (int8_t*)direntries[index].filename, FILENAME_LEN);
    dentry->filetype = direntries[index].filetype;
    dentry->inode_num = direntries[index].inode_num;
    return 0;
}

/* int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
 *   Description: Copy data of certain length from a specified offset from the start of the data blocks of a specified inode into a byte buffer.
 *                Only reads up to the number of bytes specified by the inode.
 *   Inputs: uint32_t inode: Inode index to be read
 *           uint32_t offset: offset in bytes for which byte in the inode should be read
 *           uint8_t* buf: Byte buffer to where the data will be copied
 *           uint32_t length: Length of bytes to be read.
 *   Outputs: Copies data from datablocks indexed by inode to the buffer in arguments 
 *   Return Value: Return number of bytes read if successful 
 *                 Return -1 if unable to read anymore.     
 *   Side Effects: None
*/
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    int i;
    inode_t* curr_inode = (inode_t*) ( inode_start + inode ); // Pointer to corresponding Inode block
    uint32_t inode_length = curr_inode -> length;
    uint32_t inode_block_idx = offset / DATA_BLOCK_SIZE; // Offset is in bytes so we need to find the index of 
    uint32_t byte_idx = offset % DATA_BLOCK_SIZE;
    uint32_t bytes_read = 0;
    uint32_t* block_ptr;
    uint32_t block_idx;
    // Offset is in bytes
    if(inode<0 || inode>=n_inodes) return -1;
    if(offset>=inode_length) return -1;
    for(i = 0; i < length; i++){
        if(offset + bytes_read >= inode_length)return bytes_read;
        if(byte_idx >= DATA_BLOCK_SIZE){
            byte_idx = 0;
            inode_block_idx++;
        }
        block_idx = curr_inode->data_block_num[inode_block_idx];
        if(block_idx >= b_data_blocks) return -1;
        block_ptr = (uint32_t*) (data_block_start +  block_idx * (DATA_BLOCK_SIZE) + byte_idx);
        memcpy(buf, block_ptr, 1);
        bytes_read++;
        byte_idx++;
        buf++;
    }
    return bytes_read;
}


/* file_open(const uint8_t* fname)
 *   Description: Initializes file directory entry by initializing file position to start of file and setting the inode number 
 *   Inputs: -- const uint8_t* fname: byte array of name (up to 32 characters, zero-padded, but not necessarily including a terminal EOS or 0-byte)
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: Sets the global values of file_positino and curr_inode
*/
//open_file
int32_t file_open(const uint8_t* fname){
    return 0;
};

/* file_close(int32_t fd)
 *   Description: Does nothing for now
 *   Inputs: int32_t fd: Currently unused
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: None
*/

int32_t file_close(int32_t fd){
    return 0;
};

/* file_read(int32_t fd, const void* buf, int32_t nbytes)
 *   Description: Copies data to buffer and updates file position to where the data read ended.
 *   Inputs: -- int32_t fd: currently_unused
 *           -- const void* buf: Buffer to where data is to be copied
 *           -- int32_t nbytes: number of bytes to be read from file
 *   Outputs: Copies data from file to buffer.
 *   Return Value: none
 *   Side Effects: none
*/
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
    pcb_t* pcb = get_curr_pcb_ptr();
    uint32_t bytes_read;
    inode_t* curr_inode = (inode_t*) ( inode_start + pcb->file_descriptor_array[fd].inode ); // Pointer to corresponding Inode block
    uint32_t inode_length = curr_inode -> length;
    if(pcb->file_descriptor_array[fd].file_pos >= inode_length) return 0;
    bytes_read = read_data(pcb->file_descriptor_array[fd].inode,
     pcb->file_descriptor_array[fd].file_pos, buf, nbytes);
    if(bytes_read == -1) return -1;
    pcb->file_descriptor_array[fd].file_pos += bytes_read;
    return bytes_read;
};

/* file_write(int32_t fd, const void* buf, int32_t nbytes)
 *   Description: Does nothing for now
 *   Inputs: All unused currently
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: None
*/
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}

/* directory_open(const uint8_t* fname)
 *   Description: Sets directory counter to 0 which specifies which index directory_read should read filename from
 *   Inputs: -- const uint8_t* fname: byte array of name (up to 32 characters, zero-padded, but not necessarily including a terminal EOS or 0-byte)
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: none
*/
int32_t directory_open(const uint8_t* fname){
    return 0;
};

/* directory_write(int32_t fd, const void* buf, int32_t nbytes)
 *   Description: Does nothing for now
 *   Inputs: All unused currently
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: None
*/
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
};

/* file_read(int32_t fd, const void* buf, int32_t nbytes)
 *   Description: Read file name of the directory entry that is currently indexed by the directory_counter
 *                Update directory counter to the next directory entry
 *   Inputs: -- int32_t fd: currently_unused
 *           -- const void* buf: Buffer to where data is to be copied
 *           -- int32_t nbytes: number of bytes to be read from file
 *   Outputs: Copies data from file to buffer.
 *   Return Value: none
 *   Side Effects: none
*/
int32_t directory_read(int32_t fd, void* buf, int32_t nbytes){
    //Needs to get the directory and all the subsequent filenames after that
    dentry_t dentry;
    int32_t ret;
    pcb_t* pcb = get_curr_pcb_ptr();
    ret = read_dentry_by_index(pcb->file_descriptor_array[fd].file_pos, &dentry);
    if(ret == -1) return 0; // Check for successful reads
    strncpy((int8_t*)buf, (int8_t*)dentry.filename, FILENAME_LEN);
    pcb->file_descriptor_array[fd].file_pos++;
    ret = strlen(buf);
    return((ret>FILENAME_LEN)?FILENAME_LEN:ret);
};

/* directory_close(int32_t fd)
 *   Description: Does nothing for now
 *   Inputs: int32_t fd: Currently unused
 *   Outputs: none
 *   Return Value: none
 *   Side Effects: None
*/

int32_t directory_close(int32_t fd){
    return 0;
};
//close_directory

