#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct alecfs_superblock {
    uint64_t magic;
	unsigned short data_block_map[2048];
	unsigned short inode_block_map[2032];
};
struct alecfs_dir_record {
    char file_one[255];
    uint64_t file_one_inode_no;
    char file_two[255];
    uint64_t file_two_inode_no;
    char file_three[255];
    uint64_t file_three_inode_no;
};
struct alecfs_inode {
	unsigned int inode_num;
	unsigned int data_block_num;
	unsigned int file_size;
	unsigned int dir_child_count;
	unsigned int type;
};


const unsigned int fist_data_block = 16;
const unsigned int first_inode = 2064;

int write_to_dev(int block_num, void *data, int data_size, int fd)
{
	// lseek seeks to a specific byte but we are given a block num so we convert
	// the correct byte = the block size * the block number
	// seek_set means that the byte number we give is treated as an absolute number
	// instead of an offset from the current position
	off_t s_out = lseek(fd, block_num * 512, SEEK_SET);
	if(s_out == -1)
		return -1;
	// Actually write the data (and return any error code in one line)	
	return write(fd, data, data_size);
}

static int write_data()
{
	struct alecfs_superblock sb;
	sb.magic = 101;
	
	int fd = open("/dev/mmcblk0p3", O_RDWR);
	if (fd == -1) {
		printf("Error opening the device");
		return -1;
	}
	int ret = write_to_dev(0, &sb, sizeof(sb), fd);
	if(-1 == ret){
		printf("Error writting SB to the device");
		return -1;
	}
	printf("Super block written succesfully\n");
	struct alecfs_inode root_inode;
	root_inode.inode_num = 2064;
	root_inode.data_block_num = 16;
	root_inode.dir_child_count = 1;
	root_inode.type = 1;
	
	struct alecfs_dir_record first_file;
	strcpy(first_file.file_one, "readme.txt");
	first_file.file_one_inode_no = 2065;
	
	ret = write_to_dev(2064, &root_inode, sizeof(root_inode), fd);
	if(-1 == ret){
		printf("Error writting root_inode to the device");
		return -1;
	}
	printf("root_inode written succesfully\n");
	
	ret = write_to_dev(16, &first_file, sizeof(first_file), fd);
	if(-1 == ret){
		printf("Error writting root_dir to the device");
		return -1;
	}
	printf("root_dir written succesfully\n");

	close(fd);
	
	return 0;
}

int main(){
	write_data();
}
