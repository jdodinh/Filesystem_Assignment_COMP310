#define NUM_BLOCKS  1000
#define BLOCK_SIZE 1024
#include <stdbool.h>

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