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
fd_table fdescs;


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
        read_blocks(0, 1, &super);
        int IN_table_len = super.IN_tbl_len;
        int num_blocks = super.system_size;
        read_blocks(1, IN_table_len, &system_inodes);
        read_blocks(num_blocks - 1, 1, &system_bitmap);
        int root_IN = super.root;
        root = system_inodes.System_INodes[root_IN];
        fd_tbl_init(&fdescs);

        if (root.num_blocks <= 12) {
            for (int i = 0; i<root.num_blocks; i++) {
                read_blocks(root.pointers[i], 1, &root_dir+(i*BLOCK_SIZE));
            }
        }
        else {
            indirect ind;
            read_blocks(root.indirect, 1, &ind);
            for (int i = 0; i<12; i++) {
                read_blocks(root.pointers[i], 1, &root_dir+(i*BLOCK_SIZE));
            }
            for (int i = 12; i < root.num_blocks ; i++) {
                read_blocks(ind.pointers[i-12], 1, &root_dir + (i*BLOCK_SIZE));
            }
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
        strt = strt + siz;

        init_bitmap(&system_bitmap, strt);                      // initializing the bitmap
        write_blocks(NUM_BLOCKS-1, 1, &system_bitmap);    // writing the bitmap to the last block
        root_INode_init(&system_inodes.System_INodes[0], strt-siz, siz); //initialize the root inode
        fd_tbl_init(&fdescs);

    }
    
    else {
        printf("Please enter a valid fresh flag for the disk (0 or 1)");
        // exit(1);
        return;
    }
    return;
}

int sfs_fopen(char *name) {         // opens the given file
    int inode = check_directory(&root_dir, name);
    if (inode >= 0) {   // If a file exists, we check if it is open
        int fd = check_fd_table(&fdescs, inode);
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
        int new_fd_index = next_free_fd(&fdescs);
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
        
        fdescs.fds[new_fd_index] = fd;

        update_disk(&super, &system_inodes, &root_dir, &system_bitmap);

        return new_fd_index;
    }

    else {
        // filename exists, so we open the file
        INode file_node = system_inodes.System_INodes[inode];
        file_descriptor new_fd;
        init_fd(&new_fd, inode, 0, file_node.size);
        int new_fd_index = next_free_fd(&fdescs);
        fdescs.fds[new_fd_index] = new_fd;
        return new_fd_index;
    }
    return 0;
}                          

int sfs_fclose(int fileID) {            // closes the given file
    // int fd = check_fd_table(&fdescs, fileID);
    if (fdescs.fds[fileID].iNode_number >= 0) {
        fdescs.fds[fileID].iNode_number = -1;
        fdescs.fds[fileID].read_pointer = -1;
        fdescs.fds[fileID].write_pointer = -1;
        return 0;
    }
    return -1;
}            

int sfs_remove(char *file) {              // removes a file from the filesystem
    int inode = check_directory(&root_dir, file);  // checks if the file exists in the directory
    char buffer[BLOCK_SIZE] = { 0 };
    if (inode >= 0) {   // If a file exists, we check if it is open
        int fd = check_fd_table(&fdescs, inode);
        if (fd >= 0) {
            // File already open
            sfs_fclose(fd);
        }
        // DEALLOCATE ALL THE BLOCKS
        INode node = system_inodes.System_INodes[inode];
        for (int i = 0; i< 12; i++) {
            system_bitmap.map[node.pointers[i]] = false;
            write_blocks(node.pointers[i], 1, buffer);      // zeroing out the memory
        }
        if (node.indirect >= 0) {
            indirect ind;
            read_blocks(node.indirect, 1, &ind);
            for (int i = 0; i<IND_SIZ; i++) {
                system_bitmap.map[ind.pointers[i]] = false;
                write_blocks(ind.pointers[i], 1, buffer);
            }
            system_bitmap.map[node.indirect] = false;
            write_blocks(node.indirect, 1, buffer);
        }
        reset_inode(&system_inodes.System_INodes[inode]);
        update_disk(&super, &system_inodes, &root_dir, &system_bitmap);
        return 0;
    }
    return -1;
}                 


