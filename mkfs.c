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
	uint64_t block_size;
	uint64_t version;
	unsigned short data_block_map[2048];
	unsigned short inode_block_map[2032];
};
struct alecfs_dentry {
	char file_name[255];
	unsigned int inode_num;
};

struct alecfs_dir_record {
	char dir_name[255];
    struct alecfs_dentry files[5];
};

struct alecfs_inode {
	unsigned int inode_num;
	unsigned int data_block_num;
	unsigned int file_size;
	unsigned int dir_child_count;
	unsigned int type;
};




const unsigned int fist_data_block = 16;
const unsigned int first_inode = 128;

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
	sb.version = 1;
	sb.block_size = 512;
	
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
	root_inode.inode_num = 0;
	root_inode.data_block_num = 16;
	root_inode.dir_child_count = 1;
	root_inode.type = 1;
	
	ret = write_to_dev(128, &root_inode, sizeof(root_inode), fd);
	if(-1 == ret){
		printf("Error writting root_inode to the device");
		return -1;
	}
	printf("root_inode written succesfully\n");
	
	struct alecfs_dir_record first_file;
	struct alecfs_dentry readme_dentry* = malloc(sizeof(struct alecfs_dentry));
	struct alecfs_dentry empty_dentry;
	strcpy(first_file.dir_name, "/");
	strcpy(readme_dentry->file_name, "readme.txt");
	strcpy(empty_dentry.file_name, "");
	readme_dentry->inode_num = 1;
	empty_dentry.inode_num = 0;
	first_file.files[0] = readme_dentry;
	first_file.files[1] = empty_dentry;
	first_file.files[2] = empty_dentry;
	first_file.files[3] = empty_dentry;
	first_file.files[4] = empty_dentry;
	ret = write_to_dev(16, &first_file, sizeof(first_file), fd);
	if(-1 == ret){
		printf("Error writting root_dir to the device");
		return -1;
	}
	printf("root_dir written succesfully\n");
	
	struct alecfs_inode readme;
	readme.inode_num = 1;
	readme.data_block_num = 17;
	readme.dir_child_count = 0;
	readme.type = 0;

	ret = write_to_dev(129, &readme, sizeof(readme), fd);
	if(-1 == ret){
		printf("Error writting readme inode to the device");
		return -1;
	}
	printf("readme inode written succesfully\n");
	char readme_data[] = "this is my first file in my file system! go me!";
	ret = write_to_dev(17, &readme_data, sizeof(readme_data), fd);
	if(-1 == ret){
		printf("Error writting readme data to the device");
		return -1;
	}
	printf("readme data written succesfully\n");
	
	close(fd);
	
	return 0;
}

int main(){
	write_data();
}
