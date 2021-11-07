#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include filesystem.h

#define ALECFS_FILENAME_MAXLEN 255
#define ALECFS_BLOCK_SIZE 512
#define ALECFS_SUPER_BLOCK	0
#define ALECFS_INODE_BLOCK	2064
#define ALECFS_FIRST_DATA_BLOCK	16
#define ALECFS_MAGIC 101

struct alecfs_superblock {
    unsigned int magic;
	unsigned short data_block_map[2048];
	unsigned short inode_block_map[2032];
};

struct alecfs_sb_info {
	__u8 version;
	unsigned long imap;
	struct buffer_head *sbh;
};

static struct alecfs_inode *alecfs_get_inode(struct super_block *sb, uint64_t inode_no){
	struct buffer_head *bh;
	struct alecfs_inode *afs_inode;
	printk(KERN_ALERT "Looking up inode %ld on disk\n", inode_no);
	bh = sb_bread(sb, ALECFS_INODE_BLOCK + inode_no); 
	afs_inode = (struct alecfs_inode *)bh->b_data;
	return afs_inode;
}

static int alecfs_fill_super(struct super_block *sb, void *data, int silent){
	struct inode *root_inode;
	struct buffer_head *bh;
	struct alecfs_superblock *sb_disk;

	bh = sb_bread(sb, ALECFS_SUPER_BLOCK);
	sb_disk = (struct alecfs_superblock *)bh->b_data;
	//printk(KERN_ALERT "The magic number obtained in disk is: [%llu]\n",sb_disk->magic);
	
	if(sb_disk->magic != ALECFS_MAGIC){
		return -1;
	}
	
	/* A magic number that uniquely identifies our filesystem type */
	sb->s_magic = ALECFS_MAGIC;

	/* For all practical purposes, we will be using this s_fs_info as the super block */
	sb->s_fs_info = sb_disk;

	sb->s_maxbytes = ALECFS_BLOCK_SIZE;
	sb->s_op = NULL;

	root_inode = new_inode(sb);
	root_inode->i_ino = ALECFS_INODE_BLOCK;
	inode_init_owner(root_inode, NULL, S_IFDIR);
	root_inode->i_sb = sb;
	root_inode->i_op = NULL;
	root_inode->i_fop = NULL;
	root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = current_time(root_inode);

	root_inode->i_private = alecfs_get_inode(sb, 0);
	return 0;
}

static struct dentry *alecfs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data){
	return mount_bdev(fs_type, flags, dev_name, data, alecfs_fill_super);
}

static struct file_system_type fs_type = {
	.owner = THIS_MODULE,
	.name = "alecfs",
	.mount = alecfs_mount,
	.kill_sb = kill_block_super,
	.fs_flags = FS_REQUIRES_DEV,
};


static int __init startup(void){
	int err = register_filesystem(&fs_type);
	if(err){
		printk(KERN_ALERT "Failed to reg");
	}
	return 0;
}

static void __exit shutdown(void){
	unregister_filesystem(&fs_type);
}

module_init(startup);
module_exit(shutdown);
MODULE_LICENSE("GPL");