int sfs_fwrite(int fileID,char *buf, int length) {   // write buf characters into disk
    // file_descriptor fd = fdescs.fds[fileID];
    // int fd_index = check_fd_table(&fdescs, fdescs.fds[fileID].iNode_number);      // Check the file in the open file descriptor table
    // if (fd_index != fileID) {
    //     printf("ERROR: The given file is not open");
    //     return -1;
    // }
    if (fdescs.fds[fileID].iNode_number < 0) {
        printf("ERROR: The given file is not open\n");
        return -1;
    }
    file_descriptor fd = fdescs.fds[fileID];               // getting the file descriptor of the file
    INode i_node = system_inodes.System_INodes[fd.iNode_number];  // getting the inode of the file
    int w_ptr = fd.write_pointer;                        // getting the location of the write pointer
    int w_ptr_blk = w_ptr/ BLOCK_SIZE;             // Getting the index of the block in which the write pointer currently is
    int blk_rem = BLOCK_SIZE - (w_ptr % BLOCK_SIZE);      // number of bytes left to write in the current block
    int num_blk = ((length - blk_rem)/BLOCK_SIZE) + 2;      // see how many blocks we need read before 
    int num_extra_blocks = ((length - i_node.size + w_ptr)/BLOCK_SIZE) + 1;
    int blk_number = i_node.num_blocks;
    indirect ind;

    if (num_extra_blocks >0 ) {  // Allocate more blocks to the file
    // Check if we need to write extra blocks 
        int new_block = get_block_set(&system_bitmap, num_extra_blocks); // Getting new block set
        if (new_block >= 0) { 
            mark_blocks(&system_bitmap, new_block, num_extra_blocks);    // marking new block set as unavailable
        }
        i_node.num_blocks = blk_number + num_extra_blocks;              // new block size of file
        if (i_node.num_blocks > 12) {       // Check to see if indirect needs to be implemented
            if (i_node.indirect < 0) {      // If the file doesn't have an indirect, initialize an indirect
                i_node.indirect = get_block_set(&system_bitmap, 1);
                if (i_node.indirect >=0) {
                    mark_blocks(&system_bitmap, i_node.indirect, 1);
                }
                else {return -1;}
                if (blk_number < 12) { // The case when there is some free blocks in the direct pointers, but we also have to write to indirects
                    for (int i = blk_number; i<12; i++) {          // Filling remaining direct pointers with block index
                        i_node.pointers[i] = new_block + i - blk_number;
                    }
                    for (int i = 0; i < i_node.num_blocks - 12; i++) {  // Filling the first few indirect blocks
                        ind.pointers[i] = new_block + 12 - blk_number + i;
                    }
                    for (int i = i_node.num_blocks - 12; i < IND_SIZ; i++) {    // filling the rest with -1
                        ind.pointers[i] = -1;
                    }
                }
                else {                  // the case where we update only the indirect blocks
                    for (int i = 0; i<i_node.num_blocks - 12; i++) {
                        ind.pointers[i + blk_number - 12] = new_block + i;
                    }
                    for (int i = i_node.num_blocks - 12; i<IND_SIZ; i++) {
                        ind.pointers[i] = -1;
                    }
                }
                write_blocks(i_node.indirect, 1, &ind);
            }
            else {          // If file already has an indirect we update it 
                read_blocks(i_node.indirect, 1, &ind);          //Check if this is right!!!
                for (int i = 0; i < num_extra_blocks; i++) {
                    ind.pointers[i + blk_number - 12] = i + new_block;
                }
            }
        }
        else { // if total number of blocks fits into the direct pointers 
            for (int i = 0; i < i_node.num_blocks - blk_number; i++) {
                i_node.pointers[i+blk_number] = new_block + i;
            }
        }
    }
    num_blk = i_node.num_blocks;
    void * write_buf = (void *) malloc(num_blk*BLOCK_SIZE); // allocate a buffer of necessary length
    int bytes = 0;
    if (num_blk > 12) {
        for (int i = 0; i < 12; i++) {
            read_blocks(i_node.pointers[i], 1, (BLOCK_SIZE * i)+ write_buf); 
        }
        for (int i = 0; i < num_blk - 12; i++) {
            read_blocks(ind.pointers[i], 1, (BLOCK_SIZE * (i+12))+ write_buf);
        }
    }
    else {
        for (int i = 0; i < num_blk; i++) {
            read_blocks(i_node.pointers[i], 1, (BLOCK_SIZE * i)+ write_buf); 
        }
    }

    for (int j = 0; j < length; j++) {
        memcpy(w_ptr + write_buf + j, buf+j, 1);
        bytes++;
    }


    if (num_blk > 12) { 
        for (int i = 0; i < 12; i++) {
            write_blocks(i_node.pointers[i], 1, (BLOCK_SIZE * i)+ write_buf); 
        }
        for (int i = 0; i < num_blk - 12; i++) {
            write_blocks(ind.pointers[i], 1, (BLOCK_SIZE * (i+12)) + write_buf);
        }
    }
    else {
        for (int i = 0; i < num_blk; i++) {
            write_blocks(i_node.pointers[i], 1, (BLOCK_SIZE * i)+ write_buf); 
        }
    }

    fd.write_pointer = w_ptr + length;
    fdescs.fds[fileID] = fd;
    write_blocks(i_node.indirect, 1, &ind);     
    if (fd.write_pointer > i_node.size) {
        i_node.size = fd.write_pointer;
    }
    system_inodes.System_INodes[fd.iNode_number] = i_node;
    update_disk(&super, &system_inodes, &root_dir, &system_bitmap);
    return bytes;

}


