#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>

#include "sfs_structures.h"

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
    root->user_id = getuid();
    root->group_id = getgid();
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
        tbl->System_INodes[i].group_id = -1;
        tbl->System_INodes[i].user_id = -1;
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
    node->user_id = getuid();
    node->group_id = getgid();
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
    int strt = 0;
    int siz = write_blocks(strt, 1, &super);
    strt = strt + siz;
    int blocks = ((sizeof(INodeTable)-1)/BLOCK_SIZE) + 1;
    siz = write_blocks(strt, blocks, &table);
    strt = strt + siz;
    blocks = ((sizeof(root_directory)-1)/BLOCK_SIZE+1); 
    siz = write_blocks(strt, blocks, &root);
    siz = write_blocks(NUM_BLOCKS-1, 1, &map);
    file_descriptor fd_table[NUM_BLOCKS];
    return 0;
}


int reset_filesystem() {
    return 0;
}


int next_free_inode(INodeTable * table) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (table->System_INodes[i].valid = false) {
            return i;
        }
    }
    return -1;
}

int next_free_dentry(root_directory * root_dir) {
    for (int i = 0; i<NUM_BLOCKS; i++) {
        if (root_dir->entries[i].inode = -1) {
            return i;
        }
    }
    return -1;
}


int next_free_fd(fd_table * table) {
    for (int i = 0; i<NUM_BLOCKS; i++) {
        if (table->fds[i].iNode_number = -1) {
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


int main() {
    printf("%lu \n", sizeof(root_directory));
    return 0;
}



typedef struct SuperBlock {
    int magic;
    int block_size;
    int system_size;
    int IN_tbl_len;
    int root;
} SuperBlock; 

typedef struct block_pointer {
    int ind;
    int prev;
    int next;
} block_pointer;

typedef struct INode { // Size = 80 bytes
    unsigned short mode;
    short num_links;
    uid_t user_id;
    uid_t group_id;
    off_t size;
    int pointers[30];
    int indirect;
    bool valid;
    // int start_block;
    // int num_blocks;
} INode;

typedef struct INodeTable{ // Size = 8000 bytes
    INode System_INodes[NUM_BLOCKS];
} INodeTable;

typedef struct dir_entry {
    int inode;
    char filename[21];
} dir_entry;

typedef struct root_directory {
    dir_entry entries[NUM_BLOCKS];
} root_directory;

typedef struct FileAllocationTable {
} FileAllocationTable; 


typedef struct bitmap {
    bool map[NUM_BLOCKS];
} bitmap;


typedef struct file_descriptor {
    int read_pointer;
    int write_pointer;
    int iNode_number;
} file_descriptor;


typedef struct fd_table{
    file_descriptor fds[NUM_BLOCKS];
} fd_table;


