#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <asm/io.h>
static dev_t my_beep_no;
static struct cdev my_beep_cdev;
static int my_beep_major;
static int *address=NULL;
//ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
ssize_t my_beep_read (struct file *fp, char __user *buff, size_t count, loff_t *poff)
{
	printk(KERN_ALERT "%s\n",__FUNCTION__);
	return count;
}
//ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
ssize_t my_beep_write (struct file *fp, const char __user *buff, size_t count, loff_t *poff)
{
	printk(KERN_ALERT "%s\n",__FUNCTION__);
	return count;
}
static void my_beep_stop(void)
{
	unsigned int value=0;
	value=ioread32(address+1);
	value&=~0x1;
	iowrite32(value,address+1);
}
static void my_beep_start(void)
{
	unsigned int value=0;
	value=ioread32(address+1);
	value|=0x1;
	iowrite32(value,address+1);
}
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
long my_beep_unlocked_ioctl (struct file *fp, unsigned int cmd, unsigned long param )
{
	printk(KERN_ALERT "%s\n",__FUNCTION__);
	switch(cmd)
	{
		case 0:
			my_beep_start();
			break;
		case 1:
			my_beep_stop();
			break;
	}
	printk("cmd=%d\n",cmd);
	return cmd;
}
//int (*open) (struct inode *, struct file *);
int my_beep_open (struct inode *pnode, struct file *fp)
{
	unsigned int value=0;
	printk(KERN_ALERT "%s\n",__FUNCTION__);
	address=ioremap(0x114000A0,8);
	value=ioread32(address);
	value&=~0x1<<3;
	value&=~0x1<<2;
	value&=~0x1<<1;
	value|=0x1;
	iowrite32(value,address);
	printk(KERN_ALERT "%d\n",*address);
	return 0;
}
//int (*release) (struct inode *, struct file *);
int my_beep_release (struct inode *pnode, struct file *fp)
{
	printk(KERN_ALERT "%s\n",__FUNCTION__);
	return 0;
}
static struct file_operations my_beep_ops =
{
	.owner=THIS_MODULE,
	.unlocked_ioctl=my_beep_unlocked_ioctl,
	.open=my_beep_open,
	.release=my_beep_release,
	.read=my_beep_read,
	.write=my_beep_write,
};
static int __init my_beep_init(void)
{
	alloc_chrdev_region(&my_beep_no,0,1,"beep device");
	my_beep_major=MAJOR(my_beep_no);
	printk("major=%d\n",my_beep_major);

	my_beep_cdev.owner=THIS_MODULE;
	my_beep_cdev.ops=&my_beep_ops;
	my_beep_cdev.dev=my_beep_no;
	my_beep_cdev.count=1;

	cdev_init(&my_beep_cdev,&my_beep_ops);
	cdev_add(&my_beep_cdev,my_beep_no,1);

	printk("%s\n",__FUNCTION__);
	return 0;
}
static void __exit my_beep_exit(void)
{
	printk("%s\n",__FUNCTION__);

	unregister_chrdev_region(my_beep_no,1);

	cdev_del(&my_beep_cdev);
}
MODULE_LICENSE("Dual BSD/GPL");
module_init(my_beep_init);
module_exit(my_beep_exit);