int sfs_frseek(int fileID, int loc) {   // seek (Read) to the location from beginning 
    // int fd_index = check_fd_table(&fdescs, fdescs.fds[fileID].iNode_number);
    // if (fd_index != fileID) {
    //     printf("ERROR: The given file is not open");
    //     return -1;
    // }
    if (fdescs.fds[fileID].iNode_number < 0) {
        printf("ERROR: The given file is not open\n");
        return -1;
    }

    file_descriptor fd = fdescs.fds[fileID];               // getting the file descriptor of the file
    INode i_node = system_inodes.System_INodes[fd.iNode_number];  // getting the inode of the file
    fd.read_pointer = loc;
    fdescs.fds[fileID] = fd;
    return 0;
}                 
int sfs_fwseek(int fileID, int loc){    // seek (Write) to the location from beginning 
    // int fd_index = check_fd_table(&fdescs, fdescs.fds[fileID].iNode_number);
    if (fdescs.fds[fileID].iNode_number < 0) {
        printf("ERROR: The given file is not open\n");
        return -1;
    }

    file_descriptor fd = fdescs.fds[fileID];               // getting the file descriptor of the file
    INode i_node = system_inodes.System_INodes[fd.iNode_number];  // getting the inode of the file
    if (loc>=i_node.size) {
        printf("Error: location out of range");
        return -1;
    }
    fd.write_pointer = loc;
    fdescs.fds[fileID] = fd;
    return 0;
}            
int sfs_fread(int fileID,char *buf, int length){          // read characters from disk into buf
    int fd_index = check_fd_table(&fdescs, fdescs.fds[fileID].iNode_number);      // Check the file in the open file descriptor table
    if (fdescs.fds[fileID].iNode_number < 0) {
        // printf("ERROR: The given file is not open\n");
        return -1;
    }

    file_descriptor fd = fdescs.fds[fileID];               // getting the file descriptor of the file
    INode i_node = system_inodes.System_INodes[fd.iNode_number];  // getting the inode of the file

    if (fd.read_pointer + length > i_node.size) {
        printf("Reading will be impossible, reading buffer is not within the file range\n");
    }
    void * buffer = (void *) malloc(BLOCK_SIZE * i_node.num_blocks);

    // LOADING THE DATA INTO A FILE BUFFER (MEMORY)
    if (i_node.indirect < 0) {
        for (int i = 0; i<i_node.num_blocks; i++) {
            read_blocks(i_node.pointers[i], 1, buffer + (i*BLOCK_SIZE));
        }
    }
    else {
        indirect ind;
        read_blocks(i_node.indirect, 1, &ind);
        for (int i = 0; i<12; i++) {
            read_blocks(i_node.pointers[i], 1, buffer + (i*BLOCK_SIZE));
        }
        for (int i = 12; i<i_node.num_blocks; i++) {
            read_blocks(ind.pointers[i - 12], 1, buffer + (i*BLOCK_SIZE));
        }
    }

    int bytes = 0;
    int rd = fd.read_pointer;
    for (int i = 0; i < length; i++) {
        memcpy(buf + i, buffer + i + rd, 1);
        bytes ++;
    }
    // free(buffer);
    return bytes;
}

