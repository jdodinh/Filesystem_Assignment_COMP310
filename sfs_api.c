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
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include "sfs_api.h"

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

    if (inode >= 0) {   // If a file exists, we check if it is open
        int fd = check_fd_table(&fds, inode);
        if (fd >= 0) {
            // File already open
            return fd;
        }
    }
    // Check if the file is already open
    if (inode < 0) {
        // file name doesn't exist, we're creating a new file
        int new_dentry_index = next_free_dentry(&root_dir);
        int new_inode_index = next_free_inode(&system_inodes);
        int new_fd_index = next_free_fd(&fds);
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

int sfs_fclose(int fileID) {   // closes the given file
    return -1;
};                      
int sfs_frseek(int fileID, int loc) {    // seek (Read) to the location from beginning 
    return -1;
}                 
int sfs_fwseek(int fileID, int loc){  // seek (Write) to the location from beginning 
    return -1;
}            
int sfs_fwrite(int fileID,char *buf, int length) {   // write buf characters into disk
    return -1;
}
int sfs_fread(int fileID,char *buf, int length){          // read characters from disk into buf
    return -1;
} 
int sfs_remove(char *file) {              // removes a file from the filesystem
    return -1;
}                 



int super_init(SuperBlock * super) {
    super->block_size = BLOCK_SIZE;
    super->system_size = NUM_BLOCKS;
    super->magic = 0xACBD0005;
    super->root = 0;
    super->IN_tbl_len = NUM_BLOCKS;
    //complete with I-node table length
    return 0;
}

int root_INode_init(INode * root, int start, int size) {
    root->mode = 760;
    // root->pointers[0] = 0;
    root->num_links = 1;
    // root->user_id = getuid();
    // root->group_id = getgid();
    root->valid = true;
    root->size = sizeof(root_directory);
    for (int i = 0; i < size; i++) {                  // setting the pointers to the blocks occupied by the root directory.
        root->pointers[i] = start + i;
    }
    // root->start_block = start;
    // root->num_blocks = size;
    return 0;
}

int root_dir_init(root_directory * dir) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        dir->entries[i].inode = -1;
    }
    return 0;
}

int fd_tbl_init (fd_table * tbl) {
    for (int i = 0; i< NUM_BLOCKS; i++) {
        tbl->fds[i].iNode_number = -1;
        tbl->fds[i].read_pointer = -1;
        tbl->fds[i].write_pointer = -1;
    }
    return 0;
}

int inode_table_init(INodeTable * tbl) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        // tbl->System_INodes[i].group_id = -1;
        // tbl->System_INodes[i].user_id = -1;
        tbl->System_INodes[i].indirect = -1;
        tbl->System_INodes[i].mode = -1;
        tbl->System_INodes[i].size = 0;
        tbl->System_INodes[i].num_links = 0;
        tbl->System_INodes[i].valid = false;
        for (int j = 0; j<30; j++) {
            tbl->System_INodes[i].pointers[j] = -1;
        }
        // tbl->System_INodes[i].start_block = -1;
        // tbl->System_INodes[i].num_blocks = -1;
    }
    return 0;
}

int init_inode(INode * node) {
    node->mode = 760;
    node->num_links = 0;
    // node->user_id = getuid();
    // node->group_id = getgid();
    node->size = 0;
    node ->indirect = -1;
    for (int i = 0; i < 30; i++) {
        node->pointers[i] = -1;
    }
    node->valid = true;
    return 0;
}

int init_dentry(char * name, dir_entry * entry, int index) {
    strcpy(entry->filename, name);
    entry->inode = index;
    return 0;
}

int init_fd(file_descriptor * fd, int inode, int read_ptr, int write_ptr) {
    fd->iNode_number = inode;
    fd->read_pointer = read_ptr;
    fd -> write_pointer = write_ptr;
    return 0;
}


int init_bitmap(bitmap * map, int start) {
    for (int i = start; i<NUM_BLOCKS-1; i++) {
        map->map[i] = false;
    }
    for (int i = 0; i<start; i++) {
        map->map[i] = true;
    }
    map->map[NUM_BLOCKS-1] = true;
    return 0;
}

int store_block_pointers() {
    
    return 0;
}

int get_block_set(bitmap * system_bitmap, int req_size) {
    int len = 0;
    for (int i = 0; i<NUM_BLOCKS; i++) {
        if (system_bitmap->map[i]==false) {len++;}
        else {len = 0;}
        if (len ==req_size) {
            int index = i - req_size + 1;
        }
    }
    return -1;
}



int update_disk(SuperBlock * super, INodeTable * table, root_directory * root, bitmap * map) {
    bool debug = false;
    if (debug == true) {
        SuperBlock sup = *super;
        INodeTable tbl = *table;
        root_directory rt = *root;
        bitmap mp = *map;
    }
    int strt = 0;
    int siz = write_blocks(strt, 1, super);
    strt = strt + siz;
    int blocks = ((sizeof(INodeTable)-1)/BLOCK_SIZE) + 1;
    siz = write_blocks(strt, blocks, table);
    strt = strt + siz;
    blocks = ((sizeof(root_directory)-1)/BLOCK_SIZE+1); 
    siz = write_blocks(strt, blocks, root);
    siz = write_blocks(NUM_BLOCKS-1, 1, map);
    file_descriptor fd_table[NUM_BLOCKS];
    return 0;
}


int reset_filesystem() {
    return 0;
}


int next_free_inode(INodeTable * table) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (table->System_INodes[i].valid == false) {
            return i;
        }
    }
    return -1;
}

int next_free_dentry(root_directory * root_dir) {
    for (int i = 0; i<NUM_BLOCKS; i++) {
        if (root_dir->entries[i].inode == -1) {
            return i;
        }
    }
    return -1;
}


int next_free_fd(fd_table * table) {
    for (int i = 0; i<NUM_BLOCKS; i++) {
        if (table->fds[i].iNode_number == -1) {
            return i;
        }
    }
    return -1;
}

int check_directory(root_directory * directory, char * filename) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        char * entry = directory->entries[i].filename;
        if (strcmp(entry, filename)==0) {
            return directory->entries[i].inode;
        }
    }
    return -1;
}


int check_fd_table(fd_table * tbl, int inode) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (tbl->fds[i].iNode_number == inode) {
            return i;
        }
    }
    return -1;
}