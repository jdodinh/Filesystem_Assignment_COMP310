#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk_emu.h"
#include "sfs_structures.h"
#include "sfs_functions.h"

SuperBlock super;
INode root;
INodeTable system_table;
root_directory main_directory;
bitmap system_bitmap;



// creates the file system
void mksfs(int fresh) { 
    if (fresh == 0) {
        //open the existing file system on the disk
        if (init_disk("sfs.disk", BLOCK_SIZE, NUM_BLOCKS) == -1) {
            printf("Error occurred, couldn't load the existing disk");
            // exit(1);
            return;
        }
        // initialization of the super block

    }
    else if (fresh == 1) {
        // Initialize a new file system
        if (init_fresh_disk("sfs.disk", BLOCK_SIZE, NUM_BLOCKS) == -1) {
            printf("Error occurred, couldn't initialise new disk");
            // exit(1);
            return;
        }
        super_init(&super);                     // initializing the superblock
        int strt = 0;
        int siz = write_blocks(strt, 1, &super);             // Setting a buffer for the root block
        strt = strt + siz;
        
        root_INode_init(&root);
        system_table.System_INodes[0] = root;
        for (int i = 0; i < siz; i++) {                  // setting the pointers to the blocks occupied by the root directory.
            // block_pointer block;                         //Assuming that the number of blocks used is less than 12. 
            // block.ind = root_block + i;
            // if (i == 0) {block.prev = EOF;}
            // else {block.prev = root_block + i - 1;}
            // if (i == siz-1) {block.next = EOF;}
            // else {block.next = root_block + i + 1;}
            system_table.System_INodes[0].pointers[i] = strt + i;
        }
        // memcpy(&system_table.System_INodes[0], &root, sizeof(root));

        int blocks = ((sizeof(system_table)-1)/BLOCK_SIZE) + 1; // number of blocks that the system INode table uses up
        inode_table_init(&system_table);
        siz = write_blocks(strt, blocks, &system_table);
        strt = strt + siz;

        root_dir_init(&main_directory);                         // Initializing the root directory
        blocks = ((sizeof(root_directory)-1)/BLOCK_SIZE+1);     // number of blocks that the root directory uses up
        siz = write_blocks(strt, blocks, &main_directory);      // get the number of blocks written
        strt = strt + siz;                                      // next free block
        init_bitmap(&system_bitmap, strt);                      // initializing the bitmap
        siz = write_blocks(NUM_BLOCKS-1, 1, &system_bitmap);    // writing the bitmap to the last block      
    }
    
    else {
        printf("Please enter a valid fresh flag for the disk (0 or 1)");
        // exit(1);
        return;
    }
    return;
}

int sfs_getnextfilename(char *fname);               // get the name of the next file in directory 
int sfs_getfilesize(const char* path);              // get the size of the given file
int sfs_fopen(char *name);                          // opens the given file
int sfs_fclose(int fileID);                         // closes the given file
int sfs_frseek(int fileID, int loc);                // seek (Read) to the location from beginning   
int sfs_fwseek(int fileID, int loc);                // seek (Write) to the location from beginning 
int sfs_fwrite(int fileID,char *buf, int length);   // write buf characters into disk
int sfs_fread(int fileID,char *buf, int length);    // read characters from disk into buf
int sfs_remove(char *file);                         // removes a file from the filesystem

