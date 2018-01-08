#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <asm/types.h>
#include <linux/poll.h>
static int my_key_major;
static dev_t my_key_number;
static struct cdev my_key_cdev;
static ssize_t my_key_read (struct file *fp, char __user *buff, size_t num, loff_t *poff)
{
	printk("%s\n",__FUNCTION__);
	return num;
}
static unsigned int my_key_poll (struct file *fp, struct poll_table_struct *pt)
{
	printk("%s\n",__FUNCTION__);
	return 0;
}
static int my_key_open (struct inode *pnode, struct file *fp)
{
	printk("%s\n",__FUNCTION__);
	return 0;
}
static int my_key_release (struct inode *pnode, struct file *fp)
{
	printk("%s\n",__FUNCTION__);
	return 0;
}
static struct file_operations my_key_ops =
{
	.owner	= THIS_MODULE,
	.open	= my_key_open,
	.release= my_key_release,
	.read	= my_key_read,
	.poll	= my_key_poll,
};
static void my_key_cdev_initial(void)
{
	printk("%s\n",__FUNCTION__);
	alloc_chrdev_region(&my_key_number,0,1,"my_key device");
	my_key_major = MAJOR(my_key_number);
	printk("my_key_major = %d\n",my_key_major);

	my_key_cdev.owner	= THIS_MODULE;
	my_key_cdev.ops	= &my_key_ops;
	my_key_cdev.dev	= my_key_number;
	my_key_cdev.count	= 1;

	cdev_init(&my_key_cdev,&my_key_ops);
	cdev_add(&my_key_cdev,my_key_number,1);
}
static int __init my_key_init(void)
{
	printk("%s\n",__FUNCTION__);
	my_key_cdev_initial();
	return 0;
}
static void __exit my_key_exit(void)
{
	printk("%s\n",__FUNCTION__);
}
module_init(my_key_init);
module_exit(my_key_exit);
MODULE_LICENSE("Dual BSD/GPL");
