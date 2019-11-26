#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

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

int root_init(INode * root) {
    root->mode = 660;
    // root->pointers[0] = 0;
    root->num_links = 0;
    root->user_id = getuid();
    root->group_id = getgid();
    root->size = sizeof(root_directory);
    return 0;
}

int store_block_pointers() {
    
    return 0;
}

int main() {
    printf("%lu \n", sizeof(root_directory));
    return 0;
}