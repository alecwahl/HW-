#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include "filesystem.h"

struct alecfs_sb_info {
	__u8 version;
	unsigned long imap;
	struct buffer_head *sbh;
};

static struct alecfs_inode *alecfs_get_inode(struct super_block *sb, unsigned int inode_no){
	struct buffer_head *bh;
	struct alecfs_inode *afs_inode;
	printk(KERN_ALERT "Looking up inode %u on disk\n", inode_no);
	printk(KERN_ALERT "At address %u",ALECFS_INODE_BLOCK + inode_no);
	printk(KERN_ALERT "IS SB NULL? %p",sb);
	bh = sb_bread(sb, inode_no); 
	printk(KERN_ALERT "bh returned\n", bh);
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

        sb->s_maxbytes          = ALECFS_BLOCK_SIZE;
        sb->s_blocksize         = ALECFS_BLOCK_SIZE;
        sb->s_magic             = ALECFS_MAGIC;
        sb->s_op                = &alecfs_sops;
        sb->s_time_gran         = 1;
		
		alecfs_get_inode(sb, 127);
		alecfs_get_inode(sb, 128);
		alecfs_get_inode(sb, 129);


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