int sfs_getnextfilename(char *fname) {      // get the name of the next file in directory 
    return -1;
}               
int sfs_getfilesize(const char* path) {            // get the size of the given file
    return -1;
}


// #############################################################################################################
// ######################################     /------------------\     #########################################
// ######################################     | HELPER FUNCTIONS |     #########################################
// ######################################     \__________________/     #########################################
// #############################################################################################################


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
    root->num_blocks=((root->size-1)/BLOCK_SIZE) + 1;
    for (int i = 0; i < 12; i++) {                  // setting the pointers to the blocks occupied by the root directory.
        root->pointers[i] = start + i;
    }
    if (size>12) {
        root->indirect = get_block_set(&system_bitmap, 1);
        if (root->indirect < 0) {
            printf("Couldn't find a free block");
        }
        mark_blocks(&system_bitmap, root->indirect, 1);
        indirect root_indirect;
        for (int i = 12; i<size; i++) {
            root_indirect.pointers[i-12] = start + i;
        }
        for (int i = size-12; i<IND_SIZ; i++) {
            root_indirect.pointers[i] = -1;
        }
        write_blocks(root->indirect, 1, &root_indirect);
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
        tbl->System_INodes[i].indirect = -1;
        tbl->System_INodes[i].mode = 0;
        tbl->System_INodes[i].size = -1;
        tbl->System_INodes[i].num_links = 0;
        tbl->System_INodes[i].num_blocks = 0;
        tbl->System_INodes[i].valid = false;
        for (int j = 0; j<12; j++) {
            tbl->System_INodes[i].pointers[j] = -1;
        }

    }
    return 0;
}


int reset_inode(INode * node) {
    node->indirect = -1;
    node->mode = -1;
    node->size = 0;
    node->num_links = 0;
    node->num_blocks = 0;
    for (int j = 0; j<12; j++) {
        node->pointers[j] = -1;
    }
    node->valid = false;
    return 0;
}


int init_inode(INode * node) {
    node->mode = 760;
    node->num_links = 0;
    node->num_blocks = 0;
    node->size = 0;
    node ->indirect = -1;
    for (int i = 0; i < 12; i++) {
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
    int index;
    int len = 0;
    for (int i = 0; i<NUM_BLOCKS; i++) {
        if (system_bitmap->map[i]==false) {len++;}
        else {len = 0;}
        if (len ==req_size) {
            index = i - req_size + 1;
            return index;
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

int mark_blocks(bitmap * map, int new_blk, int num_extra) {
    for (int i=0; i < num_extra; i++) {
        map->map[i+new_blk] = true;
    }
    return 0;
}
