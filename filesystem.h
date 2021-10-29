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

struct alecfs_dir_record {
    char filename[ALECFS_FILENAME_MAXLEN];
    uint64_t inode_no;
};

struct alecfs_inode {
    mode_t mode;
    uint64_t inode_no;
    uint64_t data_block_no;

    union {
        uint64_t file_size;
        uint64_t dir_children_count;
    };
};

struct alecfs_superblock {
    uint64_t version;
    uint64_t magic;
    uint64_t blocksize;

    uint64_t inode_table_size;
    uint64_t inode_count;

    uint64_t data_block_table_size;
    uint64_t data_block_count;
};