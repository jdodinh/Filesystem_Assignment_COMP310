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

int root_INode_init(INode * root) {
    root->mode = 660;
    // root->pointers[0] = 0;
    root->num_links = 1;
    root->user_id = getuid();
    root->group_id = getgid();
    root->size = sizeof(root_directory);
    return 0;
}

int root_dir_init(root_directory * dir) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        dir->entries[i].inode = -1;
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
        for (int j = 0; j<12; j++) {
            tbl->System_INodes[i].pointers[j] = -1;
        }
    }
    return 0;
}

int init_inode(INode * node) {
    node->mode = 660;
    node->num_links = 1;
    node->user_id = getuid();
    node->group_id = getgid();
    return 0;
}

int init_bitmap(bitmap * map, int start) {
    for (int i; i<NUM_BLOCKS-1; i++) {
        map->map[i] = false;
    }
    for (int i = 0; i<start; i++) {
        map->map[i] = true;
    }
    map->map[NUM_BLOCKS-1] = true;
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




int main() {
    printf("%lu \n", sizeof(bool));
    return 0;
}


