#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk_emu.h"
#include "sfs_structures.h"
#include "sfs_functions.h"


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
        SuperBlock super;
        super_init(super);                   // initializing the superblock
        write_blocks(0, 1, &super);          // Setting a buffer for the root block
        INode root;
        root_init(root);                     // Initializing the rot block
        INodeTable system_table;
        system_table.System_INodes[0] = root;
        // memcpy(&system_table.System_INodes[0], &root, sizeof(root));
        int blocks = (sizeof(system_table)/BLOCK_SIZE) + 1; // number of blocks that the system INode table uses up
        int siz = write_blocks(1, blocks, &system_table);
        int root_block = blocks + 1;
        blocks = (sizeof(root_directory)/BLOCK_SIZE+1);     // number of blocks that the root directory uses up
        siz = write_blocks(root_block, blocks, &root);

        for (int i = 0; i < siz; i++) {                  // setting the pointers to the blocks occupied by the root directory.
            block_pointer block;
            block.ind = root_block + i;
            if (i = 0) {block.prev = NULL;}
            else {block.prev = root_block + i - 1;}
            if (i = siz-1) {block.next = NULL;}
            else {block.next = root_block + i + 1;}
            system_table.System_INodes[0].pointers[i] = block;
        }
        
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

