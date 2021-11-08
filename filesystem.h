/*
* Here is a rough outline of the alecfs partition
* --------------------------------------------------------------
* | sb | 2 data blocks | 2 inode blocks |
* --------------------------------------------------------------
* 0 X1 X2 X3 X4 4095
*
* There are 2 data blocks, 2 inode blocks, and 8 blocks used by super-block
* All blocks are 128 bytes large, the total fs is 4096 blocks
*/
#define ALECFS_FILENAME_MAXLEN 255
#define ALECFS_BLOCK_SIZE 512
#define ALECFS_SUPER_BLOCK	0
#define ALECFS_INODE_BLOCK	128
#define ALECFS_FIRST_DATA_BLOCK	16
#define ALECFS_MAGIC 101
#define ALECFS_NUM_ENTRIES 5

const unsigned long ALEC_BLOCK_SIZE = 512;

typedef struct alecfs_superblock {
    uint64_t magic;
	uint64_t block_size;
	uint64_t version;
	unsigned short data_block_map[2048];
	unsigned short inode_block_map[2032];
};
typedef struct alecfs_dentry {
	char file_name[255];
	unsigned int inode_num;
};

typedef struct alecfs_dir_record {
	char dir_name[255];
    struct alecfs_dentry files[5];
};
typedef struct alecfs_inode {
	unsigned int inode_num;
	unsigned int data_block_num;
	unsigned int file_size;
	unsigned int dir_child_count;
	unsigned int type;
};