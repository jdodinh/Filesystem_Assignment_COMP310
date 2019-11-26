#define NUM_BLOCKS  1000
#define BLOCK_SIZE 1024

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
    block_pointer pointers[12];
    int indirect;
} INode;

typedef struct INodeTable{ // Size = 8000 bytes
    INode System_INodes[NUM_BLOCKS];
} INodeTable;

typedef struct dir_entry {
    INode inode;
    char filename[21];
} dir_entry;

typedef struct root_directory {
    dir_entry entries[NUM_BLOCKS];
} root_directory;

typedef struct FileAllocationTable {
} FileAllocationTable; 


