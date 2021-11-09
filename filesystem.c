#include <linux/module.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include "filesystem.h"


//Not needed functions
void alecfs_destory_inode(struct inode *inode)
{
	return;
}
void alecfs_put_super(struct super_block *sb) {
    return;
}

//declare functions I'll use later
static struct dentry *alecfs_lookup(struct inode *dir,struct dentry *dentry, unsigned int flags);
static int alecfs_readdir(struct file *filp, struct dir_context *ctx);
ssize_t alecfs_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos);
//super block operations
static const struct super_operations alecfs_sops = {
	.destroy_inode = alecfs_destory_inode,
	.put_super = alecfs_put_super,
};
//inode ops
static struct inode_operations alecfs_inode_ops = {
	.lookup = alecfs_lookup,
};
//dir ops
const struct file_operations alecfs_dir_operations = {
	.iterate = alecfs_readdir, // tell a user-space process what files are in this dir
};
//file ops
static const struct file_operations alecfs_file_ops = {
	.read = alecfs_read,
};
//inode ops
static const struct inode_operations alecfs_file_inode_ops = {
	.getattr = simple_getattr, // implemented on my behalf in (fs.h I believe)
};
/*
Function to get inode from disk at given index
*/
static struct alecfs_inode *alecfs_get_inode(struct super_block *sb, unsigned int inode_no){
	struct buffer_head *bh;
	struct alecfs_inode *afs_inode;
	//printk(KERN_ALERT "Looking up inode %u on disk\n", ALECFS_INODE_BLOCK + inode_no);
	bh = sb_bread(sb, ALECFS_INODE_BLOCK + inode_no); 
	afs_inode = (struct alecfs_inode *)bh->b_data;
	//printk(KERN_INFO "alecfs root inode_num [%u] data block num [%u] dir_child_count [%u] type [%u].\n", afs_inode->inode_num, afs_inode->inode_num, afs_inode->dir_child_count, afs_inode->type);
	//printk(KERN_ALERT "Found inode %u on disk\n", inode_no);
	return afs_inode;
}
//Get data from disk
ssize_t alecfs_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos) {
    struct super_block *sb;
    struct inode *inode;
    struct buffer_head *bh;
    char *buffer;
    int nbytes;
	struct alecfs_inode *cur_fil_inode;
	
	//Get the inode of the file
	inode = file_inode(filp);
	sb = inode->i_sb;
    printk(KERN_ERR "SYS INODE %lu\n", inode->i_ino);
	//Get the inode in our type
	cur_fil_inode = alecfs_get_inode(sb,inode->i_ino);
    printk(KERN_ERR "INODE %u\n",
               cur_fil_inode->inode_num);
    bh = sb_bread(sb, cur_fil_inode->data_block_num);
    if (!bh) {
        printk(KERN_ERR "Failed to read data block %u\n",
               cur_fil_inode->data_block_num);
        return 0;
    }
	//Read the data
    buffer = (char *)bh->b_data + *ppos;
    nbytes = min((size_t)(cur_fil_inode->file_size - *ppos), len);
	//copy data to user
    if (copy_to_user(buf, buffer, nbytes)) {
        brelse(bh);
        printk(KERN_ERR
               "Error copying file content to userspace buffer\n");
        return -EFAULT;
    }

    brelse(bh);
    *ppos += nbytes;
    return nbytes;
}
//look up a file
static struct dentry *alecfs_lookup(struct inode *dir,struct dentry *dentry, unsigned int flags){
	struct super_block *sb;
	struct alecfs_dentry de;
	struct buffer_head *bh;
	const char *name;
	struct alecfs_dir_record *dir_rec;
	struct alecfs_inode *cur_dir_inode;
	int i;
	unsigned int zero;
	unsigned int de_inode;
	struct alecfs_inode *file_alecfs_inode;
	struct inode *file_inode;
	zero = 0;
	//get the name of the file we are looking for
	name = dentry->d_name.name;
	sb = dir->i_sb;
	//get the inode of current directory
	cur_dir_inode = alecfs_get_inode(sb,dir->i_ino);
	printk(KERN_ALERT "INODE %lu\n",dir->i_ino);
	
	if(cur_dir_inode->type != 1){
		return NULL;
	}
	//printk(KERN_ALERT "DATABLOCK %u\n",cur_dir_inode->data_block_num);
	//get the data (alecfs_dir_record) for a directory
	bh = sb_bread(sb, cur_dir_inode->data_block_num);
	if (bh == NULL) {
		printk(KERN_ALERT "could not read block\n");
		return NULL;
	}
	//go through the dentrys and check if any are the file we need
	for (i = 0; i < ALECFS_NUM_ENTRIES; i++) {
		dir_rec = (struct alecfs_dir_record*) bh->b_data;
		de = dir_rec->files[i];
		//really really really hacky solution because the inode was returning garbage for the inode number
		de_inode = i+1;
		printk(KERN_ALERT "INODE lookup %u\n",de.inode_num);
		//if the inode it's pointing at is zero then it's not set
		if (de_inode != zero) {
			//we've found
			if(strcmp(name, de.file_name) == 0){
				file_alecfs_inode =  alecfs_get_inode(sb, de_inode);
				file_inode = new_inode(sb);
				file_inode->i_ino = de_inode;
				inode_init_owner(file_inode, NULL, S_IFREG);
				file_inode->i_sb = sb;
				file_inode->i_op = &alecfs_file_inode_ops;
				file_inode->i_fop = &alecfs_file_ops;
				d_add(dentry, file_inode);
				return NULL;
			}
		}
	}

