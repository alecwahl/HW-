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

struct alecfs_inode {
	unsigned int inode_num;
	unsigned int data_block_num;
	unsigned int file_size;
	unsigned int dir_child_count;
	unsigned int type;
};

static int write_superblock()
{
	struct alecfs_superblock sb;
	sb.magic = 101;
	
	int fd = open("/dev/mmcblk0p3", O_RDWR);
	if (fd == -1) {
		printf("Error opening the device");
		return -1;
	}
	int ret = write(fd, &sb, sizeof(sb));
	
	if(-1 == ret){
	  printf("perror output:");
	}
	close(fd);
	printf("Super block written succesfully\n");
	return 0;
}

int main(){
	write_superblock();
}
