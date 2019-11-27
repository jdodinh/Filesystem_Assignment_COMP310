#ifndef HEADER_FUNCTIONS
#define HEADER_FUNCTIONS


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

#endif