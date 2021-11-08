#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include "filesystem.h"

const unsigned int BLOCK_SIZE = 512;

struct alecfs_sb_info {
	__u8 version;
	unsigned long imap;
	struct buffer_head *sbh;
};

static struct alecfs_inode *alecfs_get_inode(struct super_block *sb, unsigned int inode_no){
	struct buffer_head *bh;
	struct alecfs_inode *afs_inode;
	printk(KERN_ALERT "Looking up inode %u on disk\n", ALECFS_INODE_BLOCK + inode_no);
	bh = sb_bread(sb, ALECFS_INODE_BLOCK + inode_no); 
	afs_inode = (struct alecfs_inode *)bh->b_data;
	printk(KERN_INFO "alecfs root inode_num [%u] data block num [%u] dir_child_count [%u] type [%u].\n", afs_inode->inode_num, afs_inode->inode_num, afs_inode->dir_child_count, afs_inode->type);
	printk(KERN_ALERT "Found inode %ld on disk\n", inode_no);
	return afs_inode;
}

static struct inode_operations alecfs_inode_ops = {
};
const struct file_operations simplefs_dir_operations = {
};

void alecfs_destory_inode(struct inode *inode)
{
	return;
}
void alecfs_put_super(struct super_block *sb) {
    return;
}

const struct file_operations alecfs_file_operations = {
};

static const struct super_operations alecfs_sops = {
	.destroy_inode = alecfs_destory_inode,
	.put_super = alecfs_put_super,
};

static int alecfs_fill_super(struct super_block *sb, void *data, int silent){	

		struct alecfs_sb_info *sbi;
		struct alecfs_superblock *ms;
        struct inode *inode;
		struct buffer_head *bh;
		struct alecfs_superblock *sb_disk;
		
		bh = sb_bread(sb, 0);
		sb_disk = (struct alecfs_superblock *)bh->b_data;
		
		printk(KERN_INFO "The magic number obtained in disk is: [%llu]\n",sb_disk->magic);
		printk(KERN_INFO "simplefs filesystem of version [%llu] formatted with a block size of [%llu] detected in the device.\n", sb_disk->version, sb_disk->block_size);
		
		sb->s_magic 			= ALECFS_MAGIC;
		sb->s_fs_info 			= sb_disk;
        sb->s_blocksize         = BLOCK_SIZE;
        sb->s_op                = &alecfs_sops;

		struct alecfs_inode *afs_inode;
		bh = sb_bread(sb, 64);
		afs_inode = (struct alecfs_inode *)bh->b_data;
		printk(KERN_INFO "alecfs root inode_num [%u] data block num [%u] dir_child_count [%u] type [%u].\n", afs_inode->inode_num, afs_inode->inode_num, afs_inode->dir_child_count, afs_inode->type);
		alecfs_get_inode(sb, 0);


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