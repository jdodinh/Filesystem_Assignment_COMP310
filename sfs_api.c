/* 
Questions for the TA:
    -   Are we using the right strategy for the initialization?
    -   How exactly should we implement the INode pointers?
    -   How do we use the FUSE tests? what machine should we run on?
    -   
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk_emu.h"
#include "sfs_structures.h"
#include "sfs_functions.h"

SuperBlock super;
INode root;
INodeTable system_inodes;
root_directory root_dir;
bitmap system_bitmap;
fd_table fds;



// creates the file system
// TODO: complete the section shen fresh flag = 0
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
            printf("Error occurred, couldn't initialize new disk");
            // exit(1);
            return;
        }
        super_init(&super);                                     // initializing the superblock
        int strt = 0;
        int siz = write_blocks(strt, 1, &super);                // Setting a buffer for the root block
        strt = strt + siz;
        
        inode_table_init(&system_inodes);
        int blocks = ((sizeof(system_inodes)-1)/BLOCK_SIZE) + 1; // number of blocks that the system INode table uses up
        siz = write_blocks(strt, blocks, &system_inodes);
        strt = strt + siz;

        root_dir_init(&root_dir);                               // Initializing the root directory
        blocks = ((sizeof(root_directory)-1)/BLOCK_SIZE+1);     // number of blocks that the root directory uses up
        siz = write_blocks(strt, blocks, &root_dir);            // get the number of blocks written        
        system_inodes.System_INodes[0] = root;
        root_INode_init(&system_inodes.System_INodes[0], strt, siz); //initialize the root inode

        init_bitmap(&system_bitmap, strt);                      // initializing the bitmap
        siz = write_blocks(NUM_BLOCKS-1, 1, &system_bitmap);    // writing the bitmap to the last block   
        fd_tbl_init(&fds);
    }
    
    else {
        printf("Please enter a valid fresh flag for the disk (0 or 1)");
        // exit(1);
        return;
    }
    return;
}

int sfs_getnextfilename(char *fname);               // get the name of the next file in directory 
int sfs_getfilesize(const char* path) {            // get the size of the given file

    return 0;
}
int sfs_fopen(char *name) {         // opens the given file

    int inode = check_directory(&root_dir, name);
    if (inode < 0) {
        // file name doesn't exist, we're creating a new file
        int new_dentry_index = next_free_dentry(&root_dir);
        int new_inode_index = next_free_inode(&system_inodes);
        int new_fd_index = next_free_fd(&fds.fds);
        if (new_inode_index < 0 || new_dentry_index < 0 || new_fd_index <0) {
            printf("ERROR: Couldn't allocate INode. INode table is full, or directory is full, of file descriptor table is full");
            return -1;
        }

        INode new_inode;
        init_inode(&new_inode);
        system_inodes.System_INodes[new_inode_index] = new_inode;
    
        dir_entry new_dentry;
        init_dentry(name, &new_dentry, new_inode_index);
        root_dir.entries[new_dentry_index] = new_dentry;

        file_descriptor fd;
        init_fd(&fd, new_inode_index, 0, 0);
        
        fds.fds[new_fd_index] = fd;

        update_disk(&super, &system_inodes, &root_dir, &system_bitmap);

        return new_fd_index;
    }

    else {
        // filename exists, so we open the file
        INode file_node = system_inodes.System_INodes[inode];
        file_descriptor new_fd;
        init_fd(&new_fd, inode, 0, file_node.size);
        int new_fd_index = next_free_fd(&fds);
        fds.fds[new_fd_index] = new_fd;
        return new_fd_index;
    }

    return 0;
}                          

int sfs_fclose(int fileID);                         // closes the given file
int sfs_frseek(int fileID, int loc);                // seek (Read) to the location from beginning   
int sfs_fwseek(int fileID, int loc);                // seek (Write) to the location from beginning 
int sfs_fwrite(int fileID,char *buf, int length);   // write buf characters into disk
int sfs_fread(int fileID,char *buf, int length);    // read characters from disk into buf
int sfs_remove(char *file);                         // removes a file from the filesystem

