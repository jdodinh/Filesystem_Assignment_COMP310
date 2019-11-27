#define MAXFILENAME 16


#define NUM_BLOCKS  1000
#define BLOCK_SIZE 1024

#include <stdbool.h>



void mksfs(int fresh);                              // creates the file system
int sfs_getnextfilename(char *fname);               // get the name of the next file in directory 
int sfs_getfilesize(const char* path);              // get the size of the given file
int sfs_fopen(char *name);                          // opens the given file
int sfs_fclose(int fileID);                         // closes the given file
int sfs_frseek(int fileID, int loc);                // seek (Read) to the location from beginning   
int sfs_fwseek(int fileID, int loc);                // seek (Write) to the location from beginning 
int sfs_fwrite(int fileID,char *buf, int length);   // write buf characters into disk
int sfs_fread(int fileID,char *buf, int length);    // read characters from disk into buf
int sfs_remove(char *file);                         // removes a file from the filesystem

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




int super_init(SuperBlock * super);
int root_INode_init(INode * root, int start, int size);
int root_dir_init(root_directory * dir);
int store_block_pointers();
int inode_table_init(INodeTable * tbl);
int init_bitmap(bitmap * map, int start);
int get_block_set(bitmap * system_bitmap, int req_size);
int reset_filesystem();
int update_disk(SuperBlock * super, INodeTable * table, root_directory * root, bitmap * map); 
int next_free_inode(INodeTable * table);
int check_directory(root_directory * directory, char * filename);
int next_free_dentry(root_directory * root_dir);
int init_inode(INode * node);
int init_dentry(char * name, dir_entry * entry, int index);
int next_free_fd(fd_table * table);
int init_fd(file_descriptor * fd, int inode, int read_ptr, int write_ptr);
int fd_tbl_init (fd_table * tbl);
