int super_init(SuperBlock * super);
int root_INode_init(INode * root, int start, int size);
int root_dir_init(root_directory * dir);
int store_block_pointers();
int inode_table_init(INodeTable * tbl);
int init_bitmap(bitmap * map, int start);
int get_block_set(bitmap * system_bitmap, int req_size);