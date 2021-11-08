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

struct alecfs_inode_info {
	__u16 data_block;
	struct inode vfs_inode;
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

static struct alecfs_dir_record *alecfs_find_entry(struct dentry *dentry, struct buffer_head **bhp)
{
	struct buffer_head *bh;
	struct inode *dir = dentry->d_parent->d_inode;
	struct alecfs_inode_info *mii = container_of(dir, struct alecfs_inode_info, vfs_inode);
	struct super_block *sb = dir->i_sb;
	const char *name = dentry->d_name.name;
	struct alecfs_dir_record *final_de = NULL;
	struct alecfs_dir_record *de;
	int i;

	/* TODO 6/6: Read parent folder data block (contains dentries).
	 * Fill bhp with return value.
	 */
	bh = sb_bread(sb, mii->data_block);
	if (bh == NULL) {
		printk(KERN_ALERT "could not read block\n");
		return NULL;
	}
	*bhp = bh;
	de = ((struct alecfs_dir_record *) bh->b_data);
	if(de->file_one_inode_no != 0){
		if (strcmp(name, de->file_one) == 0) {
			return de;
		}
	}
	if(de->file_two_inode_no != 0){
		if (strcmp(name, de->file_two) == 0) {
			return de;
		}
	}
	if(de->file_three_inode_no != 0){
		if (strcmp(name, de->file_three) == 0) {
			return de;
		}
	}
	return final_de;
}

static struct dentry *alecfs_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags){
	struct super_block *sb = dir->i_sb;
	struct alecfs_dir_record *de;
	struct buffer_head *bh = NULL;
	struct inode *inode = NULL;
	
	alecfs_dir_record
}

static int alecfs_readdir(struct file *filp, struct dir_context *ctx){
	struct buffer_head *bh;
	struct alecfs_dir_record *de;
	struct alecfs_inode_info *mii;
	struct inode *inode;
	struct super_block *sb;
	
	inode = file_inode(filp);
	mii = container_of(inode, struct alecfs_inode_info, vfs_inode);
	
	sb = inode->i_sb;
	bh = sb_bread(sb, mii->data_block);
	if (bh == NULL) {
		printk(KERN_ALERT "could not read block\n");
		err = -1;
		return -1;
	}
	
	de = (struct alecfs_dir_record *) bh->b_data;
	
	if(de->file_one_inode_no != 0){
		dir_emit(ctx, de->file_one, ALECFS_FILENAME_MAXLEN, de->file_one_inode_no, DT_UNKNOWN);
	}
	if(de->file_two_inode_no != 0){
		dir_emit(ctx, de->file_two, ALECFS_FILENAME_MAXLEN, de->file_two_inode_no, DT_UNKNOWN);
	}
	if(de->file_three_inode_no != 0){
		dir_emit(ctx, de->file_three, ALECFS_FILENAME_MAXLEN, de->file_three_inode_no, DT_UNKNOWN);
	}
	return 0;
	
}

static struct inode_operations alecfs_inode_ops = {
	.lookup = alecfs_lookup,
};
const struct file_operations alecfs_dir_operations = {
	.iterate = alecfs_readdir, // tell a user-space process what files are in this dir
};

static int alecfs_fill_super(struct super_block *sb, void *data, int silent){	

		struct alecfs_sb_info *sbi;
        struct inode *root_inode;
		struct buffer_head *bh;
		struct alecfs_superblock *sb_disk;
		struct alecfs_inode *root_alecfs_inode;
		
		bh = sb_bread(sb, 0);
		sb_disk = (struct alecfs_superblock *)bh->b_data;
		//sb_disk->journal = NULL;
		
		printk(KERN_INFO "The magic number obtained in disk is: [%llu]\n",sb_disk->magic);
		printk(KERN_INFO "simplefs filesystem of version [%llu] formatted with a block size of [%llu] detected in the device.\n", sb_disk->version, sb_disk->block_size);
		sb_set_blocksize(sb, ALEC_BLOCK_SIZE);
		sb->s_magic 			= ALECFS_MAGIC;
		sb->s_fs_info 			= sb_disk;
        sb->s_blocksize         = ALEC_BLOCK_SIZE;
		sb->s_maxbytes          = ALEC_BLOCK_SIZE;
		sb->s_blocksize_bits	= 9;
        sb->s_op                = &alecfs_sops;

		root_alecfs_inode = alecfs_get_inode(sb, 0);
		root_inode = new_inode(sb);
		root_inode->i_ino =  0;
		inode_init_owner(root_inode, NULL, S_IFDIR);
		root_inode->i_sb = sb;
		root_inode->i_op = &alecfs_inode_ops;
		root_inode->i_fop = &alecfs_dir_operations;

		root_inode->i_private = root_alecfs_inode;
		sb->s_root = d_make_root(root_inode);	

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