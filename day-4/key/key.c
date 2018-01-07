#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/types.h>
unsigned int *address = NULL;
static struct cdev key_cdev;
static dev_t key_no;
static int key_major;
struct key_info
{
	char *name;
	int pin;
	int irq;
};
static struct key_info key_array[]=
{
	{"key0",0,IRQ_EINT(26)},
	{"key1",1,IRQ_EINT(27)},
	{"key2",2,IRQ_EINT(28)},
	{"key3",3,IRQ_EINT(29)},
};

void key_gpx_conf_init(void)
{
	int i =0;
	unsigned int value=0;
	address=ioremap(0x11000C00,8);
	value=ioread32(address);
	for(i=0;i<16;i++)
	{
		value|=0x100<<i;//set bit8-bit23 to 1
	}
	iowrite32(value,address);
}
ssize_t key_read (struct file *fp, char __user *buff, size_t num, loff_t *ploff)
{
	printk("%s\n",__FUNCTION__);
	return 0;
}
int key_open (struct inode *pnode, struct file *fp)
{
	printk("%s\n",__FUNCTION__);
	key_gpx_conf_init();
	return 0;
}
int key_release (struct inode *pnode, struct file *fp)
{
	printk("%s\n",__FUNCTION__);
	return 0;
}

static struct file_operations key_ops =
{
	.owner=THIS_MODULE,
	.read=key_read,
	.open=key_open,
	.release=key_release,
};
static int __init key_init(void)
{
	printk("%s\n",__FUNCTION__);
	alloc_chrdev_region(&key_no,0,1,"key device");
	key_major=MAJOR(key_no);
	printk("key_major=%d\n",key_major);

	key_cdev.owner=THIS_MODULE;
	key_cdev.dev=key_no;
	key_cdev.count=1;
	key_cdev.ops=&key_ops;

	cdev_init(&key_cdev,&key_ops);
	cdev_add(&key_cdev,key_no,1);
	
	return 0;
}
static void __exit key_exit(void)
{
	printk("%s\n",__FUNCTION__);
	unregister_chrdev_region(key_no,1);
	cdev_del(&key_cdev);
}
module_init(key_init);
module_exit(key_exit);
MODULE_LICENSE("Dual BSD/GPL");