	printk(KERN_ALERT "looked up dentry %s\n", dentry->d_name.name);

	return NULL;
}
//Read all of the files in a directory
static int alecfs_readdir(struct file *filp, struct dir_context *ctx){
	struct buffer_head *bh;
	struct alecfs_dir_record *dir_rec;
	struct alecfs_dentry de;
	struct inode *inode;
	struct super_block *sb;
	int over;
	struct alecfs_inode *cur_dir_inode;
	unsigned int zero;
	
	//Inode of the file
	inode = file_inode(filp);
	sb = inode->i_sb;
	//get the alecfs inode for the directory
	cur_dir_inode = alecfs_get_inode(sb,inode->i_ino);
	//printk(KERN_ALERT "INODE %d\n",inode->i_ino);
	if(cur_dir_inode->type != 1){
		return 0;
	}
	//printk(KERN_ALERT "DATABLOCK %u\n",cur_dir_inode->data_block_num);
	//get the data (alecfs_dir_record) for a directory
	bh = sb_bread(sb, cur_dir_inode->data_block_num);
	if (bh == NULL) {
		printk(KERN_ALERT "could not read block\n");
		return -1;
	}
	zero = 0;
	//go through all the entires and emit them
	for (; ctx->pos < ALECFS_NUM_ENTRIES; ctx->pos++) {
		dir_rec = (struct alecfs_dir_record*) bh->b_data;
		de = dir_rec->files[ctx->pos];
		//printk(KERN_ALERT "DE %lld, DE->Inode %u, file %s\n",ctx->pos,de.inode_num,de.file_name);
		//if the inode is zero then there is no file
		if (de.inode_num != zero) {
			/*
			 * Use `over` to store return value of dir_emit and exit
			 * if required.
			 */
			over = dir_emit(ctx, de.file_name, ALECFS_FILENAME_MAXLEN, de.inode_num,DT_UNKNOWN);
			if (over) {
				printk(KERN_DEBUG "Read %s from folder %s, ctx->pos: %lld\n",
					de.file_name,
					filp->f_path.dentry->d_name.name,
					ctx->pos);
				ctx->pos++;
				return 0;
			}
		}
	}
	return 0;
	
}


//function to fill super block
static int alecfs_fill_super(struct super_block *sb, void *data, int silent){	
        struct inode *root_inode;
		struct buffer_head *bh;
		struct alecfs_superblock *sb_disk;
		struct alecfs_inode *root_alecfs_inode;
		//read the inital data
		bh = sb_bread(sb, 0);
		sb_disk = (struct alecfs_superblock *)bh->b_data;
		//sb_disk->journal = NULL;
		
		printk(KERN_INFO "The magic number obtained in disk is: [%llu]\n",sb_disk->magic);
		printk(KERN_INFO "simplefs filesystem of version [%llu] formatted with a block size of [%llu] detected in the device.\n", sb_disk->version, sb_disk->block_size);
		//setup the superblock with the alecfs values
		sb_set_blocksize(sb, ALEC_BLOCK_SIZE);
		sb->s_magic 			= ALECFS_MAGIC;
		sb->s_fs_info 			= sb_disk;
        sb->s_blocksize         = ALEC_BLOCK_SIZE;
		sb->s_maxbytes          = ALEC_BLOCK_SIZE;
		sb->s_blocksize_bits	= 9;
        sb->s_op                = &alecfs_sops;
		//setup the root inode
		root_alecfs_inode = alecfs_get_inode(sb, 0);
		root_inode = new_inode(sb);
		root_inode->i_ino =  0;
		inode_init_owner(root_inode, NULL, S_IFDIR);
		root_inode->i_sb = sb;
		//set the ops
		root_inode->i_op = &alecfs_inode_ops;
		root_inode->i_fop = &alecfs_dir_operations;

		root_inode->i_private = root_alecfs_inode;
		//make it the root inode
		sb->s_root = d_make_root(root_inode);	

        return 0;
}
//mount function
static struct dentry *alecfs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data){
	//return function that calls fill_super
	return mount_bdev(fs_type, flags, dev_name, data, alecfs_fill_super);
}

//Struct to hold all the alecfs data
static struct file_system_type fs_type = {
	.owner = THIS_MODULE,
	.name = "alecfs",
	.mount = alecfs_mount,
	.kill_sb = kill_block_super,
	.fs_flags = FS_REQUIRES_DEV,
};

//initial funtion that regesters os
static int __init startup(void){
	int err = register_filesystem(&fs_type);
	if(err){
		printk(KERN_ALERT "Failed to reg");
	}
	return 0;
}
//unregester the filesystem and quit
static void __exit shutdown(void){
	unregister_filesystem(&fs_type);
}
//tell the OS about our methods
module_init(startup);
module_exit(shutdown);
MODULE_LICENSE("GPL");